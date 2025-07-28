#include <math.h>

#include <raylib.h>
#include <raymath.h>

#include "car.h"

#include "stb_ds.h"

car car_ctor()
{
    car this = {0};
    this.ray = NULL;
    return this;
}

void car_build(car *this, int n_rays)
{
    arrput(this->modelCar, ((Vector2){  1,  0 }));
    arrput(this->modelCar, ((Vector2){ -1, -1 }));
    arrput(this->modelCar, ((Vector2){ -1,  1 }));

    for (int i = 0; i < arrlen(this->modelCar); i++) {
        this->modelCar[i].x *= 4;
        this->modelCar[i].y *= 3;
    }

    float angle = -M_PI_4;
    this->n_rays = n_rays;
    for (int i = 0; i < this->n_rays; i++) {
        Ray ray = {0};

        ray.position  = (Vector3){ 0.0f, 0.0f, 0.0f };
        ray.direction = (Vector3){ 1.0f, 0.0f, 0.0f };
        ray.direction = Vector3RotateByAxisAngle(
                (Vector3){ 1.0f, 0.0f, 0.0f },
                (Vector3){ 0.0f, 0.0f, 1.0f },
                angle);

        arrput(this->ray, ray);

        angle += M_PI_2 / this->n_rays;
    }
}

void car_DrawWireFrameModel(car *this, Vector2 car_p_screen, float rot, float sca, Color col)
{
    // pair.x  = x coordinate
    // pair.y = y coordinate

    // Create translated model vector of coordinate pairs
    Vector2 *vecTransformedCoordinates = NULL;
    int verts = arrlen(this->modelCar);
    arrsetlen(vecTransformedCoordinates, verts);

    // Rotate
    for (int i = 0; i < verts; i++) {
        vecTransformedCoordinates[i].x = this->modelCar[i].x * cosf(rot) - this->modelCar[i].y * sinf(rot);
        vecTransformedCoordinates[i].y = this->modelCar[i].x  * sinf(rot) + this->modelCar[i].y * cosf(rot);
    }

    // Scale
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].x  = vecTransformedCoordinates[i].x  * sca;
        vecTransformedCoordinates[i].y = vecTransformedCoordinates[i].y * sca;
    }

    // Translate
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].x  = vecTransformedCoordinates[i].x  + car_p_screen.x;
        vecTransformedCoordinates[i].y = vecTransformedCoordinates[i].y + car_p_screen.y;
    }

    // Draw Closed Polygon
    for (int i = 0; i < verts + 1; i++)
    {
        int j = (i + 1);
        DrawLineEx(
                (Vector2){(int)vecTransformedCoordinates[i % verts].x , (int)vecTransformedCoordinates[i % verts].y},
                (Vector2){(int)vecTransformedCoordinates[j % verts].x , (int)vecTransformedCoordinates[j % verts].y},
                2, col);
    }

    arrfree(vecTransformedCoordinates);
}
