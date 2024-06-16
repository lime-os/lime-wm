#include "all.h"

static cairo_t *cr = NULL;
static cairo_surface_t *xlib_surface = NULL;
static cairo_surface_t *png_surface = NULL;

static char background_mode[16];
static unsigned long background_color;
static char background_image_path[MAX_PATH];

static cairo_surface_t *load_background_image(Display *display, const char *filename)
{
    // Create a surface to hold the original image.
    cairo_surface_t *original_image = cairo_image_surface_create_from_png(filename);
    if (cairo_surface_status(original_image) != CAIRO_STATUS_SUCCESS)
    {
        fprintf(stderr, "Failed to load background image %s\n", filename);
        return NULL;
    }

    int screen = DefaultScreen(display);
    int display_width = DisplayWidth(display, screen);
    int display_height = DisplayHeight(display, screen);
    int image_width = cairo_image_surface_get_width(original_image);
    int image_height = cairo_image_surface_get_height(original_image);

    // Create a surface to hold the scaled image.
    cairo_surface_t *scaled_image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, display_width, display_height);
    cairo_t *cr = cairo_create(scaled_image);
    cairo_scale(cr, (double)display_width / image_width, (double)display_height / image_height);
    cairo_set_source_surface(cr, original_image, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);

    // Free the original image surface from memory.
    cairo_surface_destroy(original_image);

    return scaled_image;
}

static void draw_background_image(cairo_t *cr, cairo_surface_t *png_surface)
{
    if (cr == NULL || xlib_surface == NULL || png_surface == NULL)
        return;

    cairo_set_source_surface(cr, png_surface, 0, 0);
    cairo_paint(cr);
}

static void draw_background_solid(cairo_t *cr, unsigned long color)
{
    if (cr == NULL || xlib_surface == NULL)
        return;

    double r, g, b = 0;
    hex_to_rgb(color, &r, &g, &b);

    cairo_set_source_rgb(cr, r, g, b);
    cairo_fill(cr);
    cairo_paint(cr);
}

HANDLE(Initialize)
{
    // Get configuration values.
    GET_CONFIG(background_mode, sizeof(background_mode), CFG_BUNDLE_BACKGROUND_MODE);
    GET_CONFIG(&background_color, sizeof(background_color), CFG_BUNDLE_BACKGROUND_COLOR);
    GET_CONFIG(background_image_path, sizeof(background_image_path), CFG_BUNDLE_BACKGROUND_IMAGE_PATH);

    // Prepare xlib surface.
    int screen = DefaultScreen(display);
    int width = DisplayWidth(display, screen);
    int height = DisplayHeight(display, screen);
    xlib_surface = cairo_xlib_surface_create(display, root_window, DefaultVisual(display, screen), width, height);

    // Prepare png surface if neccessary.
    if (strcmp(background_mode, "image") == 0)
    {
        char expanded_path[MAX_PATH];
        expand_path(background_image_path, expanded_path, sizeof(expanded_path));
        png_surface = load_background_image(display, expanded_path);
    }

    cr = cairo_create(xlib_surface);
}

HANDLE(Expose)
{
    if (event->xexpose.window == root_window && event->xexpose.count == 0)
    {
        if (strcmp(background_mode, "image") == 0)
        {
            draw_background_image(cr, png_surface);
        }
        else
        {
            draw_background_solid(cr, background_color);
        }
    }
}
