#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define TILE_SIZE 40

#define ROWS 15
#define COLS 20
/**
 * This is  program where attempt is done so that blinky chases static pac man. (move_blinky2 is the function name) 
 * But, blinky never reaches pac man and keeps going up and down in same positions over and over.
 * 
 * Logic to decide ROWS, COLS, TILE_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT etc is given below:
 * 
 * Each rows and column occupies 40 pixels (TILE SIZE).
 * So, 20 cols will occupy = 20 * 40 = 800 pixels (window height)
 *     15 rows will occupy = 15 * 40 = 600 pixels (window width)
 * 
 * Let's extrapolate how we can map a 2D array cells into pixel (ie x,y coords on the window)
 * 
 * Array of 15 rows and 20 cols. Each cell size is 40*40. 
 * The window itself starts at screen coordinate (100,100).
   However, drawing coordinates inside the window are relative to the window.
   Therefore, the top-left pixel inside the window is always (0,0).
 * 
 * Cell nos -> Pixel coordinate on window
 * 
 * Row 0:
 * (0,0) -> 0, 0 [top left coords of each cell is relative to the window, abs coords are of course (100,100), but in X11 apis, we always use relative ccords wrt the window.]
 * (0,1) -> 40, 0 
 * (0,2) -> 80, 0 
 * .....
 * .....
 * (0,19) -> 760, 0
 * 
 * Row 1:
 * (1,0) -> 0, 40 
 * (1,1) -> 40, 40 
 * (1,2) -> 80, 40 
 * .....
 * ....
 * (1,19) -> 760, 40 
 * 
 * Row 14:
 * (14,0) -> 0, 560
 * (14,1) -> 40, 560
 * (14,2) -> 80, 560
 * ....
 * ....
 * (14,19) -> 760, 560
 */

 /**
  * Rules:
  * 1) Game ends with pac man as winner when pac man avoids colliding with all ghosts, and eats up all the dots.
  * 2) Game ends with pac man as loser when it collides with any of the ghosts.
  * 3) Game will alternate between chase mode and scatter model periodically.
  * 4) In chase mode, the ghosts will chase pacman as per their respective logic; in this mode, ghosts can not move in reverse 
  * 5) In scatter mode, the ghosts will reverse their direction and go back to their home base.
  * 
  * Ghost chasing logic:
  * 
  * Red (Blinky)
  * ============
  * Target: Pac-Man’s current tile.
    So Blinky directly hunts you.

    Pink (Pinky)
    ============
    Target: 4 tiles ahead of Pac-Man.
    So Pinky tries to predict movement, ambush corners,cut you off.

    Blue (Inky)
    ============
    Most complex movement.
    Process:
    - Find point 2 tiles ahead of Pac-Man.
    - Compute vector from Blinky to that point.
    - Double vector.
    - Final location becomes target.

    Orange (Clyde)
    ===============
    Dual behavior:
    Far from Pac-Man: chase normally.
    Near Pac-Man: stop chasing, head toward scatter corner. Scatter corner is the home base of Clyde.
    So Clyde alternates between:
    aggressive, retreating.

    Home positions of ghosts and pac man:

    =====================
         B

     I  P  C
   [ghost area]



      Pac-Man
        M
    =====================

    gcc src/x11_api_case8.c -o bin/x11_api_case8.bin -lX11
  * 
  * 
  * 
  * 
  */
typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
} Direction;

typedef struct {
    int x;
    int y;
    int dx;
    int dy;
    unsigned long color;
    Direction direction;

} Ghost;

