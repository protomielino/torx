#include "stb_ds.h"

#include <math.h>

#include "maths.h"
#include "spline.h"

Point2D Point2D_ctor()
{
    return (Point2D){{0, 0}, 0};
}

Point2D Point2D_new(float x, float y, float len)
{
    return (Point2D){{x, y}, len};
}

Spline Spline_ctor()
{
    return (Spline){NULL, 0, true};
}

Spline Spline_new(Point2D *points, float totalSplineLength, bool isLooped)
{
    return (Spline){points, totalSplineLength, isLooped};
}

Vector2 Spline_GetSplinePoint(Spline *this, float t)
{
    int p0, p1, p2, p3;
    if (!this->isLooped) {
        p1 = (int)t + 1;
        p2 = p1 + 1;
        p3 = p2 + 1;
        p0 = p1 - 1;
    } else {
        p1 = ((int)t) % arrlen(this->points);
        p2 = (p1 + 1) % arrlen(this->points);
        p3 = (p2 + 1) % arrlen(this->points);
        p0 = p1 >= 1 ? p1 - 1 : arrlen(this->points) - 1;
    }

    t = t - (int)t;

    float tt = t * t;
    float ttt = tt * t;

    float q1 = -ttt + 2.0f*tt - t;
    float q2 = 3.0f*ttt - 5.0f*tt + 2.0f;
    float q3 = -3.0f*ttt + 4.0f*tt + t;
    float q4 = ttt - tt;

    float tx = 0.5f *
            (this->points[p0].pos.x * q1 +
             this->points[p1].pos.x * q2 +
             this->points[p2].pos.x * q3 +
             this->points[p3].pos.x * q4);
    float ty = 0.5f *
            (this->points[p0].pos.y * q1 +
             this->points[p1].pos.y * q2 +
             this->points[p2].pos.y * q3 +
             this->points[p3].pos.y * q4);

    return (Vector2){ tx, ty };
}

Vector2 Spline_GetSplineGradient(Spline *this, float t)
{
    int p0, p1, p2, p3;
    if (!this->isLooped) {
        p1 = (int)t + 1;
        p2 = p1 + 1;
        p3 = p2 + 1;
        p0 = p1 - 1;
    } else {
        p1 = ((int)t) % arrlen(this->points);
        p2 = (p1 + 1) % arrlen(this->points);
        p3 = (p2 + 1) % arrlen(this->points);
        p0 = p1 >= 1 ? p1 - 1 : arrlen(this->points) - 1;
    }

    t = t - (int)t;

    float tt = t * t;

    float q1 = -3.0f * tt + 4.0f*t - 1.0f;
    float q2 = 9.0f*tt - 10.0f*t;
    float q3 = -9.0f*tt + 8.0f*t + 1.0f;
    float q4 = 3.0f*tt - 2.0f*t;

    float tx = 0.5f *
            (this->points[p0].pos.x * q1 +
             this->points[p1].pos.x * q2 +
             this->points[p2].pos.x * q3 +
             this->points[p3].pos.x * q4);
    float ty = 0.5f *
            (this->points[p0].pos.y * q1 +
             this->points[p1].pos.y * q2 +
             this->points[p2].pos.y * q3 +
             this->points[p3].pos.y * q4);

    return (Vector2){ tx, ty };
}

float Spline_CalculateSegmentLength(Spline *this, int node)
{
    float length = 0.0f;
    float stepSize = 0.1;

    Vector2 old_point, new_point;
    old_point = Spline_GetSplinePoint(this, (float)node);

    for (float t = 0; t < 1.0f; t += stepSize) {
        new_point = Spline_GetSplinePoint(this, (float)node + t);
        length += sqrtf((new_point.x - old_point.x)*(new_point.x - old_point.x) +
                        (new_point.y - old_point.y)*(new_point.y - old_point.y));
        old_point = new_point;
    }

    return length;
}


float Spline_GetNormalisedOffset(Spline *this, float p)
{
    // Which node is the base?
    int i = 0;
    while (p > this->points[i].length) {
        p -= this->points[i].length;
        i++;
    }

    // The fractional is the offset
    return (float)i + (p / this->points[i].length);
}


void Spline_UpdateSplineProperties(Spline *this)
{
    // Use to cache local spline lengths and overall spline length
    this->totalSplineLength = 0.0f;

    if (this->isLooped) {
        // Each node has a succeeding length
        for (int i = 0; i < arrlen(this->points); i++) {
            this->points[i].length = Spline_CalculateSegmentLength(this, i);
            this->totalSplineLength += this->points[i].length;
        }
    } else {
        for (int i = 1; i < arrlen(this->points) - 2; i++) {
            this->points[i].length = Spline_CalculateSegmentLength(this, i);
            this->totalSplineLength += this->points[i].length;
        }
    }
}

void Spline_DrawSelf(Spline *this, float ox, float oy, Color col)
{
    if (this->isLooped) {
        for (float t = 0; t < (float)arrlen(this->points) - 0; t += 0.1f) {
            Vector2 pos = Spline_GetSplinePoint(this, t);

            Vector2 pixel_s;
            pixel_s = WorldToScreen(pos);
            DrawPixel(pixel_s.x, pixel_s.y, col);

            DrawPixel(pos.x, pos.y, col);
        }
    } else { // Not Looped
        for (float t = 0; t < (float)arrlen(this->points) - 3; t += 0.01f) {
            Vector2 pos = Spline_GetSplinePoint(this, t);

            // TODO: implement world to scale pert

            DrawPixel(pos.x, pos.y, col);
        }
    }
}
