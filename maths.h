#ifndef MATHS_H_
#define MATHS_H_

// Convert coordinates from World Space --> Screen Space
void WorldToScreen(float worldX, float worldY, int *screenX, int *screenY);
// Convert coordinates from Screen Space --> World Space
void ScreenToWorld(int screenX, int screenY, float *worldX, float *worldY);
float function (float x);


#endif /* MATHS_H_ */
