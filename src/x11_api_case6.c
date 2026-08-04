#include <X11/Xlib.h>
#include <unistd.h>
#include <time.h> 

#include "./common/util.h"
/**
 * Run time env is same as all other cases.
 * Demos how to make multiple objects (apples) move in each frame without
 * causing the screen to flicker in the perspective of human eye.
 *
 */

void set_balls_random(Display *display,
                      Window window,
                      Pixmap buffer,
                      GC gc,
                      int screen,
                      XColor blue,
                      XColor red,
                      ArcPos* arcCoords,
                      int arcs_count,
                      int window_width,
                      int window_height) {

    /* clear off-screen buffer */
    //set drawing color to blue.                    
    XSetForeground(display, gc, blue.pixel);
    //fill window with blue color. essentially its like clearing window effect.                 
    XFillRectangle(display,
                   buffer,
                   gc,
                   0,
                   0,
                   window_width,
                   window_height);

    /* draw apples */

    XSetForeground(display, gc, red.pixel);

    for (int i = 0; i < arcs_count; i++) {

        /* left half */
        XFillArc(display, buffer, gc,
                 arcCoords[i].x,
                 arcCoords[i].y,
                 20, 25,
                 0, 360 * 64);

        /* right half */
        XFillArc(display, buffer, gc,
                 arcCoords[i].x + 10,
                 arcCoords[i].y,
                 20, 25,
                 0, 360 * 64);

        /* stem */

        XSetForeground(display, gc,
                       BlackPixel(display, screen));

        XDrawLine(display, buffer, gc,
                  arcCoords[i].x + 15,
                  arcCoords[i].y,
                  arcCoords[i].x + 10,
                  arcCoords[i].y - 10);

        XSetForeground(display, gc, red.pixel);
    }

    /* copy finished frame to real window */

    XCopyArea(display,
              buffer,
              window,
              gc,
              0, 0,
              700, 600,
              0, 0);

    XFlush(display);
}

int main() {

    Display *display = XOpenDisplay(NULL);

    if (display == NULL) {
        return 1;
    }

    int screen = DefaultScreen(display);

    int window_width = 700;
    int widow_height = 600;
    int arc_width = 20;
    int arc_height = 25;
    int arcs_count = 40;


    Colormap colormap = DefaultColormap(display, screen);

    XColor blue;
    XParseColor(display, colormap, "#a4dff4", &blue);
    XAllocColor(display, colormap, &blue);

    XColor red;
    XParseColor(display, colormap, "#a10922", &red);
    XAllocColor(display, colormap, &red);

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100, 100,
        window_width, widow_height,
        2,
        red.pixel,//window border color - this anyway wont apply, due to WindowManager running on my machine.
        blue.pixel
    );

    XSelectInput(display, window,   
                ExposureMask |
                KeyPressMask |
                ButtonPressMask |
                StructureNotifyMask);

    XMapWindow(display, window);

    GC gc = XCreateGC(display, window, 0, NULL);

    Pixmap buffer = XCreatePixmap(
    display,
    window,
    window_width,
    widow_height,
    DefaultDepth(display, screen)
);

    

    int x_pixel_inc_counter = 0;
    int y_pixel_inc_counter = 0;
    ArcPos* arcXyCcords = getArcPositions(window_width, widow_height, arc_width, arc_height, arcs_count );
    int coords_reinited = 0;
   while (1) {

    for (int i = 0; i < arcs_count; i++) {

        printf("Arc %d -> x=%f, y=%f\n",
               i,
               arcXyCcords[i].x,
               arcXyCcords[i].y);

        /* constant movement */

        arcXyCcords[i].x += 0.5;
        arcXyCcords[i].y += 0.5;

        /* reset if outside window */

        if (arcXyCcords[i].x + arc_width >= window_width) {
            arcXyCcords[i].x = 0;
        }

        if (arcXyCcords[i].y + arc_height >= widow_height) {
            arcXyCcords[i].y = 0;
        }
    }

    set_balls_random(display,
                     window,
                     buffer,
                     gc,
                     screen,
                     blue,
                     red,
                     arcXyCcords,
                     arcs_count,
                     window_width,
                     widow_height);

    struct timespec ts;

    ts.tv_sec = 0;
    ts.tv_nsec = 16000000; // ~60 FPS

    nanosleep(&ts, NULL);
}

    return 0;
}

