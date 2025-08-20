#include "raylib.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define da_clear(da)                                                           \
  do {                                                                         \
    (da).count = 0;                                                            \
  } while (0)

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

// GAME STATE
//
typedef struct {
  unsigned int score;
  unsigned int lives;
  unsigned int level;
  bool isGameOver;
} GameState;

static GameState game = {
    .lives = 3, .score = 0, .level = 1, .isGameOver = false};

// Laser
//
typedef struct {
  Vector2 pos;
  int speed;
  bool isActive;
  Color color;
} Laser;

typedef struct {
  Laser *items;
  int count;
  int capacity;
} Lasers;

Laser Laser_Create(Vector2 pos, int speed, Color color) {
  Laser laser = {pos, speed, true, color};
  return laser;
}

void Laser_Update(Laser *laser) {
  laser->pos.y += laser->speed;
  if (laser->isActive) {
    if (laser->pos.y > GetScreenHeight() - 100 || laser->pos.y < 25) {
      laser->isActive = false;
    }
  }
}
void Laser_Draw(Laser *laser) {
  if (laser->isActive) {

    DrawRectangleV(laser->pos, (Vector2){4, 15}, laser->color);
  }
}

Rectangle Laser_GetRect(Laser *laser) {
  return (Rectangle){laser->pos.x, laser->pos.y, 4, 15};
}

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

Rectangle Block_GetRect(Block *block) {
  return (Rectangle){block->pos.x, block->pos.y, 3, 3};
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
  int obstacleWidth =
      (int)(sizeof(OBSTACLE_GRID[0]) / sizeof(OBSTACLE_GRID[0][0])) * 3;
  float gap = (GetScreenWidth() - (4 * obstacleWidth)) / 5;
  for (size_t i = 0; i < 4; i++) {
    float offsetX = (i + 1) * gap + i * obstacleWidth;
    obstacles[i] = Obstacle_Create((Vector2){offsetX, GetScreenHeight() - 200});
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

    Color laserColor = {237, 41, 57, 255};
    Laser laser = Laser_Create(
        (Vector2){alien->pos.x + AlienImages[alien->type - 1].width / 2,
                  alien->pos.y + AlienImages[alien->type - 1].height},
        6, laserColor);

    da_append(*alienLasers, laser);
  }
}

Rectangle Alien_GetRect(Alien *alien) {
  return (Rectangle){alien->pos.x, alien->pos.y,
                     AlienImages[alien->type - 1].width,
                     AlienImages[alien->type - 1].height};
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
    int speed = 0;

    if (game.level > 1) {
      speed = game.level * 0.5;
    }

    alien->pos.y += distance + speed;
  }
}

