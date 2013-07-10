#ifndef SCREEN_H
#define SCREEN_H

void *fb_request(int width, int height, int depth);
void fb_draw_image(void *fb, void *img, int img_w, int img_h);

#endif
