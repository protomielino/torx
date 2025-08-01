#include <math.h>

#include <raylib.h>
#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

int main(void)
{
    // Inizializzazione della finestra
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "physics test");

    // Variabili per la posizione, la velocità e la massa
    Vector2 position = { screenWidth / 2.0f, screenHeight / 2.0f };
    Vector2 velocity = { 0.0f, 0.0f };
    float mass = 1.0f; // Massa del punto
    const float speed = 200.0f; // Velocità massima
    Vector2 force = { 0.0f, 0.0f }; // Forza netta

    Vector2 mouse_pos = {};

    SetTargetFPS(30); // Imposta il frame rate

    while (!WindowShouldClose()) {
        // Reset della forza
        force = (Vector2){ 0.0f, 0.0f };

        mouse_pos = (Vector2){ GetMouseX(), GetMouseY() };
        force = Vector2Subtract(mouse_pos, position);

        // Aggiornamento della forza in base ai tasti cursore
        if (IsKeyDown(KEY_UP)) force.y = -mass * speed;
        if (IsKeyDown(KEY_DOWN)) force.y = mass * speed;
        if (IsKeyDown(KEY_LEFT)) force.x = -mass * speed;
        if (IsKeyDown(KEY_RIGHT)) force.x = mass * speed;
        if (IsKeyDown(KEY_Q)) mass -= 0.1f;
        if (IsKeyDown(KEY_W)) mass += 0.1f;
        if (mass < 0.1f) mass = 0.1f;
        // Calcolo dell'accelerazione
        Vector2 acceleration = Vector2Scale(force, 1.0f / mass);

        float dt = GetFrameTime();
        // Aggiornamento della velocità
        velocity = Vector2Add(velocity, Vector2Scale(acceleration, dt));
        velocity = Vector2Scale(velocity, 0.999f);

        // Aggiornamento della posizione
        position = Vector2Add(position, Vector2Scale(velocity, dt));

        // Inizio del disegno
        BeginDrawing();
        ClearBackground(BLACK);

        // Disegna il punto
        DrawCircle((int)position.x, (int)position.y, mass*2.0f, RED);

        // Disegna la linea della direzione
        Vector2 direction = Vector2Add(position, velocity);
        DrawLine((int)position.x, (int)position.y, (int)direction.x, (int)direction.y, LIGHTGRAY);

        // Disegna la linea della velocità
        DrawLine((int)position.x, (int)position.y, (int)mouse_pos.x, (int)mouse_pos.y, GREEN);

        char *text = (char*)TextFormat("a: %.3f", Vector2Length(acceleration));
        DrawText(text, 10, 50, 20, WHITE);
        text = (char*)TextFormat("v: %.3f", Vector2Length(velocity));
        DrawText(text, 10, 80, 20, WHITE);
        text = (char*)TextFormat("f: %.3f", Vector2Length(force));
        DrawText(text, 10, 110, 20, WHITE);
        text = (char*)TextFormat("m: %.3f", mass);
        DrawText(text, 10, 140, 20, WHITE);

        DrawFPS(10, 10);

        EndDrawing();
    }

    // Chiusura della finestra
    CloseWindow();
    return 0;
}
