#include "raylib.h"

int main(void) {
  InitWindow(800, 450, "Space Invaders");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("It works with submodule!", 250, 200, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
