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

#define MAX_PREV_DIRECTIONS 4
/**
 * This is  extended version of previous numbered program where movemnt of blinky is erroneous.
 * We need to fix blinky's movement
 * .
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
    NONE,
    CHANGE_AXIS
} Direction;

typedef struct {
    int x;
    int y;
    int prev_x;
    int prev_y;
    int dx;
    int dy;
    unsigned long color;
    Direction current_direction;
    Direction prev_direction;
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

private Direction get_direction_to_reduce_col_diff(Ghost* gc, int col_diff) {
    if (col_diff != 0) {
        if (col_diff > 0) {
            //pac man is ahead of blinky in x axis.
            return RIGHT;

        }
        else if (col_diff < 0){
            //pacman is above blinky in y axis
            return LEFT;
        }
    }
    else {
        //cols are aligned.
        return CHANGE_AXIS;

    }
}

private Direction get_direction_opposite_of_reducing_col_diff(Ghost* gc, int col_diff) {
    if (col_diff != 0) {
        if (col_diff > 0) {
            //pac man is ahead of blinky in x axis.
            return LEFT;

        }
        else if (col_diff < 0){
            //pacman is above blinky in y axis
            return RIGHT;
        }
    }
     else {
        //cols are aligned.
        return CHANGE_AXIS;

    }
}

private Direction get_direction_to_reduce_row_diff(Ghost* gc, int row_diff) {
    if (row_diff != 0) {
        if (row_diff > 0) {
            //pacman is below blinky in y axis
            return DOWN;

        }
        else if (row_diff < 0){
            //pacman is above blinky in y axis
            return UP;
        }
    }
     else {
        //rows are aligned.
        return CHANGE_AXIS;

    }
}

private Direction get_direction_opposite_of_reducing_row_diff(Ghost* gc, int row_diff) {
    if (row_diff != 0) {
        if (row_diff > 0) {
            //pacman is below blinky in y axis
            return UP;

        }
        else if (row_diff < 0){
            //pacman is above blinky in y axis
            gc->attempted_directions[gc->cur_attempted_direction_index] = DOWN:
            gc->cur_attempted_direction_index++;
            return DOWN;
        }
    }
     else {
        //rows are aligned.
        return CHANGE_AXIS;

    }
}

private int move_blinky_in_direction(Ghost* gc, Direction direction) {
    int movement_done = 0;
    if (direction == RIGHT) {
        dx = TILE_SIZE; 
        dy = 0;
        if (!is_wall(g->x + dx + 15, g->y + dy + 15)) {
            g->x = g->x + dx;
            g->y = g->y + dy;
            g->prev_direction = g->current_direction;
            g->current_direction = RIGHT;
            movement_done = 1;
        }
    }
    else if (direction == LEFT) {
        dx = -TILE_SIZE; 
        dy = 0;
        if (!is_wall(g->x + dx + 15, g->y + dy + 15)) {
            g->x = g->x + dx;
            g->y = g->y + dy;
            g->prev_direction = g->current_direction;
            g->current_direction = LEFT;
            movement_done = 1;
        }
    }
    else if (direction == UP) {
        dx = 0; 
        dy = -TILE_SIZE;
        if (!is_wall(g->x + dx + 15, g->y + dy + 15)) {
            g->x = g->x + dx;
            g->y = g->y + dy;
            g->prev_direction = g->current_direction;
            g->current_direction = UP;
            movement_done = 1;
        }
    }
    else if (direction == DOWN) {
        dx = 0; 
        dy = TILE_SIZE;
        if (!is_wall(g->x + dx + 15, g->y + dy + 15)) {
            g->x = g->x + dx;
            g->y = g->y + dy;
            g->prev_direction = g->current_direction;
            g->current_direction = DOWN;
            movement_done = 1;
        }
    }
    return movement_done;
}

private int wall_exists_in_direction(Ghost* gc, Direction direction) {
    int wall_exists = 0;
    if (direction == RIGHT) {
        dx = TILE_SIZE; 
        dy = 0;
        if (is_wall(g->x + dx + 15, g->y + dy + 15)) {
            wall_exists = 1;
        }
    }
    else if (direction == LEFT) {
        dx = -TILE_SIZE; 
        dy = 0;
        if (is_wall(g->x + dx + 15, g->y + dy + 15)) {
            wall_exists = 1;
        }
    }
    else if (direction == UP) {
        dx = 0; 
        dy = -TILE_SIZE;
        if (is_wall(g->x + dx + 15, g->y + dy + 15)) {
            wall_exists = 1;
        }
    }
    else if (direction == DOWN) {
        dx = 0; 
        dy = TILE_SIZE;
        if (is_wall(g->x + dx + 15, g->y + dy + 15)) {
            wall_exists = 1;
        }
    }
    return wall_exists;
}

void init_blinky_movement(Ghost *g, int row_diff, int col_diff ) {
    if (g->current_direction == NONE) {
        int dx = 0;
        int dy = 0;
        //no previous direction recorded, we must decide the direction in which blinky should move.
        Direction next_direction = get_direction_to_reduce_col_diff(g, col_diff);
        int movement_done = move_blinky_in_direction(g, next_direction);
        if (!movement_done) {
            next_direction = get_direction_to_reduce_row_diff(g, row_diff);
            movement_done = move_blinky_in_direction(g, next_direction);

            if (!movement_done) {
                //in prescribed manner, we could not reduce col/row diff.
                //we need to now go in opposite directions of reducing col/row diff.
                next_direction = get_direction_opposite_of_reducing_col_diff(g, col_diff);
                movement_done = move_blinky_in_direction(g, next_direction);
                if (!movement_done) {
                    next_direction = get_direction_opposite_of_reducing_row_diff(g, row_diff);
                    movement_done = move_blinky_in_direction(g, next_direction);
                    if (!movement_done) {
                        printf("Your maze is goofed up, it is cursed, no way to move around here!!\n");
                        exit(0);
                    }
                }
            }
        }
    }
}



Direction get_next_direction_for_current_direction(Ghost* g, int row_diff, int col_diff) {
    Direction next_direction = NONE;
   if (g->current_direction == RIGHT || g->current_direction == LEFT) {
        //we are stuck traversing in x axis. try to move in y axis with dignity first.
        next_direction = get_direction_to_reduce_col_diff(g, col_diff);
        //check if wall exists in this next_direction
        int wall_exists = wall_exists_in_direction(g, next_direction);
        if (wall_exists) {
            //reverse direction in same axis.
            next_direction = get_direction_opposite_of_reducing_col_diff(g, col_diff);
            wall_exists = wall_exists_in_direction(g, next_direction);
            //we are out of all options. we will now try only way out ie LEFT (opp of current direction)
            if (wall_exists) {
                if (g->current_direction == RIGHT)
                    next_direction = LEFT;
                else if (g->current_direction == LEFT)
                    next_direction = RIGHT;
            }
        }
    }
    if (g->current_direction == UP || g->current_direction == DOWN) {
        //we are stuck traversing in x axis. try to move in y axis with dignity first.
        next_direction = get_direction_to_reduce_col_diff(g, col_diff);
        //check if wall exists in this next_direction
        int wall_exists = wall_exists_in_direction(g, next_direction);
        if (wall_exists) {
            //reverse direction in same axis.
            next_direction = get_direction_opposite_of_reducing_col_diff(g, col_diff);
            wall_exists = wall_exists_in_direction(g, next_direction);
            //we are out of all options. we will now try only way out ie LEFT (opp of current direction)
            if (wall_exists) {
                if (g->current_direction == RIGHT)
                    next_direction = LEFT;
                else if (g->current_direction == LEFT)
                    next_direction = RIGHT;
            }
        }
    }
    return next_direction;
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

    if (row_diff == 0 && col_diff == 0) {
        printf("Blinky has already collided with pacman. Game over!!\n")
        exit(1);
    }
    
    int movement_done = 0;
    if (g->current_direction == NONE) {
        init_blinky_movement();
    }
    else {
        //try to move blinky as per its current direction.
        //if it fails, try to attempt other 3 directions.
        //one of the attemps will definitely pass, else
        //your maze is incorrect, as there is no way away from wall
        //even after trying out all 4 directions.
        Direction next_direction = g->current_direction;
        while (!movement_done) {
            movement_done = move_blinky_in_direction(gc, next_direction);
            next_direction = get_next_direction_for_current_direction(gc, row_diff, col_diff);
        }
        
    }
    else if (g->current_direction == DOWN || g->current_direction == UP) {
        //try to move blinky as per its current direction.
        //if it fails, try to attempt other 3 directions.
        //one of the attemps will definitely pass, else
        //your maze is incorrect, as there is no way away from wall
        //even after trying out all 4 directions.
        movement_done = move_blinky_in_direction(gc, g->current_direction);
        if (!movement_done) {
            //movement could not be done in y axis. try reducing distance in x axis then.
            Direction col_diff_reducing_direction = get_direction_to_reduce_col_diff(gc, col_diff);
            movement_done = move_blinky_in_direction(gc, col_diff_reducing_direction);
            if (!movement_done) {
                //now , all genuine ways to reduce row and col diff are exhausted.
                //we will try moving in other 2 possible directions that is opposite of reducing row and col diff.
                Direction col_diff_reducing_opposite_direction = get_direction_opposite_of_reducing_col_diff(gc, col_diff);
                movement_done = move_blinky_in_direction(gc, col_diff_reducing_opposite_direction);
                if (!movement_done) {
                    Direction row_diff_reducing_opposite_direction = get_direction_opposite_of_reducing_row_diff(gc, row_diff);
                    movement_done = move_blinky_in_direction(gc, row_diff_reducing_opposite_direction);
                    if (!movement_done) {
                        printf("You are unlucky, Aborting!\n");
                        exit(1);
                    }
                }
            }
        }
    }    
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

    /**
     *    int x;
    int y;
    int prev_x;
    int prev_y;
    int dx;
    int dy;
    unsigned long color;
    Direction direction;
     */
    ghosts[0] = (Ghost){320, 80, -1, -1, 2, 0, 0xFF0000, NONE, NONE};//red
    ghosts[1] = (Ghost){40, 120, -1, -1, -2, 0, 0x00FFFF, NONE, NONE};//blue
    ghosts[2] = (Ghost){480, 120, -1, -1, 0, 2, 0xFF69B4, NONE, NONE};//pink
    ghosts[3] = (Ghost){720, 120, -1, -1, 0, -2, 0xFFA500, NONE, NONE};//orange

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
            if (i == 0)
                move_blinky3(&ghosts[i], pacman_x, pacman_y);
            else
                move_ghosts(&ghosts[i], pacman_x, pacman_y);
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