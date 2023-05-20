use cgmath::{Matrix4, Vector2};
use noise::NoiseFn;
use wgpu::{util::DeviceExt, BindGroupLayout, Device, RenderPass};

use crate::mesh::{Mesh, Vertex};

const CHUNK_SIZE_XZ: usize = 16;
const CHUNK_SIZE_Y: usize = 256;

const TEX_CELL_COUNT_XY: usize = 8;
const TEX_CELL_SIZE_XY: f32 = 1.0 / TEX_CELL_COUNT_XY as f32;

#[derive(Clone, Copy, PartialEq)]
pub enum Block {
    Air,
    Grass,
    Dirt,
    Stone,
    Water,
    Sand,
    Bedrock,
}

pub enum Direction {
    North,
    East,
    South,
    West,
    Up,
    Down,
}

impl Block {
    pub fn is_solid(&self) -> bool {
        match self {
            Self::Air | Self::Water => false,
            _ => true,
        }
    }

    fn get_tex_position(&self, direction: Direction) -> (u32, u32) {
        match self {
            Self::Grass => match direction {
                Direction::Up => (0, 0),
                Direction::Down => (2, 0),
                _ => (1, 0),
            },
            Self::Dirt => (2, 0),
            Self::Stone => (3, 0),
            Self::Water => (4, 0),
            Self::Sand => (5, 0),
            Self::Bedrock => (6, 0),
            Self::Air => (7, 7),
        }
    }

    fn get_tex_coords(&self, direction: Direction) -> (f32, f32) {
        let (x, y) = self.get_tex_position(direction);

        (x as f32 * TEX_CELL_SIZE_XY, y as f32 * TEX_CELL_SIZE_XY)
    }
}

pub struct Chunk {
    blocks: [[[Block; CHUNK_SIZE_XZ]; CHUNK_SIZE_Y]; CHUNK_SIZE_XZ],
    solid_mesh: Option<Mesh>,
    model_buffer: Option<wgpu::Buffer>,
    model_bind_group: Option<wgpu::BindGroup>,
    chunk_position: Vector2<i32>,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct ModelUniform {
    // We can't use cgmath with bytemuck directly so we'll have
    // to convert the Matrix4 into a 4x4 f32 array
    pub view_proj: [[f32; 4]; 4],
}

impl Chunk {
    pub fn new(chunk_position: Vector2<i32>, perlin: &noise::Perlin) -> Self {
        let mut blocks = [[[Block::Air; CHUNK_SIZE_XZ]; CHUNK_SIZE_Y]; CHUNK_SIZE_XZ];

        for x in 0..CHUNK_SIZE_XZ {
            for z in 0..CHUNK_SIZE_XZ {
                let xp = (chunk_position.x * CHUNK_SIZE_XZ as i32 + x as i32) as f64 / 150.0;
                let zp = (chunk_position.y * CHUNK_SIZE_XZ as i32 + z as i32) as f64 / 150.0;

                let perlin_value = (1.0 + perlin.get([xp, zp])) / 2.0;

                let height = 5 + (perlin_value * 50.0) as usize;

                // println!("[{}, {}] {}", xp, zp, perlin_value);

                blocks[x][0][z] = Block::Bedrock;
                for y in 1..height {
                    blocks[x][y][z] = Block::Stone;
                }
                //== blocks[x][4][z] = Block::Dirt;
                blocks[x][height][z] = Block::Grass;
            }
        }

        Chunk {
            blocks,
            solid_mesh: None,
            model_buffer: None,
            model_bind_group: None,
            chunk_position,
        }
    }

    pub fn build_mesh(
        &mut self,
        device: &Device,
        model_layout: &BindGroupLayout,
        neighbor_north: Option<&Chunk>,
        neighbor_east: Option<&Chunk>,
        neighbor_south: Option<&Chunk>,
        neighbor_west: Option<&Chunk>,
    ) {
        let model = ModelUniform {
            view_proj: Matrix4::from_translation(
                (
                    self.chunk_position.x as f32 * CHUNK_SIZE_XZ as f32,
                    0.0,
                    self.chunk_position.y as f32 * CHUNK_SIZE_XZ as f32,
                )
                    .into(),
            )
            .into(),
        };

        if let Some(_model_bind_group) = &self.model_bind_group {
            if let Some(_model_buffer) = &self.model_buffer {
                todo!()
                // TODO: Use at least exisitng bind group. Possibly use same buffer as well if the new buffer is smaller.
            }
            unreachable!()
        } else {
            let model_buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: None,
                contents: bytemuck::cast_slice(&[model]),
                usage: wgpu::BufferUsages::UNIFORM,
            });