int maze[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},//1 indicates wall 0 indicates path
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1},
    {1,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

void draw_maze(Display *display,
               Pixmap buffer,
               GC gc,
               unsigned long wall_color,
               unsigned long dot_color)
{
    for (int row = 0; row < ROWS; row++) {

        for (int col = 0; col < COLS; col++) {

            int x = col * TILE_SIZE;
            int y = row * TILE_SIZE;

            if (maze[row][col] == 1) {

                XSetForeground(display, gc, wall_color);

                /**
                 * int XFillRectangle(
                    Display *display,
                    Drawable d,
                    GC gc,
                    int x,
                    int y,
                    unsigned int width,
                    unsigned int height
                );
                 */
                XFillRectangle(display,
                               buffer,
                               gc,
                               x,
                               y,
                               TILE_SIZE,
                               TILE_SIZE);
            }
            else {

                XSetForeground(display, gc, dot_color);

                /**
                 * int XFillArc(Display *display,
                    Drawable d,
                    GC gc,
                    int x, int y,
                    unsigned int width, unsigned int height,
                    int angle1, int angle2);
                 */
                XFillArc(display,
                         buffer,
                         gc,
                         x + TILE_SIZE/2 - 3,
                         y + TILE_SIZE/2 - 3,
                         6,
                         6,
                         0,
                         360 * 64);
            }
        }
    }
}

void draw_pacman(Display *display,
                 Pixmap buffer,
                 GC gc,
                 int x,
                 int y,
                 int direction)
{
    int start_angle = 0;
    int sweep_angle = 300 * 64;

    if (direction == XK_Right) {
        start_angle = 30 * 64;
    }

    if (direction == XK_Left) {
        start_angle = 210 * 64;
    }

    if (direction == XK_Up) {
        start_angle = 120 * 64;
    }

    if (direction == XK_Down) {
        start_angle = 300 * 64;
    }

    XSetForeground(display, gc, 0xFFFF00);

    XFillArc(display,
             buffer,
             gc,
             x,
             y,
             30,
             30,
             start_angle,
             sweep_angle);
}

void draw_ghost(Display *display,
                Pixmap buffer,
                GC gc,
                Ghost ghost)
{
    int x = ghost.x;
    int y = ghost.y;

    XSetForeground(display, gc, ghost.color);

    /* head */
    XFillArc(display,
             buffer,
             gc,
             x,
             y,
             30,
             30,
             0,
             180 * 64);

    /* body */
    XFillRectangle(display,
                   buffer,
                   gc,
                   x,
                   y + 15,//since head is a semi circle of width, height = 30, the head will occupy 15 pixels from y coordinate. This why 15 is addded to the body rectangle's y coordinate.
                   30,
                   15);

    /* bottom waves */
    /*XFillArc(display, buffer, gc, x,      y + 20, 10, 10, 0, 360 * 64);
    XFillArc(display, buffer, gc, x + 10, y + 20, 10, 10, 0, 360 * 64);
    XFillArc(display, buffer, gc, x + 20, y + 20, 10, 10, 0, 360 * 64);*/

    /* eyes */
    XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));

    XFillArc(display, buffer, gc,
             x + 6, y + 8,
             7, 10,
             0, 360 * 64);

    XFillArc(display, buffer, gc,
             x + 17, y + 8,
             7, 10,
             0, 360 * 64);

    /* pupils */
    XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));

    XFillArc(display, buffer, gc,
             x + 8, y + 11,
             3, 3,
             0, 360 * 64);

    XFillArc(display, buffer, gc,
             x + 19, y + 11,
             3, 3,
             0, 360 * 64);
}

/**
 * Notice how is_wall is invoked by pac_man and ghosts with their current coords + half of their respective width.
 * This is because, current coords actually refer to top left coords for pac_man and ghost. A proper way to ascertian that 
 * the coords are lying on a wall is to check that the center of the pac_man/ghost is on the wall. Thats why we are adding the
 * current coords with half of their respective width.
 */
int is_wall(int x, int y)
{
    int col = x / TILE_SIZE;
    int row = y / TILE_SIZE;
    /**
     * The maze is stored as a grid of tiles, not as individual pixels.
     * x and y are individual coords for a pixel, and to know which grid this ccords fall into,
     * we will need to divide by the cell size , which is TILE_SIZE. and note that we have used int as data type
     * to store divide by TILE_SIZE result.
     * 
     * You can check the accuracy of this by just replaying a sample grid in paper.
     */

    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) {
        return 1;
    }

    return maze[row][col] == 1;
}

