#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "./common/bfs.h"
#include "./common/pacman.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PACMAN_SIZE 30

#define BLINKY_X11_X_COORD 320 //320/40 = 8 = col number in 2D maze array
#define BLINKY_X11_Y_COORD 80 //80/40 = 2 = row number in 2D maze array
#define PINKY_X11_X_COORD 480 //320/40 = 8 = col number in 2D maze array
#define PINKY_X11_Y_COORD 120 //80/40 = 2 = row number in 2D maze array
#define INKY_X11_X_COORD 40 //40/40 = 1 = col number in 2D maze array
#define INKY_X11_Y_COORD 120 //120/40 = 3 = row number in 2D maze array


/**
 * 
 * gcc src/x11_api_case10.c -o bin/x11_api_case10.bin -lX11
 * 
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

typedef struct {
    int x;//x11 x coord
    int y;//x11 y coord
    int dx;
    int dy;
    unsigned long color;
} Ghost;

typedef struct {
    Cell current_cell;
    Cell parent_cell;

} ParentForCurrentCell;

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

int dots[ROWS][COLS];

void init_dots_array(int dots[ROWS][COLS]) {
    for (int i = 0 ; i < ROWS; i++){
        for (int j = 0 ; j < COLS; j++) {
            if (maze[i][j] == 0) {
                dots[i][j] = 1;//dot present
            }
            else if (maze[i][j] == 1) {
                dots[i][j] = -1;//no dot at this coord
            }
        }
    }
}

void init_2d_int_array(int input[][COLS]) {
    for (int i = 0 ; i < ROWS ; i++) {
        for (int j = 0 ; j < COLS ; j++) {
            input[i][j] = -1;      
        }   
    }
}

void draw_maze(Display *display,
               Pixmap buffer,
               GC gc,
               unsigned long wall_color,
               unsigned long dot_color,
               Cell pacman_cell)
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
                if (pacman_cell.x == row && pacman_cell.y == col) {
                    dots[row][col] = -1;
                }
                if (dots[row][col] == 1) {
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
             PACMAN_SIZE,
             PACMAN_SIZE,
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

void move_ghost(Ghost *ghost)
{
    int new_x = ghost->x + ghost->dx;
    int new_y = ghost->y + ghost->dy;

    if (is_wall(maze, new_x + 15, new_y + 15)) 
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

/**
 * Step 1: Find a point 2 tiles ahead of Pac-Man

Suppose Pac-Man is moving right.

. . P → . T
P = Pac-Man
T = point 2 tiles ahead of Pac-Man

If Pac-Man is moving left, up, or down, the target point is computed 2 tiles in that direction. (The original arcade game also has the famous "up" overflow bug, similar to Pinky's.)

Step 2: Draw a vector from Blinky to that point

Example:

B ------------> T

where

B = Blinky
T = point 2 tiles ahead of Pac-Man

Suppose

B = (5,5)

T = (8,7)

The vector is

(8-5, 7-5)

= (3,2)
Step 3: Double that vector

Double it:

(3,2)

↓

(6,4)
Step 4: Add it to Blinky's position

Starting from Blinky:

B = (5,5)

+

(6,4)

=

(11,9)

So Inky's target becomes

(11,9)
 */
Cell get_inky_target_position(int maze[ROWS][COLS], int pacman_x11_x_coord, int pacman_x11_y_coord,int pacman_movement_direction, Cell blinky_current_position) {

    int inky_obtained_position_ahead_of_pacman = 0;
    Cell inky_target_cell = {-1, -1};

    int inky_ahead_of_pacman_by_steps = 1;
    /**
     * cell_ahead_of_pacman will definitely be a valid cell and not a wall, as it is obtained by calling get_cell_X_positions_ahead_of_pacman.
     * get_cell_X_positions_ahead_of_pacman ensures that you dont end up with walls or become out of maze.
     */
    Cell cell_ahead_of_pacman = {-1, -1};
    while (!inky_obtained_position_ahead_of_pacman) {
        cell_ahead_of_pacman = get_cell_X_positions_ahead_of_pacman(maze, pacman_x11_x_coord,  pacman_x11_y_coord, inky_ahead_of_pacman_by_steps--,pacman_movement_direction);
        if (cell_ahead_of_pacman.x != -1 && cell_ahead_of_pacman.y != -1) {
            inky_obtained_position_ahead_of_pacman = 1;
        }
    }

    if (inky_obtained_position_ahead_of_pacman) {
        Cell vector = {cell_ahead_of_pacman.x - blinky_current_position.x, cell_ahead_of_pacman.y - blinky_current_position.y};
        Cell doubled_vector = {vector.x * 2, vector.y * 2};
        //ensure that the doubled_vector is a valid path within the maze.
        inky_target_cell.x = blinky_current_position.x + doubled_vector.x;
        inky_target_cell.y = blinky_current_position.y + doubled_vector.y;
    }
    inky_target_cell = validate_cell(maze, inky_target_cell, ROWS - 1, COLS - 1);
    //printf("get_inky_target_position: exiting with %d, %d\n", inky_target_cell.x, inky_target_cell.y);
    return inky_target_cell;
}

