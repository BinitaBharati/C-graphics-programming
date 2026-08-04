#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define TILE_SIZE 40
#define PACMAN_MOVE_BY_PIXELS 5

Cell getXYcoordinatesMaze(int x11_x_coord, int x11_y_coord) {
    /**
     * Maze is a 2d array, this function takes the x11 coordinates as 
     * input and returns the coords within the 2D array supporting the
     * maze.
     * 
     * x11 ccords is for the entore screen, maze coords are the array indices.
     */
     int row_index = x11_y_coord / TILE_SIZE;
    int col_index = x11_x_coord / TILE_SIZE;

    Cell cell = {row_index, col_index};
    return cell;
}

X11_Cell getX11coordinatesMaze(int array_row_index, int array_col_index) {
    /**
     * Maze is a 2d array, this function takes the array coordinates (row and col indices) as input
     * and returns the X11 coords at the input array coords.
     * 
     * x11 ccords is for the entire screen, maze coords are the array indices.
     */
     int x11_x_coord = array_col_index * TILE_SIZE;
    int x11_y_coord = array_row_index * TILE_SIZE;

    X11_Cell cell = {x11_x_coord, x11_y_coord};
    return cell;
}

/**
 * Notice how is_wall is invoked by pac_man and ghosts with their current coords + half of their respective width.
 * This is because, current coords actually refer to top left coords for pac_man and ghost. A proper way to ascertian that 
 * the coords are lying on a wall is to check that the center of the pac_man/ghost is on the wall. Thats why we are adding the
 * current coords with half of their respective width.
 */
int is_wall(int maze[ROWS][COLS] , int x11_x_coord, int x11_y_coord)
{
    int col = (x11_x_coord ) / TILE_SIZE;
    int row = (x11_y_coord ) / TILE_SIZE;
    /**
     * The maze is stored as a grid of tiles, not as individual pixels.
     * x and y are individual coords for a pixel, and to know which grid this coords fall into,
     * we will need to divide by the cell size , which is TILE_SIZE. and note that we have used int as data type
     * to store divide by TILE_SIZE result.
     * 
     * You can check the accuracy of this by just replaying a sample grid in paper.
     */
    ////printf("bini100 row %d, col %d\n",row,col);

    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) {
        ////printf("billi123\n");
        return 1;
    }
    ////printf("billi345 %d\n",maze[row][col]);
    return maze[row][col] == 1;
}

