#ifndef MATHS_H_
#define MATHS_H_

#include <raylib.h>
#include <raymath.h>

// Convert coordinates from World Space --> Screen Space
Vector2 WorldToScreen(Vector2 world);
// Convert coordinates from Screen Space --> World Space
Vector2 ScreenToWorld(Vector2 screen);
float function (float x);


#endif /* MATHS_H_ */
