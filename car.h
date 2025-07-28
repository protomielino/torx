#ifndef CAR_H_
#define CAR_H_

#include <raylib.h>

typedef struct
{
    Vector2 p_world;
    Vector2 p_screen;
    Vector2 g_world;
    Vector2 g_screen;
    Vector2 *modelCar;
    Ray     *ray;
    int n_rays;
} car;

car car_ctor();
void car_build(car *this, int n_rays);
void car_DrawWireFrameModel(car *this, Vector2 car_p_screen, float rot, float sca, Color col);

#endif /* CAR_H_ */
