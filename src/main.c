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

// ALIEN
//
typedef struct {
  int type;
  Vector2 pos;
} Alien;

static Texture2D AlienImages[3];

typedef struct {
  Alien *items;
  int count;
  int capacity;
} Aliens;

Alien Alien_Create(int type, Vector2 pos) {
  Alien alien = {.type = type, .pos = pos};
  return alien;
}

Aliens Aliens_Create() {
  Aliens aliens = {0};
  int cell_size = 55;
  int row_size = 55;

  for (size_t row = 0; row < 5; row++) {
    for (size_t col = 0; col < 11; col++) {
      int alienType;
      if (row == 0) {
        alienType = 3;
      } else if (row == 1 || row == 2) {
        alienType = 2;
      } else {
        alienType = 1;
      }

      float x = 75 + col * row_size;
      float y = 110 + row * cell_size;

      Alien alien = Alien_Create(alienType, (Vector2){x, y});
      da_append(aliens, alien);
    }
  }
  return aliens;
}

void Alien_Shoot(Aliens *aliens, Lasers *alienLasers, double alienShootInterval,
                 double *timeLastAlienFired) {
  double currentTime = GetTime();
  if (currentTime - *timeLastAlienFired >= alienShootInterval &&
      aliens->count > 0) {

    *timeLastAlienFired = currentTime;
    int randomIndex = GetRandomValue(0, aliens->count - 1);
    Alien *alien = &aliens->items[randomIndex];

    Laser laser = Laser_Create(
        (Vector2){alien->pos.x + AlienImages[alien->type - 1].width / 2,
                  alien->pos.y + AlienImages[alien->type - 1].height},
        6);

    da_append(*alienLasers, laser);
  }
}

void Aliens_Load_images() {
  AlienImages[0] = LoadTexture("assets/alien_1.png");
  AlienImages[1] = LoadTexture("assets/alien_2.png");
  AlienImages[2] = LoadTexture("assets/alien_3.png");
}

void Aliens_Unload_images() {
  for (size_t i = 0; i < 3; i++) {
    UnloadTexture(AlienImages[i]);
  }
}

void Alien_Update(Alien *alien, int direction) { alien->pos.x += direction; }

void Alien_Draw(Alien *alien) {
  DrawTextureV(AlienImages[alien->type - 1], alien->pos, WHITE);
}

void Aliens_Move_down(Aliens *aliens, int distance) {
  for (size_t i = 0; i < aliens->count; ++i) {
    Alien *alien = &aliens->items[i];
    alien->pos.y += distance;
  }
}

void Aliens_Move(Aliens *aliens, int *direction) {
  for (size_t i = 0; i < aliens->count; ++i) {
    Alien *alien = &aliens->items[i];
    if (alien->pos.x + AlienImages[alien->type - 1].width > GetScreenWidth()) {
      *direction = -1;
      Aliens_Move_down(aliens, 4);
    }

    if (alien->pos.x <= 0) {
      *direction = 1;
      Aliens_Move_down(aliens, 4);
    }
    Alien_Update(alien, *direction);
  }
}

void Aliens_Draw(Aliens *aliens) {
  for (size_t i = 0; i < aliens->count; ++i) {
    Alien *alien = &aliens->items[i];
    Alien_Draw(alien);
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
  Aliens_Load_images();

  Lasers alienLasers = {0};
  Obstacle *obstacles = Obstacles_Create();
  Aliens aliens = Aliens_Create();

  Spaceship sp = Spaceship_Create();

  static double lastPlayerFireTime = 0;
  static double lastAlienFireTime = 0;
  static double alienShootInterval = 0.50;

  int aliensDirection = 1;

  while (!WindowShouldClose()) {
    double now = GetTime();
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
      Spaceship_Move_right(&sp);
    } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
      Spaceship_Move_left(&sp);
    } else if (IsKeyDown(KEY_SPACE) && now - lastPlayerFireTime > 0.25) {
      Spaceship_Fire(&sp);
      lastPlayerFireTime = now;
    }

    Lasers_Update(&sp.lasers);
    Aliens_Move(&aliens, &aliensDirection);
    Alien_Shoot(&aliens, &alienLasers, alienShootInterval, &lastAlienFireTime);
    Lasers_Update(&alienLasers);

    BeginDrawing();
    ClearBackground(grey);
    Spaceship_Draw(&sp);
    Aliens_Draw(&aliens);
    Obstacles_Draw(obstacles, 4);
    Lasers_Draw(&sp.lasers);
    Lasers_Draw(&alienLasers);
    EndDrawing();
  }

  Spaceship_Unload(&sp);
  Aliens_Unload_images();
  CloseWindow();
  return 0;
}
