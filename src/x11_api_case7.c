#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <time.h> 

#include "./common/util.h"
/**
 * Run time env is same as all other cases.
 * Demos how to make multiple objects (apples) move in each frame without
 * causing the screen to flicker in the perspective of human eye.
 *
 */

void draw_apple(Display *display, GC gc, unsigned long  apple_color_pixel, Pixmap buffer,
                ArcPos* arcCoords,  int apple_arc_width,int apple_arc_height, int i) {

        XSetForeground(display, gc, apple_color_pixel);
        XFillArc(display, buffer, gc,
                arcCoords[i].x,
                arcCoords[i].y,
                apple_arc_width, apple_arc_height,//20, 25
                0, 360 * 64);

        /* right half */
        XFillArc(display, buffer, gc,
                 arcCoords[i].x + 10,
                 arcCoords[i].y,
                 apple_arc_width, apple_arc_height,
                 0, 360 * 64);

        /* stem */

        /*XSetForeground(display, gc,
                       BlackPixel(display, screen));

        XDrawLine(display, buffer, gc,
                  arcCoords[i].x + 15,
                  arcCoords[i].y,
                  arcCoords[i].x + 10,
                  arcCoords[i].y - 10);*/

}

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
                      int window_height, 
                      int apple_arc_width,//20
                      int apple_arc_height,//25
                      double last_eater_x,
                      double last_eater_y,
                      int eater_width,//25
                      int eater_height,//30
                      XColor eater_color,
                      KeySym key) {

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

    /**
     * Calculate apple center width wise:
     * Assume that apple coords x starts at 0 to make the calculation simple.
   
     * center = (min_x  + max_x )/2
     * Left half:

        starts at x
        ends at x + 20

        Right half:

        starts at x + 10
        ends at x + 10 + 20

        So overall:

        min_x = x
        max_x = x + 10 + 20

        Therefore:

        center_x = (min_x + max_x) / 2

        Substitute:

        center_x = (x + (x + 10 + 20)) / 2

        Simplify:

        center_x = (2x + 30) / 2
                 = x + 15
     * 

     * Calculate apple center height wise:
     * Assume that apple coords y starts at 0 to make the calculation simple.
   
     * center = (min_y  + max_y )/2
     * Left half:

        starts at y
        ends at y + 25

        Right half:

        starts at y
        ends at y + 20

        So overall:

        min_x = y
        max_x = y + 25

        Therefore:

        center_y = (min_y + max_y) / 2

        Substitute:

        center_y = (y + (y + 25)) / 2

        Simplify:

        center_y = (2y + 25) / 2
                 = y + 12.5

    * Calculate eater center width wise:
     * Assume that apple coords x, and coords y starts at 0,0 to make the calculation simple.
   
     * center_x = (min_x  + max_x )/2
              = (0 + 25)/2 = 12.5
     

        center_y = (min_y  + max_y )/2
                 = (0 + 30) / 2
                 = 15
     * 
     * 
     */
      double apple_center_x = arcCoords[i].x + 15;
      double apple_center_y = arcCoords[i].y + 12.5;

      double eater_center_x = last_eater_x + 12.5;
      double eater_center_y = last_eater_y + 15;

      double dx = apple_center_x - eater_center_x;
      double dy = apple_center_y - eater_center_y;

      /**
       * This comes from the Pythagorean theorem. The adjective form of “Pythagoras” is “Pythagorean”, it was made by “Pythagoras” greek guy.

        If you wanted the real distance:

        distance * distance = dx * dx + dy * dy

        distance = sqrt(dx * dx + dy * dy)

        But games often skip the square root as computation of sqrt by cpu is costly.
       */

    double distance_squared = dx * dx + dy * dy;
     
    double radius_thresold = 30/2 //apple overall width/2 = radius of apple overall width
                            + 
                            eater_width/2; //ball width/2 

    if (distance_squared < radius_thresold * radius_thresold) //this is because distance is squared as per Pythogoreas theorem, so even on RHS we have to square the radius_thresold
    {
        //collision happened between apple and eater
        printf("apple eaten\n");
        arcCoords[i].colliding_with_eater = 1;
        //draw_apple(display, gc, BlackPixel(display, screen), buffer,
                   //arcCoords, apple_arc_width, apple_arc_height, i);
    }

    if (arcCoords[i].colliding_with_eater == 0) {
         draw_apple(display, gc, red.pixel, buffer,
                   arcCoords, apple_arc_width, apple_arc_height, i);
    }
  
    }

    int start_angle = 0;
    int sweep_angle = 360 * 64;
    if (last_eater_x != -1 && last_eater_y != -1 ) {
        XSetForeground(display, gc,
                       eater_color.pixel);

            if (key == XK_Left)  { start_angle = 210 * 64; sweep_angle = 300 * 64;}
            if (key == XK_Right) { start_angle = 30 * 64; sweep_angle = 300 * 64;}
            if (key == XK_Up)    { start_angle = 120 * 64; sweep_angle = 300 * 64;}
            if (key == XK_Down)  { start_angle = 300 * 64; sweep_angle = 300 * 64;}
            
        
     
        XFillArc(display, buffer, gc,
             last_eater_x, last_eater_y,
             eater_width, eater_height,
             start_angle, sweep_angle);
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

void increment_apple_position(int arcs_count,  ArcPos* arcXyCcords, 
    int apple_width, int apple_height, int window_width, int window_height) {
    for (int i = 0; i < arcs_count; i++) {

        /*printf("Arc %d -> x=%f, y=%f\n",
               i,
               arcXyCcords[i].x,
               arcXyCcords[i].y);*/

        /* constant movement */

        arcXyCcords[i].x += 0.2;
        arcXyCcords[i].y += 0.2;

        /* reset if outside window */

        if (arcXyCcords[i].x + apple_width >= window_width) {
            arcXyCcords[i].x = 0;
        }

        if (arcXyCcords[i].y + apple_height >= window_height) {
            arcXyCcords[i].y = 0;
        }
    }
}

int main() {

    Display *display = XOpenDisplay(NULL);

    if (display == NULL) {
        return 1;
    }

    int screen = DefaultScreen(display);

    int window_width = 700;
    int window_height = 600;
    int apple_width = 20;
    int apple_height = 25;
    int arcs_count = 40;
    int eater_width = 25;
    int eater_height = 30;


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
        window_width, window_height,
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
    window_height,
    DefaultDepth(display, screen)
);

 XColor eater_color;
    XParseColor(display, colormap, "#3409a1", &eater_color);
    XAllocColor(display, colormap, &eater_color);

    

    int x_pixel_inc_counter = 0;
    int y_pixel_inc_counter = 0;
    ArcPos* arcXyCcords = getArcPositions(window_width, window_height, apple_width, apple_height, arcs_count );
    int coords_reinited = 0;

    double eater_x = 0;
    double eater_y = 0;
    int dx , dy = 0;
    KeySym key = 0;
   while (1) {
      while (XPending(display))//XPending is non blocking call, it returns the numner of evenst waiting for attention
      {
            XEvent event;
            XNextEvent(display, &event);//blovking call, but, since its called inside non blocking XPending call, this call wont block.

              if (event.type == KeyPress) {
               key = XLookupKeysym(&event.xkey, 0);

                if (key == XK_Escape || key == XK_q) {
                    XFreeGC(display, gc);
                    XCloseDisplay(display);
                    return 0;
                }

                if (key == XK_Left)  { dx = -4; dy = 0; }
                if (key == XK_Right) { dx =  4; dy = 0; }
                if (key == XK_Up)    { dx = 0;  dy = -4; }
                if (key == XK_Down)  { dx = 0;  dy =  4; }

                eater_x += dx;
                eater_y += dy;
                //increment_apple_position(arcs_count, arcXyCcords);
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
                     window_height,
                     apple_width,
                     apple_height,
                     eater_x,
                     eater_y,
                     eater_width,
                     eater_height,
                     eater_color,
                     key
                    );
            }
     }

    increment_apple_position(arcs_count, arcXyCcords, apple_width, apple_height, window_width, window_height);
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
                     window_height,
                     apple_width,
                     apple_height,
                     eater_x,
                     eater_y,
                     eater_width,
                     eater_height,
                     eater_color,
                     key
                    );

    struct timespec ts;

    ts.tv_sec = 0;
    ts.tv_nsec = 16000000; // ~60 FPS

    nanosleep(&ts, NULL);
}

    return 0;
}

