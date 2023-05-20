use wgpu::{
    util::{BufferInitDescriptor, DeviceExt},
    Device, RenderPass,
};

#[repr(C)]
#[derive(Copy, Clone, Debug, bytemuck::Pod, bytemuck::Zeroable)]
pub struct Vertex {
    pub position: [f32; 3],
    pub tex_coords: [f32; 2],
    pub tint: f32,
}

impl Vertex {
    const ATTRIBS: [wgpu::VertexAttribute; 3] =
        wgpu::vertex_attr_array![0 => Float32x3, 1 => Float32x2, 2 => Float32];

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
    pub fn from_data(device: &Device, vertices: &[Vertex], indices: Option<&[u32]>) -> Self {
        let vertex_buffer = device.create_buffer_init(&BufferInitDescriptor {
            label: None,
            contents: bytemuck::cast_slice(vertices),
            usage: wgpu::BufferUsages::VERTEX,
        });

        let mut index_buffer: Option<wgpu::Buffer> = None;
        let item_count: u32;

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
