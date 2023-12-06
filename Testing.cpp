#include "raylib.h"
#include <raymath.h>
#include <vector>
#include <string>

const float FPS = 60;
const float TIMESTEP = 1.0f / FPS;
const float FRICTION = 0.99f;

float speedIncrement = 0.1f;
bool gameOverSoundPlayed = false;
int score = 0;

Sound SFX6;

enum EnemyType {
    GRUNT,
    SPRINTER,
    HEAVY,
};

enum PowerUpType {
    HEAL_BASE,
    UP_PLAYER_SIZE,
    INCREASE_SCORE1,
    SLOW,
};

struct PowerUp {
    PowerUpType type;
    Vector2 position;
    float duration;
    float slowFactor;
};

std::vector<PowerUp> powerUps;

PowerUp createPowerUp(const int screenWidth, const int screenHeight) {
    PowerUp newPowerUp;
    newPowerUp.type = static_cast<PowerUpType>(GetRandomValue(0, 3));
    newPowerUp.position = {(float)GetRandomValue(30, screenWidth - 30), (float)GetRandomValue(30, screenHeight - 30)};
    newPowerUp.duration = 5.0f; // Adjust duration as needed

    if (newPowerUp.type == SLOW) {
        newPowerUp.slowFactor = 0.5f; // Adjust the slow factor as needed
    } else {
        newPowerUp.slowFactor = 1.0f; // Default value for other power-ups
    }
    return newPowerUp;
}

struct Base { //Base Health 
    int health;
    float radius;
    Vector2 basePos;
};

struct Player {
    bool isDragging;
    float radius;
    float originalRadius;
    Vector2 velocity;
    Vector2 playerPos;
    Vector2 acceleration;
};

struct AnimationFrame {
    Rectangle frameRec;
    Vector2 position;
    float updateTime;
    float runningTime;
};

struct Animation {
    Texture2D spriteSheet;
    AnimationFrame* frames;
    int frameCount;
    int currentFrame;
};

struct Enemy {
    int enemyHealth;
    EnemyType type;
    Color color;
    Vector2 target;
    Rectangle rect;
    float speed;
    float hitCooldown;
    Animation animation;
};

Enemy createEnemy (const int screenWidth, const int screenHeight, Vector2 target, Animation& gruntAnim, Animation& sprinterAnim, Animation& heavyAnim) {
    const int gruntSpawn = 60;
    const int sprinterSpawn = 30;
    const int heavySpawn = 10;

    int randomValue = GetRandomValue(1, 100);

    EnemyType type;
    if (randomValue <= gruntSpawn) {
        type = GRUNT;
    }
    else if (randomValue <= gruntSpawn + sprinterSpawn) {
        type = SPRINTER;
    }
    else {
        type = HEAVY;
    }
    
    float safeDistance = 400.0f;
    Vector2 spawnPos;

    do {
        spawnPos = (Vector2){(float)GetRandomValue(30, screenWidth - 30), (float)GetRandomValue(30, screenHeight - 30)};
    } while (Vector2Distance(spawnPos, target) < safeDistance);

    Enemy newEnemy;
    newEnemy.rect = (Rectangle){spawnPos.x, spawnPos.y, 30, 30};
    newEnemy.type = type;
    newEnemy.enemyHealth = 1;
    newEnemy.target = target;
    newEnemy.hitCooldown = 0.0f;
    switch (type) {
        case GRUNT:
            newEnemy.animation = gruntAnim;
            newEnemy.color = RED;
            newEnemy.speed = 0.75f;
            break;
        case SPRINTER:
            newEnemy.animation = sprinterAnim;
            newEnemy.color = YELLOW;
            newEnemy.speed = 1.0f;
            break;
        case HEAVY:
            newEnemy.animation = heavyAnim;
            newEnemy.color = GRAY;
            newEnemy.enemyHealth = 2;
            newEnemy.speed = 0.25f;
            break;
    }
    return newEnemy;
}

void updateEnemy (Enemy &enemy, Vector2 target, float deltaTime) {
    Vector2 direction = Vector2Normalize(Vector2Subtract(target, (Vector2){enemy.rect.x, enemy.rect.y}));
    enemy.speed += speedIncrement * deltaTime;
    enemy.rect.x += direction.x * enemy.speed;
    enemy.rect.y += direction.y * enemy.speed;
}

