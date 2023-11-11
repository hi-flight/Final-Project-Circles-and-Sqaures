#include "raylib.h"
#include <cmath>

// Define enemy types
typedef enum {
    ENEMY_ORDINARY,
    ENEMY_SPEED_DEMON,
    ENEMY_ARMORED,
} EnemyType;

// Define enemy structure
typedef struct {
    Rectangle rect;       // Rectangle to represent the enemy
    Vector2 targetPosition;
    EnemyType type;
    Color color;
    bool active;
    bool helmet; // Flag to indicate if armored enemy has a helmet
    int hits;    // Number of hits required to defeat an enemy
} Enemy;

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Tower Defense Example");

    SetTargetFPS(60); // Set the target frame rate

    Vector2 ballPosition = {screenWidth / 2, screenHeight / 2};
    Vector2 ballVelocity = {0, 0};
    bool isFlinging = false;
    Vector2 flingStartPos = {0, 0};
    float flingSpeed = 50.0f; // Slower flinging speed in pixels per second

    int health = 3; // Total health segments

    Enemy enemies[3];
    for (int i = 0; i < 3; i++) {
        // Set enemy initial positions to random values within the screen boundaries
        enemies[i].rect = (Rectangle){(float)GetRandomValue(0, screenWidth - 30), (float)GetRandomValue(0, screenHeight - 30), 30, 30}; // Rectangle to represent the enemy
        enemies[i].targetPosition = ballPosition;
        enemies[i].type = (EnemyType)GetRandomValue(0, 2);
        enemies[i].active = true;
        enemies[i].hits = 1; // All enemies start with 1 hit required
        
        // Configure enemy characteristics based on type
        switch (enemies[i].type) {
            case ENEMY_ORDINARY:
                enemies[i].color = RED;
                enemies[i].helmet = false;
                break;
            case ENEMY_SPEED_DEMON:
                enemies[i].color = YELLOW;
                enemies[i].helmet = false;
                break;
            case ENEMY_ARMORED:
                enemies[i].color = GRAY;
                enemies[i].helmet = true; // Armored enemies start with helmets
                enemies[i].hits = 2;      // Armored enemies require 2 hits to defeat
                break;
        }
    }

    // Variables for tracking time
    double lastFrameTime = GetTime();
    double deltaTime = 0;

    while (!WindowShouldClose()) {
        // Calculate delta time (time elapsed since the last frame)
        double currentFrameTime = GetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Input handling
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            flingStartPos = GetMousePosition();
            isFlinging = true;
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isFlinging) {
            Vector2 flingEndPos = GetMousePosition();
            // Calculate the velocity based on delta time
            ballVelocity.x = (flingEndPos.x - flingStartPos.x) / flingSpeed;
            ballVelocity.y = (flingEndPos.y - flingStartPos.y) / flingSpeed;
            isFlinging = false;
        }

        // Update ball position based on delta time
        ballPosition.x += ballVelocity.x * deltaTime;
        ballPosition.y += ballVelocity.y * deltaTime;

        // Check if the ball is outside the screen
        if (ballPosition.x < -25 || ballPosition.x > screenWidth + 25 || 
            ballPosition.y < -25 || ballPosition.y > screenHeight + 25) {
            // Decrease health
            health--;

            // Reset the ball's position to the center
            ballPosition = (Vector2){screenWidth / 2, screenHeight / 2};
            ballVelocity = (Vector2){0, 0};
        }

        // Update enemy positions based on delta time
        // Update enemy positions based on delta time
    for (int i = 0; i < 10; i++) {
        if (enemies[i].active) {
            // Move enemies toward the ball position
            float dx = enemies[i].targetPosition.x - enemies[i].rect.x;
            float dy = enemies[i].targetPosition.y - enemies[i].rect.y;
            float distance = sqrtf(dx * dx + dy * dy);
            float directionX = dx / distance;
            float directionY = dy / distance;
            // Adjust the enemy movement speed for slower movement
            float speed = (enemies[i].type == ENEMY_SPEED_DEMON) ? 100.0f : 50.0f; // Reduced speed values
            enemies[i].rect.x += directionX * speed * deltaTime;
            enemies[i].rect.y += directionY * speed * deltaTime;

            // Check if an enemy reaches the ball
            if (CheckCollisionRecs(enemies[i].rect, (Rectangle){ballPosition.x - 25, ballPosition.y - 25, 50, 50})) {
                // Handle enemy collisions based on type
                if (enemies[i].type == ENEMY_ORDINARY) {
                    // Decrease health for ordinary enemy
                    health--;
                } else if (enemies[i].type == ENEMY_SPEED_DEMON) {
                    // Speed up the ball for speed demon enemy
                    ballVelocity.x *= 2;
                    ballVelocity.y *= 2;
                } else if (enemies[i].type == ENEMY_ARMORED) {
                    // Check if armored enemy still has a helmet
                    if (enemies[i].helmet) {
                        // Remove the helmet on the first hit
                        enemies[i].helmet = false;
                    } else {
                        // If the helmet is already removed, decrease health on the second hit
                        health--;
                        // Deactivate the enemy
                        enemies[i].active = false;
                    }
                }
            }
        }
    }


        // Spawn new enemies
        for (int i = 0; i < 3; i++) {
            if (!enemies[i].active) {
                // Set enemy positions to random values within the screen boundaries
                enemies[i].rect = (Rectangle){(float)GetRandomValue(0, screenWidth - 30), (float)GetRandomValue(0, screenHeight - 30), 30, 30};
                enemies[i].targetPosition = ballPosition;
                enemies[i].type = (EnemyType)GetRandomValue(0, 2);
                enemies[i].active = true;
                enemies[i].hits = 1; // Reset hits required
                enemies[i].helmet = (enemies[i].type == ENEMY_ARMORED); // Reset helmet status
                // Configure enemy characteristics based on type
                switch (enemies[i].type) {
                    case ENEMY_ORDINARY:
                        enemies[i].color = RED;
                        break;
                    case ENEMY_SPEED_DEMON:
                        enemies[i].color = YELLOW;
                        break;
                    case ENEMY_ARMORED:
                        enemies[i].color = GRAY;
                        break;
                }
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw the ball
        DrawCircleV(ballPosition, 25, RED);

        // Draw health bar
        for (int i = 0; i < health; i++) {
            DrawRectangle(20 + i * 40, 20, 30, 10, GREEN);
        }

        // Draw enemies
        for (int i = 0; i < 3; i++) {
            if (enemies[i].active) {
                DrawRectangleRec(enemies[i].rect, enemies[i].color);
                if (enemies[i].helmet) {
                    // Draw helmet for armored enemy
                    DrawCircle(enemies[i].rect.x + enemies[i].rect.width / 2, enemies[i].rect.y - 5, 10, DARKGRAY);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
