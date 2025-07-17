#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <raylib.h>
#include <raymath.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "spline.h"
#include "maths.h"

#define WIDTH 1000
#define HEIGHT 700

Vector2 WINDOW_DIM = { WIDTH, HEIGHT };


// pan-zoom variables
Vector2 offset = { 0.0f, 0.0f };
Vector2 scale = { 1.0f, 1.0f };

Vector2 startPan = { 0.0f, 0.0f };

Vector2 selectedCell = { 0.0f, 0.0f };

// track-lines variables
Spline path, trackLeft, trackRight, racingLine;    // Various splines
int nodes;    // Number of nodes in spline
float displacement[20] = {0}; // Displacement along spline node normal
int iterations;
float marker;
int selectedNode;
Vector2 *modelCar;

void DrawWireFrameModel(const Vector2 *vecModelCoordinates, float x, float y, float r, float s, Color col)
{
    // pair.x  = x coordinate
    // pair.y = y coordinate

    // Create translated model vector of coordinate pairs
    Vector2 *vecTransformedCoordinates = NULL;
    int verts = arrlen(vecModelCoordinates);
    arrsetlen(vecTransformedCoordinates, verts);

    // Rotate
    for (int i = 0; i < verts; i++) {
        vecTransformedCoordinates[i].x = vecModelCoordinates[i].x * cosf(r) - vecModelCoordinates[i].y * sinf(r);
        vecTransformedCoordinates[i].y = vecModelCoordinates[i].x  * sinf(r) + vecModelCoordinates[i].y * cosf(r);
    }

    // Scale
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].x  = vecTransformedCoordinates[i].x  * s;
        vecTransformedCoordinates[i].y = vecTransformedCoordinates[i].y * s;
    }

    // Translate
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].x  = vecTransformedCoordinates[i].x  + x;
        vecTransformedCoordinates[i].y = vecTransformedCoordinates[i].y + y;
    }

    // Draw Closed Polygon
    for (int i = 0; i < verts + 1; i++)
    {
        int j = (i + 1);
        DrawLineEx(
                (Vector2){(int)vecTransformedCoordinates[i % verts].x , (int)vecTransformedCoordinates[i % verts].y},
                (Vector2){(int)vecTransformedCoordinates[j % verts].x , (int)vecTransformedCoordinates[j % verts].y},
                5, col);
    }
}

bool buildTrack()
{
    for (int i = 0; i < nodes; i++)
    {
        //path.points.push_back(
        //  { 30.0f * sinf((float)i / (float)nodes * 3.14159f * 2.0f) + ScreenWidth() / 2,
        //  30.0f * cosf((float)i / (float)nodes * 3.14159f * 2.0f) + ScreenHeight() / 2 });

        // Could use allocation functions for thes now, but just size via
        // append
        arrput(trackLeft.points, Point2D_ctor());
        arrput(trackRight.points, Point2D_ctor());
        arrput(racingLine.points, Point2D_ctor());
    }

    // A hand crafted track
    arrput(path.points, Point2D_new(  81.8f, 196.0f, 0.0f ));
    arrput(path.points, Point2D_new( 108.0f, 210.0f, 0.0f ));
    arrput(path.points, Point2D_new( 152.0f, 216.0f, 0.0f ));
    arrput(path.points, Point2D_new( 182.0f, 185.6f, 0.0f ));
    arrput(path.points, Point2D_new( 190.0f, 159.0f, 0.0f ));
    arrput(path.points, Point2D_new( 198.0f, 122.0f, 0.0f ));
    arrput(path.points, Point2D_new( 226.0f,  93.0f, 0.0f ));
    arrput(path.points, Point2D_new( 224.0f,  41.0f, 0.0f ));
    arrput(path.points, Point2D_new( 204.0f,  15.0f, 0.0f ));
    arrput(path.points, Point2D_new( 158.0f,  24.0f, 0.0f ));
    arrput(path.points, Point2D_new( 146.0f,  52.0f, 0.0f ));
    arrput(path.points, Point2D_new( 157.0f,  93.0f, 0.0f ));
    arrput(path.points, Point2D_new( 124.0f, 129.0f, 0.0f ));
    arrput(path.points, Point2D_new(  83.0f, 104.0f, 0.0f ));
    arrput(path.points, Point2D_new(  77.0f,  62.0f, 0.0f ));
    arrput(path.points, Point2D_new(  40.0f,  57.0f, 0.0f ));
    arrput(path.points, Point2D_new(  21.0f,  83.0f, 0.0f ));
    arrput(path.points, Point2D_new(  33.0f, 145.0f, 0.0f ));
    arrput(path.points, Point2D_new(  30.0f, 198.0f, 0.0f ));
    arrput(path.points, Point2D_new(  48.0f, 210.0f, 0.0f ));

    for (int i = 0; i < arrlen(path.points); i++) {
        path.points[i].x *= 4;
        path.points[i].y *= 3;
    }


    arrput(modelCar, ((Vector2){ 2,  0 }));
    arrput(modelCar, ((Vector2){ 0, -1 }));
    arrput(modelCar, ((Vector2){ 0,  1 }));

    for (int i = 0; i < arrlen(modelCar); i++) {
        modelCar[i].x *= 4;
        modelCar[i].y *= 3;
    }

    Spline_UpdateSplineProperties(&path);

    return true;
}

