#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define ROWS 15
#define COLS 20
#define CONSIDER_OPEN_PATH_BLOCKED 2

typedef struct {
    int x;//current x index in maze. NOT x11 x ccordinate
    int y;//current y index in maze. NOT x11 y ccordinate
} Cell;

typedef struct {
    int x;//current x11 x coord in maze. 
    int y;//current x11 y coord in maze. 
} X11_Cell;

void init_1d_array(Cell* input) {
    for (int i = 0 ; i < ROWS*COLS ; i++) {
        input[i] = (Cell){-1,-1};      
    }
}

//you dont need to specify ROWS dimension, as input itself is a pointer to the first row. And knowledge of COLS helps compiler to undretand statements like input[1][2]
//you can also write this input arg as: `Cell (*input)[COLS]`

void init_2d_array(Cell input[][COLS]) {
    for (int i = 0 ; i < ROWS ; i++) {
        for (int j = 0 ; j < COLS ; j++) {
            input[i][j] = (Cell){-1,-1};      
        }   
    }
}

int is_cell_visited(Cell* visited_cells, Cell check_cell) {
    int cell_visited = 0;
    int visted_cell_index = 0;
    while (visted_cell_index < ROWS*COLS) {
        Cell temp = visited_cells[visted_cell_index++];
        if (temp.x == check_cell.x && temp.y == check_cell.y) {
                cell_visited = 1;
                break;

        }
    }
    return cell_visited;
}

int is_open_path_at_coord_to_be_considered_blocked(int open_x_coord, int open_y_coord, int consider_path_as_walls[ROWS][COLS]) {
    if (consider_path_as_walls[open_x_coord][open_y_coord] == CONSIDER_OPEN_PATH_BLOCKED) {
        return 1;
    }
    return 0;
}

Cell* print_traversal_path(Cell parent_info[ROWS][COLS],
                        int src_x_coord, int src_y_coord, 
                        int destn_x_coord, int destn_y_coord) {

   Cell* actual_traversal_path = malloc(ROWS * COLS * sizeof(Cell));
    if (actual_traversal_path == NULL) {
        return NULL;
    }
    init_1d_array(actual_traversal_path);

    Cell parent_cell = parent_info[destn_x_coord][destn_y_coord];
    if (parent_cell.x == -1 && parent_cell.y == -1) {
        printf("Oopsy, path to destination %d,%d not found :-(\n",destn_x_coord,destn_y_coord);
        return actual_traversal_path;
    }
    Cell reverse_traversal_path[ROWS*COLS];
    init_1d_array(reverse_traversal_path);
    int reverse_traversal_path_index = 0;
    reverse_traversal_path[reverse_traversal_path_index++] = (Cell){destn_x_coord, destn_y_coord};

    while(1) {
        if (parent_cell.x == -1 && parent_cell.y == -1) {
            //printf("Oopsy, full traversal path not available to reach source %d,%d\n.",src_x_coord,src_y_coord);
            break;
        }
        if (parent_cell.x == src_x_coord && parent_cell.y == src_y_coord) {
            reverse_traversal_path[reverse_traversal_path_index++] = parent_cell;
            //printf("Great!! We have got full traversal path from %d,%d to %d,%d\n.",src_x_coord,src_y_coord,destn_x_coord,destn_y_coord);
            break;
        }
        reverse_traversal_path[reverse_traversal_path_index++] = parent_cell;
        parent_cell = parent_info[parent_cell.x][parent_cell.y];
    }

  
    int actual_traversal_path_index = 0;
    printf("Final path to reach from %d,%d to %d,%d\n",src_x_coord,src_y_coord,destn_x_coord,destn_y_coord);
    for (int i = reverse_traversal_path_index-1 ; i >= 0 ; i--) {
        actual_traversal_path[actual_traversal_path_index++] = reverse_traversal_path[i];
    }

    for (int i = 0 ; i < reverse_traversal_path_index ; i++){
        printf("%d,%d\n",actual_traversal_path[i].x,actual_traversal_path[i].y);
    }
    return actual_traversal_path;

}

