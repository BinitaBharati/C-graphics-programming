#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>

/**
 * 
 Same as case 1 except the below.
 Instead of XFlush,  XNextEvent(display, &event); has been used to send the commands to the X11 server.
 And blue color has been used in the background instead of pink.
 * 
 */

int main() {
    Display *display = XOpenDisplay(NULL);

    int screen = DefaultScreen(display);

    Colormap colormap = DefaultColormap(display, screen);

    XColor blue;
    XParseColor(display, colormap, "#17b2ea", &blue);
    XAllocColor(display, colormap, &blue);

    XColor red;
    XParseColor(display, colormap, "#a10922", &red);
    XAllocColor(display, colormap, &red);

    /**
     * Window XCreateSimpleWindow(
    Display *display,
    Window parent,
    int x_position,
    int y_position,
    unsigned int width,
    unsigned int height,
    unsigned int border_width,
    unsigned long border,
    unsigned long background
);
     */
    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        1000, 100,
        400, 300,
        100,
        red.pixel,
        blue.pixel
    );

    XSelectInput(display, window, ExposureMask);//ExposureMask event  is generated  when the X11 window gets visible/exposed.

    XMapWindow(display, window);

         
    XEvent event;
    XNextEvent(display, &event);

     /**
     * Without infinite loop, your program will close before the graphics can appear on the screen.
     */
    while (1){}


    return 0;
}