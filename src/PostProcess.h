#ifndef POSTPROCESS_H
#define POSTPROCESS_H

void setup_postprocess();
void do_postprocess(unsigned int shader, unsigned int color_slot, unsigned int depth_slot);

void render_start_postprocess();
void render_end_postprocess();

void resize_postprocess();

#endif // POSTPROCESS_H