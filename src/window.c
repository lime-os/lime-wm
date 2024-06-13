#include "all.h"

FrameMap *frame_map = NULL;
int frame_map_size = 0;

Window dragged_window = 0;
bool is_dragging = false;
int drag_start_x = 0, drag_start_y = 0;
int window_start_x = 0, window_start_y = 0;

static const long event_mask =
    SubstructureRedirectMask |
    SubstructureNotifyMask |
    ExposureMask |
    ButtonPressMask |
    ButtonReleaseMask |
    PointerMotionMask;

static void configure_window(Display *display, XConfigureRequestEvent *configure_request)
{
    XWindowChanges changes;
    changes.x = configure_request->x;
    changes.y = configure_request->y;
    changes.width = configure_request->width;
    changes.height = configure_request->height;
    changes.border_width = configure_request->border_width;
    changes.sibling = configure_request->above;
    changes.stack_mode = configure_request->detail;

    XConfigureWindow(
        display,
        configure_request->window,
        configure_request->value_mask,
        &changes);
}

static void focus_window(Display *display, Window window)
{
    XRaiseWindow(display, window);
}

static void start_dragging(Display *display, Window window, int mouse_x, int mouse_y)
{
    XWindowAttributes attr;
    if (!XGetWindowAttributes(display, window, &attr))
    {
        return;
    }

    is_dragging = true;
    dragged_window = window;
    drag_start_x = mouse_x;
    drag_start_y = mouse_y;
    window_start_x = attr.x;
    window_start_y = attr.y;
}

static void update_dragging(Display *display, int mouse_x, int mouse_y)
{
    int dx = mouse_x - drag_start_x;
    int dy = mouse_y - drag_start_y;

    XMoveWindow(display, dragged_window, window_start_x + dx, window_start_y + dy);
}

static void stop_dragging()
{
    is_dragging = false;
}

static void register_frame(Window frame_window, Window child_window)
{
    FrameMap *buffer = realloc(frame_map, (frame_map_size + 1) * sizeof(FrameMap));
    if (buffer == NULL)
    {
        perror("Failed to allocate memory while registering to frame map\n");
        exit(EXIT_FAILURE);
    }

    frame_map = buffer;
    frame_map[frame_map_size].frame_window = frame_window;
    frame_map[frame_map_size].child_window = child_window;
    frame_map_size++;
}

static bool is_frame(Window window)
{
    for (int i = 0; i < frame_map_size; i++)
    {
        if (frame_map[i].frame_window == window)
        {
            return true;
        }
    }
    return false;
}

static Window *find_parent_frame(Window window)
{
    for (int i = 0; i < frame_map_size; i++)
    {
        if(frame_map[i].child_window == window)
        {
            return &frame_map[i].frame_window;
        }
    }

    return NULL;
}

static void unregister_frame(Window frame_window)
{
    int wasFound = false;

    // Find the index of the frame to be removed.
    for (int i = 0; i < frame_map_size; i++)
    {
        if (frame_map[i].frame_window == frame_window)
        {
            wasFound = true;
            // Shift all elements after the found index to the left.
            for (int j = i; j < frame_map_size - 1; j++)
            {
                frame_map[j] = frame_map[j + 1];
            }
            break;
        }
    }

    if (wasFound)
    {
        frame_map_size--;

        // Resize the array to the new size.
        FrameMap *buffer = realloc(frame_map, frame_map_size * sizeof(FrameMap));
        if (buffer == NULL && frame_map_size > 0)
        {
            perror("Failed to allocate memory while unregistering from frame map\n");
            exit(EXIT_FAILURE);
        }
        frame_map = buffer;
    }
}

static void draw_frame(Display *display, Window frame_window, int width, int height)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_xlib_surface_create(display, frame_window, DefaultVisual(display, 0), width, height);
    cr = cairo_create(surface);

    // Draw title bar.
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_rectangle(cr, 0, 0, width, 15);
    cairo_fill(cr);

    // Set the color for the border around the window.
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

static void redraw_frame(Display *display, Window frame_window)
{
    XWindowAttributes attr;
    XGetWindowAttributes(display, frame_window, &attr);
    draw_frame(display, frame_window, attr.width, attr.height);
}

static void create_frame(Display *display, Window root_window, Window target_window)
{
    XWindowAttributes attr;
    XGetWindowAttributes(display, target_window, &attr);

    Window frame_window = XCreateSimpleWindow(
        display,
        root_window,
        attr.x, attr.y,
        attr.width, attr.height,
        2, 0x000000, 0xFFFFFF);

    register_frame(frame_window, target_window);

    XSelectInput(display, frame_window, event_mask);
    XAddToSaveSet(display, target_window);
    XReparentWindow(display, target_window, frame_window, 0, 15);
    XMapWindow(display, frame_window);
    XMapWindow(display, target_window);

    draw_frame(display, frame_window, attr.width, attr.height + 15);
}

static void destroy_frame(Display *display, Window frame_window)
{
    XDestroyWindow(display, frame_window);
    unregister_frame(frame_window);
}

HANDLE(ConfigureRequest)
{
    configure_window(display, &event->xconfigurerequest);
}

HANDLE(MapRequest)
{
    create_frame(display, root_window, event->xmaprequest.window);
}

HANDLE(DestroyNotify)
{
    Window *frame_window = find_parent_frame(event->xdestroywindow.window);
    if (frame_window != NULL)
    {
        destroy_frame(display, *frame_window);
    }
}

HANDLE(ButtonPress)
{
    if (event->xbutton.button == Button1)
    {
        XButtonEvent button_event = event->xbutton;

        focus_window(display, button_event.window);

        if (is_dragging == false)
        {
            start_dragging(display, button_event.window, button_event.x_root, button_event.y_root);
        }
    }
}

HANDLE(ButtonRelease)
{
    if (event->xbutton.button == Button1 && is_dragging == true)
    {
        stop_dragging();
    }
}

HANDLE(MotionNotify)
{
    if (is_dragging == true)
    {
        XMotionEvent motion_event = event->xmotion;
        update_dragging(display, motion_event.x_root, motion_event.y_root);
    }
}

HANDLE(Expose)
{
    if (event->xexpose.count == 0)
    {
        Window exposed_window = event->xexpose.window;
        if (is_frame(exposed_window))
        {
            redraw_frame(display, exposed_window);
        }
    }
}
