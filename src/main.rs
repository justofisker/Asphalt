// #![windows_subsystem = "windows"]

use asphalt::run;

fn main() {
    pollster::block_on(run());
}
