#include <math.h>

#include <raylib.h>
#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

typedef struct Vehicle
{
    Vector2 position;
    Vector2 velocity;
    float acceleration;
    float maxSpeed;
    float mass;
    float steeringAngle;
} Vehicle;

//    ## 1. Legge di Newton
//    - **Seconda legge di Newton**: La forza netta (F) che agisce su un corpo è uguale al prodotto della massa (m) del corpo e della sua accelerazione (a). Questa relazione è espressa dalla formula:
//      \[
//      F = m \cdot a
//      \]
//
//    ## 2. Forze
//    - **Forze applicate**: Identificare tutte le forze che agiscono sul corpo, come la gravità, la frizione, la tensione, e le forze di contatto.
//    - **Forza netta**: Calcolare la forza netta risultante sommando tutte le forze vettorialmente.
//
//    ## 3. Massa e Accelerazione
//    - **Massa (m)**: La quantità di materia in un corpo, che influisce sulla sua resistenza all'accelerazione.
//    - **Accelerazione (a)**: La variazione della velocità del corpo nel tempo, che può essere calcolata come:
//      \[
//      a = \frac{\Delta v}{\Delta t}
//      \]
//      dove \(\Delta v\) è la variazione di velocità e \(\Delta t\) è l'intervallo di tempo.
//
//    ## 4. Diagrammi di Forza
//    - **Free Body Diagram (FBD)**: Rappresentazione grafica delle forze che agiscono su un corpo, utile per visualizzare e analizzare le forze in gioco.
//
//    ## 5. Equazioni del Moto
//    - Utilizzare le equazioni del moto per descrivere il comportamento del corpo nel tempo, come le equazioni di moto uniformemente accelerato.
//
//    ## 6. Unità di Misura
//    - Assicurarsi di utilizzare le unità di misura corrette (ad esempio, Newton per la forza, chilogrammi per la massa, metri al secondo quadrato per l'accelerazione).
//
//    ## 7. Applicazioni Pratiche
//    - Considerare situazioni pratiche come il moto su un piano inclinato, il moto circolare, e le collisioni, che possono richiedere ulteriori analisi.
//

void UpdateVehicle(Vehicle *vehicle)
{
    //      F = m \cdot a
}

int _main(void)
{
    // Inizializza la finestra
    InitWindow(800, 600, "Vehicle Simulation");
    SetTargetFPS(30);

    // Inizializza il veicolo
    Vehicle vehicle = {0};
    vehicle.position = (Vector2){400, 300};
    vehicle.velocity = (Vector2){0, 0};
    vehicle.acceleration = 0.0f;
    vehicle.maxSpeed = 5.0f;
    vehicle.mass = 1.0f;
    vehicle.steeringAngle = 0.0f;

    while (!WindowShouldClose()) {
        UpdateVehicle(&vehicle);

        // Disegna
        BeginDrawing(); {
            ClearBackground(RAYWHITE);
            DrawCircle((int)vehicle.position.x, (int)vehicle.position.y, 20, BLUE);
            DrawLine((int)vehicle.position.x, (int)vehicle.position.y,
                    (int)(vehicle.position.x + vehicle.velocity.x * 10),
                    (int)(vehicle.position.y + vehicle.velocity.y * 10), RED);
        } EndDrawing();
    }

    CloseWindow();
    return 0;
}

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
