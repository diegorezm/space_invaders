#include "raylib.h"
#include <stdio.h>

// SPACESHIP
//
typedef struct {
  Texture2D image;
  Vector2 pos;
  int speed;
} Spaceship;

Spaceship Spaceship_Create() {
  Texture2D image = LoadTexture("assets/spaceship.png");

  Vector2 pos = {.x = (GetScreenWidth() - image.width) / 2,
                 .y = GetScreenHeight() - image.height};

  int speed = 7;

  Spaceship spaceship = {image, pos, speed};
  return spaceship;
}

void Spaceship_Draw(Spaceship *sp) {
  DrawTextureV(sp->image, sp->pos, RAYWHITE);
}
void Spaceship_Move_right(Spaceship *sp) {
  sp->pos.x += sp->speed;

  if (sp->pos.x > GetScreenWidth() - sp->image.width) {
    sp->pos.x = GetScreenWidth() - sp->image.width;
  }
}
void Spaceship_Move_left(Spaceship *sp) {
  sp->pos.x -= sp->speed;

  if (sp->pos.x < 0) {
    sp->pos.x = 0;
  }
}

void Spaceship_Fire(Spaceship *sp) {}
void Spaceship_Unload(Spaceship *sp) { UnloadTexture(sp->image); }

// Laser
//
typedef struct {
  Vector2 pos;
  int speed;
  bool isActive;
} Laser;

Laser Laser_Create(Vector2 pos, int speed) {
  Laser laser = {pos, speed, true};
  return laser;
}

void Laser_Update(Laser *laser) {
  laser->pos.y += laser->speed;
  if (laser->isActive) {
    if (laser->pos.y > GetScreenHeight() || laser->pos.y < 0) {
      laser->isActive = false;
    }
  }
}
void Laser_Draw(Laser *laser) {
  if (laser->isActive) {
    Color laserColor = {243, 216, 63, 255};
    DrawRectangle(laser->pos.x, laser->pos.y, 4, 15, laserColor);
  }
}

int main(void) {
  Color grey = {29, 29, 27, 255};
  int screenWidth = 750;
  int screenHeigth = 700;

  InitWindow(screenWidth, screenHeigth, "Space Invaders");

  Spaceship sp = Spaceship_Create();
  Laser laser = Laser_Create((Vector2){.x = 100, .y = 100}, 7);

  SetTargetFPS(60);

  while (!WindowShouldClose()) {

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
      Spaceship_Move_right(&sp);
    } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
      Spaceship_Move_left(&sp);
    }

    Laser_Update(&laser);

    BeginDrawing();
    ClearBackground(grey);
    Spaceship_Draw(&sp);
    Laser_Draw(&laser);
    EndDrawing();
  }
  Spaceship_Unload(&sp);
  CloseWindow();
  return 0;
}