int main(int argc, char **argv)
{
    // Initialization
    //---------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------
    // pan-zoom
    //----------------------------------------------------------------------------------
    // Initialise offset so 0,0 in world space is middle of the screen
    offset = (Vector2){ -WIDTH/2, -HEIGHT/2 };
    scale = Vector2One();

    //----------------------------------------------------------------------------------
    // track-lines
    //----------------------------------------------------------------------------------
    path = Spline_ctor();
    trackLeft = Spline_ctor();
    trackRight = Spline_ctor();
    racingLine = Spline_ctor();;    // Various splines

    nodes = 20;    // Number of nodes in spline

    iterations = 1;
    marker = 1.0f;
    selectedNode = -1;

    modelCar = NULL;

    buildTrack();

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
        Vector2 mouse = { (float)GetMouseX(), (float)GetMouseY()};

        // For panning, we need to capture the screen location when the user starts
        // to pan...
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            startPan = mouse;
        }

        // ...as the mouse moves, the screen location changes. Convert this screen
        // coordinate change into world coordinates to implement the pan. Simples.
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            offset = Vector2Subtract(
                    offset,
                    Vector2Divide(
                            Vector2Subtract(
                                    mouse,
                                    startPan),
                            scale));

            // Start "new" pan for next epoch
            startPan = mouse;
        }

        // For zoom, we need to extract the location of the cursor before and after the
        // scale is changed. Here we get the cursor and translate into world space...
        Vector2 mouseWorld_BeforeZoom = {};
        mouseWorld_BeforeZoom = ScreenToWorld(mouse);


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
        mouseWorld_AfterZoom = ScreenToWorld(mouse);
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
        }

        // Check if node is selected with mouse
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        }


        // Handle iteration count
        if (IsKeyDown(KEY_A)) {
            iterations++;
        } else if (IsKeyDown(KEY_S)) {
            iterations--;
        }
        if (iterations < 0)
            iterations = 0;

        // Check if node is selected with mouse
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int i = 0; i < arrlen(path.points); i++) {
                float d = sqrtf(powf(path.points[i].x - GetMouseX(), 2) + powf(path.points[i].y - GetMouseY(), 2));
                if (d < 10.0f) {
                    selectedNode = i;
                    break;
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            selectedNode = -1;

        // Move selected node
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && selectedNode >= 0) {
            path.points[selectedNode].x = GetMouseX();
            path.points[selectedNode].y = GetMouseY();
            Spline_UpdateSplineProperties(&path);
        }


        // Move car around racing line
        marker += 2.0f * GetFrameTime();
        if (marker >= (float)racingLine.totalSplineLength)
            marker -= (float)racingLine.totalSplineLength;

        // Calculate track boundary points
        float trackWidth = 25.0f;
        for (int i = 0; i < arrlen(path.points); i++) {
            Point2D p1 = Spline_GetSplinePoint(&path, i);
            Point2D g1 = Spline_GetSplineGradient(&path, i);
            float glen = sqrtf(g1.x*g1.x + g1.y*g1.y);

            trackLeft.points[i].x  = p1.x + trackWidth * ( -g1.y / glen );
            trackLeft.points[i].y  = p1.y + trackWidth * (  g1.x / glen );
            trackRight.points[i].x = p1.x - trackWidth * ( -g1.y / glen );
            trackRight.points[i].y = p1.y - trackWidth * (  g1.x / glen );
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing(); {
            ClearBackground(BLACK);

            //----------------------------------------------------------------------------------
            // pan-zoom
            //----------------------------------------------------------------------------------

            // Clip

            Vector2 worldNW = ScreenToWorld((Vector2){0,0});
            float worldLeft = worldNW.x;
            float worldTop = worldNW.y;
            Vector2 worldSE = ScreenToWorld(WINDOW_DIM);
            float worldRight = worldSE.x;
            float worldBottom = worldSE.y;


            // Draw Main Axes a 10x10 Unit Grid
            // Draw 10 horizontal lines
            int linesDrawn = 0;
            for (float y = 0.0f; y <= 10.0f; y++) {
                if (y >= worldTop && y <= worldBottom) {
                    Vector2 s = { 0.0f, y };
                    Vector2 e = { 10.0f, y };

                    Vector2 pixel_s = {};
                    Vector2 pixel_e = {};

                    pixel_s = WorldToScreen(s);
                    pixel_e = WorldToScreen(e);

                    DrawLine(pixel_s.x, pixel_s.y, pixel_e.x, pixel_e.y, WHITE);
                    linesDrawn++;
                }
            }

            // Draw 10 vertical lines
            for (float x = 0.0f; x <= 10.0f; x++) {
                if (x >= worldLeft && x <= worldRight) {
                    Vector2 s = { x, 0.0f };
                    Vector2 e = { x, 10.0f };

                    Vector2 pixel_s = {};
                    Vector2 pixel_e = {};

                    pixel_s = WorldToScreen(s);
                    pixel_e = WorldToScreen(e);

                    DrawLine(pixel_s.x, pixel_s.y, pixel_e.x, pixel_e.y, WHITE);
                    linesDrawn++;
                }
            }

            // Draw selected cell

            // We can easily determine where the mouse is in world space. In fact we already
            // have this frame so just reuse the values
            if (IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON)) {
                selectedCell = (Vector2){(int)mouseWorld_AfterZoom.x, (int)mouseWorld_AfterZoom.y};
            }

            // Draw selected cell by filling with red circle. Convert cell coords
            // into screen space, also scale the radius
            Vector2 c;
            int cr;
            c = WorldToScreen(Vector2AddValue(selectedCell, 0.5f));
            cr = 0.3f * scale.x;
            DrawCircle(c.x, c.y, cr, RED);
            DrawText(TextFormat("Lines Drawn: %d", linesDrawn), 2, 2, 10, WHITE);       // Draw text (using default font)


            // Draw Chart
            float worldPerScreenWidthPixel = (worldRight - worldLeft) / WIDTH;
//            float worldPerScreenHeightPixel = (worldBottom - worldTop) / HEIGHT;
            Vector2 p;
            Vector2 op = {};
            op = WorldToScreen(
                        (Vector2){
                            worldLeft - worldPerScreenWidthPixel,
                            -function((worldLeft - worldPerScreenWidthPixel) - 5.0f) + 5.0f
                        });
            for (float x = worldLeft; x < worldRight; x+=worldPerScreenWidthPixel) {
                float y = -function(x - 5.0f) + 5.0f;
                p = WorldToScreen((Vector2){ x, y });
                DrawLine(op.x, op.y, p.x, p.y, GREEN);
                op = p;
            }


            //----------------------------------------------------------------------------------
            // track-lines
            //----------------------------------------------------------------------------------

            // Draw Track
            float res = 0.1f;
            for (float t = 0.0f; t < arrlen(path.points); t += res) {
#if 0
                Point2D pl1 = Spline_GetSplinePoint(&trackLeft, t);
                Point2D pr1 = Spline_GetSplinePoint(&trackRight, t);
                Point2D pl2 = Spline_GetSplinePoint(&trackLeft, t + res);
                Point2D pr2 = Spline_GetSplinePoint(&trackRight, t + res);

                DrawTriangle(
                        (Vector2){pr1.x, pr1.y},
                        (Vector2){pl1.x, pl1.y},
                        (Vector2){pr2.x, pr2.y},
                        GRAY);
                DrawTriangle(
                        (Vector2){pl1.x, pl1.y},
                        (Vector2){pl2.x, pl2.y},
                        (Vector2){pr2.x, pr2.y},
                        GRAY);
#endif

                // Reset racing line
                for (int i = 0; i < arrlen(racingLine.points); i++) {
                    racingLine.points[i] = path.points[i];
                    displacement[i] = 0;
                }
                Spline_UpdateSplineProperties(&racingLine);
            }

            for (int n = 0; n < iterations; n++) {
                for (int i = 0; i < arrlen(racingLine.points); i++) {
                    // Get locations of neighbour nodes
                    Point2D pointRight = racingLine.points[(i + 1) % arrlen(racingLine.points)];
                    Point2D pointLeft = racingLine.points[(i + arrlen(racingLine.points) - 1) % arrlen(racingLine.points)];
                    Point2D pointMiddle = racingLine.points[i];

                    // Create vectors to neighbours
                    Point2D vectorLeft = { pointLeft.x - pointMiddle.x, pointLeft.y - pointMiddle.y };
                    Point2D vectorRight = { pointRight.x - pointMiddle.x, pointRight.y - pointMiddle.y };

                    // Normalise neighbours
                    float lengthLeft = sqrtf(vectorLeft.x*vectorLeft.x + vectorLeft.y*vectorLeft.y);
                    Point2D leftn = { vectorLeft.x / lengthLeft, vectorLeft.y / lengthLeft };
                    float lengthRight = sqrtf(vectorRight.x*vectorRight.x + vectorRight.y*vectorRight.y);
                    Point2D rightn = { vectorRight.x / lengthRight, vectorRight.y / lengthRight };

                    // Add together to create bisector vector
                    Point2D vectorSum = { rightn.x + leftn.x, rightn.y + leftn.y };
                    float len = sqrtf(vectorSum.x*vectorSum.x + vectorSum.y*vectorSum.y);
                    vectorSum.x /= len; vectorSum.y /= len;

                    // Get point gradient and normalise
                    Point2D g = Spline_GetSplineGradient(&path, i);
                    float glen = sqrtf(g.x*g.x + g.y*g.y);
                    g.x /= glen; g.y /= glen;

                    // Project required correction onto point tangent to give displacment
                    float dp = -g.y*vectorSum.x + g.x * vectorSum.y;

                    // Shortest path
                    displacement[i] += (dp * 0.3f);

                    // Curvature
//                    displacement[(i + 1) % arrlen(racingLine.points)] += dp * -0.2f;
                    displacement[(i - 1 + arrlen(racingLine.points)) % arrlen(racingLine.points)] += dp * -0.2f;
                }

                // Clamp displaced points to track width
                for (int i = 0; i < arrlen(racingLine.points); i++) {
                    if (displacement[i] >= trackWidth) displacement[i] = trackWidth;
                    if (displacement[i] <= -trackWidth) displacement[i] = -trackWidth;

                    Point2D g = Spline_GetSplineGradient(&path, i);
                    float glen = sqrtf(g.x*g.x + g.y*g.y);
                    g.x /= glen; g.y /= glen;

                    racingLine.points[i].x = path.points[i].x + -g.y * displacement[i];
                    racingLine.points[i].y = path.points[i].y + g.x * displacement[i];
                }
            }

            Spline_DrawSelf(&path, 0, 0, WHITE);
            Spline_DrawSelf(&trackLeft, 0, 0, RED);
            Spline_DrawSelf(&trackRight, 0, 0, GREEN);

            Spline_UpdateSplineProperties(&racingLine);
            Spline_DrawSelf(&racingLine, 0, 0, YELLOW);

            for (int i = 0; i < arrlen(path.points); i++) {
                Vector2 point = (Vector2){ path.points[i].x, path.points[i].y };
                Vector2 p_screen = WorldToScreen(point);
                DrawCircle(p_screen.x, p_screen.y, 5, RED);
            }

            Point2D car_p = Spline_GetSplinePoint(&racingLine, marker);
            Point2D car_g = Spline_GetSplineGradient(&racingLine, marker);
            DrawWireFrameModel(modelCar, car_p.x, car_p.y, atan2f(car_g.y, car_g.x), 4.0f, WHITE);

            DrawFPS(20, 20);
        } EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    // Libera la memoria allocata

    return 0;
}
