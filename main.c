#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <float.h>
#include <stdint.h>

#include <raylib.h>
#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "learning/real.h"
#include "learning/policy.h"
#include "maths.h"
#include "spline.h"
#include "track.h"
#include "car.h"

#define WIDTH 1000
#define HEIGHT 700

#define X_DIRECTION ((Vector3){ 1.0f, 0.0f, 0.0f })
#define Y_DIRECTION ((Vector3){ 0.0f, 1.0f, 0.0f })
#define Z_DIRECTION ((Vector3){ 0.0f, 0.0f, 1.0f })

Vector2 WINDOW_DIM = { WIDTH, HEIGHT };


// pan-zoom variables
bool panning;
Vector2 offset = { 0.0f, 0.0f };
Vector2 scale = { 1.0f, 1.0f };

Vector2 startPan = { 0.0f, 0.0f };

Vector2 selectedCell = { 0.0f, 0.0f };

#define MAX_DISTANCE 100.0f
#define MAX_ACTION 100.0f

int encodeSensors(real sensors[], int n_sensors)
{
    // Variabile per il numero intero a 32 bit senza segno
    uint32_t encodedValue = 0;

    for (int i = 0; i < n_sensors; ++i) {
        uint8_t norm_dist = (uint8_t) ( (7.0f / MAX_DISTANCE) * sensors[i] );
        if (norm_dist > 7) {
            norm_dist = 7;
        }
        // Shift e OR per codificare il valore
        encodedValue |= (norm_dist << (i * 3));
    }

    return encodedValue;
}

float* decodeAction(int32_t action, int n_actions)
{
    action = 0xFF2D16A3;
    // Variabile per il numero intero a 32 bit senza segno
    float *decodedAction = (float*) calloc(n_actions, sizeof(float));

    for (int i = 0; i < n_actions; ++i) {
        // Shift e AND per decodificare il valore
        uint8_t decoded = (action >> (i * 7)) & 255;

        decodedAction[i] = (MAX_ACTION / 255.0f) * (float)decoded;
        if (decodedAction[i] > MAX_ACTION) {
            decodedAction[i] = MAX_ACTION;
        }
    }

    return decodedAction;
}


