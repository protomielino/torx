#include <raylib.h>
#include <raymath.h>

#include "track.h"

#include "stb_ds.h"

track track_ctor(int n_nodes)
{
    track this = {0};

    this.path = Spline_ctor();
    this.trackLeft = Spline_ctor();
    this.trackRight = Spline_ctor();
    this.racingLine = Spline_ctor();;    // Various splines

    this.nodes = n_nodes;    // Number of nodes in spline

    arrsetlen(this.displacement, this.nodes);

    this.iterations = 1;
    this.marker = 5.0f;
    this.selectedNode = -1;

    return this;
}

bool track_build(track *this)
{
    for (int i = 0; i < this->nodes; i++)
    {
        //this->path.points.push_back(
        //  { 30.0f * sinf((float)i / (float)nodes * 3.14159f * 2.0f) + ScreenWidth() / 2,
        //  30.0f * cosf((float)i / (float)nodes * 3.14159f * 2.0f) + ScreenHeight() / 2 });

        // Could use allocation functions for thes now, but just size via
        // append
        arrput(this->trackLeft.points, Point2D_ctor());
        arrput(this->trackRight.points, Point2D_ctor());
        arrput(this->racingLine.points, Point2D_ctor());
    }

    float res = 360.0f / (float)this->nodes;
    for (float a = 0.0f; a < 360.0f; a += res) {
        arrput(this->path.points, Point2D_new(
                cos(DEG2RAD*a) * 100.0f,
                sin(DEG2RAD*a) * 100.0f,
                0.0f ));
    }
//    // A hand crafted track
//    arrput(this->path.points, Point2D_new(  81.8f, 196.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 108.0f, 210.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 152.0f, 216.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 182.0f, 185.6f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 190.0f, 159.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 198.0f, 122.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 226.0f,  93.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 224.0f,  41.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 204.0f,  15.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 158.0f,  24.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 146.0f,  52.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 157.0f,  93.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new( 124.0f, 129.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new(  83.0f, 104.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new(  77.0f,  62.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new(  40.0f,  57.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new(  21.0f,  83.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new(  33.0f, 145.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new(  30.0f, 198.0f, 0.0f ));
//    arrput(this->path.points, Point2D_new(  48.0f, 210.0f, 0.0f ));

//    for (int i = 0; i < arrlen(this->path.points); i++) {
//        this->path.points[i].pos.x = (this->path.points[i].pos.x -WIDTH/2) * 4.0f;
//        this->path.points[i].pos.y = (this->path.points[i].pos.y -HEIGHT/2) * 3.0f;;
//    }


    Spline_UpdateSplineProperties(&this->path);

    return true;
}
