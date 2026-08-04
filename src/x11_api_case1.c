#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>

/**
 * 
 * Env settings:
 * - The X11 client program runs on Ubuntu 26. 
 * - XFCE desktop environment is installed using xfce4 and xfce4-goodies, which also installs the Xfwm window manager.
 * 
 * 
 * Demo the below:
 * - Try to configure a X11 window with width, height, border width and background color.
 * - Use XFlush to send the commands to X11 server. We need to infinitely wait, 
 *   as the client program should keep running, else the grpahics that has to appear 
 *   may never appear, as the client program that initiated the graphics gets closed.
 * - All configured attributes of X11 window may not work, as we have Window Manager running.
 * - Lets see what properties of a X11 window can be configured.
 * 
 * 
 * Results:
 * - The below window attributes were configured as per code:
 *   - width, height of window.
 *   - background color.
 * 
 * But, the border color, width, x_position, y_position could not be configured.
 * It is because Window Manager was ovveridding these attrs.
 * 
 * Compile with:
 *  gcc src/x11_api_case5.c -o bin/x11_api_case5.bin -lX11
 * 
 */

int main() {
    Display *display = XOpenDisplay(NULL);

    int screen = DefaultScreen(display);

    Colormap colormap = DefaultColormap(display, screen);

    XColor pink;
    XParseColor(display, colormap, "#e493ed", &pink);
    XAllocColor(display, colormap, &pink);

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
        pink.pixel
    );

    XSelectInput(display, window, ExposureMask);//ExposureMask event  is generated  when the X11 window gets visible/exposed.

    XMapWindow(display, window);

    XFlush(display);

    /**
     * Without infinite loop, your program will close before the graphics can appear on the screen.
     */
    while (1) {
         
        /*XEvent event;
        XNextEvent(display, &event);

       if (event.type == Expose) {
            printf("Window Expose event triggered\n")
            //XSetWindowBackground(display, window, pink.pixel);
            //XClearWindow(display, window);
        } */
    }

    return 0;
}