            let camera_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
                layout: model_layout,
                entries: &[wgpu::BindGroupEntry {
                    binding: 0,
                    resource: model_buffer.as_entire_binding(),
                }],
                label: Some("camera_bind_group"),
            });

            self.model_buffer = Some(model_buffer);
            self.model_bind_group = Some(camera_bind_group);
        }

        let mut vertices: Vec<Vertex> = vec![];

        const TINT_WEST_EAST: f32 = 0.8;
        const TINT_SOUTH_NORTH: f32 = 0.6;
        const TINT_UP: f32 = 1.0;
        const TINT_DOWN: f32 = 0.4;

        for x in 0..CHUNK_SIZE_XZ {
            for y in 0..CHUNK_SIZE_Y {
                for z in 0..CHUNK_SIZE_XZ {
                    let block = &self.blocks[x][y][z];
                    if !block.is_solid() {
                        continue;
                    }
                    if x == 0
                        && (!neighbor_west.is_some()
                            || !neighbor_west.unwrap().blocks[CHUNK_SIZE_XZ - 1][y][z].is_solid())
                        || (x != 0 && !self.blocks[x - 1][y][z].is_solid())
                    {
                        // West
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::West);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_WEST_EAST},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_WEST_EAST},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_WEST_EAST},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_WEST_EAST},
                        ]);
                    }
                    if x == CHUNK_SIZE_XZ - 1
                        && (!neighbor_east.is_some()
                            || !neighbor_east.unwrap().blocks[0][y][z].is_solid())
                        || (x != CHUNK_SIZE_XZ - 1 && !self.blocks[x + 1][y][z].is_solid())
                    {
                        // East
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::East);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_WEST_EAST},
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_WEST_EAST},
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_WEST_EAST},
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_WEST_EAST},
                        ]);
                    }
                    if y == 0 || !self.blocks[x][y - 1][z].is_solid() {
                        // Bottom
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::Down);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_DOWN},
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_DOWN},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_DOWN},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_DOWN},
                        ]);
                    }
                    if y == CHUNK_SIZE_Y - 1 || !self.blocks[x][y + 1][z].is_solid() {
                        // Top
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::Up);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_UP},
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_UP},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_UP},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_UP},
                        ]);
                    }
                    if z == 0
                        && (!neighbor_south.is_some()
                            || !neighbor_south.unwrap().blocks[x][y][CHUNK_SIZE_XZ - 1].is_solid())
                        || (z != 0 && !self.blocks[x][y][z - 1].is_solid())
                    {
                        // South
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::South);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_SOUTH_NORTH},
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_SOUTH_NORTH},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_SOUTH_NORTH},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_SOUTH_NORTH},
                        ]);
                    }
                    if z == CHUNK_SIZE_XZ - 1
                        && (!neighbor_north.is_some()
                            || !neighbor_north.unwrap().blocks[x][y][0].is_solid())
                        || (z != CHUNK_SIZE_XZ - 1 && !self.blocks[x][y][z + 1].is_solid())
                    {
                        // North
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::North);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_SOUTH_NORTH},
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY], tint: TINT_SOUTH_NORTH},
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_SOUTH_NORTH},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY], tint: TINT_SOUTH_NORTH},
                        ]);
                    }
                }
            }
        }

        let index_count = vertices.len() / 4 * 6;

        let mut indices: Vec<u32> = Vec::with_capacity(index_count);
        // Saftey:
        // We are setting the length to the exact same value as the capacity.
        unsafe {
            indices.set_len(index_count);
        }

        for i in 0..(vertices.len() / 4) {
            indices[i * 6 + 0] = 0 + 4 * i as u32;
            indices[i * 6 + 1] = 1 + 4 * i as u32;
            indices[i * 6 + 2] = 2 + 4 * i as u32;
            indices[i * 6 + 3] = 2 + 4 * i as u32;
            indices[i * 6 + 4] = 3 + 4 * i as u32;
            indices[i * 6 + 5] = 0 + 4 * i as u32;
        }

        self.solid_mesh = Some(Mesh::from_data(
            device,
            vertices.as_slice(),
            Some(indices.as_slice()),
        ));
    }

    pub fn build_mesh_in_context(
        x: usize,
        z: usize,
        device: &Device,
        model_layout: &BindGroupLayout,
        chunks: &mut Vec<Vec<Chunk>>,
    ) {
        let (before_x, after_x) = chunks.split_at_mut(x);
        let (at_x, after_x) = after_x.split_at_mut(1);
        let (before_z, after_z) = at_x[0].split_at_mut(z);
        let (at_z, after_z) = after_z.split_at_mut(1);
        let chunk = &mut at_z[0];

        let mut neighbor_west: Option<&Chunk> = None;
        if x != 0 {
            neighbor_west = Some(&before_x[x - 1][z]);
        }
        let neighbor_east: Option<&Chunk> = after_x.get(0).map_or(None, |c| c.get(z));
        let mut neighbor_south: Option<&Chunk> = None;
        if z != 0 {
            neighbor_south = Some(&before_z[z - 1]);
        }
        let neighbor_north = after_z.get(0);

        chunk.build_mesh(
            &device,
            &model_layout,
            neighbor_north,
            neighbor_east,
            neighbor_south,
            neighbor_west,
        );
    }

    pub fn draw<'a>(&'a self, render_pass: &mut RenderPass<'a>) {
        if let Some(solid_mesh) = &self.solid_mesh {
            if let Some(model_bind_group) = &self.model_bind_group {
                render_pass.set_bind_group(2, model_bind_group, &[]);
                solid_mesh.draw(render_pass);
            }
        }
    }
}
