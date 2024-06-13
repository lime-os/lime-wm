#include "all.h"

static void render_background(Display *display, Window root_window)
{
    XSetWindowAttributes attr;
    attr.background_pixel = 0xfcfcfc;
    XChangeWindowAttributes(display, root_window, CWBackPixel, &attr);
    XClearWindow(display, root_window);
    XFlush(display);
}

HANDLE(Expose) {
    render_background(display, root_window);
}