Cell get_cell_X_positions_ahead_of_pacman(int maze[ROWS][COLS], int pacman_x11_x_coord, int pacman_x11_y_coord, int AHEAD_OF_PACMAN_BY_STEP, 
                                        int pacman_movement_direction) {
     //printf("bini100\n");
    //check in which direction can pacman possibly move next ?
    //we need to check which tile is AHEAD_OF_PACMAN_BY_STEP steps ahead of pacman current position.
     int met_with_wall = 0;
     if (pacman_movement_direction == XK_Left) {
        //printf("get_cell_X_positions_ahead_of_pacman pacman moving leftwards\n");
        //check if pacman can move AHEAD_OF_PACMAN_BY_PIXELS tiles to left, if so, set target cell to that cell.
        for (int i = 1 ; i <= AHEAD_OF_PACMAN_BY_STEP ; i++) {
            int current_x11_top_left_x_coord = pacman_x11_x_coord-(TILE_SIZE*i);
            if (is_wall(maze, current_x11_top_left_x_coord, pacman_x11_y_coord ) //top left of the ith left PACMAN_MOVE_BY_PIXELS th pixel from pac man 
                || 
                is_wall(maze, current_x11_top_left_x_coord , pacman_x11_y_coord + TILE_SIZE - 1 ) //bottom left of the ith left PACMAN_MOVE_BY_PIXELS th pixels from pac man 
                ||
                is_wall(maze, current_x11_top_left_x_coord + TILE_SIZE - 1 , pacman_x11_y_coord ) //top right of the ith left PACMAN_MOVE_BY_PIXELS th pixels from pac man
                ||
                is_wall(maze, current_x11_top_left_x_coord + TILE_SIZE - 1 , pacman_x11_y_coord + TILE_SIZE - 1) //bottom right of the ith left PACMAN_MOVE_BY_PIXELS th pixels from pac man

            ) 
            {
                //printf("leftwards met with wall\n");
                met_with_wall = 1;
                break;
            }
        }
        if (!met_with_wall) {
            Cell pacman_cell = getXYcoordinatesMaze(pacman_x11_x_coord, pacman_x11_y_coord);
            //printf("leftwards bini21 %d,%d\n",pacman_cell.x, pacman_cell.y);
            Cell test = getXYcoordinatesMaze(pacman_x11_x_coord - (TILE_SIZE * AHEAD_OF_PACMAN_BY_STEP), pacman_x11_y_coord);
            //printf("leftwards bini22 %d,%d\n",test.x, test.y);
            return getXYcoordinatesMaze(pacman_x11_x_coord - (TILE_SIZE * AHEAD_OF_PACMAN_BY_STEP), pacman_x11_y_coord);//this is the top left corner of the PINKY_AHEAD_OF_PACMAN_BY_STEP th pixel left of pacman.
        }
     }

     if (pacman_movement_direction == XK_Right) {
        //printf("get_cell_X_positions_ahead_of_pacman pacman moving rightwards\n");

        //control is here means, we could not move to the AHEAD_OF_PACMAN_BY_STEP th left cell of pacman due to meeting a wall somewhere along the path.
        met_with_wall = 0;
        //check if pacman can move by AHEAD_OF_PACMAN_BY_PIXELS tiles to right, if so, set target cell to that cell.
        for (int i = 1 ; i <= AHEAD_OF_PACMAN_BY_STEP ; i++) {
            int current_x11_top_left_x_coord = pacman_x11_x_coord+(TILE_SIZE*i);
            if (is_wall(maze, current_x11_top_left_x_coord, pacman_x11_y_coord ) //top left of the ith right PACMAN_MOVE_BY_PIXELS th pixel from pac man  
                || 
                is_wall(maze, current_x11_top_left_x_coord , pacman_x11_y_coord + TILE_SIZE - 1) //bottom left of the ith right PACMAN_MOVE_BY_PIXELS th pixels from pac man 
                ||
                is_wall(maze, current_x11_top_left_x_coord + TILE_SIZE - 1 , pacman_x11_y_coord ) //top right of the ith right PACMAN_MOVE_BY_PIXELS th pixels from pac man
                ||
                is_wall(maze, current_x11_top_left_x_coord + TILE_SIZE - 1 , pacman_x11_y_coord + TILE_SIZE - 1 ) //bottom right of the ith right PACMAN_MOVE_BY_PIXELS th pixels from pac man

            ) 
            {
                //printf("rightwards met with wall\n");
                met_with_wall = 1;
                break;
            }
        }

        if (!met_with_wall) {
            Cell pacman_cell = getXYcoordinatesMaze(pacman_x11_x_coord, pacman_x11_y_coord);
            //printf("rightwards bini21 %d,%d\n",pacman_cell.x, pacman_cell.y);
            Cell test = getXYcoordinatesMaze(pacman_x11_x_coord+(TILE_SIZE * AHEAD_OF_PACMAN_BY_STEP), pacman_x11_y_coord);
            //printf("rightwards bini22 %d,%d\n",test.x, test.y);
            return getXYcoordinatesMaze(pacman_x11_x_coord+(TILE_SIZE * AHEAD_OF_PACMAN_BY_STEP), pacman_x11_y_coord);//this is the top left corner of the PACMAN_MOVE_BY_PIXELS th tile right of pacman.
        }
     }

      if (pacman_movement_direction == XK_Up) {
        //printf("get_cell_X_positions_ahead_of_pacman pacman moving upwards\n");
        //control is here means, we could neither move pacman AHEAD_OF_PACMAN_BY_STEP times PACMAN_MOVE_BY_PIXELS to the left nor AHEAD_OF_PACMAN_BY_STEP times PACMAN_MOVE_BY_PIXELS to the right, 
        //as a wall was encountered in the path in both left and right directions.
        //check if pacman can move by AHEAD_OF_PACMAN_BY_STEP *  PACMAN_MOVE_BY_PIXELS upwards, if so, set target cell to that cell.
        for (int i = 1 ; i <= AHEAD_OF_PACMAN_BY_STEP ; i++) {
            int current_x11_top_left_y_coord = pacman_x11_y_coord-(TILE_SIZE*i);
            if (is_wall(maze, pacman_x11_x_coord, current_x11_top_left_y_coord ) //top left of the ith top PACMAN_MOVE_BY_PIXELS th pixel from pac man  
                || 
                is_wall(maze, pacman_x11_x_coord + TILE_SIZE - 1, current_x11_top_left_y_coord) //top right of the ith top PACMAN_MOVE_BY_PIXELS th pixel from pac man  
                ||
                is_wall(maze, pacman_x11_x_coord  , current_x11_top_left_y_coord + TILE_SIZE - 1 ) //bottom left of the ith top PACMAN_MOVE_BY_PIXELS th pixel from pac man  
                ||
                is_wall(maze, pacman_x11_x_coord + TILE_SIZE - 1 , current_x11_top_left_y_coord + TILE_SIZE - 1 ) //bottom right of the ith top PACMAN_MOVE_BY_PIXELS th pixel from pac man  

            ) 
            {
                //printf("upwards met with wall\n");
                met_with_wall = 1;
                break;
            }
        }

        if (!met_with_wall) {
            Cell pacman_cell = getXYcoordinatesMaze(pacman_x11_x_coord, pacman_x11_y_coord);
            //printf("upwards bini21 %d,%d\n",pacman_cell.x, pacman_cell.y);
            Cell test = getXYcoordinatesMaze(pacman_x11_x_coord, pacman_x11_y_coord-(TILE_SIZE*AHEAD_OF_PACMAN_BY_STEP));
            //printf("upwards bini22 %d,%d\n",test.x, test.y);
            return getXYcoordinatesMaze(pacman_x11_x_coord, pacman_x11_y_coord-(TILE_SIZE*AHEAD_OF_PACMAN_BY_STEP));//this is the top left corner of the AHEAD_OF_PACMAN_BY_STEP times PACMAN_MOVE_BY_PIXELS th pixel top of pacman.
        }
      }

       if (pacman_movement_direction == XK_Down) {
        //printf("get_cell_X_positions_ahead_of_pacman pacman moving downwards\n");
            //control is here means, we could neither move pacman left, right or in upwards direction; only option left is move downwards
            //check if pacman can move by AHEAD_OF_PACMAN_BY_STEP *  PACMAN_MOVE_BY_PIXELS downwards, if so, set target cell to that cell.
            for (int i = 1 ; i <= AHEAD_OF_PACMAN_BY_STEP ; i++) {
                int current_x11_top_left_y_coord = pacman_x11_y_coord+(TILE_SIZE*i);
                if (is_wall(maze, pacman_x11_x_coord, current_x11_top_left_y_coord ) //top left of the ith down PACMAN_MOVE_BY_PIXELS th pixel from pac man  
                    || 
                    is_wall(maze, pacman_x11_x_coord + TILE_SIZE - 1, current_x11_top_left_y_coord) //top right of the ith down PACMAN_MOVE_BY_PIXELS th pixel from pac man  
                    ||
                    is_wall(maze, pacman_x11_x_coord  , current_x11_top_left_y_coord + TILE_SIZE - 1 ) //bottom left of the ith down PACMAN_MOVE_BY_PIXELS th pixel from pac man  
                    ||
                    is_wall(maze, pacman_x11_x_coord + TILE_SIZE - 1 , current_x11_top_left_y_coord + TILE_SIZE - 1 ) //bottom right of the ith down PACMAN_MOVE_BY_PIXELS th pixel from pac man  

                ) 
                {
                    //printf("downwards met with wall\n");
                    met_with_wall = 1;
                    break;
                }
            }

            if (!met_with_wall) {
                Cell pacman_cell = getXYcoordinatesMaze(pacman_x11_x_coord, pacman_x11_y_coord);
                //printf("downwards bini21 %d,%d\n",pacman_cell.x, pacman_cell.y);
                Cell test = getXYcoordinatesMaze(pacman_x11_x_coord, pacman_x11_y_coord+(TILE_SIZE*AHEAD_OF_PACMAN_BY_STEP));
                //printf("downwards bini22 %d,%d\n",test.x, test.y);
                return getXYcoordinatesMaze(pacman_x11_x_coord, pacman_x11_y_coord+(TILE_SIZE*AHEAD_OF_PACMAN_BY_STEP));//this is the top left corner of the AHEAD_OF_PACMAN_BY_STEP times PACMAN_MOVE_BY_PIXELS th pixel down of pacman.
            }
       }
     //if here means, we could not find a cell in which pinky could move in order to ambush pacman.
     Cell return_cell = {-1, -1};
     return return_cell;
}

