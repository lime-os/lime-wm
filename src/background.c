#include "all.h"

static void _render_background(Display *display, Window root_window)
{
    XSetWindowAttributes attr;
    attr.background_pixel = 0xfcfcfc;
    XChangeWindowAttributes(display, root_window, CWBackPixel, &attr);
    XClearWindow(display, root_window);
    XFlush(display);
}

// ---
// Handlers
// ---

HANDLE(Expose) {
    _render_background(display, root_window);
}
