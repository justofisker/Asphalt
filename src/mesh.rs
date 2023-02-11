use std::f32::consts::E;

use cgmath::Vector3;
use wgpu::{
    util::{BufferInitDescriptor, DeviceExt},
    Device, RenderPass,
};

#[repr(C)]
#[derive(Copy, Clone, Debug, bytemuck::Pod, bytemuck::Zeroable)]
pub struct Vertex {
    pub position: [f32; 3],
    pub tex_coords: [f32; 2],
}

impl Vertex {
    const ATTRIBS: [wgpu::VertexAttribute; 2] =
        wgpu::vertex_attr_array![0 => Float32x3, 1 => Float32x2];

    pub fn desc<'a>() -> wgpu::VertexBufferLayout<'a> {
        use std::mem;

        wgpu::VertexBufferLayout {
            array_stride: mem::size_of::<Self>() as wgpu::BufferAddress,
            step_mode: wgpu::VertexStepMode::Vertex,
            attributes: &Self::ATTRIBS,
        }
    }
}

pub struct Mesh {
    pub vertex_buffer: wgpu::Buffer,
    pub index_buffer: Option<wgpu::Buffer>,
    pub item_count: u32,
    pub aabb: [[f32; 3]; 2],
}

impl Mesh {
    pub fn create_cube(device: &wgpu::Device, width: f32, height: f32, depth: f32) -> Self {
        #[rustfmt::skip]
        let vertices: &[Vertex] = &[
            Vertex { position: [-0.5 * width, -0.5 * height,  0.5 * depth], tex_coords: [1.0, 1.0] },
            Vertex { position: [ 0.5 * width, -0.5 * height,  0.5 * depth], tex_coords: [0.0, 1.0] },
            Vertex { position: [ 0.5 * width,  0.5 * height,  0.5 * depth], tex_coords: [0.0, 0.0] },
            Vertex { position: [-0.5 * width,  0.5 * height,  0.5 * depth], tex_coords: [1.0, 0.0] },
            Vertex { position: [-0.5 * width, -0.5 * height, -0.5 * depth], tex_coords: [1.0, 1.0] },
            Vertex { position: [ 0.5 * width, -0.5 * height, -0.5 * depth], tex_coords: [0.0, 1.0] },
            Vertex { position: [ 0.5 * width,  0.5 * height, -0.5 * depth], tex_coords: [0.0, 0.0] },
            Vertex { position: [-0.5 * width,  0.5 * height, -0.5 * depth], tex_coords: [1.0, 0.0] },
            Vertex { position: [ 0.5 * width, -0.5 * height, -0.5 * depth], tex_coords: [1.0, 1.0] },
            Vertex { position: [ 0.5 * width, -0.5 * height,  0.5 * depth], tex_coords: [0.0, 1.0] },
            Vertex { position: [ 0.5 * width,  0.5 * height,  0.5 * depth], tex_coords: [0.0, 0.0] },
            Vertex { position: [ 0.5 * width,  0.5 * height, -0.5 * depth], tex_coords: [1.0, 0.0] },
            Vertex { position: [-0.5 * width, -0.5 * height, -0.5 * depth], tex_coords: [1.0, 1.0] },
            Vertex { position: [-0.5 * width, -0.5 * height,  0.5 * depth], tex_coords: [0.0, 1.0] },
            Vertex { position: [-0.5 * width,  0.5 * height,  0.5 * depth], tex_coords: [0.0, 0.0] },
            Vertex { position: [-0.5 * width,  0.5 * height, -0.5 * depth], tex_coords: [1.0, 0.0] },
            Vertex { position: [-0.5 * width,  0.5 * height, -0.5 * depth], tex_coords: [1.0, 0.0] },
            Vertex { position: [-0.5 * width,  0.5 * height,  0.5 * depth], tex_coords: [0.0, 0.0] },
            Vertex { position: [ 0.5 * width,  0.5 * height,  0.5 * depth], tex_coords: [0.0, 1.0] },
            Vertex { position: [ 0.5 * width,  0.5 * height, -0.5 * depth], tex_coords: [1.0, 1.0] },
            Vertex { position: [-0.5 * width, -0.5 * height, -0.5 * depth], tex_coords: [1.0, 0.0] },
            Vertex { position: [-0.5 * width, -0.5 * height,  0.5 * depth], tex_coords: [0.0, 0.0] },
            Vertex { position: [ 0.5 * width, -0.5 * height,  0.5 * depth], tex_coords: [0.0, 1.0] },
            Vertex { position: [ 0.5 * width, -0.5 * height, -0.5 * depth], tex_coords: [1.0, 1.0] },
        ];

        #[rustfmt::skip]
        const INDICES: &[u32] = &[
            0, 1, 2, 2, 3, 0,
            4, 6, 5, 7, 6, 4,
            8, 10, 9, 11, 10, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 22, 21, 23, 22, 20,
        ];

        Self::from_data(device, vertices, Some(INDICES))
    }

    pub fn create_cube_uniform(device: &wgpu::Device, size: f32) -> Self {
        Self::create_cube(device, size, size, size)
    }

    pub fn from_data(device: &Device, vertices: &[Vertex], indices: Option<&[u32]>) -> Self {
        let vertex_buffer = device.create_buffer_init(&BufferInitDescriptor {
            label: None,
            contents: bytemuck::cast_slice(vertices),
            usage: wgpu::BufferUsages::VERTEX,
        });

        let mut index_buffer: Option<wgpu::Buffer> = None;
        let mut item_count: u32 = 0;

        if let Some(indices) = indices {
            index_buffer = Some(device.create_buffer_init(&BufferInitDescriptor {
                label: None,
                contents: bytemuck::cast_slice(indices),
                usage: wgpu::BufferUsages::INDEX,
            }));

            item_count = indices.len() as u32;
        } else {
            item_count = vertices.len() as u32;
        }

        let mut aabb_low = vertices[0].position;
        let mut aabb_high = vertices[0].position;

        for vertex in vertices.iter() {
            if aabb_low[0] > vertex.position[0] {
                aabb_low[0] = vertex.position[0];
            } else if aabb_high[0] < vertex.position[0] {
                aabb_high[0] = vertex.position[0];
            }
            if aabb_low[1] > vertex.position[1] {
                aabb_low[1] = vertex.position[1];
            } else if aabb_high[1] < vertex.position[1] {
                aabb_high[1] = vertex.position[1];
            }
            if aabb_low[2] > vertex.position[2] {
                aabb_low[2] = vertex.position[2];
            } else if aabb_high[2] < vertex.position[2] {
                aabb_high[2] = vertex.position[2];
            }
        }

        Mesh {
            vertex_buffer,
            index_buffer: index_buffer,
            item_count,
            aabb: [aabb_low, aabb_high],
        }
    }

    pub fn draw<'a>(&'a self, render_pass: &mut RenderPass<'a>) {
        if let Some(index_buffer) = &self.index_buffer {
            render_pass.set_vertex_buffer(0, self.vertex_buffer.slice(..));
            render_pass.set_index_buffer(index_buffer.slice(..), wgpu::IndexFormat::Uint32);
            render_pass.draw_indexed(0..self.item_count, 0, 0..1);
        } else {
            render_pass.set_vertex_buffer(0, self.vertex_buffer.slice(..));
            render_pass.draw(0..self.item_count, 0..1);
        }
    }
}
