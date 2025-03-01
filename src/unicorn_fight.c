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
#include <stdio.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define ROCKS_LENGTH 300
#define PLAYER_SPEED 10
#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT 65
#define ROCK_SIZE 50
#define ROCK_INACTIVE_POS -1

#define AXIS_DEADZONE .03f

typedef struct {
    Rectangle position;
} Rock;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static int screenWidth = 1920;
static int screenHeight = 1080;
static const int deathMoveSpeed = 8;

static Rectangle player = {0, 0, PLAYER_WIDTH, PLAYER_HEIGHT};
static Rock rocks[ROCKS_LENGTH] = {0};
static int rateOfNewDeath = 20;

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;

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
    SetTargetFPS(60);
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
    rateOfNewDeath = 20;

    unicorn = LoadTexture("./resources/unicorn.png");

    player.x = screenWidth / 2;
    player.y = screenHeight / 2;

    for (int i = 0; i < ROCKS_LENGTH; i++) {

        rocks[i] = (Rock){(Rectangle){ROCK_INACTIVE_POS, ROCK_INACTIVE_POS, ROCK_SIZE, ROCK_SIZE}};
    }
}

// Update game (one frame)
void UpdateGame(void) {
    if (!gameOver) {

        if (IsKeyPressed('P') || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) pause = !pause;

        if (pause) return;

        for (int i = 0; i < ROCKS_LENGTH; i++) {
            Rock ds = rocks[i];
            if (CheckCollisionRecs(player, ds.position)) {
                gameOver = true;
                return;
            }
        }

        float x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

        if (x > AXIS_DEADZONE || x < AXIS_DEADZONE * 0) player.x += x * PLAYER_SPEED;
        if (y > AXIS_DEADZONE || y < AXIS_DEADZONE * 0) player.y += y * PLAYER_SPEED;

        // move the death squares
        for (int i = 0; i < ROCKS_LENGTH; i++) {
            Rock ds = rocks[i];
            if (ds.position.x != ROCK_INACTIVE_POS && ds.position.y != ROCK_INACTIVE_POS) {
                if (ds.position.y > screenHeight) {
                    rocks[i].position.x = ROCK_INACTIVE_POS;
                    rocks[i].position.y = ROCK_INACTIVE_POS;
                    continue;
                }

                rocks[i].position.y += deathMoveSpeed;
            }
        }

        if (rateOfNewDeath > 4 && framesCounter % 40 == 0) {
            rateOfNewDeath--;
        }

        // spawn a new death square
        // printf("%d\n", framesCounter);
        if (framesCounter % rateOfNewDeath == 0) {
            int x = GetRandomValue(0, screenWidth);

            for (int i = 0; i < ROCKS_LENGTH; i++) {
                Rock ds = rocks[i];
                if (ds.position.x == ROCK_INACTIVE_POS && ds.position.y == ROCK_INACTIVE_POS) {
                    rocks[i].position.x = x;
                    rocks[i].position.y = ROCK_INACTIVE_POS * ROCK_SIZE;

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

        DrawTexture(unicorn, player.x, player.y, WHITE);

        for (int i = 0; i < ROCKS_LENGTH; i++) {
            Rock ds = rocks[i];

            if (ds.position.x != ROCK_INACTIVE_POS) {
                DrawRectangleRec(ds.position, PINK);
            }
        }

        if (pause) DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
    } else
        DrawText("PRESS [ENTER/START] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER/START] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);

    EndDrawing();
}

// Unload game variables
void UnloadGame(void) {
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void) {
    UpdateGame();
    DrawGame();
}
