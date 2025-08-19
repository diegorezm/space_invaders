#include "raylib.h"

static const int SCREEN_WIDTH = 1200;
static const int SCREEN_HEIGTH = 800;

typedef struct {
  int x;
  int y;
  int speedX;
  int speedY;
  int radius;
} Ball;

Ball Ball_Create(int x, int y, int radius, int speedX, int speedY) {
  Ball b = {x, y, speedX, speedY, radius};
  return b;
}

void Ball_Update(Ball *ball, int screenWidth, int screenHeight) {
  ball->x += ball->speedX;
  ball->y += ball->speedY;

  if ((ball->x + ball->radius) >= screenWidth ||
      (ball->x - ball->radius) <= 0) {
    ball->speedX *= -1;
  }
  if ((ball->y + ball->radius) >= screenHeight ||
      (ball->y - ball->radius) <= 0) {
    ball->speedY *= -1;
  }
}

void Ball_Draw(const Ball *ball) {
  DrawCircle(ball->x, ball->y, ball->radius, RED);
}

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGTH, "Space Invaders");
  SetTargetFPS(60);
  Ball ball;

  ball = Ball_Create(0, 0, 12, 12, 12);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    int x = MeasureText("Hello world!", 20);
    Ball_Draw(&ball);
    DrawText("Hello world!", SCREEN_HEIGTH / 2, x, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