/**
 * Pinky always stays 4 tiles ahead of pac man; its trying to ambush pacman. But, in case pacman is a intersection, 
 * and there are multiple ways out of that interesection, than inky may move to a path which pacman does not choose.
 * So, inky can only try, but can not be 100% sure.
 * 
 * In Pac-Man, Pinky aims for a tile four steps ahead of Pac-Man. Can Pinky cross over Pac-Man to reach that target, 
 * or should it always take an alternate path that avoids passing through Pac-Man?
 * 
 * Here's how it works:

    - Every time Pinky decides on a new path (at intersections), it computes a target tile.
    - That target tile is 4 tiles ahead of Pac-Man.
    - Pinky then uses the shortest path toward that target tile. 
    - Pinky can not teleport itself crossing through pac man to reach ahead.
    - It has to find a path in such a way that it can ambush pacman without crossing over pacman to do so.
    - If you are both in a long, straight hallway with no side turns, Pinky will just walk directly toward you (trying to reach that target tile ahead of you) until you either turn around or crash into each other.
    - Basic idea is to ambush pac man.

 * 
 */
Cell get_pinky_target_position(Ghost* inky, int pacman_x11_x_coord, int pacman_x11_y_coord, int pacman_movement_direction) {

    int pinky_obtained_target_position = 0;
    int pinky_ahead_of_pacman_by_steps = 4;
    Cell pinky_target_cell = {-1, -1};

    while (!pinky_obtained_target_position) {
        pinky_target_cell = get_cell_X_positions_ahead_of_pacman(maze, pacman_x11_x_coord,  pacman_x11_y_coord, pinky_ahead_of_pacman_by_steps--,pacman_movement_direction);
        if (pinky_target_cell.x != -1 && pinky_target_cell.y != -1) {
            pinky_obtained_target_position = 1;
        }
    }
    //printf("get_pinky_target_position: exiting with %d, %d\n", pinky_target_cell.x, pinky_target_cell.y);
    
    return pinky_target_cell;
}

/**
 * Blinky always chases pac man. So blinky should be positioned such a way that it looks like it is chasing pacman.
 */
int set_blinky_position(Ghost* blinky, Cell blinky_new_position) {

    int blinky_reached_new_position = 0;

    if(blinky_new_position.x == -1 && blinky_new_position.y == -1) {
        //blinky has already reached pacman. put back blinky back to its start position
        blinky->x = BLINKY_X11_X_COORD;
        blinky->y = BLINKY_X11_Y_COORD;
        blinky_reached_new_position = -1;
        //printf("reset blinky to home\n");
        return blinky_reached_new_position;
    }

    int desired_blinky_x = blinky_new_position.y * TILE_SIZE;
    int desired_blinky_y = blinky_new_position.x * TILE_SIZE;


    if (blinky->x == desired_blinky_x && blinky->y == desired_blinky_y) {
        blinky->x = desired_blinky_x;
        blinky->y = desired_blinky_y;
        blinky_reached_new_position = 1;
    }
    else {
         if (blinky->x < desired_blinky_x)
        {
            blinky->x += PACMAN_MOVE_BY_PIXELS;
        }
        else  if (blinky->x > desired_blinky_x)
        {
            blinky->x -= PACMAN_MOVE_BY_PIXELS;
        }

        if (blinky->y < desired_blinky_y)
        {
            blinky->y += PACMAN_MOVE_BY_PIXELS;
        }
        else  if (blinky->y > desired_blinky_y)
        {
            blinky->y -= PACMAN_MOVE_BY_PIXELS;
        }
    }
    return blinky_reached_new_position;

}