void health (Enemy &enemy, const Player &player, Base &base, float deltaTime) {
    Rectangle playerRect = {(player.playerPos.x - player.radius), (player.playerPos.y - player.radius), player.radius * 2, player.radius * 2};

    if (CheckCollisionCircleRec(base.basePos, base.radius, enemy.rect)) {
        PlaySound(SFX6);
        enemy.enemyHealth = 0;
        base.health--;
    } 
    else if (enemy.hitCooldown <= 0.0f && CheckCollisionRecs(playerRect, enemy.rect)) {
        enemy.enemyHealth--;
        enemy.hitCooldown = 0.5f;
    }

    if (enemy.hitCooldown > 0.0f) {
        enemy.hitCooldown -= deltaTime;
    }

}

void applyPowerUp(Player& player, Base& base, std::vector<Enemy>& enemies, const PowerUp& powerUp) {
    switch (powerUp.type) {
        case HEAL_BASE:
            base.health = 3;
            break;
        case UP_PLAYER_SIZE:
            player.radius *= 1.1f;
            break;
        case INCREASE_SCORE1:
            score += 100;
            break;
        case SLOW:
            for (Enemy& enemy : enemies) {
                enemy.speed *= powerUp.slowFactor;
            }
            break;
    }
}

void InitAnimation(Animation* anim, Texture2D spriteSheet, int frameCount, int frameWidth, int frameHeight) {
    anim->spriteSheet = spriteSheet;
    anim->frames = (AnimationFrame*)malloc(frameCount * sizeof(AnimationFrame));
    anim->frameCount = frameCount;
    anim->currentFrame = 0;

    for (int i = 0; i < frameCount; ++i) {
        anim->frames[i].frameRec = (Rectangle){static_cast<float>(frameWidth * i), 0.0f, static_cast<float>(frameWidth), static_cast<float>(frameHeight)};
        anim->frames[i].updateTime = 1.0f / 12.0f; // Example frame rate
        anim->frames[i].runningTime = 0.0f;
    }
}

void UpdateAnimation(Animation* anim, Enemy& enemy, float deltaTime) {
    anim->frames[anim->currentFrame].runningTime += deltaTime;

    if (anim->frames[anim->currentFrame].runningTime >= anim->frames[anim->currentFrame].updateTime) {
        anim->frames[anim->currentFrame].runningTime = 0.0f;

        if (enemy.type == HEAVY) {
            if (enemy.enemyHealth == 2) {
                // Limit to the first two frames
                anim->currentFrame = (anim->currentFrame + 1) % 2;
            } else if (enemy.enemyHealth == 1) {
                
                // Use frames 3 and 4 (index 2 and 3)
                anim->currentFrame = 2 + ((anim->currentFrame - 2 + 1) % 2);
            }
        } else {
            anim->currentFrame = (anim->currentFrame + 1) % anim->frameCount;
        }
    }
}


void drawEnemy(const Enemy &enemy) {
    AnimationFrame frame = enemy.animation.frames[enemy.animation.currentFrame];
    
    Rectangle destRec = {enemy.rect.x, enemy.rect.y, enemy.rect.width, enemy.rect.height};

    DrawTexturePro(
        enemy.animation.spriteSheet, 
        frame.frameRec, 
        destRec, 
        Vector2{0, 0},
        0.0f,
        WHITE
    );
}
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

void DrawHealthBar(int x, int y, int width, int height, int currentHealth, int maxHealth, Color barColor, Color borderColor) {
    // Draw the border
    DrawRectangle(x - 2, y - 2, width + 4, height + 4, borderColor);

    // Calculate the percentage of health remaining
    float healthPercentage = (float)currentHealth / maxHealth;

    // Calculate the width of the colored portion of the health bar
    int barWidth = (int)(width * healthPercentage);

    // Draw the colored portion of the health bar
    DrawRectangle(x, y, barWidth, height, barColor);
}