void move_ghost(Ghost *ghost)
{
    int new_x = ghost->x + ghost->dx;
    int new_y = ghost->y + ghost->dy;

    if (is_wall(new_x + 15, new_y + 15)) 
    //check if ghost middle coordinates are on the wall. ghost width is 30, so half width is 15 pixels, which is middle of ghost.
    {

        /* reverse direction */
        ghost->dx = -ghost->dx;
        ghost->dy = -ghost->dy;
    }
    else {
        ghost->x = new_x;
        ghost->y = new_y;
    }
}

void move_blinky(Ghost *blinky, int pac_man_x, int pac_man_y) {
    /**
     * Move blinky towards pac man.
     */

     if (blinky->x > pac_man_x && blinky->y > pac_man_y) {
        /**
         * pac-man
         *         blinky
         */
        if (blinky->direction == NONE) {
            //you can move left, as blinky was not moving till this point of time. Will only happen the first time window opens.
            //else you can not move in reverse 
            int temp_x = blinky->x - 2;
            if (!is_wall(temp_x, blinky->y)) {
                blinky->x = temp_x;
                blinky->direction = LEFT;
            }
            else {
                //can not move left, try to move up next.
                int temp_y = blinky->y - 2;
                if (!is_wall(blinky->x, temp_y)) {
                    blinky->y = temp_y;
                    blinky->direction = UP;
                }
                else {
                    //can not even move up. no other opton, but to try to move right.
                    int temp_x = blinky->x + 2;
                    if (!is_wall(temp_x, blinky->y)) {
                        blinky->x = temp_x;
                        blinky->direction = RIGHT;
                    }
                    else {
                        //only option left is to move down now.
                        int temp_y = blinky->y + 2;
                        if (!is_wall(blinky->x, temp_y)) {
                            blinky->y = temp_y;
                            blinky->direction = DOWN;
                        }

                    }

                }

            }
        }
        else if (blinky->direction == LEFT) {
            int temp_x = blinky->x - 2;
            if (!is_wall(temp_x, blinky->y)) {
                blinky->x = temp_x;
                blinky->direction = LEFT;
            }
            else {
                //cant move left, can not go reverse, so, must try to go up first.
                int temp_y = blinky->y - 2;
                if (!is_wall(blinky->x, temp_y)) {
                    blinky->y = temp_y;
                    blinky->direction = UP;
                }
                else {
                    //try to go right now
                    int temp_x = blinky->x + 2;
                    if (!is_wall(temp_x, blinky->y)) {
                        blinky->x = temp_x;
                        blinky->direction = RIGHT;
                    }
                    else {
                        //no other option, but to go down now.
                        int temp_y = blinky->y + 2;
                        if (!is_wall(blinky->x, temp_y)) {
                            blinky->y = temp_y;
                            blinky->direction = DOWN;
                        }

                    }
                }

            }

        }
        else if (blinky->direction == RIGHT) {
            //try to go further right
            int temp_x = blinky->x + 2;
            if (!is_wall(temp_x, blinky->y)) {
                blinky->x = temp_x;
                blinky->direction = RIGHT;
            }
            else {
                //try to go up now.
                int temp_y = blinky->y - 2;
                if (!is_wall(blinky->x, temp_y)) {
                    blinky->y = temp_y;
                    blinky->direction = UP;
                }
                else {
                    //try to go left now.
                    int temp_x = blinky->x - 2;
                    if (!is_wall(temp_x, blinky->y)) {
                        blinky->x = temp_x;
                        blinky->direction = LEFT;
                    }
                    else {
                        //no other option, try to go down.
                        int temp_y = blinky->y + 2;
                        if (!is_wall(blinky->x, temp_y)) {
                            blinky->y = temp_y;
                            blinky->direction = DOWN;
                        }

                    }
                }
            }

        }
         else if (blinky->direction == UP) {
                int temp_y = blinky->y - 2;
                if (!is_wall(blinky->x, temp_y)) {
                    blinky->y = temp_y;
                    blinky->direction = UP;
                }
                else 
                {
                    //cant move up, try to go left
                    int temp_x = blinky->x - 2;
                    if (!is_wall(temp_x, blinky->y)) {
                        blinky->x = temp_x;
                        blinky->direction = LEFT;
                    }
                    else {
                        //try to go right now
                        int temp_x = blinky->x + 2;
                        if (!is_wall(temp_x, blinky->y)) {
                            blinky->x = temp_x;
                            blinky->direction = RIGHT;
                        }
                        else {
                            //no other option, but to go down now.
                            int temp_y = blinky->y + 2;
                            if (!is_wall(blinky->x, temp_y)) {
                                blinky->y = temp_y;
                                blinky->direction = DOWN;
                            }

                        }
                    }

            }

        }
         else if (blinky->direction == DOWN) {
                //try to go further down
                int temp_y = blinky->y + 2;
                if (!is_wall(blinky->x, temp_y)) {
                    blinky->y = temp_y;
                    blinky->direction = DOWN;
                }
                else 
                {
                    //cant move down, try to go left
                    int temp_x = blinky->x - 2;
                    if (!is_wall(temp_x, blinky->y)) {
                        blinky->x = temp_x;
                        blinky->direction = LEFT;
                    }
                    else {
                         //try to go up now
                        int temp_y = blinky->y - 2;
                        if (!is_wall(blinky->x, temp_y)) {
                            blinky->y = temp_y;
                            blinky->direction = UP;
                        }
                        else {
                            //try to go right now, no other way
                            int temp_x = blinky->x + 2;
                            if (!is_wall(temp_x, blinky->y)) {
                                blinky->x = temp_x;
                                blinky->direction = RIGHT;
                            }

                        }
                    }

            }

        }
      
        
     }

    




}

