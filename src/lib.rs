use std::{io::BufReader, num::NonZeroU32, vec};

use camera::{Camera, CameraController, CameraUniform};
use chunk::Chunk;
use instant::{Duration, Instant};
use rand::Rng;
use wgpu::{util::DeviceExt, BindGroupLayoutEntry};
use wgpu_glyph::{ab_glyph, GlyphBrushBuilder, GlyphBrush, Section, Text};
use winit::{
    event::*,
    event_loop::{ControlFlow, EventLoop},
    window::{Fullscreen, Icon, Window, WindowBuilder},
};

mod camera;
mod chunk;
mod mesh;
mod texture;

#[cfg(target_arch = "wasm32")]
use wasm_bindgen::prelude::*;

#[cfg_attr(target_arch = "wasm32", wasm_bindgen(start))]
pub async fn run() {
    cfg_if::cfg_if! {
        if #[cfg(target_arch = "wasm32")] {
            std::panic::set_hook(Box::new(console_error_panic_hook::hook));
            console_log::init_with_level(log::Level::Warn).expect("Couldn't initialize logger");
        } else {
            env_logger::init();
        }
    }

    let event_loop = EventLoop::new();
    let window = WindowBuilder::new()
        .with_inner_size(winit::dpi::LogicalSize {
            width: 1280,
            height: 720,
        })
        .with_title("Aspahlt")
        .build(&event_loop)
        .unwrap();

    let _ = window.set_cursor_grab(winit::window::CursorGrabMode::Confined);
    window.set_cursor_visible(false);

    // window.set_ime_allowed(true);

    if let Ok(img) = image::load_from_memory(include_bytes!("../res/icon/icon-256.png")) {
        window.set_window_icon(
            Icon::from_rgba(img.to_rgba8().to_vec(), img.width(), img.height()).ok(),
        );
    }

    #[cfg(target_arch = "wasm32")]
    {
        // Winit prevents sizing with CSS, so we have to set
        // the size manually when on web.
        window.set_inner_size(winit::dpi::PhysicalSize::new(1280, 720));

        use winit::platform::web::WindowExtWebSys;
        web_sys::window()
            .and_then(|win| win.document())
            .and_then(|doc| {
                let dst = doc.get_element_by_id("wasm-example")?;
                let canvas = web_sys::Element::from(window.canvas());
                dst.append_child(&canvas).ok()?;
                Some(())
            })
            .expect("Couldn't append canvas to document body.");
    }

    let mut state = State::new(window).await;

    let mut last_render_time = Instant::now();
    event_loop.run(move |event, _, control_flow| match event {
        Event::WindowEvent {
            ref event,
            window_id,
        } if window_id == state.window.id() => {
            if !state.input(event) {
                match event {
                    WindowEvent::CloseRequested
                    // | WindowEvent::KeyboardInput {
                    //     input:
                    //         KeyboardInput {
                    //             state: ElementState::Pressed,
                    //             virtual_keycode: Some(VirtualKeyCode::Escape),
                    //             ..
                    //         },
                    //     ..
                    // }
                     => *control_flow = ControlFlow::Exit,
                    WindowEvent::Resized(physical_size) => {
                        state.resize(*physical_size);
                    }
                    WindowEvent::ScaleFactorChanged { new_inner_size, .. } => {
                        state.resize(**new_inner_size);
                    }
                    _ => {}
                }
            }
        }
        Event::RedrawRequested(window_id) if window_id == state.window().id() => {
            let now = instant::Instant::now();
            let dt = now - last_render_time;
            last_render_time = now;
            state.update(dt);
            match state.render(dt) {
                Ok(_) => {}
                Err(wgpu::SurfaceError::Lost) => state.resize(state.size),
                Err(wgpu::SurfaceError::OutOfMemory) => *control_flow = ControlFlow::Exit,
                Err(e) => eprint!("{:?}", e),
            }
        }
        Event::MainEventsCleared => {
            state.window.request_redraw();
        }
        _ => {}
    });
}

struct State {
    surface: wgpu::Surface,
    device: wgpu::Device,
    queue: wgpu::Queue,
    config: wgpu::SurfaceConfiguration,
    size: winit::dpi::PhysicalSize<u32>,
    render_pipeline: wgpu::RenderPipeline,
    chunks: Vec<Vec<Chunk>>,
    diffuse_bind_group: wgpu::BindGroup,
    diffuse_texture: texture::Texture,
    depth_texture: texture::Texture,
    camera: Camera,
    camera_uniform: CameraUniform,
    camera_buffer: wgpu::Buffer,
    camera_controller: CameraController,
    camera_bind_group: wgpu::BindGroup,
    window: Window,
    clear_color: wgpu::Color,
    glyph_brush: GlyphBrush<()>,
    staging_belt: wgpu::util::StagingBelt,
    recent_frame_times: Vec<Duration>,
}

