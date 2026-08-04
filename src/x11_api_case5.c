#include <X11/Xlib.h>
#include <unistd.h>
#include <time.h> 
/**
 * Run time env is same as all other cases.
 * Demos 
 * - Same as case 4, exept that monster eye brows will keep moving up and down.
 * - this movement will happen unprompted, ie without any action by user.
 * - XNextEvent(display, &event); has been removed, because we dont expect any input from user.
 * - if we keep the blocking XNextEvent method, then only for the first time automatic Exposure event
 *   is sent by server, so , XNextEvent will unblock and code control goes below that displays the animation.
 * - sleep has been introduced so that eye brows moving can be registered by human eyes, else
 *   they move too fast to be perceived by human eyes.
 */

int main() {

    Display *display = XOpenDisplay(NULL);

    if (display == NULL) {
        return 1;
    }

    int screen = DefaultScreen(display);

    int width = 700;
    int height = 600;

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100, 100,
        width, height,
        2,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );

    XSelectInput(display, window, ExposureMask);

    XMapWindow(display, window);

    GC gc = XCreateGC(display, window, 0, NULL);

    int iteration_no = 0;
    while (1) {


        /* clear background */
            XClearWindow(display, window);

            //set drawing color to black.
            XSetForeground(display, gc, BlackPixel(display, screen));
            
            /* monster head drawn in filled black circle.
            int XFillArc(Display *display,
             Drawable d,
             GC gc,
             int x, int y,
             unsigned int width, unsigned int height,
             int angle1, int angle2);
              */
            XFillArc(display, window, gc,
                     180, 120,
                     340, 340,
                     0, 360 * 64);

            //set drawing color to white.
            XSetForeground(display, gc,
                           WhitePixel(display, screen));

            /* monster eyes drawn in filled white circle.
            250 - 180 = 70
            210 - 120 = 90
             */
            //1st eye
            XFillArc(display, window, gc,
                     250, 210,
                     70, 70,
                     0, 360 * 64);

            //2nd eye
            XFillArc(display, window, gc,
                     380, 210,
                     70, 70,
                     0, 360 * 64);

            /* pupils */
            //set drawing color to black.
            XSetForeground(display, gc,
                           BlackPixel(display, screen));

            //1st pupil
            XFillArc(display, window, gc,
                     275, 235,
                     25, 25,
                     0, 360 * 64);
            //2nd pupil
            XFillArc(display, window, gc,
                     405, 235,
                     25, 25,
                     0, 360 * 64);

             //set drawing color to white.
            /* white eyebrows */
            XSetForeground(display, gc,
                           WhitePixel(display, screen));

            /**
            int XDrawLine(
            Display *display,
            Drawable drawable,
            GC gc,
            int x1,
            int y1,
            int x2,
            int y2
            );
             */
            if (iteration_no % 2 == 0) {
                XDrawLine(display, window, gc,
                      250, 190,
                      320, 210);

                XDrawLine(display, window, gc,
                      380, 210,
                      450, 190);
            }
            else {
                  XDrawLine(display, window, gc,
                      250, 170,
                      320, 190);

                XDrawLine(display, window, gc,
                      380, 190,
                      450, 170);
            }
            iteration_no++;
           
            /* white nose */
            XDrawLine(display, window, gc,
                      340, 280,
                      320, 330);

            XDrawLine(display, window, gc,
                      320, 330,
                      360, 330);

            XDrawLine(display, window, gc,
                      360, 330,
                      340, 280);

            /* mouth
            XDrawArc does not fill the arc unlike XFillArc */
            XDrawArc(display, window, gc,
                     250, 320,
                     180, 100,
                     180 * 64,
                     180 * 64);

            /* monster teeth */
            XPoint tooth1[] = {
                {290, 370},
                {305, 410},
                {320, 370}
            };

            XPoint tooth2[] = {
                {340, 370},
                {355, 415},
                {370, 370}
            };

            XPoint tooth3[] = {
                {390, 370},
                {405, 410},
                {420, 370}
            };

            XFillPolygon(display, window, gc,
                         tooth1, 3,
                         Convex, CoordModeOrigin);

            XFillPolygon(display, window, gc,
                         tooth2, 3,
                         Convex, CoordModeOrigin);

            XFillPolygon(display, window, gc,
                         tooth3, 3,
                         Convex, CoordModeOrigin);

            /* switch back to black */
            XSetForeground(display, gc,
                           BlackPixel(display, screen));

            /* left horn */
            XDrawLine(display, window, gc,
                      240, 150,
                      190, 40);

            XDrawLine(display, window, gc,
                      190, 40,
                      290, 120);

            /* right horn */
            XDrawLine(display, window, gc,
                      460, 150,
                      510, 40);

            XDrawLine(display, window, gc,
                      510, 40,
                      410, 120);

            /* scars */
            XDrawLine(display, window, gc,
                      220, 320,
                      260, 360);

            XDrawLine(display, window, gc,
                      240, 320,
                      280, 360);

            XFlush(display);

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000; // 100 ms
        nanosleep(&ts, NULL);
    }

    return 0;
}