int main(int argc, char **argv)
{
    // Initialization
    //---------------------------------------------------------------------------------------

    int episode = 0;

    // Definizione dei parametri
    const int n_sensors = 5;  // 5 sensori
    const int n_actions = 4;   // 3 azioni (sterzata, accelerazione, freno) con 3 intensità

    // Creare un'istanza della politica discreta
    DiscretePolicy* policy = DiscretePolicy_ctor(n_sensors, n_actions, 0.1, 0.8, 0.8, false, 0.1, 0.0);


    bool go = false;

    //----------------------------------------------------------------------------------
    // pan-zoom
    //----------------------------------------------------------------------------------
    // Initialise offset so 0,0 in world space is middle of the screen
    offset = (Vector2){ -WIDTH/2, -HEIGHT/2 };
    scale = Vector2One();

    //----------------------------------------------------------------------------------
    // track-lines
    //----------------------------------------------------------------------------------
    track track = track_ctor(4);
    track_build(&track);

    car car = car_ctor();
    car_build(&car, 5);

    InitWindow(WIDTH, HEIGHT, "Racing Line");

    SetTargetFPS(30);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------
        // pan-zoom
        //----------------------------------------------------------------------------------
        // Just grab a copy of mouse coordinates for convenience
        Vector2 mouse_scren = { (float)GetMouseX(), (float)GetMouseY()};
        Vector2 mouse_world = ScreenToWorld(mouse_scren);

        if (!IsKeyDown(KEY_E)) {
            // For panning, we need to capture the screen location when the user starts
            // to pan...
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                startPan = mouse_scren;
            }

            // ...as the mouse moves, the screen location changes. Convert this screen
            // coordinate change into world coordinates to implement the pan. Simples.
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                offset = Vector2Subtract(
                        offset,
                        Vector2Divide(
                                Vector2Subtract(
                                        mouse_scren,
                                        startPan),
                                        scale));

                // Start "new" pan for next epoch
                startPan = mouse_scren;
            }
        }

        // For zoom, we need to extract the location of the cursor before and after the
        // scale is changed. Here we get the cursor and translate into world space...
        Vector2 mouseWorld_BeforeZoom = {};
        mouseWorld_BeforeZoom = ScreenToWorld(mouse_scren);


        // ...change the scale as required...
        if (GetMouseWheelMove() > 0) {
            scale.x *= 1.1f;
            scale.y *= 1.1f;
        } else if (GetMouseWheelMove() < 0) {
            scale.x *= 0.9f;
            scale.y *= 0.9f;
        }

        // ...now get the location of the cursor in world space again - It will have changed
        // because the scale has changed, but we can offset our world now to fix the zoom
        // location in screen space, because we know how much it changed laterally between
        // the two spatial scales. Neat huh? ;-)
        Vector2 mouseWorld_AfterZoom = {};;
        mouseWorld_AfterZoom = ScreenToWorld(mouse_scren);
        offset = Vector2Add(
                offset,
                Vector2Subtract(
                        mouseWorld_BeforeZoom,
                        mouseWorld_AfterZoom));

        //----------------------------------------------------------------------------------
        // track-lines
        //----------------------------------------------------------------------------------

        // Handle iteration count
        if (IsKeyDown(KEY_A)) {
            track.iterations++;
        } else if (IsKeyDown(KEY_S)) {
            track.iterations--;
        }
        if (track.iterations < 0)
            track.iterations = 0;

        int n_path_points = arrlen(track.path.points);

        // Check if node is selected with mouse
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int i = 0; i < n_path_points; i++) {
                float d = sqrtf(powf(track.path.points[i].pos.x - mouse_world.x, 2) + powf(track.path.points[i].pos.y - mouse_world.y, 2));
                if (d < 10.0f) {
                    track.selectedNode = i;
                    break;
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            track.selectedNode = -1;

        // Move selected node
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && track.selectedNode >= 0) {
            track.path.points[track.selectedNode].pos.x = mouse_world.x;
            track.path.points[track.selectedNode].pos.y = mouse_world.y;
//            Spline_UpdateSplineProperties(&track.path);
        }

        // Move car around racing line
        if (IsKeyDown(KEY_X)) 
            track.marker += 1.0f * GetFrameTime();
        if (IsKeyDown(KEY_Z)) 
            track.marker -= 1.0f * GetFrameTime();

        if (IsKeyPressed(KEY_ONE)) {
            go = !go;
        }
        if (go)
            track.marker += 1.0f * GetFrameTime();

        if (track.marker >= (float)track.racingLine.totalSplineLength)
            track.marker -= (float)track.racingLine.totalSplineLength;
        if (track.marker < 0)
            track.marker = 0;

        // Calculate track boundary points
        float trackWidth = 10.0f;
        int n_points = n_path_points;
        for (int i = 0; i < n_points; i++) {
            Vector2 p1 = Spline_GetSplinePoint(&track.path, i);
            Vector2 g1 = Spline_GetSplineGradient(&track.path, i);
            g1 = Vector2Normalize(g1);
            track.trackLeft.points[i].pos.x  = p1.x - trackWidth * -g1.y;
            track.trackLeft.points[i].pos.y  = p1.y - trackWidth *  g1.x;
            track.trackRight.points[i].pos.x = p1.x + trackWidth * -g1.y;
            track.trackRight.points[i].pos.y = p1.y + trackWidth *  g1.x;
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing(); {
            ClearBackground(BLACK);

            //----------------------------------------------------------------------------------
            // track-lines
            //----------------------------------------------------------------------------------

            // Draw Track
            float res = 0.01f;

            for (float t = 0.0f; t < n_path_points; t += res) {
#if 1
                Vector2 pl1 = WorldToScreen(Spline_GetSplinePoint(&track.trackLeft, t));
                Vector2 pr1 = WorldToScreen(Spline_GetSplinePoint(&track.trackRight, t));
                Vector2 pl2 = WorldToScreen(Spline_GetSplinePoint(&track.trackLeft, t + res));
                Vector2 pr2 = WorldToScreen(Spline_GetSplinePoint(&track.trackRight, t + res));

                DrawLineV(pl1, pl2, GRAY);
                DrawLineV(pr1, pr2, GRAY);
//                DrawTriangleLines((Vector2){pr1.x, pr1.y}, (Vector2){pl1.x, pl1.y}, (Vector2){pr2.x, pr2.y}, GRAY);
//                DrawTriangleLines((Vector2){pl1.x, pl1.y}, (Vector2){pl2.x, pl2.y}, (Vector2){pr2.x, pr2.y}, GRAY);
#endif
            }

            int n_racingLine_points = arrlen(track.racingLine.points);

            // Reset racing line
            for (int i = 0; i < n_racingLine_points; i++) {
                track.racingLine.points[i] = track.path.points[i];
                track.displacement[i] = 0;
            }
            Spline_UpdateSplineProperties(&track.racingLine);

            for (int n = 0; n < track.iterations; n++) {
                for (int i = 0; i < n_racingLine_points; i++) {
                    // Get locations of neighbour nodes
                    Point2D pointRight = track.racingLine.points[(i + 1) % n_racingLine_points];
                    Point2D pointLeft = track.racingLine.points[(i + n_racingLine_points - 1) % n_racingLine_points];
                    Point2D pointMiddle = track.racingLine.points[i];

                    // Create vectors to neighbours
                    Vector2 vectorLeft  = { pointLeft.pos.x - pointMiddle.pos.x, pointLeft.pos.y - pointMiddle.pos.y };
                    Vector2 vectorRight = { pointRight.pos.x - pointMiddle.pos.x, pointRight.pos.y - pointMiddle.pos.y };

                    // Normalise neighbours
                    float lengthLeft = sqrtf(vectorLeft.x*vectorLeft.x + vectorLeft.y*vectorLeft.y);
                    Vector2 leftn = { vectorLeft.x / lengthLeft, vectorLeft.y / lengthLeft };
                    float lengthRight = sqrtf(vectorRight.x*vectorRight.x + vectorRight.y*vectorRight.y);
                    Vector2 rightn = { vectorRight.x / lengthRight, vectorRight.y / lengthRight };

                    // Add together to create bisector vector
                    Vector2 vectorSum = Vector2Add(rightn, leftn);;
                    float len = sqrtf(vectorSum.x*vectorSum.x + vectorSum.y*vectorSum.y);
                    vectorSum = Vector2Scale(vectorSum, 1/len);

                    // Get point gradient and normalise
                    Vector2 g = Spline_GetSplineGradient(&track.path, i);
                    float glen = sqrtf(g.x*g.x + g.y*g.y);
                    g.x /= glen; g.y /= glen;

                    // Project required correction onto point tangent to give displacment
                    float dp = -g.y*vectorSum.x + g.x * vectorSum.y;

                    // Shortest path
                    track.displacement[i] += (dp * 0.3f);

                    // Curvature
                    //displacement[(i + 1) % arrlen(racingLine.points)] += dp * -0.2f;
                    track.displacement[(i - 1 + n_racingLine_points) % n_racingLine_points] += dp * -0.2f;
                }

                // Clamp displaced points to track width
                int n_racingLine_points = n_racingLine_points;
                for (int i = 0; i < n_racingLine_points; i++) {
                    if (track.displacement[i] >=  trackWidth)
                        track.displacement[i] =   trackWidth;
                    if (track.displacement[i] <= -trackWidth)
                        track.displacement[i] =  -trackWidth;

                    Vector2 g = Spline_GetSplineGradient(&track.path, i);
                    float glen = sqrtf(g.x*g.x + g.y*g.y);
                    g.x /= glen; g.y /= glen;

                    track.racingLine.points[i].pos.x = track.path.points[i].pos.x + -g.y * track.displacement[i];
                    track.racingLine.points[i].pos.y = track.path.points[i].pos.y +  g.x * track.displacement[i];
                }
            }

            Spline_DrawSelf(&track.path, 0, 0, WHITE);
            Spline_DrawSelf(&track.trackLeft, 0, 0, RED);
            Spline_DrawSelf(&track.trackRight, 0, 0, GREEN);

//            Spline_UpdateSplineProperties(&track.racingLine);
            Spline_DrawSelf(&track.racingLine, 0, 0, YELLOW);

            for (int i = 0; i < n_path_points; i++) {
                Vector2 point_world = (Vector2){ track.path.points[i].pos.x, track.path.points[i].pos.y };
                Vector2 p_screen = WorldToScreen(point_world);
                DrawCircle(p_screen.x, p_screen.y, 5, RED);
            }


            car.p_world = Spline_GetSplinePoint(&track.racingLine, track.marker);
            car.p_screen = WorldToScreen(car.p_world);
            car.g_world = Spline_GetSplineGradient(&track.racingLine, track.marker);
            car.g_screen = WorldToScreen(car.g_world);
            car_DrawWireFrameModel(&car, car.p_screen, atan2f(car.g_world.y, car.g_world.x), scale.x, WHITE);

            float a = -M_PI_4;
            for (int r = 0; r < arrlen(car.ray); r++) {
                car.ray[r].position = (Vector3){ car.p_screen.x, car.p_screen.y, 0.0f };
                car.ray[r].direction = Vector3RotateByAxisAngle(
                        (Vector3){ car.g_world.x, car.g_world.y, 0.0f },
                        (Vector3){ 0.0f, 0.0f, 1.0f },
                        a);
                car.ray[r].direction = Vector3Normalize(car.ray[r].direction);

                a += (M_PI_2 / (float)(arrlen(car.ray)-1));
            }

            RayCollision *collision = NULL;

            for (int r = 0; r < car.n_rays; r++) {
                float start = track.marker;
                float end = track.path.totalSplineLength;
                float interval = res;
                float range = end + start; // Estendiamo il range per coprire il ciclo

                for (float t = start; t <= range; t += interval) {
                    float circular_t = fmodf(t, range);
                    if (circular_t > end) {
                        circular_t = fmodf(circular_t, end);
                    }
                    //                printf("%.1f\n", circular_t);
                    Vector2 pl1 = WorldToScreen(Spline_GetSplinePoint(&track.trackLeft, circular_t));
                    Vector2 pr1 = WorldToScreen(Spline_GetSplinePoint(&track.trackRight, circular_t));
                    Vector2 pl2 = WorldToScreen(Spline_GetSplinePoint(&track.trackLeft, circular_t + res));
                    Vector2 pr2 = WorldToScreen(Spline_GetSplinePoint(&track.trackRight, circular_t + res));

                    // ray checking

                    // quads
                    Vector3 gl0 = (Vector3){ pl1.x, pl1.y, -1.0f };
                    Vector3 gl1 = (Vector3){ pl1.x, pl1.y,  1.0f };
                    Vector3 gl2 = (Vector3){ pl2.x, pl2.y,  1.0f };
                    Vector3 gl3 = (Vector3){ pl2.x, pl2.y, -1.0f };
                    Vector3 gr0 = (Vector3){ pr1.x, pr1.y, -1.0f };
                    Vector3 gr1 = (Vector3){ pr1.x, pr1.y,  1.0f };
                    Vector3 gr2 = (Vector3){ pr2.x, pr2.y,  1.0f };
                    Vector3 gr3 = (Vector3){ pr2.x, pr2.y, -1.0f };

                    // Check ray collision against quad
                    RayCollision collisionLeft = GetRayCollisionQuad(car.ray[r], gl0, gl1, gl2, gl3);
                    RayCollision collisionRight = GetRayCollisionQuad(car.ray[r], gr0, gr1, gr2, gr3);

                    if (collisionLeft.hit) { // && (collisionLeft.distance < collisionRight.distance)) {
                        DrawCircle(collisionLeft.point.x, collisionLeft.point.y, 5.0f, PURPLE);
                        arrput(collision, collisionLeft);
                        t = range; // escape for loop
                    }
                    if (collisionRight.hit) { //  && (collisionRight.distance < collisionLeft.distance)) {
                        DrawCircle(collisionRight.point.x, collisionRight.point.y, 5.0f, GREEN);
                        arrput(collision, collisionRight);
                        t = range; // escape for loop
                    }
                }
            }

            int n_collisions = arrlen(collision);
            real collision_dist[n_collisions] = {};

            if (n_collisions > 0) {
                for (int c = 0; c < n_collisions; c++) {
                    DrawLine(
                            car.ray[c].position.x,
                            car.ray[c].position.y,
                            collision[c].point.x,
                            collision[c].point.y,
                            YELLOW);

                    collision_dist[c] = collision[c].distance;
                }

                // Ottenere lo stato attuale dalla lettura dei sensori
                // Dobbiamo convertire il vettore di stati in un formato utilizzabile
                uint32_t state = encodeSensors(collision_dist, n_collisions);

                // Selezionare un'azione in base allo stato attuale
                int32_t action = DiscretePolicy_SelectAction(policy, state, 0.0, -1);

                float *actions = decodeAction(action, n_actions);

//                accel += actions[0] - MAX_ACTION/2;
//                brake += actions[1] - MAX_ACTION/2;
//                stear += actions[2] - MAX_ACTION/2;

//                for (int i = 0; i < n_actions; i++) {
//                    printf("[%d]%f ", i, actions[i]);
//                }
//                printf("\n");

                // Simulazione della ricompensa
                real reward = (action == 0) ? 1.0 : -1.0; // Ricompensa positiva per l'azione 0, negativa per le altre

                free(actions);

                // Aggiornare la politica con la ricompensa ricevuta
                DiscretePolicy_SelectAction(policy, state, reward, action);

                // Stampa dell'azione selezionata e della ricompensa
                printf("Episodio: %d, Sensori: [", episode);
                for (int i = 0; i < n_sensors; ++i) {
                    printf(" %f", collision_dist[i]);
                }
                printf(" ] \t Azione: %d, Ricompensa: %f\n", action, reward);
                episode ++;
            }

            arrfree(collision);

            DrawFPS(20, 20);
        } EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    // Libera la memoria allocata

    arrfree(car.modelCar);
    arrfree(track.path.points);
    arrfree(track.trackLeft.points);
    arrfree(track.trackRight.points);
    arrfree(track.racingLine.points);

    return 0;
}