int set_pinky_position(Ghost* inky, Cell inky_new_position) {

    int inky_reached_new_position = 0;

    if(inky_new_position.x == -1 && inky_new_position.y == -1) {
        //inky has already reached target. put back inky back to its start/home position
        inky->x = PINKY_X11_X_COORD;
        inky->y = PINKY_X11_Y_COORD;
        inky_reached_new_position = -1;
        return inky_reached_new_position;
    }

    int desired_inky_x = inky_new_position.y * TILE_SIZE;
    int desired_inky_y = inky_new_position.x * TILE_SIZE;
    ////printf("pinky_x %d, pinky_y %d\n",inky->x,inky->y);
    ////printf("desired_inky_x %d, desired_inky_y %d\n",desired_inky_x,desired_inky_y);

    if (inky->x == desired_inky_x && inky->y == desired_inky_y) {
        inky->x = desired_inky_x;
        inky->y = desired_inky_y;
        inky_reached_new_position = 1;
    }
    else {
         if (inky->x < desired_inky_x)
        {
            inky->x += PACMAN_MOVE_BY_PIXELS;
        }
        else  if (inky->x > desired_inky_x)
        {
            inky->x -= PACMAN_MOVE_BY_PIXELS;
        }

        if (inky->y < desired_inky_y)
        {
            inky->y += PACMAN_MOVE_BY_PIXELS;
        }
        else  if (inky->y > desired_inky_y)
        {
            inky->y -= PACMAN_MOVE_BY_PIXELS;
        }
    }
    return inky_reached_new_position;
}

int set_inky_position(Ghost* inky, Cell inky_new_position, int inky_current_traversal_path_index) {

    int inky_reached_new_position = 0;

    if(inky_new_position.x == -1 && inky_new_position.y == -1) {
        //inky has not found a valid traversal path at all. inky stays in its home position, ie inky does not move.
        inky->x = INKY_X11_X_COORD;
        inky->y = INKY_X11_Y_COORD;
        inky_reached_new_position = -1;
        return inky_reached_new_position;
    }

    int desired_inky_x = inky_new_position.y * TILE_SIZE;
    int desired_inky_y = inky_new_position.x * TILE_SIZE;
    ////printf("pinky_x %d, pinky_y %d\n",inky->x,inky->y);
    ////printf("desired_inky_x %d, desired_inky_y %d\n",desired_inky_x,desired_inky_y);

    if (inky->x == desired_inky_x && inky->y == desired_inky_y) {
        inky->x = desired_inky_x;
        inky->y = desired_inky_y;
        inky_reached_new_position = 1;
    }
    else {
         if (inky->x < desired_inky_x)
        {
            inky->x += PACMAN_MOVE_BY_PIXELS;
        }
        else  if (inky->x > desired_inky_x)
        {
            inky->x -= PACMAN_MOVE_BY_PIXELS;
        }

        if (inky->y < desired_inky_y)
        {
            inky->y += PACMAN_MOVE_BY_PIXELS;
        }
        else  if (inky->y > desired_inky_y)
        {
            inky->y -= PACMAN_MOVE_BY_PIXELS;
        }
    }
    return inky_reached_new_position;
}

