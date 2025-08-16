#include "raylib.h"

static const int SCREEN_WIDTH = 1200;
static const int SCREEN_HEIGTH = 800;

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGTH, "Space Invaders");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    int x = MeasureText("Hello world!", 20);
    DrawText("Hello world!", SCREEN_HEIGTH / 2, x, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
