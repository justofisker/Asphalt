use cgmath::{
    num_traits::clamp, Deg, Euler, InnerSpace, Quaternion, Rad, Rotation, Rotation3, Vector3, Zero,
};
use instant::Duration;
use winit::{
    dpi::PhysicalPosition,
    event::{ElementState, KeyboardInput, MouseButton, VirtualKeyCode, WindowEvent},
    window::{CursorGrabMode, Window},
};

pub struct Camera {
    pub position: cgmath::Point3<f32>,
    pub rotation: cgmath::Euler<Rad<f32>>,
    pub aspect: f32,
    pub fovy: f32,
    pub znear: f32,
    pub zfar: f32,
}

impl Camera {
    pub fn new(aspect: f32, fovy: f32, znear: f32, zfar: f32) -> Self {
        Camera {
            position: (10.0, 10.0, 10.0).into(),
            rotation: Euler {
                x: Deg(-45.0).into(),
                y: Deg(225.0).into(),
                z: Rad(0.0),
            },
            aspect,
            fovy,
            znear,
            zfar,
        }
    }

    pub fn build_view_projection_matrix(&self) -> cgmath::Matrix4<f32> {
        let (sin_pitch, cos_pitch) = self.rotation.x.0.sin_cos();
        let (sin_yaw, cos_yaw) = self.rotation.y.0.sin_cos();
        let view = cgmath::Matrix4::look_to_rh(
            self.position,
            Vector3::new(cos_pitch * cos_yaw, sin_pitch, cos_pitch * sin_yaw).normalize(),
            cgmath::Vector3::unit_y(),
        );
        let proj = cgmath::perspective(cgmath::Deg(self.fovy), self.aspect, self.znear, self.zfar);
        return OPENGL_TO_WGPU_MATRIX * proj * view;
    }
}

#[rustfmt::skip]
const OPENGL_TO_WGPU_MATRIX: cgmath::Matrix4<f32> = cgmath::Matrix4::new(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.0, 0.0, 0.5, 1.0,
);

#[repr(C)]
#[derive(Debug, Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct CameraUniform {
    // We can't use cgmath with bytemuck directly so we'll have
    // to convert the Matrix4 into a 4x4 f32 array
    pub view_proj: [[f32; 4]; 4],
}

impl CameraUniform {
    pub fn new() -> Self {
        use cgmath::SquareMatrix;
        Self {
            view_proj: cgmath::Matrix4::identity().into(),
        }
    }

    pub fn update_view_proj(&mut self, camera: &Camera) {
        self.view_proj = camera.build_view_projection_matrix().into();
    }
}

pub struct CameraController {
    speed: f32,
    is_forward_pressed: bool,
    is_backward_pressed: bool,
    is_left_pressed: bool,
    is_right_pressed: bool,
    is_space_pressed: bool,
    is_shift_pressed: bool,
    is_grabbed: bool,
    cursor_relative_motion: PhysicalPosition<f64>,
}

impl CameraController {
    pub fn new(speed: f32) -> Self {
        Self {
            speed,
            is_forward_pressed: false,
            is_backward_pressed: false,
            is_left_pressed: false,
            is_right_pressed: false,
            is_space_pressed: false,
            is_shift_pressed: false,
            is_grabbed: true,
            cursor_relative_motion: (0, 0).into(),
        }
    }

    pub fn process_events(&mut self, event: &WindowEvent, window: &Window) -> bool {
        match event {
            WindowEvent::KeyboardInput {
                input:
                    KeyboardInput {
                        state,
                        virtual_keycode: Some(keycode),
                        ..
                    },
                ..
            } => {
                let is_pressed = *state == ElementState::Pressed;
                match keycode {
                    VirtualKeyCode::W => {
                        self.is_forward_pressed = is_pressed;
                        true
                    }
                    VirtualKeyCode::A => {
                        self.is_left_pressed = is_pressed;
                        true
                    }
                    VirtualKeyCode::S => {
                        self.is_backward_pressed = is_pressed;
                        true
                    }
                    VirtualKeyCode::D => {
                        self.is_right_pressed = is_pressed;
                        true
                    }
                    VirtualKeyCode::Space => {
                        self.is_space_pressed = is_pressed;
                        true
                    }
                    VirtualKeyCode::Escape => {
                        if is_pressed {
                            self.is_grabbed = !self.is_grabbed;
                            if self.is_grabbed {
                                let _ = window.set_cursor_grab(CursorGrabMode::Confined);
                                let inner_size = window.inner_size();
                                let _ = window.set_cursor_position(PhysicalPosition::new(
                                    inner_size.width / 2,
                                    inner_size.height / 2,
                                ));
                            } else {
                                let _ = window.set_cursor_grab(CursorGrabMode::None);
                            }
                            window.set_cursor_visible(!self.is_grabbed);
                        }
                        true
                    }
                    _ => false,
                }
            }
            WindowEvent::CursorMoved { position, .. } => {
                if self.is_grabbed {
                    let inner_size = window.inner_size();

                    self.cursor_relative_motion = (
                        self.cursor_relative_motion.x + position.x - inner_size.width as f64 / 2.0,
                        self.cursor_relative_motion.y + position.y - inner_size.height as f64 / 2.0,
                    )
                        .into();

                    let _ = window.set_cursor_position(PhysicalPosition::new(
                        inner_size.width / 2,
                        inner_size.height / 2,
                    ));
                }
                true
            }
            WindowEvent::ModifiersChanged(state) => {
                self.is_shift_pressed = state.shift();
                true
            }
            _ => false,
        }
    }

    pub fn update_camera(&mut self, camera: &mut Camera, delta: Duration) {
        camera.rotation.y =
            Rad((camera.rotation.y.0 + self.cursor_relative_motion.x as f32 / 300.0) % 360.0);
        camera.rotation.x = clamp(
            camera.rotation.x - Rad(self.cursor_relative_motion.y as f32) / 300.0,
            Deg(-85.0).into(),
            Deg(85.0).into(),
        );
        let mut movement = Vector3::<f32>::zero();
        if self.is_forward_pressed {
            movement.x += 1.0;
        }
        if self.is_backward_pressed {
            movement.x -= 1.0;
        }
        if self.is_right_pressed {
            movement.z += 1.0;
        }
        if self.is_left_pressed {
            movement.z -= 1.0;
        }
        if self.is_space_pressed {
            movement.y += 1.0;
        }
        if self.is_shift_pressed {
            movement.y -= 1.0;
        }
        if !movement.is_zero() {
            movement = movement.normalize() * delta.as_secs_f32() * self.speed;
            camera.position += Quaternion::from_axis_angle(Vector3::unit_y(), -camera.rotation.y)
                .rotate_vector(movement);
        }
        self.cursor_relative_motion = (0.0, 0.0).into();
    }
}
