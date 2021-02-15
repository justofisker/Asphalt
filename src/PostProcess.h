#ifndef POSTPROCESS_H
#define POSTPROCESS_H

void PostProcess_Setup();
void PostProcess_RenderToBuffer(unsigned int shader, unsigned int color_slot, unsigned int depth_slot);

void PostProcess_CaptureBuffer();
void PostProcess_ReleaseBuffer();

void PostProcess_ResizeBuffer();

#endif // POSTPROCESS_H