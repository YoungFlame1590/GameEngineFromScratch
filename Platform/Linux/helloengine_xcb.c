/*
 * helloengine_xcb.c
 *
 * A hello-world example of creating a window using XCB (X C Binding,
 * the C language binding of the X protocol).
 * Flow:
 *   1. Connect to the X server and get the default screen;
 *   2. Create graphics contexts (GCs) and the main window, set the title;
 *   3. Map the window to make it visible, then enter the event loop;
 *   4. Press any key to exit the program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>

int main(void)
{
    /* ---- Variable declarations ---- */
    xcb_connection_t *pConn;     /* Connection to the X server */
    xcb_screen_t *pScreen;       /* Default screen info (size, depth, root window, etc.) */
    xcb_window_t window;         /* Window ID */
    xcb_gcontext_t foreground;   /* Foreground graphics context (GC, defines colors used for drawing) */
    xcb_gcontext_t background;   /* Background graphics context */
    xcb_generic_event_t *pEvent; /* Event received from the X server */
    uint32_t mask = 0;           /* Attribute mask, indicates which fields in values are valid */
    uint32_t values[2];          /* Array of attribute values corresponding to the mask */
    uint8_t isQuit = 0;          /* Exit flag; the event loop ends when set to 1 */

    char title[] = "Hello Engine!";                  /* Window title */
    char title_icon[] = "Hello Engine! (iconified)"; /* Title shown when minimized */

    /* ---- Connect to the X server and get the default screen ---- */
    /* The first argument is the display name; NULL(0) uses the display specified by $DISPLAY.
       The second is an error pointer; NULL(0) ignores connection errors. */
    pConn = xcb_connect(0, 0);
    /* Take the root screen from the connection info as the default target for creating windows/GCs */
    pScreen = xcb_setup_roots_iterator(xcb_get_setup(pConn)).data;

    /* ---- Create the foreground graphics context (GC) ---- */
    /* Creating a GC requires a target window; the root window is used here */
    window = pScreen->root;

    foreground = xcb_generate_id(pConn);                  /* Ask the server to allocate a new resource ID */
    mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES; /* Fields set: foreground color + whether to record graphics exposures */
    values[0] = pScreen->black_pixel;                     /* Set the foreground color to black */
    values[1] = 0;                                        /* GRAPHICS_EXPOSURES is 0: do not record graphics exposure events */
    xcb_create_gc(pConn, foreground, window, mask, values);

    /* ---- Create the background graphics context (GC); the background color is white ---- */
    background = xcb_generate_id(pConn);
    mask = XCB_GC_BACKGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    values[0] = pScreen->white_pixel; /* Set the background color to white */
    values[1] = 0;
    xcb_create_gc(pConn, background, window, mask, values);

    /* ---- Create the actual application window ---- */
    window = xcb_generate_id(pConn);                                /* Allocate a new resource ID for the window */
    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;                   /* Fields set: background pixel + event mask */
    values[0] = pScreen->white_pixel;                               /* Window background is white */
    values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS; /* Watch for "window expose" and "key press" events */
    xcb_create_window(pConn, XCB_COPY_FROM_PARENT,                  /* Inherit depth, class, and visual from the parent */
                      window, pScreen->root,                        /* New window ID; parent window is the root window */
                      20, 20, 640, 480, 10,                         /* Position (20,20), size 640x480, border width 10 px */
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,                /* Window class: can display content and receive input */
                      pScreen->root_visual, mask, values);

    /* ---- Set the window title properties ---- */
    xcb_change_property(pConn, XCB_PROP_MODE_REPLACE, /* Overwrite the old property value in REPLACE mode */
                        window,
                        XCB_ATOM_WM_NAME,   /* Title property used by the window manager */
                        XCB_ATOM_STRING, 8, /* Property type is a string, each element takes 8 bits */
                        strlen(title), title);

    /* Set the title shown when the window is minimized (iconified) */
    xcb_change_property(pConn, XCB_PROP_MODE_REPLACE, window,
                        XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
                        strlen(title_icon), title_icon);

    /* ---- Map the window to the screen (make it visible) and flush requests ---- */
    xcb_map_window(pConn, window);
    /* Send all accumulated requests to the X server at once (XCB queues asynchronously by default) */
    xcb_flush(pConn);

    /* ---- Event loop ---- */
    /* Block and wait for server events; exit the loop on connection error (NULL) or when isQuit is set */
    while ((pEvent = xcb_wait_for_event(pConn)) && !isQuit)
    {
        /* The event type is stored in the low 7 bits of response_type; the high bit indicates
           whether the event was sent by another client. Mask it with &~0x80 (equivalent to &0x7f)
           to get the real event type before comparison. */
        switch (pEvent->response_type & ~0x80)
        {
        case XCB_EXPOSE: /* Window content is exposed and needs repainting; draw a rectangle here */
        {
            xcb_rectangle_t rect = {20, 20, 60, 80};
            xcb_poly_fill_rectangle(pConn, window, foreground, 1, &rect);
            xcb_flush(pConn);
        }
        break;
        case XCB_KEY_PRESS: /* Any key: set the exit flag to end the program */
            isQuit = 1;
            break;
        }
        free(pEvent); /* Each event is allocated by xcb_wait_for_event; must be freed after use */
    }

    xcb_disconnect(pConn);

    return 0;
}
