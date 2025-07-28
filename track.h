#ifndef TRACK_H_
#define TRACK_H_

#include <raylib.h>

#include "spline.h"

typedef struct
{
    // Various splines
    Spline path;
    Spline trackLeft;
    Spline trackRight;
    Spline racingLine;

    int nodes;    // Number of nodes in spline
    float *displacement; // Displacement along spline node normal
    int iterations;
    float marker;
    int selectedNode;
} track;

track track_ctor(int n_nodes);
bool track_build(track *this);

#endif /* TRACK_H_ */
