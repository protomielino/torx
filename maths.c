#include "maths.h"

// pan-zoom variables
extern float offsetX;
extern float offsetY;
extern float scaleX;
extern float scaleY;

// Convert coordinates from World Space --> Screen Space
void WorldToScreen(float worldX, float worldY, int *screenX, int *screenY)
{
    *screenX = (int)((worldX - offsetX) * scaleX);
    *screenY = (int)((worldY - offsetY) * scaleY);
}

// Convert coordinates from Screen Space --> World Space
void ScreenToWorld(int screenX, int screenY, float *worldX, float *worldY)
{
    *worldX = ((float)screenX / scaleX) + offsetX;
    *worldY = ((float)screenY / scaleY) + offsetY;
}

float function (float x)
{
    return cos(x);
}

