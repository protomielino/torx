#include "maths.h"

// pan-zoom variables
extern Vector2 offset;
extern Vector2 scale;

// Convert coordinates from World Space --> Screen Space
Vector2 WorldToScreen(Vector2 world)
{
    Vector2 screen = Vector2Multiply(Vector2Subtract(world, offset), scale);
    screen = (Vector2) { (int)screen.x, (int)screen.y};
    return screen;
}

// Convert coordinates from Screen Space --> World Space
Vector2 ScreenToWorld(Vector2 screen)
{
    Vector2 scr = { (float)screen.x, (float)screen.y};
    Vector2 world = Vector2Add(Vector2Divide(scr, scale), offset);
    return world;
}

float function (float x)
{
    return cos(x);
}