/**
 * Below move_blinky2 is error prone, blinky just keeps moving
 * up and down in same position again and again.
 * 
 * To fix this, you need to keep track of blinky's positions, so that you dont go back
 * to the same position again.
########
#G#   P#
# # ####
#      #
########
 */
void move_blinky2(Ghost *g, int pacman_x, int pacman_y)
{
    int ghost_row = g->y / TILE_SIZE;
    int ghost_col = g->x / TILE_SIZE;
        printf("ghsot row =%d, ghost_col = %d\n", ghost_row, ghost_col);


    int pac_row = pacman_y / TILE_SIZE;
    int pac_col = pacman_x / TILE_SIZE;
    //printf("pacman row =%d, pacman col = %d\n", pac_row, pac_col);


    int dx = 0;
    int dy = 0;

    int row_diff = pac_row - ghost_row;
    int col_diff = pac_col - ghost_col;

    //printf("col_diff =%d, row_diff =%d\n", col_diff, row_diff);

    //Try to move in whichever way you can reduce distance from pac man.
    //First try reducing x distance.
    if (col_diff != 0) {
        dx = (col_diff > 0) ? TILE_SIZE : -TILE_SIZE;
        if (is_wall(g->x + dx + 15, g-> y + 15)) {
            //we  tried to move ghost in x axis but there is wall at that position.
            printf("hit wall in x direction\n");
            //dx = 0; 
            //Now, try reducing y distance.
            dy = (row_diff > 0) ? TILE_SIZE : -TILE_SIZE;
            if (is_wall(g->x + 15, g->y + dy + 15)) {
                printf("hit wall in y direction\n");
                //dy = 0;
                //conventional way to reduce distance has failed, as in both x and y axis, we are met with wall.
                // we have no option but to atempt going in opposite direction to whatever the dx and dy values were
                //that were met with a wall. First try moving y in opposite of dy.
                dy = -dy;
                if (is_wall(g->x + 15, g->y + dy + 15)) {
                    //try moving the x in opposite direction of dx.
                    dx = -dx;
                    if (is_wall(g->x + dx + 15, g-> y + 15)){
                        dx=0; dy=0;
                        printf("you are cursed\n");
                    }
                }

            }
        }
    }
    else {
        //the col diff is 0 between pac man and blinky, that means that x axis are aligned.
        //lets try to align y axis now.
        if (row_diff != 0) {
             dy = (row_diff > 0) ? TILE_SIZE : -TILE_SIZE;
              if (is_wall(g->x + 15, g->y + dy + 15)) {
                printf("hit wall in y direction\n");
                //dy = 0;
                //conventional way to reduce distance has failed, as y axis, we are met with wall.
                // we have no option but to atempt going in opposite direction to whatever the dy value
                //that was met with a wall. First try moving y in opposite of dy.
                dy = -dy;
                if (is_wall(g->x + 15, g->y + dy + 15)) {
                    printf("hit wall in y axis even in opposite direction\n");
                    //although our x axis had aligned, we have no other option, but to try moving the x in opposite direction of dx.
                    dx = TILE_SIZE;
                    if (is_wall(g->x + dx + 15, g-> y + 15)) {
                        dx=-dx;
                        if (is_wall(g->x + dx + 15, g-> y + 15)) { 
                            printf("you are cursed\n");
                        }
                        
                    }
                }

            }

        }
        else {
            printf("blinky has reached pac man!");
        }
    }
   
    

    g->x += dx;
    g->y += dy;
}

