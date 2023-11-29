#include "raylib.h"
#include <raymath.h>

const float FPS = 60;
const float TIMESTEP = 1.0f / FPS;
const float FRICTION = 0.99f;

struct Base{ //Base Health 
    int health;
    Vector2 basePos;
};

struct Player{
    bool isDragging;
    float radius;
    Vector2 velocity;
    Vector2 playerPos;
    Vector2 acceleration;
};

// Computes for impulse given the following parameters :
// elasticity, relative velocity, collision normal, and the inverse masses of the two objects
// float GetImpulse(float elasticity, Vector2 relative_velocity, Vector2 collision_normal, float inverse_mass_a, float inverse_mass_b) {
//     float numerator = -(1 + elasticity) * Vector2DotProduct(relative_velocity, collision_normal);
//     float denominator = Vector2DotProduct(collision_normal, collision_normal) * (inverse_mass_a + inverse_mass_b);
//     return numerator / denominator;
// }

// // Handles the collision between a circle and an AABB
// void HandleCircleAABBCollision(Ball& ball, Wall& wall) {
//     Vector2 q = GetClosestPointToAABB(ball.position, wall.position, {wall.width, wall.height});

//     float distance = Vector2Distance(ball.position, q);

//     if (distance <= ball.radius) {
//         Vector2 relative_velocity = Vector2Subtract(ball.velocity, wall.velocity);
//         Vector2 collision_normal = Vector2Subtract(ball.position, q);
//         if (Vector2DotProduct(collision_normal, relative_velocity) < 0) {
//             float impulse = GetImpulse(ELASTICITY, relative_velocity, collision_normal, ball.inverse_mass, wall.inverse_mass);

//             ball.velocity = Vector2Add(ball.velocity, Vector2Scale(collision_normal, impulse * ball.inverse_mass));
//             wall.velocity = Vector2Subtract(wall.velocity, Vector2Scale(collision_normal, impulse * wall.inverse_mass));
//         }
//     }
// };

int main() {
    Base playerBase;
    Player player1;
 
    float accumulator = 0;
    const int screenWidth = 800;
    const int screenHeight = 600;

    playerBase.health = 3;
    player1.radius = 20.0f;
    player1.isDragging = false;
    player1.playerPos = {400,300};
    player1.acceleration = {0,0};

    InitWindow(screenWidth, screenHeight, "PLAYER MOVING");

    Vector2 mouse_drag_start = Vector2Zero();
    SetTargetFPS(60); // Set the target frame rate
    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        Vector2 mouse_position = GetMousePosition();
        Vector2 cue_stick_force = Vector2Zero();

        if(player1.playerPos.x - 10 < 0){
            player1.velocity.x *= -1;
        }
        if(player1.playerPos.x + 10 > 800){
            player1.velocity.x *= -1;
        }
        if(player1.playerPos.y - 10 < 0){
            player1.velocity.y *= -1;
        }
        if(player1.playerPos.y + 10 > 600){
            player1.velocity.y *= -1;
        }

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePosition = GetMousePosition();
            if (!player1.isDragging) {
                mouse_drag_start = mousePosition;
                player1.isDragging = true;
            }
        }

        else if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && player1.isDragging){
            Vector2 mouse_drag_end = GetMousePosition();

            if (CheckCollisionPointCircle(mouse_drag_start, player1.playerPos, player1.radius)) {
                player1.velocity = Vector2Subtract(mouse_drag_start, mouse_drag_end);
                player1.isDragging = false;
            }
        }

        // Vector2 shot_dir = Vector2Subtract(mouse_position, mouse_drag_start);
        // float shot_vector_distance = Vector2Length(shot_dir);
        // if (shot_vector_distance > 150.0f) {
        //     shot_vector_distance = 150.0f;
        // }
        // shot_dir = Vector2Normalize(shot_dir);
        accumulator += delta_time;

        while (accumulator >= TIMESTEP) {
            player1.velocity = Vector2Add(player1.velocity, Vector2Scale(player1.acceleration, TIMESTEP));
            player1.playerPos = Vector2Add(player1.playerPos, Vector2Scale(player1.velocity, TIMESTEP));

            player1.velocity = Vector2Add(player1.velocity, Vector2Scale(player1.acceleration, TIMESTEP));
            player1.velocity = Vector2Scale(player1.velocity, FRICTION);
            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        // DrawCircle(screenWidth / 2, screenHeight / 2, 75.0f, YELLOW); // Base
        DrawCircleV(player1.playerPos, player1.radius, RED);
        
        if (player1.isDragging) {
            //Vector2 mouse_drag_vector = Vector2Scale(shot_dir, shot_vector_distance);
            //DrawLineEx(mouse_drag_start, mouse_drag_end, 2, RED);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