int main()
{
    Display *display = XOpenDisplay(NULL);

    if (display == NULL) {
        //printf("Cannot open display\n");
        return 1;
    }

    int screen = DefaultScreen(display);

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100,//x11 start_x_position of window within the whole screen
        100,//x11 start_y_position of window within the whole screen
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

    init_dots_array(dots);


    int pacman_x = 360;// 9 in 2d array y coordinate
    int pacman_y = 520;// 13 in 2d array x coordinate (13,9) is also not a wall.
    int direction = XK_Right;

    // Memory for 4 Ghost structs is allocated on the stack.
    // Each array element has its own address.
    // Since this is an array, the elements are stored in contiguous memory.
    Ghost ghosts[4];

    ghosts[0] = (Ghost){BLINKY_X11_X_COORD, BLINKY_X11_Y_COORD, 2, 0, 0xFF0000};//red (2,8) is the (x,y) coordinate in the 2d array. At this position the value in maze is 0, ie there is no wall.
    //init traversal path for blinky to chase pac man's current position
    Cell blinky_cell = getXYcoordinatesMaze(ghosts[0].x, ghosts[0].y);

    Cell pacman_cell = getXYcoordinatesMaze(pacman_x, pacman_y);
    Cell old_pacman_cell = pacman_cell;
    
    int consider_path_as_walls[ROWS][COLS];
    init_2d_int_array(consider_path_as_walls);
    Cell* blinky_traversal_path = find_path_from_src_coord_to_destination_coord(blinky_cell.x, blinky_cell.y, 
                                                   pacman_cell.x, pacman_cell.y, 
                                                   maze, consider_path_as_walls);
    int blinky_current_traversal_path_index = 0;
    int current_blinky_traversal_path_index = blinky_current_traversal_path_index;

    ghosts[1] = (Ghost){40, 120, -2, 0, 0x00FFFF};//blue
    Cell inky_cell = getXYcoordinatesMaze(ghosts[1].x, ghosts[1].y);
    int consider_path_as_walls_in_inkys_perspective[ROWS][COLS];
    init_2d_int_array(consider_path_as_walls_in_inkys_perspective);
    //The below is done so that inky does not cross over pacman in order to reach X tiles ahead.
    consider_path_as_walls_in_inkys_perspective[pacman_cell.x][pacman_cell.y] = CONSIDER_OPEN_PATH_BLOCKED;    
    //printf("inky blocked cell %d,%d\n", pacman_cell.x, pacman_cell.y);
    Cell inky_target_cell = get_inky_target_position(maze, pacman_x, pacman_y, direction, getXYcoordinatesMaze(ghosts[0].x, ghosts[0].y));
    //printf("inky target cell %d,%d\n", inky_target_cell.x, inky_target_cell.y);
    Cell* inky_traversal_path = NULL;
    int inky_current_traversal_path_index = 0;
    int current_inky_traversal_path_index = inky_current_traversal_path_index;
    
    if (inky_target_cell.x != -1 && inky_target_cell.y != -1) {
        inky_traversal_path = find_path_from_src_coord_to_destination_coord(inky_cell.x, inky_cell.y, 
                                                   inky_target_cell.x, inky_target_cell.y, 
                                                   maze, consider_path_as_walls_in_inkys_perspective);
    }
   

    ghosts[2] = (Ghost){480, 120, 0, 2, 0xFF69B4};//pink
    Cell pinky_cell = getXYcoordinatesMaze(ghosts[2].x, ghosts[2].y);
    int consider_path_as_walls_in_pinkys_perspective[ROWS][COLS];
    init_2d_int_array(consider_path_as_walls_in_pinkys_perspective);
    //The below is done so that inky does not cross over pacman in order to reach X tiles ahead.
    consider_path_as_walls_in_pinkys_perspective[pacman_cell.x][pacman_cell.y] = CONSIDER_OPEN_PATH_BLOCKED;    
    //printf("inky blobked cell %d,%d\n", pacman_cell.x, pacman_cell.y);
    Cell pinky_target_cell = get_pinky_target_position(&ghosts[2], pacman_x, pacman_y, direction);
    //printf("inky target cell %d,%d\n", pinky_target_cell.x, pinky_target_cell.y);

    Cell* pinky_traversal_path = find_path_from_src_coord_to_destination_coord(pinky_cell.x, pinky_cell.y, 
                                                   pinky_target_cell.x, pinky_target_cell.y, 
                                                   maze, consider_path_as_walls_in_pinkys_perspective);
    int pinky_current_traversal_path_index = 0;
    int current_pinky_traversal_path_index = pinky_current_traversal_path_index;

    ghosts[3] = (Ghost){720, 120, 0, -2, 0xFFA500};//orange

    while (1) {
        while (XPending(display)) {
           
            int pacman_within_path = 0;
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
                 /**
                 *  If you continously press the keys on pacman, the pacman ends up moving smoothly on the screen,
                 *  but the issue happens with blinky that does not move at all , and seems to keep going back and forth 
                 *  in same place, till the key for pacman is released.
                 * 
                 *  This is because the control still comes out of  `while (XPending(display))`
                 *  and goes back out of while at which the blinky position is recomputed (assuming blinky's next position is not hitting a wall.) 
                 *  after the recomputation, we try to navigate from the 0th index of `blinky_traversal_path`, which is nothing but blinky's orginal position.
                 *  So, on the screen, it looks like blinky is oscillating in same position back and forth.
                 */
                    ////printf("Received key press event\n");

                    KeySym key = XLookupKeysym(&event.xkey, 0);

                    int new_x = pacman_x;
                    int new_y = pacman_y;

                    if (key == XK_Left) {
                        new_x -= PACMAN_MOVE_BY_PIXELS;
                        direction = XK_Left;
                        if (!is_wall(maze, new_x, new_y ) //top left of pac man 
                        && 
                        !is_wall(maze, new_x , new_y + PACMAN_SIZE) //bottom left of pac man
                        ) 
                        {
                            pacman_within_path = 1;
                        
                        }
                    }
                    else if (key == XK_Right) {
                        new_x += PACMAN_MOVE_BY_PIXELS;
                        direction = XK_Right;
                        if (!is_wall(maze, new_x + PACMAN_SIZE, new_y ) //top right of pac man 
                        && 
                        !is_wall(maze, new_x + PACMAN_SIZE, new_y + PACMAN_SIZE ) // bottom right of pac man
                        ) 
                        {
                            pacman_within_path = 1;
                        
                        }
                    }
                    else if (key == XK_Up || key == XK_Down) {
                        if (key == XK_Up) {
                             new_y -= PACMAN_MOVE_BY_PIXELS;
                            direction = XK_Up;
                        }
                        else if (key == XK_Down) {
                           new_y += PACMAN_MOVE_BY_PIXELS;
                           direction = XK_Down;
                        }

                        if (!is_wall(maze, new_x, new_y ) //top left of pac man 
                        && 
                        !is_wall(maze, new_x + PACMAN_SIZE, new_y ) // top right of pac man
                        &&
                        !is_wall(maze, new_x , new_y + PACMAN_SIZE) // bottom left of pac man
                        &&
                        !is_wall(maze, new_x + PACMAN_SIZE, new_y + PACMAN_SIZE) // bottom right of pac man
                        ) 
                        {
                            pacman_within_path = 1;  //pacman is not going to hit a wall, it is within the path considering its whole dimension.
                        }
                       
                    }

                if (pacman_within_path) {
                    pacman_x = new_x;
                    pacman_y = new_y;

                    blinky_cell = getXYcoordinatesMaze(ghosts[0].x, ghosts[0].y);
                    pinky_cell = getXYcoordinatesMaze(ghosts[2].x, ghosts[2].y);
                    inky_cell = getXYcoordinatesMaze(ghosts[1].x, ghosts[1].y);
                    pacman_cell = getXYcoordinatesMaze(pacman_x, pacman_y);
                    /**
                     * The below if condition fixes blinky's oscillating behaviour while pacman key is continously pressed down.
                     */
                    if (old_pacman_cell.x != pacman_cell.x ||  old_pacman_cell.y != pacman_cell.y)
                    {
                        //pac man has reached new tile (new position in 2 d array)
                        //re-calculate blinky tarversal path only when pacman reaches a new cell in 2d array.
                        //but before that atleast allow blinky to fully traverse the current tile
                        blinky_traversal_path = find_path_from_src_coord_to_destination_coord(blinky_cell.x, blinky_cell.y, 
                                                   pacman_cell.x, pacman_cell.y, 
                                                   maze, consider_path_as_walls);
                        blinky_current_traversal_path_index = 0;

                         //re-calculate inky traversal path only when pacman reaches a new cell in 2d array.
                        consider_path_as_walls_in_pinkys_perspective[old_pacman_cell.x][old_pacman_cell.y] = -1;   
                        consider_path_as_walls_in_pinkys_perspective[pacman_cell.x][pacman_cell.y] = CONSIDER_OPEN_PATH_BLOCKED;     
                        pinky_target_cell = get_pinky_target_position(&ghosts[2], pacman_x, pacman_y, direction);
                        pinky_traversal_path = find_path_from_src_coord_to_destination_coord(pinky_cell.x, pinky_cell.y, 
                                                   pinky_target_cell.x, pinky_target_cell.y, 
                                                   maze, consider_path_as_walls_in_pinkys_perspective);
                        pinky_current_traversal_path_index = 0;

                        //re-calculate inky's traversal path only when pacman reaches a new cell in 2d array.
                        consider_path_as_walls_in_inkys_perspective[old_pacman_cell.x][old_pacman_cell.y] = -1;   
                        consider_path_as_walls_in_inkys_perspective[pacman_cell.x][pacman_cell.y] = CONSIDER_OPEN_PATH_BLOCKED;     
                        inky_target_cell = get_inky_target_position(maze, pacman_x, pacman_y, direction, blinky_cell);
                        if (inky_target_cell.x != -1 && inky_target_cell.y != -1) {
                            inky_traversal_path = find_path_from_src_coord_to_destination_coord(inky_cell.x, inky_cell.y, 
                                                   inky_target_cell.x, inky_target_cell.y, 
                                                   maze, consider_path_as_walls_in_inkys_perspective);
                            inky_current_traversal_path_index = 0;
                        }
                       

                        old_pacman_cell = pacman_cell;
                    }
                }
            }
        }
        ////printf("Exited keypress event\n");
        /**
         * Even though you keep pressing the key down to move pac man, the control wil still come here
         * after every keystroke, ie continuous pressing of key is counted as individual stroke one after another.
         */
        XSetForeground(display, gc, BlackPixel(display, screen));//set drawing color to black

        //Fill the window with black color, essentially setting window background to black.
        XFillRectangle(display,
                       buffer,
                       gc,
                       0,
                       0,
                       WINDOW_WIDTH,
                       WINDOW_HEIGHT);

        Cell pacman_new_cell = getXYcoordinatesMaze(pacman_x, pacman_y);
        draw_maze(display,
                buffer,
                gc,
                WhitePixel(display, screen),
                0xFFFF00,
                pacman_new_cell);

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
            //updates can be done directly to the content of the adress.
            if (i == 0) {
                if (blinky_current_traversal_path_index >= ROWS*COLS) {
                    blinky_current_traversal_path_index = 0;
                }
                
                int blinky_reached_desired_current_traversal_position = set_blinky_position(&ghosts[i], blinky_traversal_path[blinky_current_traversal_path_index]);
                if (blinky_reached_desired_current_traversal_position == -1) {
                    //even though pacman may be static, recalculate blinky_traversal_path, as blinky has already reached pacman, and we
                    //now need to calculate traversal path from blinky home position to pacman position, as once, blinky reaches pacman, 
                    //we reset blinky back to its home position.
                    blinky_cell = getXYcoordinatesMaze(ghosts[0].x, ghosts[0].y);
                    blinky_traversal_path = find_path_from_src_coord_to_destination_coord(blinky_cell.x, blinky_cell.y, 
                                                   pacman_cell.x, pacman_cell.y, 
                                                   maze, consider_path_as_walls);
                    blinky_current_traversal_path_index = 0;
                }
                else if (blinky_reached_desired_current_traversal_position) {
                    blinky_current_traversal_path_index++;
                    
                }
                draw_ghost(display,
                       buffer,
                       gc,
                       ghosts[i]);

            }
            else  if (i == 2) {
                if (pinky_current_traversal_path_index >= ROWS*COLS) {
                    pinky_current_traversal_path_index = 0;
                }
                //printf("moving pinky to %d,%d\n",pinky_traversal_path[pinky_current_traversal_path_index].x, pinky_traversal_path[pinky_current_traversal_path_index].y);
                int pinky_reached_desired_current_traversal_position = set_pinky_position(&ghosts[i], pinky_traversal_path[pinky_current_traversal_path_index]);
                if (pinky_reached_desired_current_traversal_position == -1) {
                    //even though pacman may be static, recalculate pinky_traversal_path, as inky has already reached pacman, and we
                    //now need to calculate traversal path from inky home position to new target position X steps ahead of pacman position,
                    //This is because once inky reaches pacman, we reset inky back to it's home position (in method set_pinky_position itself).
                    consider_path_as_walls_in_pinkys_perspective[old_pacman_cell.x][old_pacman_cell.y] = -1;   
                    consider_path_as_walls_in_pinkys_perspective[pacman_cell.x][pacman_cell.y] = CONSIDER_OPEN_PATH_BLOCKED;    
                    //printf("pinky reset\n"); 
                    pinky_target_cell = get_pinky_target_position(&ghosts[2], pacman_x, pacman_y, direction);
                    pinky_cell = getXYcoordinatesMaze(ghosts[2].x, ghosts[2].y);
                    //printf("recomputing pinky's traversal path from %d,%d to %d,%d\n",pinky_cell.x, pinky_cell.y, pinky_target_cell.x, pinky_target_cell.y); 

                    pinky_traversal_path = find_path_from_src_coord_to_destination_coord(pinky_cell.x, pinky_cell.y, 
                                                pinky_target_cell.x, pinky_target_cell.y, 
                                                maze, consider_path_as_walls_in_pinkys_perspective);
                    pinky_current_traversal_path_index = 0;
                }
                else if (pinky_reached_desired_current_traversal_position) {
                    pinky_current_traversal_path_index++;
                    
                }
                ////printf("index before=%d\n", pinky_current_traversal_path_index);
                draw_ghost(display,
                       buffer,
                       gc,
                       ghosts[i]);

            }
            else  if (i == 1) {
                int inky_reached_desired_current_traversal_position = -99;
                if (inky_current_traversal_path_index >= ROWS*COLS) {
                    inky_current_traversal_path_index = 0;
                }
                if (inky_traversal_path != NULL) {
                    printf("moving inky to %d,%d\n",inky_traversal_path[inky_current_traversal_path_index].x, inky_traversal_path[inky_current_traversal_path_index].y);
                    inky_reached_desired_current_traversal_position = set_inky_position(&ghosts[i], inky_traversal_path[inky_current_traversal_path_index], inky_current_traversal_path_index);

                }
                if (inky_reached_desired_current_traversal_position == -1) {
                    //even though pacman may be static, recalculate pinky_traversal_path, as inky has already reached pacman, and we
                    //now need to calculate traversal path from inky home position to new target position X steps ahead of pacman position,
                    //This is because once inky reaches pacman, we reset inky back to it's home position (in method set_pinky_position itself).
                    consider_path_as_walls_in_inkys_perspective[old_pacman_cell.x][old_pacman_cell.y] = -1;   
                    consider_path_as_walls_in_inkys_perspective[pacman_cell.x][pacman_cell.y] = CONSIDER_OPEN_PATH_BLOCKED;    
                    printf("inky reset\n"); 
                    inky_target_cell = get_inky_target_position(maze, pacman_x, pacman_y, direction,getXYcoordinatesMaze(ghosts[0].x, ghosts[0].y));
                    if (inky_target_cell.x != -1 && inky_target_cell.y != -1) {
                        inky_cell = getXYcoordinatesMaze(ghosts[1].x, ghosts[1].y);
                        printf("recomputing inky's traversal path from %d,%d to %d,%d\n",inky_cell.x, inky_cell.y, inky_target_cell.x, inky_target_cell.y); 

                        inky_traversal_path = find_path_from_src_coord_to_destination_coord(inky_cell.x, inky_cell.y, 
                                                    inky_target_cell.x, inky_target_cell.y, 
                                                    maze, consider_path_as_walls_in_inkys_perspective);
                        inky_current_traversal_path_index = 0;
                    }
                    
                }
                else if (inky_reached_desired_current_traversal_position) {
                    inky_current_traversal_path_index++;
                    
                }
                ////printf("index before=%d\n", pinky_current_traversal_path_index);
                draw_ghost(display,
                       buffer,
                       gc,
                       ghosts[i]);

            }
            else {
                //move_ghosts(&ghosts[i], pacman_x, pacman_y);
                draw_ghost(display,
                       buffer,
                       gc,
                       ghosts[i]);
            }              
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