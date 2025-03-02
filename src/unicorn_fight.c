/*******************************************************************************************
 *
 *   raylib - classic game: snake
 *
 *   Sample game developed by Ian Eito, Albert Martos and Ramon Santamaria
 *
 *   This game has been created using raylib v1.3 (www.raylib.com)
 *   raylib is licensed under an unmodified zlib/libpng license (View raylib.h
 *for details)
 *
 *   Copyright (c) 2015 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define FRAME_RATE 60

// How many pixels the player can move per frame
#define PLAYER_SPEED 10
// this is the height the player image
#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT 65

#define ROCKS_LENGTH 25
#define ROCK_SIZE 50
#define ROCK_SPEED 4
#define SPAWN_RATE 80 // frames that pass until a new spawn

//this one is for the kiddo. 
#define ALLOW_LOSING false

#define AXIS_DEADZONE .04f

typedef struct {
    Vector2 position;
    Vector2 speed;
    bool active;
    bool punched;
} Rock;

typedef struct {
    Vector2 a;
    Vector2 b;
    Vector2 c;
} Triangle;

typedef struct {
    Rectangle position;
    float rotation;
    bool punch;
    Triangle fist;
} Player;

Vector2 Vector2RotateAroundAxis(Vector2 pointToRotate, Vector2 rotationPoint, float angle) {
    Vector2 translatedPoint = {pointToRotate.x - rotationPoint.x, pointToRotate.y - rotationPoint.y};
    Vector2 rotatedPoint = Vector2Rotate(translatedPoint, angle);
    Vector2 finalPoint = {rotatedPoint.x + rotationPoint.x, rotatedPoint.y + rotationPoint.y};

    return finalPoint;
}

Triangle TriangleRotateAroundAxis(Triangle t, Vector2 axis, float angle) {
    t.a = Vector2RotateAroundAxis(t.a, axis, angle);
    t.b = Vector2RotateAroundAxis(t.b, axis, angle);
    t.c = Vector2RotateAroundAxis(t.c, axis, angle);
    return t;
}

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static int screenWidth = 1920;
static int screenHeight = 1080;
static Vector2 screenMiddle = {0};

static Player player = {{0, 0, PLAYER_WIDTH, PLAYER_HEIGHT}, 0, false, (Triangle){(Vector2){0, 0}, (Vector2){0, 0}, (Vector2){0, 0}}};
static Rock rocks[ROCKS_LENGTH] = {0};

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;
static int score = 0;

static int lastPunchFrame = 0;

static Texture2D unicorn = {0};
//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);        // Initialize game
static void UpdateGame(void);      // Update game (one frame)
static void DrawGame(void);        // Draw game (one frame)
static void UnloadGame(void);      // Unload game
static void UpdateDrawFrame(void); // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Unicon Defense!");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(FRAME_RATE);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame(); // Unload loaded data (textures, sounds, models...)

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Initialize game variables
void InitGame(void) {
    framesCounter = 0;
    gameOver = false;
    pause = false;
    score = 0;

    unicorn = LoadTexture("./resources/unicorn.png");

    player.position.x = screenWidth / 2.0f;
    player.position.y = screenHeight / 2.0f;
    player.rotation = 0;
    player.punch = false;
    lastPunchFrame = 0;

    screenMiddle.x = player.position.x;
    screenMiddle.y = player.position.y;

    for (int i = 0; i < ROCKS_LENGTH; i++) {
        rocks[i] = (Rock){(Vector2){0, 0}, {0, 0}, false, false};
    }
}

// Update game (one frame)
void UpdateGame(void) {
    if (!gameOver) {

        if (IsKeyPressed('P') || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) pause = !pause;

        if (pause) return;

        for (int i = 0; i < ROCKS_LENGTH; i++) {
            Rock ds = rocks[i];
            Rectangle pos = {player.position.x - PLAYER_WIDTH / 2.0f, player.position.y - PLAYER_HEIGHT / 2.0f, player.position.width, player.position.height};
            if (ALLOW_LOSING && CheckCollisionCircleRec(ds.position, ROCK_SIZE, pos)) {
                gameOver = true;
                return;
            }
        }

        // Player Move
        float x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

        float xr = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        float yr = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

        if ((xr > AXIS_DEADZONE || xr < -AXIS_DEADZONE) && (yr > AXIS_DEADZONE || yr < -AXIS_DEADZONE)) {
            // printf("%f %f %f %f \n", xr, yr, atan2(xr, yr), atan2(xr, yr) * (180 / PI));
            player.rotation = atan2(yr, xr) * (180 / PI);
        }

        if (x > AXIS_DEADZONE || x < -AXIS_DEADZONE) player.position.x += x * PLAYER_SPEED;
        if (y > AXIS_DEADZONE || y < -AXIS_DEADZONE) player.position.y += y * PLAYER_SPEED;

        // Player Actions
        // printf("%d %d\n", lastPunchFrame + FRAME_RATE * 10, framesCounter);
        if (!player.punch || (lastPunchFrame + 10) < framesCounter) {
            player.punch = false;
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) {
                player.punch = true;
                lastPunchFrame = framesCounter;
            }
        }

        // calculate the position of the player's punch
        if (player.punch) {
            player.fist.a.x = player.position.x + PLAYER_WIDTH / 2.0f;
            player.fist.a.y = player.position.y;
            player.fist.b.x = player.fist.a.x + PLAYER_WIDTH * 2;
            player.fist.b.y = player.fist.a.y + PLAYER_HEIGHT / 2.0f * 2;
            player.fist.c.x = player.fist.a.x + PLAYER_WIDTH * 2;
            player.fist.c.y = player.fist.a.y + (PLAYER_HEIGHT / 2.0f * -1) * 2;

            Vector2 rotationPoint = {player.position.x, player.position.y};
            player.fist = TriangleRotateAroundAxis(player.fist, rotationPoint, player.rotation * DEG2RAD);
        }

        // Rocks Move
        for (int i = 0; i < ROCKS_LENGTH; i++) {
            Rock ds = rocks[i];
            if (ds.active) {
                // despawn the rock
                if ((ds.speed.y > 0 && ds.position.y > screenHeight) || (ds.speed.y < 0 && ds.position.y < 0) || (ds.speed.x > 0 && ds.position.x > screenWidth) ||
                    (ds.speed.x < 0 && ds.position.x < 0)) {
                    rocks[i].active = false;
                    rocks[i].punched = false;
                    continue;
                }

                if (player.punch && ds.punched == false 
                    && (CheckCollisionCircleLine(ds.position, ROCK_SIZE, player.fist.b, player.fist.c)
                    || CheckCollisionCircleLine(ds.position, ROCK_SIZE, player.fist.a, player.fist.b)
                    || CheckCollisionCircleLine(ds.position, ROCK_SIZE, player.fist.a, player.fist.c))) {
                    rocks[i].speed.x = ROCK_SPEED * 4 * cos(atan2(yr, xr));
                    rocks[i].speed.y = ROCK_SPEED * 4 * sin(atan2(yr, xr));
                    rocks[i].punched = true;
                    score++;
                }

                rocks[i].position.y += rocks[i].speed.y;
                rocks[i].position.x += rocks[i].speed.x;
            }
        }

        // Rock Spawn
        if (framesCounter % SPAWN_RATE == 0) {
            int x = GetRandomValue(0, screenWidth);
            int y = GetRandomValue(0, screenHeight);

            // Get some variance in where the projectile is going to go.
            int box = screenWidth / 4;
            int varianceX = GetRandomValue(box * -1, box);
            int varianceY = GetRandomValue(box * -1, box);

            // target - projectile
            int deltaX = (screenMiddle.x + varianceX) - x;
            int deltaY = (screenMiddle.y + varianceY) - y;

            float angle = atan2(deltaY, deltaX);

            // move the rock off screen
            while ((x > 0 && x < screenWidth) && (y > 0 && y < screenHeight)) {
                x -= deltaX;
                y -= deltaY;
            }

            for (int i = 0; i < ROCKS_LENGTH; i++) {
                // find some inactive rock
                if (!rocks[i].active) {
                    rocks[i].position.x = x;
                    rocks[i].position.y = y;
                    rocks[i].active = true;

                    // set the rock to go in the direction of the middle of the screen
                    rocks[i].speed.x = ROCK_SPEED * cos(angle);
                    rocks[i].speed.y = ROCK_SPEED * sin(angle);

                    // printf("New Death! %d %f, %f\n", i, ds.x, ds.y);
                    break;
                }
            }
        }

        if (!pause) {
            framesCounter++;
        }

    } else {
        if (IsKeyPressed(KEY_ENTER) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
            InitGame();
            gameOver = false;
        }
    }
}

// Draw game (one frame)
void DrawGame(void) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver) {

        for (int i = 0; i < ROCKS_LENGTH; i++) {
            Rock ds = rocks[i];

            if (ds.active) {
                DrawCircleV(ds.position, ROCK_SIZE, PINK);
            }
        }

        Vector2 origin = {PLAYER_WIDTH / 2.0f, PLAYER_HEIGHT / 2.0f};
        Rectangle source = {0, 0, PLAYER_WIDTH, PLAYER_HEIGHT};
        DrawTexturePro(unicorn, source, player.position, origin, player.rotation, WHITE);
        if (player.punch) {
            DrawTriangle(player.fist.a, player.fist.b, player.fist.c, RED);
        }

        char s[5] = "0000\0";

        sprintf(s, "%0*d", 4, score);

        DrawText(s, screenWidth - MeasureText(s, 40) - 4, 2, 40, GRAY);

        if (pause) DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
    } else
        DrawText("PRESS [ENTER/START] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER/START] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);

    EndDrawing();
}

// Unload game variables
void UnloadGame(void) {
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
    UnloadTexture(unicorn);
}

// Update and Draw (one frame)
void UpdateDrawFrame(void) {
    UpdateGame();
    DrawGame();
}
