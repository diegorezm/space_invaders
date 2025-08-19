#include "raylib.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OBSTACLE_GRID_ROWS 13
#define OBSTACLE_GRID_COLS 23

static const int OBSTACLE_GRID[13][23] = {
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}};

#define da_append(da, x)                                                       \
  do {                                                                         \
    if ((da).count >= (da).capacity) {                                         \
      if ((da).capacity == 0) {                                                \
        (da).capacity = 256;                                                   \
      } else {                                                                 \
        (da).capacity *= 2;                                                    \
      }                                                                        \
      (da).items = realloc((da).items, (da).capacity * sizeof(*(da).items));   \
    }                                                                          \
    (da).items[(da).count++] = (x);                                            \
  } while (0)

#define da_remove_at(da, index)                                                \
  do {                                                                         \
    if ((index) >= 0 && (index) < (da).count) {                                \
      (da).items[index] = (da).items[(da).count - 1];                          \
      (da).count--;                                                            \
    }                                                                          \
  } while (0)

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
    DrawRectangleV(laser->pos, (Vector2){4, 15}, laserColor);
  }
}

// Lasers list
typedef struct {
  Laser *items;
  int count;
  int capacity;
} Lasers;

void Lasers_Remove_inactive(Lasers *lasers) {
  for (int i = 0; i < lasers->count; i++) {
    Laser laser = lasers->items[i];
    if (!laser.isActive) {
      da_remove_at(*lasers, i);
    }
  }
}

void Lasers_Draw(Lasers *lasers) {
  for (int i = 0; i < lasers->count; ++i) {
    Laser_Draw(&lasers->items[i]);
  }
}

void Lasers_Update(Lasers *lasers) {
  for (int i = 0; i < lasers->count; i++) {
    Laser_Update(&lasers->items[i]);
  }
  Lasers_Remove_inactive(lasers);
}

// BLOCK
//
typedef struct {
  Vector2 pos;
} Block;

typedef struct {
  Block *items;
  int count;
  int capacity;
} Blocks;

Block Block_Create(Vector2 pos) {
  Block block = {pos};
  return block;
}

void Block_Draw(Block *block) {
  Color blockColor = {243, 216, 63, 255};
  DrawRectangle(block->pos.x, block->pos.y, 3, 3, blockColor);
}

// OBSTACLE
//
typedef struct {
  Vector2 pos;
  int grid[OBSTACLE_GRID_ROWS][OBSTACLE_GRID_COLS];
  Blocks blocks;
} Obstacle;

Obstacle Obstacle_Create(Vector2 pos) {
  // grid that represents the obstacle

  Blocks blocks = {0};
  Obstacle obstacle = {0};
  obstacle.pos = pos;

  memcpy(obstacle.grid, OBSTACLE_GRID, sizeof(OBSTACLE_GRID));

  // positioning blocks in the correct grid space
  for (size_t row = 0; row < OBSTACLE_GRID_ROWS; ++row) {
    for (size_t col = 0; col < OBSTACLE_GRID_COLS; ++col) {
      if (OBSTACLE_GRID[row][col] == 1) {
        float pos_x = obstacle.pos.x + col * 3;
        float pos_y = obstacle.pos.y + row * 3;

        Block block = Block_Create((Vector2){pos_x, pos_y});
        da_append(blocks, block);
      }
    }
  }

  obstacle.blocks = blocks;

  return obstacle;
}

void Obstacle_Draw(Obstacle *obstacle) {
  for (size_t i = 0; i < obstacle->blocks.count; ++i) {
    Block_Draw(&obstacle->blocks.items[i]);
  }
}

Obstacle *Obstacles_Create() {
  Obstacle *obstacles = malloc(4 * sizeof(Obstacle));
  int obstacleWidth = sizeof(OBSTACLE_GRID[0]) * 3;
  float gap = (GetScreenWidth() - (4 * obstacleWidth)) / 5;
  for (size_t i = 0; i < 4; i++) {
    float offsetX = (i + 1) * gap + i * obstacleWidth;
    obstacles[i] = Obstacle_Create((Vector2){offsetX, GetScreenHeight() - 100});
  }
  return obstacles;
}

void Obstacles_Draw(Obstacle *obstacles, size_t count) {
  for (size_t i = 0; i < count; i++) {
    Obstacle_Draw(&obstacles[i]);
  }
}

// SPACESHIP
//
typedef struct {
  Texture2D image;
  Vector2 pos;
  int speed;
  Lasers lasers;
} Spaceship;

Spaceship Spaceship_Create() {
  Texture2D image = LoadTexture("assets/spaceship.png");

  Vector2 pos = {.x = (GetScreenWidth() - image.width) / 2,
                 .y = GetScreenHeight() - image.height};

  int speed = 7;
  Lasers lasers = {0};

  Spaceship spaceship = {image, pos, speed, lasers};
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

void Spaceship_Fire(Spaceship *sp) {
  Laser laser = {
      (Vector2){.x = sp->pos.x + sp->image.width / 2 - 2, .y = sp->pos.y - 15},
      -6, true};

  da_append(sp->lasers, laser);
}
void Spaceship_Unload(Spaceship *sp) { UnloadTexture(sp->image); }

int main(void) {
  Color grey = {29, 29, 27, 255};
  int screenWidth = 750;
  int screenHeigth = 700;

  InitWindow(screenWidth, screenHeigth, "Space Invaders");
  SetTargetFPS(60);

  Obstacle *obstacles = Obstacles_Create();

  Spaceship sp = Spaceship_Create();

  static double lastFireTime = 0;

  while (!WindowShouldClose()) {
    double now = GetTime();
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
      Spaceship_Move_right(&sp);
    } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
      Spaceship_Move_left(&sp);
    } else if (IsKeyDown(KEY_SPACE) && now - lastFireTime > 0.25) {
      Spaceship_Fire(&sp);
      lastFireTime = now;
    }

    Lasers_Update(&sp.lasers);

    BeginDrawing();
    ClearBackground(grey);
    Spaceship_Draw(&sp);
    Obstacles_Draw(obstacles, 4);
    Lasers_Draw(&sp.lasers);
    EndDrawing();
  }
  Spaceship_Unload(&sp);
  CloseWindow();
  return 0;
}