Cell* find_path_from_src_coord_to_destination_coord(int src_x_coord, int src_y_coord, 
                                                   int destn_x_coord, int destn_y_coord, 
                                                   int maze[ROWS][COLS], int consider_path_as_walls[ROWS][COLS]) {
    printf("BFS here1\n");
    //start BFS from source coords till you reach destination coords.
    Cell queue[ROWS*COLS];
    int queue_head_index = 0, queue_tail_index = 0;
    init_1d_array(queue);

    Cell visited_cells[ROWS*COLS] ;
    init_1d_array(visited_cells);
    int visted_cells_index = 0;

    Cell parent_info[ROWS][COLS];
    init_2d_array(parent_info);

    //put source in tail==head of empty queue
    queue[queue_tail_index++] = (Cell){src_x_coord, src_y_coord};

    while (queue_tail_index != queue_head_index) {
        //queue is not empty.
        //pop queue from head end.
        //printf("bini1: queue_head_index %d \n",queue_head_index);

        Cell current_cell = queue[queue_head_index++];
        //printf("handling x,y %d,%d\n",current_cell.x, current_cell.y);
        if (current_cell.x == destn_x_coord && current_cell.y == destn_y_coord) 
        {
            //the traversal from source coords to destination coords is completed.
            //the traversed path would already be saved in parent_info by now.
            //printf("yay!!\n");
            break;
        }

         //put into visited_cells if not already visited
        if (!is_cell_visited(visited_cells, current_cell)) {
            visited_cells[visted_cells_index++] = current_cell;
            //printf("put x,y %d,%d in visited\n",current_cell.x, current_cell.y);

            //find neighbours of current_cell. In this case neighbours are all cells in UP, DOWN, LEFT and RIGHT direction that are not a wall cell.
            //in maze you cant move diagonally.
            //wall cells contain the value 1.       
            int current_cell_x = current_cell.x;
            int current_cell_y = current_cell.y;
            //check LEFT neighbour
            if ( current_cell_x-1 >=0 ) {
                //valid x coordinate, but may not be a pathway, ie a wall could be at this coordinate.
                if (maze[current_cell_x-1][current_cell_y] != 1 && !is_open_path_at_coord_to_be_considered_blocked(current_cell_x-1, current_cell_y, consider_path_as_walls))//valid path, not a wall
                {
                    //also
                    Cell neighbour_cell = {current_cell_x-1, current_cell_y};
                    if (!is_cell_visited(visited_cells, neighbour_cell)) {
                        //this checks ensures that there is only a single path to reach a node, 
                        //when in the maze multiple path may exist to the same node.
                        //ie parent[node x coord][node y coord] is updated only once.
                        //even though in there may be multiple open next pathways to reach node[node x coord][node y coord]
                        //put to queue
                        //printf("put %d,%d into queue\n",current_cell_x-1, current_cell_y);
                        queue[queue_tail_index++] = neighbour_cell;
                        //save the path between current cell and neighbour cell
                        parent_info[current_cell_x-1][current_cell_y] = current_cell;
                    }
                }
            }
            //check RIGHT neighbour
            if ( current_cell_x+1 < ROWS ) {
                //valid y coordinate, but may not be a pathway, ie a wall could be at this coordinate.
                if (maze[current_cell_x+1][current_cell_y] != 1 && !is_open_path_at_coord_to_be_considered_blocked(current_cell_x+1, current_cell_y, consider_path_as_walls))//valid path
                {
                    Cell neighbour_cell = {current_cell_x+1, current_cell_y};
                    if (!is_cell_visited(visited_cells, neighbour_cell)) {
                        //printf("put %d,%d into queue\n",current_cell_x+1, current_cell_y);
                        //put to queue
                        queue[queue_tail_index++] = neighbour_cell;
                        //save the path between current cell and neighbour cell
                        parent_info[current_cell_x+1][current_cell_y] = current_cell;
                    }            
                }
            }
            //check UP neighbour
            if ( current_cell_y-1 >= 0 ) {
                //valid y coordinate, but may not be a pathway, ie a wall could be at this coordinate.
                if (maze[current_cell_x][current_cell_y-1] != 1 && !is_open_path_at_coord_to_be_considered_blocked(current_cell_x, current_cell_y-1, consider_path_as_walls))//valid path
                {
                    Cell neighbour_cell = {current_cell_x, current_cell_y-1};
                    if (!is_cell_visited(visited_cells, neighbour_cell)) {
                        //printf("put %d,%d into queue\n",current_cell_x, current_cell_y-1);
                        //put to queue
                        queue[queue_tail_index++] = neighbour_cell;
                        //save the path between current cell and neighbour cell
                        parent_info[current_cell_x][current_cell_y-1] = current_cell;
                    }
                }
            }
            //check DOWN neighbour
            if ( current_cell_y+1 < COLS ) {
                //valid y coordinate, but may not be a pathway, ie a wall could be at this coordinate.
                if (maze[current_cell_x][current_cell_y+1] != 1 && !is_open_path_at_coord_to_be_considered_blocked(current_cell_x, current_cell_y+1, consider_path_as_walls))//valid path
                {
                    Cell neighbour_cell = {current_cell_x, current_cell_y+1};  
                    if (!is_cell_visited(visited_cells, neighbour_cell)) {
                        //printf("put %d,%d into queue\n",current_cell_x, current_cell_y+1);
                        //put to queue
                        queue[queue_tail_index++] = neighbour_cell;
                        //save the path between current cell and neighbour cell
                        parent_info[current_cell_x][current_cell_y+1] = current_cell;
                    }
                   
                }
            }

        }

    }
    printf("ready to print trevaresl path\n");
    //print traversal path.
    return print_traversal_path(parent_info, src_x_coord, src_y_coord, destn_x_coord,  destn_y_coord);
 }
