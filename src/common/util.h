#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef struct {
    double x;
    double y;
    short colliding_with_eater;
} ArcPos;

int overlaps(ArcPos a, ArcPos b, int arc_width, int arc_height)
{
    return !(a.x + arc_width  <= b.x ||
             b.x + arc_width  <= a.x ||
             a.y + arc_height <= b.y ||
             b.y + arc_height <= a.y);
}

ArcPos* getArcPositions(int window_width, int window_height, int arc_width, int arc_height, int ARC_COUNT) {

    ArcPos* arcs = malloc(sizeof(ArcPos) * ARC_COUNT);

    /**
     *  time(NULL)
        Gets the current time (number of seconds since Jan 1, 1970).
        srand(...)
        Uses that value as the seed for the random number generator used by rand().

        Function signature: srand(unsigned int seed)

        Without this, rand() produces the same sequence every time the program runs.
     */
    srand(time(NULL));

    int count = 0;

    while (count < ARC_COUNT) {

        ArcPos candidate;

        candidate.x = rand() % (window_width  - arc_width);
        candidate.y = rand() % (window_height - arc_height);
        candidate.colliding_with_eater = 0;//false

        int collision = 0;

        for (int i = 0; i < count; i++) {

            if (overlaps(candidate, arcs[i], arc_width, arc_height)) {
                collision = 1;
                break;
            }
        }

        if (!collision) {
            arcs[count] = candidate;
            count++;
        }
    }
    return arcs;
}