void Aliens_Move(Aliens *aliens, int *direction) {
  for (size_t i = 0; i < aliens->count; ++i) {
    Alien *alien = &aliens->items[i];
    if (alien->pos.x + AlienImages[alien->type - 1].width >
        GetScreenWidth() - 25) {
      *direction = -1;
      Aliens_Move_down(aliens, 4);
    }

    if (alien->pos.x <= 25) {
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

// MYSTERYSHIP
//
typedef struct {
  Texture2D image;
  Vector2 pos;
  int speed;
  bool isAlive;
} Mysteryship;

Mysteryship Mysteryship_Create(Vector2 pos) {
  Texture2D image = LoadTexture("assets/mystery.png");
  Mysteryship mystery = {image, pos, 7, false};
  return mystery;
}

void Mysteryship_Spawn(Mysteryship *mystery) {
  mystery->pos.y = 90;
  int side = GetRandomValue(0, 1);

  if (side == 0) {
    mystery->pos.x = 25;
    mystery->speed = 3;
  } else {
    mystery->pos.x = GetScreenWidth() + mystery->image.width - 25;
    mystery->speed = -3;
  }
  mystery->isAlive = true;
}

void Mysteryship_Update(Mysteryship *mystery) {
  if (mystery->isAlive) {
    mystery->pos.x += mystery->speed;
    if (mystery->pos.x > GetScreenWidth() - mystery->image.width - 25 ||
        mystery->pos.x < 25) {
      mystery->isAlive = false;
    }
  }
}

Rectangle Mysteryship_GetRect(Mysteryship *mystery) {
  if (mystery->isAlive) {
    return (Rectangle){mystery->pos.x, mystery->pos.y, mystery->image.width,
                       mystery->image.height};
  }
  return (Rectangle){mystery->pos.x, mystery->pos.y, 0, 0};
}

void Mysteryship_Draw(Mysteryship *mystery) {
  if (mystery->isAlive) {
    DrawTextureV(mystery->image, mystery->pos, WHITE);
  }
}

void Mysteryship_Unload(Mysteryship *mystery) { UnloadTexture(mystery->image); }

// SPACESHIP
//
typedef struct {
  Texture2D image;
  Vector2 pos;
  int speed;
  Lasers lasers;
  Sound laserSound;
} Spaceship;

Spaceship Spaceship_Create() {
  Texture2D image = LoadTexture("assets/spaceship.png");

  Vector2 pos = {.x = (GetScreenWidth() - image.width) / 2,
                 .y = GetScreenHeight() - image.height - 100};

  int speed = 7;
  Lasers lasers = {0};
  Sound sound = LoadSound("assets/laser.ogg");

  Spaceship spaceship = {image, pos, speed, lasers, sound};
  return spaceship;
}

void Spaceship_Draw(Spaceship *sp) {
  DrawTextureV(sp->image, sp->pos, RAYWHITE);
}
void Spaceship_Move_right(Spaceship *sp) {
  sp->pos.x += sp->speed;

  if (sp->pos.x > GetScreenWidth() - sp->image.width - 20) {
    sp->pos.x = GetScreenWidth() - sp->image.width - 20;
  }
}
void Spaceship_Move_left(Spaceship *sp) {
  sp->pos.x -= sp->speed;

  if (sp->pos.x < 20) {
    sp->pos.x = 20;
  }
}

void Spaceship_Fire(Spaceship *sp) {
  Color laserColor = {243, 216, 63, 255};
  Laser laser = Laser_Create(
      (Vector2){sp->pos.x + sp->image.width / 2 - 2, sp->pos.y - 15}, -6,
      laserColor);

  da_append(sp->lasers, laser);
}

Rectangle Spaceship_GetRect(Spaceship *sp) {
  return (Rectangle){sp->pos.x, sp->pos.y, sp->image.width, sp->image.height};
}

void Spaceship_Unload(Spaceship *sp) { UnloadTexture(sp->image); }

// GAME ENTITIES
//

typedef struct {
  Spaceship sp;
  Aliens aliens;
  Obstacle *obstacles;
  Mysteryship mystery;
  Lasers alienLasers;
  int aliensDirection;

  // timers
  double lastPlayerFireTime;
  double lastAlienFireTime;
  double lastTimeMysteryShipSpawn;
  double alienShootInterval;
  float mysteryShipSpawnInterval;

  // sounds
  Music music;
  Sound explosion;
} Game;

void Game_Init(Game *g) {
  g->sp = Spaceship_Create();
  da_clear(g->sp.lasers); // reset spaceship lasers

  g->aliens = Aliens_Create();
  g->aliensDirection = 1;

  if (g->obstacles)
    free(g->obstacles);
  g->obstacles = Obstacles_Create();

  g->mystery = Mysteryship_Create((Vector2){100, 100});

  da_clear(g->alienLasers); // reset alien lasers

  // timers
  g->lastPlayerFireTime = 0;
  g->lastAlienFireTime = 0;
  g->lastTimeMysteryShipSpawn = 0;

  g->alienShootInterval = 0.35;

  g->mysteryShipSpawnInterval = GetRandomValue(10, 20);

  // sounds
  g->music = LoadMusicStream("assets/music.ogg");
  g->explosion = LoadSound("assets/explosion.ogg");
}

void Game_NextLevel(Game *g) {
  game.level++;

  // Reset aliens
  g->aliens = Aliens_Create();
  g->aliensDirection = 1;

  // Reset obstacles
  free(g->obstacles);
  g->obstacles = Obstacles_Create();

  // Clear lasers
  da_clear(g->sp.lasers);
  da_clear(g->alienLasers);

  // Reset timers
  g->lastAlienFireTime = 0;
  g->lastTimeMysteryShipSpawn = 0;
  g->mysteryShipSpawnInterval = GetRandomValue(10, 20);
  g->alienShootInterval = 0.35 + (game.level - 1) * 0.05;

  // Respawn mystery ship
  g->mystery = Mysteryship_Create((Vector2){100, 100});
}

void CheckForCollisions(Aliens *aliens, Spaceship *sp, Obstacle *obstacles,
                        Mysteryship *mystery, Lasers alienLasers,
                        Sound explosion, Game *g) {
  // Check collisions for the spaceship lasers
  for (size_t i = 0; i < sp->lasers.count; i++) {
    Laser *laser = &sp->lasers.items[i];
    // Player hit the mystery spaceship
    if (CheckCollisionRecs(Mysteryship_GetRect(mystery),
                           Laser_GetRect(laser))) {
      mystery->isAlive = false;
      laser->isActive = false;
      game.score += 500;
      PlaySound(explosion);
    }

    for (size_t j = 0; j < aliens->count; j++) {
      Alien *alien = &aliens->items[j];
      // Player hit alien
      if (CheckCollisionRecs(Laser_GetRect(laser), Alien_GetRect(alien))) {
        PlaySound(explosion);
        da_remove_at(*aliens, j);

        laser->isActive = false;
        switch (alien->type) {
        case 1:
          game.score += 100;
          break;
        case 2:
          game.score += 200;
          break;
        case 3:
          game.score += 300;
          break;
        default:
          break;
        }

        if (aliens->count <= 0) {
          Game_NextLevel(g);
        }
      }
    }

    for (size_t t = 0; t < 4; t++) {
      Obstacle *obstacle = &obstacles[t];
      for (size_t j = 0; j < obstacle->blocks.count; j++) {
        // Player hit obstacle
        if (CheckCollisionRecs(Laser_GetRect(laser),
                               Block_GetRect(&obstacle->blocks.items[j]))) {

          da_remove_at(obstacle->blocks, j);
          laser->isActive = false;
        }
      }
    }
  }

  // check collisions for alien lasers
  for (size_t i = 0; i < alienLasers.count; i++) {
    Laser *laser = &alienLasers.items[i];

    // Alien laser hit player
    if (CheckCollisionRecs(Laser_GetRect(laser), Spaceship_GetRect(sp))) {
      printf("You got hit! You have %d lives!\n", game.lives);
      laser->isActive = false;
      game.lives--;
      if (game.lives <= 0) {
        game.isGameOver = true;
      }
    }

    for (size_t t = 0; t < 4; t++) {
      Obstacle *obstacle = &obstacles[t];
      for (size_t j = 0; j < obstacle->blocks.count; j++) {
        // Alien laser hit obstacle
        if (CheckCollisionRecs(Laser_GetRect(laser),
                               Block_GetRect(&obstacle->blocks.items[j]))) {

          da_remove_at(obstacle->blocks, j);
          laser->isActive = false;
        }
      }
    }
  }

  // check for alien collisions
  for (size_t i = 0; i < aliens->count; i++) {
    Alien *alien = &aliens->items[i];
    for (size_t t = 0; t < 4; t++) {
      Obstacle *obstacle = &obstacles[t];
      for (size_t j = 0; j < obstacle->blocks.count; j++) {
        // Alien hit obstacle
        if (CheckCollisionRecs(Alien_GetRect(alien),
                               Block_GetRect(&obstacle->blocks.items[j]))) {

          da_remove_at(obstacle->blocks, j);
        }
      }
    }

    // Alien hit player
    if (CheckCollisionRecs(Alien_GetRect(alien), Spaceship_GetRect(sp))) {
      game.lives--;
      if (game.lives <= 0) {
        game.isGameOver = true;
      }
      printf("You got hit!\n");
    }
  }
}

void saveHighestScoreToFile(int score) {
  FILE *f = fopen("score.txt", "w");
  if (!f) {
    perror("Failed to open score.txt for writing");
    return;
  }
  fprintf(f, "%d\n", score); // write score as text
  fclose(f);
}

int getHighestScoreFromFile() {
  FILE *f = fopen("score.txt", "r");

  if (!f) {
    printf("ERROR: No score.txt file found.\n");
    return 0;
  }

  int score = 0;

  if (fscanf(f, "%d", &score) != 1) {
    printf("ERROR: something went wrong while reading the contents of "
           "score.txt.\n");
    score = 0;
  }

  fclose(f);
  return score;
}

int main(void) {
  Color grey = {29, 29, 27, 255};
  Color yellow = {243, 216, 63, 255};
  int offset = 50;
  int screenWidth = 750;
  int screenHeigth = 700;

  InitWindow(screenWidth + offset, screenHeigth + 2 * offset, "Space Invaders");
  InitAudioDevice();

  Aliens_Load_images();
  Font font = LoadFontEx("assets/monogram.ttf", 64, 0, 0);
  int highestScore = getHighestScoreFromFile();
  Texture2D spaceshipImage = LoadTexture("assets/spaceship.png");
  Game gameEntities = {0};
  Game_Init(&gameEntities);

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    UpdateMusicStream(gameEntities.music);
    // -------------------
    // UPDATE
    // -------------------
    if (!game.isGameOver) {
      double now = GetTime();
      // Player input
      if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        Spaceship_Move_right(&gameEntities.sp);
      } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        Spaceship_Move_left(&gameEntities.sp);
      }

      if (IsKeyDown(KEY_SPACE) &&
          now - gameEntities.lastPlayerFireTime > 0.25) {
        Spaceship_Fire(&gameEntities.sp);
        gameEntities.lastPlayerFireTime = now;
      }

      // Mystery ship spawn
      if (now - gameEntities.lastTimeMysteryShipSpawn >
          gameEntities.mysteryShipSpawnInterval) {
        Mysteryship_Spawn(&gameEntities.mystery);
        gameEntities.lastTimeMysteryShipSpawn = now;
        gameEntities.mysteryShipSpawnInterval = GetRandomValue(10, 20);
      }

      // Collisions
      CheckForCollisions(&gameEntities.aliens, &gameEntities.sp,
                         gameEntities.obstacles, &gameEntities.mystery,
                         gameEntities.alienLasers, gameEntities.explosion,
                         &gameEntities);

      // Updates
      Mysteryship_Update(&gameEntities.mystery);
      Lasers_Update(&gameEntities.sp.lasers);
      Aliens_Move(&gameEntities.aliens, &gameEntities.aliensDirection);
      Alien_Shoot(&gameEntities.aliens, &gameEntities.alienLasers,
                  gameEntities.alienShootInterval,
                  &gameEntities.lastAlienFireTime);
      Lasers_Update(&gameEntities.alienLasers);
    }

    // -------------------
    // DRAW
    // -------------------
    BeginDrawing();
    ClearBackground(grey);
    DrawRectangleRoundedLines((Rectangle){10, 10, 780, 780}, 0.18f, 20, yellow);
    DrawLineEx((Vector2){25, 730}, (Vector2){775, 730}, 2, yellow);

    if (game.isGameOver) {
      DrawTextEx(font, "[PRESS ENTER]", (Vector2){50, 740}, 34, 2, yellow);
      DrawTextEx(font, "GAME OVER", (Vector2){570, 740}, 34, 2, yellow);

      if (game.score > highestScore) {
        highestScore = game.score;
        saveHighestScoreToFile(game.score);
      }

      if (IsKeyPressed(KEY_ENTER)) {
        game.lives = 3;
        game.score = 0;
        game.isGameOver = false;

        Game_Init(&gameEntities);
      }
    } else {
      float x = 50.0;

      for (size_t i = 0; i < game.lives; i++) {
        DrawTextureV(spaceshipImage, (Vector2){x, 745}, WHITE);
        x += 50;
      }

      char levelNumberText[16];
      snprintf(levelNumberText, sizeof(game.level), "%02d", game.level);

      char levelText[24];
      sprintf(levelText, "LEVEL %s", levelNumberText);

      DrawTextEx(font, levelText, (Vector2){570, 740}, 34, 2, yellow);
    }

    DrawTextEx(font, "SCORE", (Vector2){50, 15}, 34, 2, yellow);

    char scoreText[16];
    snprintf(scoreText, sizeof(scoreText), "%05d", game.score);
    DrawTextEx(font, scoreText, (Vector2){50, 40}, 34, 2, yellow);

    char highestScoreText[16];
    snprintf(highestScoreText, sizeof(highestScoreText), "%05d", highestScore);

    DrawTextEx(font, "HIGH-SCORE", (Vector2){570, 15}, 34, 2, yellow);
    DrawTextEx(font, highestScoreText, (Vector2){660, 40}, 34, 2, yellow);

    Spaceship_Draw(&gameEntities.sp);
    Aliens_Draw(&gameEntities.aliens);
    Obstacles_Draw(gameEntities.obstacles, 4);
    Mysteryship_Draw(&gameEntities.mystery);
    Lasers_Draw(&gameEntities.sp.lasers);
    Lasers_Draw(&gameEntities.alienLasers);

    EndDrawing();
  }

  UnloadTexture(spaceshipImage);
  Spaceship_Unload(&gameEntities.sp);
  Aliens_Unload_images();
  Mysteryship_Unload(&gameEntities.mystery);
  UnloadMusicStream(gameEntities.music);
  UnloadSound(gameEntities.explosion);
  UnloadSound(gameEntities.sp.laserSound);
  free(gameEntities.obstacles);
  CloseAudioDevice();

  CloseWindow();
  return 0;
}