Cell validate_cell(int maze[ROWS][COLS], Cell input_cell, int max_row_index, int max_col_index) {
    int maze_x_coord = input_cell.x;
    int maze_y_coord = input_cell.y;
    if (input_cell.x > max_row_index) {
        maze_x_coord = max_row_index - maze_x_coord;
    }
    if (input_cell.y > max_col_index) {
        maze_y_coord = max_col_index - maze_x_coord;
    }

    X11_Cell x11_cell = getX11coordinatesMaze(maze_x_coord, maze_y_coord);
    if (is_wall(maze, x11_cell.x, x11_cell.y)) {
        //Try moving one tile in left, right, up and down positions one by one.
        //In any one of these positions, you should be able to find a open pathway.
        maze_x_coord = maze_x_coord - 1;//move one tile left
        x11_cell = getX11coordinatesMaze(maze_x_coord, maze_y_coord);
        if (is_wall(maze, x11_cell.x, x11_cell.y)) {
            maze_x_coord = maze_x_coord + 2;//move one tile right
            x11_cell = getX11coordinatesMaze(maze_x_coord, maze_y_coord);
            if (is_wall(maze, x11_cell.x, x11_cell.y)) {
                maze_x_coord = maze_x_coord - 2;//move one tile up
                maze_y_coord = maze_y_coord + 1;//move one tile up
                x11_cell = getX11coordinatesMaze(maze_x_coord, maze_y_coord);
                 if (is_wall(maze, x11_cell.x, x11_cell.y)) {
                    maze_y_coord = maze_y_coord - 2;//move one tile down
                    x11_cell = getX11coordinatesMaze(maze_x_coord, maze_y_coord);
                    if (is_wall(maze, x11_cell.x, x11_cell.y)) {
                        Cell invalid_cell = {-1, -1};
                        return invalid_cell;
                    }
                 }
            }
        }
        Cell valid_cell = {maze_x_coord, maze_y_coord};
        return valid_cell;
    }


 }
