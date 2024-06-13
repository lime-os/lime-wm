#include "all.h"

int main()
{
    Display *display;

    display = XOpenDisplay(NULL);
    if (display)
    {
        Window root_window;
        root_window = DefaultRootWindow(display);
        initialize_event_loop(display, root_window);
        XCloseDisplay(display);
    }
    else
    {
        fprintf(stderr, "Failed to open display\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