int main() {
    InitAudioDevice();

    Sound BGM = LoadSound("BGM.ogg");
    Sound SFX1 = LoadSound("ShootSFX.ogg");
    Sound SFX2 = LoadSound("DieSFX.ogg");
    Sound SFX3 = LoadSound("PowerupSFX.ogg");
    Sound SFX4 = LoadSound("WallHitSFX.ogg");
    Sound SFX5 = LoadSound("KillSFX.ogg");
    SFX6 = LoadSound("HeavyDMGSFX.ogg");

    SetSoundVolume(BGM, 0.7f);
    SetSoundVolume(SFX3, 0.5f);

    Base playerBase;
    Player player1;
    Animation sprinterAnim, gruntAnim, heavyAnim;

    int highscore = 0;
    float accumulator = 0;
    float gameOverDelay = 1.0f;
    bool isGameOver = false;
    const float maxAccumulator = 0.1f;
    const int screenWidth = 800;
    const int screenHeight = 600;


    InitWindow(screenWidth, screenHeight, "PLAYER MOVING");
    SetTargetFPS(60); // Set the target frame rate

    Texture2D sprinterSprite = LoadTexture("SPRINTER.png");
    Texture2D gruntSprite = LoadTexture("GRUNT.png");
    Texture2D heavySprite = LoadTexture("HEAVY.png");


    InitAnimation(&sprinterAnim, sprinterSprite, 3, 16, 18);
    InitAnimation(&gruntAnim, gruntSprite, 2, 16, 17);
    InitAnimation(&heavyAnim, heavySprite, 4, 16, 17); 

    playerBase.health = 3;
    playerBase.radius = 50.0f;
    player1.radius = 20.0f;
    player1.isDragging = false;
    player1.playerPos = {screenWidth / 2.0f, screenHeight / 2.0f};;
    player1.acceleration = {0,0};

    Rectangle restartButton = {screenWidth / 2 - 50, screenHeight / 2 + 60, 100, 30};

    playerBase.basePos = {screenWidth / 2, screenHeight / 2};
    player1.velocity = {0, 0};

    Vector2 mouse_drag_start = Vector2Zero();
    

    std::vector<Enemy> enemies;
    for (int i = 0; i < 3; ++i) {
        enemies.push_back(createEnemy(screenWidth, screenHeight, playerBase.basePos, gruntAnim, sprinterAnim, heavyAnim));
    }

    float spawnTimer = 0.0f;
    const float spawnInterval = 10.0f;

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        spawnTimer += delta_time;

        if(!IsSoundPlaying(BGM)){
            PlaySound(BGM);
        }
        
        if (playerBase.health <= 0) {
            isGameOver = true;
            if(!gameOverSoundPlayed){
                PlaySound(SFX2);
                gameOverSoundPlayed = true;
            }
            gameOverDelay -= delta_time;
        }

        if (!isGameOver) {
            Vector2 mouse_position = GetMousePosition();
            Vector2 cue_stick_force = Vector2Zero();

            if (player1.playerPos.x - player1.radius < 0 || player1.playerPos.x + player1.radius > screenWidth) {
                PlaySound(SFX4);
                player1.velocity.x *= -1;
                player1.playerPos.x = Clamp(player1.playerPos.x, player1.radius, screenWidth - player1.radius);
            }

            if (player1.playerPos.y - player1.radius < 0 || player1.playerPos.y + player1.radius > screenHeight) {
                PlaySound(SFX4);
                player1.velocity.y *= -1;
                player1.playerPos.y = Clamp(player1.playerPos.y, player1.radius, screenHeight - player1.radius);
            }

            if (player1.playerPos.x - player1.radius < 0 || player1.playerPos.x + player1.radius > screenWidth ||
                player1.playerPos.y - player1.radius < 0 || player1.playerPos.y + player1.radius > screenHeight) {

                player1.playerPos = {400,300};
            }

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePosition = GetMousePosition();
                if (!player1.isDragging) {
                    mouse_drag_start = mousePosition;
                    player1.isDragging = true;
                }
            }

            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && player1.isDragging) {
                Vector2 mouse_drag_end = GetMousePosition();
                player1.velocity = Vector2Subtract(mouse_drag_start, mouse_drag_end);
                player1.isDragging = false;
                PlaySound(SFX1);
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

            if (spawnTimer >= spawnInterval) {
                spawnTimer = 0.0f;
                for (int i = 0; i < 1; i++) {
                    enemies.push_back(createEnemy(screenWidth, screenHeight, playerBase.basePos, gruntAnim, sprinterAnim, heavyAnim));
                }
            }

            for (Enemy &enemy : enemies) {
                UpdateAnimation(&enemy.animation, enemy, delta_time);
                updateEnemy(enemy, playerBase.basePos, delta_time);
                health(enemy, player1, playerBase, delta_time);

                if (enemy.enemyHealth <= 0) {
                    PlaySound(SFX5);
                    enemy = createEnemy(screenWidth, screenHeight, playerBase.basePos, gruntAnim, sprinterAnim, heavyAnim);
                    score += 10;
                }
            }

            if (score > highscore) {
                highscore = score;
            }

            if (GetRandomValue(0, 1000) < 5) { // Adjust the probability as needed
                powerUps.push_back(createPowerUp(screenWidth, screenHeight));
            }

            for (auto it = powerUps.begin(); it != powerUps.end(); /* no increment here */) {
                if (CheckCollisionCircleRec(player1.playerPos, player1.radius, {it->position.x, it->position.y, 10, 10})) {
                    PlaySound(SFX3);
                    applyPowerUp(player1, playerBase, enemies, *it);
                    it = powerUps.erase(it);
                } else {
                    it++;
                }
            }

            for (auto it = powerUps.begin(); it != powerUps.end(); /* no increment here */) {
                it->duration -= delta_time;
                if (it->duration <= 0.0f) {
                    it = powerUps.erase(it);
                } else {
                    it++;
                }
            }

            for (const PowerUp &powerUp : powerUps) {
                // Draw different shapes or symbols based on the power-up type
                switch (powerUp.type) {
                    case HEAL_BASE:
                        DrawCircleV(powerUp.position, 10, GREEN); // Green circle for heal
                        break;
                    case UP_PLAYER_SIZE:
                        if (player1.radius <= 40.0f){
                            DrawRectangleV(Vector2{ powerUp.position.x - 5, powerUp.position.y - 5 }, Vector2{ 20, 20 }, BLUE);
                        }
                        break;
                    case INCREASE_SCORE1:
                        DrawTriangle({ powerUp.position.x - 5, powerUp.position.y + 5 }, { powerUp.position.x + 5, powerUp.position.y + 5 }, { powerUp.position.x, powerUp.position.y - 5 }, PURPLE); // Purple triangle for immunity
                        break;
                    case SLOW:
                        DrawLineV({ powerUp.position.x - 5, powerUp.position.y - 5 }, { powerUp.position.x + 5, powerUp.position.y + 5 }, ORANGE); 
                        DrawLineV({ powerUp.position.x - 5, powerUp.position.y + 5 }, { powerUp.position.x + 5, powerUp.position.y - 5 }, ORANGE);
                        break;
                }
            }
        }        

        BeginDrawing();
        ClearBackground(BLACK);

        // DrawCircle(screenWidth / 2, screenHeight / 2, 75.0f, YELLOW); // Base
        DrawCircleV(player1.playerPos, player1.radius, RED);

        if (playerBase.health > 0) {
            DrawCircleLines(playerBase.basePos.x, playerBase.basePos.y, playerBase.radius, RED);
        
            DrawHealthBar(playerBase.basePos.x - playerBase.radius, playerBase.basePos.y + playerBase.radius + 10, playerBase.radius * 2, 10, playerBase.health, 3, RED, BLACK);
        }

        // if (player1.isDragging) {
        //     //Vector2 mouse_drag_vector = Vector2Scale(shot_dir, shot_vector_distance);
        //     //DrawLineEx(mouse_drag_start, mouse_drag_end, 2, RED);
        // }

        for (const Enemy &enemy : enemies) {
            if (enemy.enemyHealth > 0) {
                drawEnemy(enemy);
            }
        }
        
        DrawText(TextFormat("Score: %d", score), 30, 30, 20, WHITE);

        if (isGameOver && gameOverDelay <= 0) {
            // Draw game over message
            DrawText("Game Over", screenWidth / 2 - MeasureText("Game Over", 20) / 2, screenHeight / 2, 20, WHITE);
            DrawText(TextFormat("High Score: %d", highscore), screenWidth / 2 - MeasureText("High Score: ", 20) / 2, screenHeight / 2 + 20, 20, WHITE);
            DrawRectangleRec(restartButton, GRAY);
            DrawText("Restart", restartButton.x + 10, restartButton.y + 5, 20, BLACK);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePosition = GetMousePosition();
                if (CheckCollisionPointRec(mousePosition, restartButton)) {
                    score = 0;
                    playerBase.health = 3;
                    player1.playerPos = {screenWidth / 2.0f, screenHeight / 2.0f};
                    isGameOver = false;
                    gameOverDelay = 1.0f;
                    spawnTimer = 0.0f;
                    player1.velocity = {0, 0};
                    player1.acceleration = {0, 0};
                    player1.radius = 20.0f;
                    gameOverSoundPlayed = false;
                    enemies.clear();
                    for (int i = 0; i < 3; ++i) {
                        enemies.push_back(createEnemy(screenWidth, screenHeight, playerBase.basePos, gruntAnim, sprinterAnim, heavyAnim));
                    }
                }
            }
        }

        EndDrawing();
    }

    UnloadTexture(sprinterSprite);
    UnloadTexture(gruntSprite);
    UnloadTexture(heavySprite);
    free(sprinterAnim.frames);
    free(gruntAnim.frames);
    free(heavyAnim.frames);

    UnloadSound(BGM);
    UnloadSound(SFX1);
    UnloadSound(SFX2);
    UnloadSound(SFX3);
    UnloadSound(SFX4);
    UnloadSound(SFX5);
    UnloadSound(SFX6);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}