void move_blinky3(Ghost *g, int pacman_x, int pacman_y)
{
    int ghost_row = g->y / TILE_SIZE;
    int ghost_col = g->x / TILE_SIZE;
        printf("ghsot row =%d, ghost_col = %d\n", ghost_row, ghost_col);


    int pac_row = pacman_y / TILE_SIZE;
    int pac_col = pacman_x / TILE_SIZE;
    //printf("pacman row =%d, pacman col = %d\n", pac_row, pac_col);


    int dx = 0;
    int dy = 0;

    int row_diff = pac_row - ghost_row;
    int col_diff = pac_col - ghost_col;

    //printf("col_diff =%d, row_diff =%d\n", col_diff, row_diff);

    //Try to move in whichever way you can reduce distance from pac man.
    //First try reducing x distance.
    if (col_diff != 0) {
        dx = (col_diff > 0) ? TILE_SIZE : -TILE_SIZE;
        if (is_wall(g->x + dx + 15, g-> y + 15)) {
            //we  tried to move ghost in x axis but there is wall at that position.
            printf("hit wall in x direction\n");
            //dx = 0; 
            //Now, try reducing y distance.
            dy = (row_diff > 0) ? TILE_SIZE : -TILE_SIZE;
            if (is_wall(g->x + 15, g->y + dy + 15)) {
                printf("hit wall in y direction\n");
                //dy = 0;
                //conventional way to reduce distance has failed, as in both x and y axis, we are met with wall.
                // we have no option but to atempt going in opposite direction to whatever the dx and dy values were
                //that were met with a wall. First try moving y in opposite of dy.
                dy = -dy;
                if (is_wall(g->x + 15, g->y + dy + 15)) {
                    //try moving the x in opposite direction of dx.
                    dx = -dx;
                    if (is_wall(g->x + dx + 15, g-> y + 15)){
                        dx=0; dy=0;
                        printf("you are cursed\n");
                    }
                }

            }
        }
    }
    else {
        //the col diff is 0 between pac man and blinky, that means that x axis are aligned.
        //lets try to align y axis now.
        if (row_diff != 0) {
             dy = (row_diff > 0) ? TILE_SIZE : -TILE_SIZE;
              if (is_wall(g->x + 15, g->y + dy + 15)) {
                printf("hit wall in y direction\n");
                //dy = 0;
                //conventional way to reduce distance has failed, as y axis, we are met with wall.
                // we have no option but to atempt going in opposite direction to whatever the dy value
                //that was met with a wall. First try moving y in opposite of dy.
                dy = -dy;
                if (is_wall(g->x + 15, g->y + dy + 15)) {
                    printf("hit wall in y axis even in opposite direction\n");
                    //although our x axis had aligned, we have no other option, but to try moving the x in opposite direction of dx.
                    dx = TILE_SIZE;
                    if (is_wall(g->x + dx + 15, g-> y + 15)) {
                        dx=-dx;
                        if (is_wall(g->x + dx + 15, g-> y + 15)) { 
                            printf("you are cursed\n");
                        }
                        
                    }
                }

            }

        }
        else {
            printf("blinky has reached pac man!");
        }
    }
   
    

    g->x += dx;
    g->y += dy;
}