impl State {
    async fn new(window: Window) -> Self {
        let size = window.inner_size();

        let instance = wgpu::Instance::new(wgpu::InstanceDescriptor {
            backends: wgpu::Backends::all(),
            dx12_shader_compiler: Default::default(),
        });

        // Saftey:
        // The surface needs to live as long as the winodw that create it.
        // State owns the window so this should be safe
        let surface = unsafe { instance.create_surface(&window) }.unwrap();

        let adapter = instance
            .request_adapter(&wgpu::RequestAdapterOptions {
                power_preference: wgpu::PowerPreference::HighPerformance,
                compatible_surface: Some(&surface),
                force_fallback_adapter: false,
            })
            .await
            .unwrap();

        let (device, queue) = adapter
            .request_device(
                &wgpu::DeviceDescriptor {
                    features: wgpu::Features::empty(),
                    limits: if cfg!(target_arch = "wasm32") {
                        wgpu::Limits::downlevel_webgl2_defaults()
                    } else {
                        wgpu::Limits::default()
                    },
                    label: None,
                },
                None,
            )
            .await
            .unwrap();

        let surface_caps = surface.get_capabilities(&adapter);

        let surface_format = surface_caps
            .formats
            .iter()
            .copied()
            .filter(|f| f.is_srgb())
            .next()
            .unwrap_or(surface_caps.formats[0]);

        let config = wgpu::SurfaceConfiguration {
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            format: surface_format,
            width: NonZeroU32::new(size.width).unwrap().into(),
            height: size.height,
            present_mode: wgpu::PresentMode::AutoNoVsync,
            alpha_mode: surface_caps.alpha_modes[0],
            view_formats: vec![],
        };

        surface.configure(&device, &config);

        let mut diffuse_texture: Option<texture::Texture> = None;

        #[cfg(target_arch = "wasm32")]
        {
            let diffuse_bytes = include_bytes!("../res/texture/blocks.png");
            diffuse_texture = Some(
                texture::Texture::from_bytes(&device, &queue, diffuse_bytes, "blocks.png").unwrap(),
            );
        }
        #[cfg(not(target_arch = "wasm32"))]
        if let Ok(file) = std::fs::File::open("res/texture/blocks.png") {
            let reader = BufReader::new(file);
            if let Ok(img) = image::load(reader, image::ImageFormat::Png) {
                diffuse_texture = Some(
                    texture::Texture::from_image(&device, &queue, &img, Some("blocks.png"))
                        .unwrap(),
                );
            }
        }

        if diffuse_texture.is_none() {
            panic!("Failed to load \"res/texture/blocks.png\"");
        }

        let diffuse_texture = diffuse_texture.unwrap();

        let depth_texture =
            texture::Texture::create_depth_texture(&device, &config, "depth_texture");

        let texture_bind_group_layout =
            device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
                entries: &[
                    wgpu::BindGroupLayoutEntry {
                        binding: 0,
                        visibility: wgpu::ShaderStages::FRAGMENT,
                        ty: wgpu::BindingType::Texture {
                            sample_type: wgpu::TextureSampleType::Float { filterable: true },
                            view_dimension: wgpu::TextureViewDimension::D2,
                            multisampled: false,
                        },
                        count: None,
                    },
                    wgpu::BindGroupLayoutEntry {
                        binding: 1,
                        visibility: wgpu::ShaderStages::FRAGMENT,
                        ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
                        count: None,
                    },
                ],
                label: Some("texture_bind_group_layout"),
            });

        let diffuse_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            layout: &texture_bind_group_layout,
            entries: &[
                wgpu::BindGroupEntry {
                    binding: 0,
                    resource: wgpu::BindingResource::TextureView(&diffuse_texture.view),
                },
                wgpu::BindGroupEntry {
                    binding: 1,
                    resource: wgpu::BindingResource::Sampler(&diffuse_texture.sampler),
                },
            ],
            label: Some("diffuse_bind_group"),
        });

        let camera = Camera::new(
            config.width as f32 / config.height as f32,
            90.0,
            0.1,
            1000.0,
        );

        let camera_controller = CameraController::new(75.0);

        let mut camera_uniform = CameraUniform::new();
        camera_uniform.update_view_proj(&camera);

        let camera_buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Camera Buffer"),
            contents: bytemuck::cast_slice(&[camera_uniform]),
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
        });

        let camera_bind_group_layout =
            device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
                entries: &[wgpu::BindGroupLayoutEntry {
                    binding: 0,
                    visibility: wgpu::ShaderStages::VERTEX,
                    ty: wgpu::BindingType::Buffer {
                        ty: wgpu::BufferBindingType::Uniform,
                        has_dynamic_offset: false,
                        min_binding_size: None,
                    },
                    count: None,
                }],
                label: Some("camera_bind_group_layout"),
            });

        let camera_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            layout: &camera_bind_group_layout,
            entries: &[wgpu::BindGroupEntry {
                binding: 0,
                resource: camera_buffer.as_entire_binding(),
            }],
            label: Some("camera_bind_group"),
        });

        let shader = device.create_shader_module(wgpu::include_wgsl!("shader.wgsl"));

        let model_bind_group_layout =
            device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
                label: Some("Model Bind Group Layout"),
                entries: &[BindGroupLayoutEntry {
                    binding: 0,
                    visibility: wgpu::ShaderStages::VERTEX,
                    ty: wgpu::BindingType::Buffer {
                        ty: wgpu::BufferBindingType::Uniform,
                        has_dynamic_offset: false,
                        min_binding_size: None,
                    },
                    count: None,
                }],
            });

        let render_pipeline_layout =
            device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
                label: Some("Render Pipeline Layout"),
                bind_group_layouts: &[
                    &texture_bind_group_layout,
                    &camera_bind_group_layout,
                    &model_bind_group_layout,
                ],
                push_constant_ranges: &[],
            });

        let render_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("Render Pipeline"),
            layout: Some(&render_pipeline_layout),
            vertex: wgpu::VertexState {
                module: &shader,
                entry_point: "vs_main",
                buffers: &[mesh::Vertex::desc()],
            },
            fragment: Some(wgpu::FragmentState {
                module: &shader,
                entry_point: "fs_main",
                targets: &[Some(wgpu::ColorTargetState {
                    format: config.format,
                    blend: Some(wgpu::BlendState::REPLACE),
                    write_mask: wgpu::ColorWrites::ALL,
                })],
            }),
            primitive: wgpu::PrimitiveState {
                topology: wgpu::PrimitiveTopology::TriangleList,
                strip_index_format: None,
                front_face: wgpu::FrontFace::Ccw,
                cull_mode: Some(wgpu::Face::Back),
                polygon_mode: wgpu::PolygonMode::Fill,
                unclipped_depth: false,
                conservative: false,
            },
            depth_stencil: Some(wgpu::DepthStencilState {
                format: texture::Texture::DEPTH_FORMAT,
                depth_write_enabled: true,
                depth_compare: wgpu::CompareFunction::Less,
                stencil: wgpu::StencilState::default(),
                bias: wgpu::DepthBiasState::default(),
            }),
            multisample: wgpu::MultisampleState {
                count: 1,
                mask: !0,
                alpha_to_coverage_enabled: false,
            },
            multiview: None,
        });

        let mut chunks: Vec<Vec<Chunk>> = vec![];

        let perlin = noise::Perlin::new(rand::thread_rng().gen());

        const CHUNK_ARR_SIZE: usize = 32;

        for x in 0..CHUNK_ARR_SIZE as i32 {
            let mut row: Vec<Chunk> = vec![];
            for y in 0..CHUNK_ARR_SIZE as i32 {
                row.push(chunk::Chunk::new((x - CHUNK_ARR_SIZE as i32 / 2, y - CHUNK_ARR_SIZE as i32 / 2).into(), &perlin));
            }
            chunks.push(row);
        }

        for x in 0..CHUNK_ARR_SIZE {
            for z in 0..CHUNK_ARR_SIZE {
                Chunk::build_mesh_in_context(x, z, &device, &model_bind_group_layout, &mut chunks);
            }
        }

        let notosans = ab_glyph::FontArc::try_from_slice(include_bytes!("../res/font/NotoSans-Regular.ttf")).unwrap();

        let glyph_brush = GlyphBrushBuilder::using_font(notosans).build(&device, surface_format);

        let staging_belt = wgpu::util::StagingBelt::new(1024);

        Self {
            window,
            surface,
            device,
            queue,
            config,
            size,
            render_pipeline,
            chunks,
            diffuse_bind_group,
            diffuse_texture,
            depth_texture,
            camera,
            camera_controller,
            camera_uniform,
            camera_buffer,
            camera_bind_group,
            clear_color: wgpu::Color {
                r: 0.1,
                g: 0.2,
                b: 1.0,
                a: 1.0,
            },
            glyph_brush,
            staging_belt,
            recent_frame_times: vec![],
        }
    }

    pub fn window(&self) -> &Window {
        &self.window
    }

    fn resize(&mut self, new_size: winit::dpi::PhysicalSize<u32>) {
        if new_size.width > 0 && new_size.height > 0 {
            self.size = new_size;
            self.config.width = new_size.width;
            self.config.height = new_size.height;
            self.depth_texture =
                texture::Texture::create_depth_texture(&self.device, &self.config, "depth_texture");
            self.surface.configure(&self.device, &self.config);
        }
    }
    fn input(&mut self, event: &WindowEvent) -> bool {
        // match event {
        //     WindowEvent::CursorMoved { position, .. } => {
        //         self.clear_color.r = position.x / self.size.width as f64;
        //         self.clear_color.g = position.y / self.size.height as f64;
        //     }
        //     _ => {}
        // }
        match event {
            WindowEvent::KeyboardInput {
                input:
                    KeyboardInput {
                        state: ElementState::Pressed,
                        virtual_keycode: Some(VirtualKeyCode::F11),
                        ..
                    },
                ..
            } => {
                if self.window.fullscreen().is_some() {
                    self.window.set_fullscreen(None);
                } else {
                    self.window
                        .set_fullscreen(Some(Fullscreen::Borderless(self.window.current_monitor())))
                }
            }
            _ => {}
        };
        self.camera_controller
            .process_events(event, &mut self.window)
    }

    fn update(&mut self, delta: Duration) {
        self.camera_controller
            .update_camera(&mut self.camera, delta);
        self.camera_uniform.update_view_proj(&self.camera);
        self.queue.write_buffer(
            &self.camera_buffer,
            0,
            bytemuck::cast_slice(&[self.camera_uniform]),
        );
    }

    fn render(&mut self, delta: Duration) -> Result<(), wgpu::SurfaceError> {
        let output = self.surface.get_current_texture()?;

        let view = output
            .texture
            .create_view(&wgpu::TextureViewDescriptor::default());

        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("Render encoder"),
            });

        {
            let mut render_pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
                label: Some("Render pass"),
                color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                    view: &view,
                    resolve_target: None,
                    ops: wgpu::Operations {
                        load: wgpu::LoadOp::Clear(self.clear_color),
                        store: true,
                    },
                })],
                depth_stencil_attachment: Some(wgpu::RenderPassDepthStencilAttachment {
                    view: &self.depth_texture.view,
                    depth_ops: Some(wgpu::Operations {
                        load: wgpu::LoadOp::Clear(1.0),
                        store: true,
                    }),
                    stencil_ops: None,
                }),
            });

            render_pass.set_pipeline(&self.render_pipeline);
            render_pass.set_bind_group(0, &self.diffuse_bind_group, &[]);

            render_pass.set_bind_group(1, &self.camera_bind_group, &[]);
            for row in &mut self.chunks {
                for chunk in row {
                    chunk.draw(&mut render_pass);
                }
            }
        }

        let size = self.window.inner_size();

        self.recent_frame_times.push(delta);
        if self.recent_frame_times.len() >= 12 {
            self.recent_frame_times.remove(0);
        }

        let mut fps: f64 = 0.0;
        for dt in &self.recent_frame_times {
            fps += dt.as_secs_f64();
        }
        fps = fps / self.recent_frame_times.len() as f64;
        fps = 1.0 / fps;

        self.glyph_brush.queue(Section {
            screen_position: (20.0, 20.0),
            bounds: (size.width as f32, size.height as f32),
            text: vec![Text::new(format!("FPS: {}", fps as u32).as_str())
                .with_color([1.0, 1.0, 1.0, 1.0])
                .with_scale(40.0)],
            ..Default::default()
        });

        let _ = self.glyph_brush.draw_queued(&self.device, &mut self.staging_belt, &mut encoder, &view, size.width, size.height);

        self.staging_belt.finish();
        self.queue.submit(std::iter::once(encoder.finish()));
        output.present();

        self.staging_belt.recall();

        Ok(())
    }
}
