#ifndef SPLINE_H_
#define SPLINE_H_

#include <raylib.h>

#include <stdbool.h>

typedef struct
{
    float x;
    float y;
    float length;
} Point2D;
Point2D Point2D_ctor();
Point2D Point2D_new(float x, float y, float len);

typedef struct
{
    Point2D *points;
    float totalSplineLength;
    bool isLooped;
} Spline;
Spline Spline_ctor();
Spline Spline_new(Point2D *points, float totalSplineLength, bool isLooped);
Point2D Spline_GetSplinePoint(Spline *this, float t);
Point2D Spline_GetSplineGradient(Spline *this, float t);
float Spline_CalculateSegmentLength(Spline *this, int node);
float Spline_GetNormalisedOffset(Spline *this, float p);
void Spline_UpdateSplineProperties(Spline *this);
void Spline_DrawSelf(Spline *this, float ox, float oy, Color col);

#endif /* SPLINE_H_ */