int main()
{
    Display *display = XOpenDisplay(NULL);

    if (display == NULL) {
        printf("Cannot open display\n");
        return 1;
    }

    int screen = DefaultScreen(display);

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100,//start_x_position of window within the whole screen
        100,//start_u_position of window within the whole screen
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        2,//border width
        WhitePixel(display, screen),//border color
        BlackPixel(display, screen)//background color
    );

    XSelectInput(display,
                 window,
                 ExposureMask |
                 KeyPressMask);

    XMapWindow(display, window);

    GC gc = XCreateGC(display, window, 0, NULL);

    Pixmap buffer = XCreatePixmap(
        display,
        window,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        DefaultDepth(display, screen)
    );

    int pacman_x = 360;
    int pacman_y = 520;

    int direction = XK_Right;

    // Memory for 4 Ghost structs is allocated on the stack.
    // Each array element has its own address.
    // Since this is an array, the elements are stored in contiguous memory.
    Ghost ghosts[4];

    ghosts[0] = (Ghost){320, 80, 2, 0, 0xFF0000, NONE};//red
    ghosts[1] = (Ghost){40, 120, -2, 0, 0x00FFFF, NONE};//blue
    ghosts[2] = (Ghost){480, 120, 0, 2, 0xFF69B4, NONE};//pink
    ghosts[3] = (Ghost){720, 120, 0, -2, 0xFFA500, NONE};//orange

    while (1) {

        while (XPending(display)) {

            XEvent event;
            XNextEvent(display, &event);

            if (event.type == KeyPress) {

                KeySym key = XLookupKeysym(&event.xkey, 0);

                int new_x = pacman_x;
                int new_y = pacman_y;

                if (key == XK_Left) {
                    new_x -= 5;
                    direction = XK_Left;
                }

                if (key == XK_Right) {
                    new_x += 5;
                    direction = XK_Right;
                }

                if (key == XK_Up) {
                    new_y -= 5;
                    direction = XK_Up;
                }

                if (key == XK_Down) {
                    new_y += 5;
                    direction = XK_Down;
                }

                if (!is_wall(new_x + 15, new_y + 15)) {
                    pacman_x = new_x;
                    pacman_y = new_y;
                }
            }
        }

        XSetForeground(display, gc, BlackPixel(display, screen));//set drawing color to black

        //Fill the window with black color, essentially setting window background to black.
        XFillRectangle(display,
                       buffer,
                       gc,
                       0,
                       0,
                       WINDOW_WIDTH,
                       WINDOW_HEIGHT);

        draw_maze(display,
                  buffer,
                  gc,
                  WhitePixel(display, screen),
                  0xFFFF00);

        draw_pacman(display,
                    buffer,
                    gc,
                    pacman_x,
                    pacman_y,
                    direction);

        for (int i = 0; i < 4; i++) {
            //we are passing address, because if we dont, then a copy will get passed, and any updates done to that copy will not effect this ghost.
            //the each ghosts[i] is created on the stack, so we are passing address from stack to the move_ghost method.
            //notice that move_ghost does not return anything ie returns void, so, only we are passing address to the method, so that
            //updates can be doen directly to the content of the adress.
            if (i == 0){
                move_blinky2(&ghosts[i], pacman_x, pacman_y);
            }
            else {
                    move_ghost(&ghosts[i]);

            }
            draw_ghost(display,
                       buffer,
                       gc,
                       ghosts[i]);
        }

        XCopyArea(display,
                  buffer,
                  window,
                  gc,
                  0,
                  0,
                  WINDOW_WIDTH,
                  WINDOW_HEIGHT,
                  0,
                  0);

        XFlush(display);

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 16000000;

        nanosleep(&ts, NULL);
    }

    return 0;
}