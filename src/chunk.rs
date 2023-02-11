use wgpu::Device;

use crate::mesh::{Mesh, Vertex};

const CHUNK_SIZE_XZ: usize = 16;
const CHUNK_SIZE_Y: usize = 256;

const TEX_CELL_COUNT_XY: usize = 8;
const TEX_CELL_SIZE_XY: f32 = 1.0 / TEX_CELL_COUNT_XY as f32;

#[derive(Clone, Copy, PartialEq)]
pub enum Block {
    Air,
    Dirt,
    Grass,
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
            Block::Air => false,
            _ => true,
        }
    }

    fn get_tex_position(&self, direction: Direction) -> (u32, u32) {
        match self {
            Block::Grass => match direction {
                Direction::Up => (0, 0),
                Direction::Down => (2, 0),
                _ => (1, 0),
            },
            Block::Dirt => (2, 0),
            Block::Bedrock => (6, 0),
            _ => (7, 7),
        }
    }

    fn get_tex_coords(&self, direction: Direction) -> (f32, f32) {
        let (x, y) = self.get_tex_position(direction);

        (x as f32 * TEX_CELL_SIZE_XY, y as f32 * TEX_CELL_SIZE_XY)
    }
}

pub struct Chunk {
    blocks: [[[Block; CHUNK_SIZE_XZ]; CHUNK_SIZE_Y]; CHUNK_SIZE_XZ],
}

impl Chunk {
    pub fn new() -> Self {
        let mut blocks = [[[Block::Air; CHUNK_SIZE_XZ]; CHUNK_SIZE_Y]; CHUNK_SIZE_XZ];

        for x in 0..CHUNK_SIZE_XZ {
            for z in 0..CHUNK_SIZE_XZ {
                blocks[x][0][z] = Block::Bedrock;
                for y in 1..5 {
                    blocks[x][y][z] = Block::Dirt;
                }
                blocks[x][5][z] = Block::Grass;
            }
        }

        Chunk { blocks }
    }

    pub fn build_mesh(&self, device: &Device) -> Mesh {
        let mut vertices: Vec<Vertex> = vec![];

        for x in 0..CHUNK_SIZE_XZ {
            for y in 0..CHUNK_SIZE_Y {
                for z in 0..CHUNK_SIZE_XZ {
                    let block = &self.blocks[x][y][z];
                    if !block.is_solid() {
                        continue;
                    }
                    if x == 0 || !self.blocks[x - 1][y][z].is_solid() {
                        // West
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::West);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                        ]);
                    }
                    if x == CHUNK_SIZE_XZ - 1 || !self.blocks[x + 1][y][z].is_solid() {
                        // East
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::East);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                        ]);
                    }
                    if y == 0 || !self.blocks[x][y - 1][z].is_solid() {
                        // Bottom
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::Down);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                        ]);
                    }
                    if y == CHUNK_SIZE_Y - 1 || !self.blocks[x][y + 1][z].is_solid() {
                        // Top
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::Up);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                        ]);
                    }
                    if z == 0 || !self.blocks[x][y][z - 1].is_solid() {
                        // South
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::South);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 0.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                        ]);
                    }
                    if z == CHUNK_SIZE_XZ - 1 || !self.blocks[x][y][z + 1].is_solid() {
                        // North
                        let (tex_x, tex_y) = block.get_tex_coords(Direction::North);
                        #[rustfmt::skip]
                        vertices.extend([
                            Vertex {position: [ 0.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 1.0 + x as f32, 0.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 1.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 1.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 1.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
                            Vertex {position: [ 0.0 + x as f32, 1.0 + y as f32, 1.0 + z as f32 ], tex_coords: [ tex_x + 0.0 * TEX_CELL_SIZE_XY , tex_y + 0.0 * TEX_CELL_SIZE_XY]},
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

        Mesh::from_data(device, vertices.as_slice(), indices.as_slice())
    }
}
