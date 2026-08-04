#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    /**
     * Opens a connection to the X server with the default display. Returns a Display* → your handle to the display system.
     * 
     * Why NULL?
        It uses the DISPLAY environment variable (e.g. :0)
        That tells it:
        - which machine
        - which display
        - which screen

        It reads the DISPLAY environment variable, e.g.:
        echo $DISPLAY
        :0

        Connecting to a remote X server:
        You can explicitly pass a display string:

        Display *display = XOpenDisplay("hostname:display.screen");
        Example:
        XOpenDisplay("192.168.1.10:0.0");

        Connect to machine 192.168.1.10
        Use display 0
        Screen 0
     * 
     */
    Display *display = XOpenDisplay(NULL);//
    if (display == NULL) {
        fprintf(stderr, "Cannot open X display\n");
        return 1;
    }

    /**
     * It queries the X11 server (via your display connection).
     * Returns the default screen number for that display.
     */
    int screen = DefaultScreen(display);
    int width = 600;
    int height = 400;

    /**
     * (100, 100) → x and y position of the window relative to the parent (root window).
     * width, height → size of the window. 
     * In X11, applications can suggest a window position, but the window manager ultimately decides where the window is placed. 
     * As a result, coordinates passed to XCreateSimpleWindow may be ignored.
     * 
     * 1 → border width in pixels.
     * 
     * BlackPixel(display, screen) → border color of the window.
     * 
     * WhitePixel(display, screen) → background color of the window.
 */

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        1000, 1000,
        width, height,
        2000,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );

    XSelectInput(display, window,
                 ExposureMask |
                 KeyPressMask |
                 ButtonPressMask |
                 StructureNotifyMask);

    XMapWindow(display, window);

XClearWindow(display, window);

XFlush(display);
    GC gc = XCreateGC(display, window, 0, NULL);

    int ball_x = 100;
    int ball_y = 100;
    int ball_size = 40;

    int dx = 3;
    int dy = 2;

    while (1) {
        while (XPending(display)) {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == ConfigureNotify) {
                width = event.xconfigure.width;
                height = event.xconfigure.height;
            }

            if (event.type == ButtonPress) {
                ball_x = event.xbutton.x - ball_size / 2;
                ball_y = event.xbutton.y - ball_size / 2;
            }

            if (event.type == KeyPress) {
                KeySym key = XLookupKeysym(&event.xkey, 0);

                if (key == XK_Escape || key == XK_q) {
                    XFreeGC(display, gc);
                    XCloseDisplay(display);
                    return 0;
                }

                if (key == XK_Left)  { dx = -4; dy = 0; }
                if (key == XK_Right) { dx =  4; dy = 0; }
                if (key == XK_Up)    { dx = 0;  dy = -4; }
                if (key == XK_Down)  { dx = 0;  dy =  4; }

                ball_x += dx;
                ball_y += dy;
            }
        }

        /**
         * Stop the ball from crossing window boundary in x axis within the window.
         */
        if (ball_x <= 0 || ball_x + ball_size >= width) {
             if (ball_x <= 0) {
                ball_x = 0;
            }
            if (ball_x + ball_size >= width) {
                ball_x = width - ball_size;
            }
        }

        /**
         * Stop the ball from crossing window boundary in y axis within the window.
         */
        if (ball_y <= 0 || ball_y + ball_size >= height) {

            if (ball_y <= 0) {
                ball_y = 0;
            }
            if (ball_y + ball_size >= height) {
                ball_y = height - ball_size;
            }
        }


        XSetForeground(display, gc, WhitePixel(display, screen));
        XFillRectangle(display, window, gc, 0, 0, width, height);

        XSetForeground(display, gc, BlackPixel(display, screen));
        XFillArc(display, window, gc,
                 ball_x, ball_y,
                 ball_size, ball_size,
                 0, 360 * 64);

        XFlush(display);

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 16000000; // 16 ms
        nanosleep(&ts, NULL);
    }

    return 0;
}