#ifndef GAME_H_
#define GAME_H_

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

#define MAX_WIDTH 800 * 4
#define MAX_HEIGHT 450

//ENUMS
enum GameState{
	GAME_STATE_LEVEL,
	GAME_STATE_LEVEL_EDITOR,
};

//STRUCTS
typedef struct Sprite{
	EntityHeader entityHeader;
	Vec2f pos;
	Vec2f size;
	Renderer2D_Color color;
	float alpha;
}Sprite;

typedef struct Particle{
	size_t ID;
	Vec2f pos;
	Vec2f velocity;
	Vec2f acceleration;
	bool isBended;
	bool isHovered;
}Particle;

typedef struct Pixel{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
}Pixel;

typedef struct Collision{
	size_t ID;
}Collision;

typedef struct Player{
	Vec2f pos;
	Vec2f size;
	Vec2f velocity;
	Vec2f acceleration;
	float walkForce;
	float jumpForce;
	bool onGround;
	bool collidedWithMovingParticle;
	bool collidedWithStaticParticle;
}Player;

typedef struct Level{
	Pixel staticParticlesBuffer[MAX_WIDTH * MAX_HEIGHT];
	Vec2f playerPos;
	char name[STRING_SIZE];
}Level;

//GLOBAL VARIABLES

static Pixel backgroundColor = { 0, 0, 0, 255 };
static Pixel rockColor = { 255, 255, 255, 255 };
static Pixel metalColor = { 128, 128, 128, 255 };

static int WIDTH = 800;
static int HEIGHT = 450;

static int levelWidth = 800 * 4;
static int levelHeight = 450;

static float BENDING_RADIUS = 25;
static float BENDING_RADIUS_MARGIN = 10;
static float BENDING_MAGNITUDE = 0.05;

static float GRAVITY = 0.05;
static float AIR_FRICTION = 0.98;
static float COLLISION_DAMPING = 0.9;

static float PLAYER_JUMP_ACCELERATION = 4.5;
static float PLAYER_GRAVITY = 0.15;
static float PLAYER_SIDE_ACCELERATION = 0.35;
static float PLAYER_SIDE_RESITANCE = 0.90;

enum GameState currentGameState;

Level currentLevel;

//WORLD VARIABLES
Array particles;
Pixel *staticParticlesBuffer;
Collision *collisionBuffer;
Collision *clearedCollisionBuffer;
Player player;

Array sprites;

//RENDER VARIABLES

Renderer2D_Renderer renderer;
Pixel *screenBuffer;
Renderer2D_Texture screenTexture;
int screenTextureSize;

//FUNCTIONS

//world.c

int getBufferIndex(float, float);

bool checkPixelEquals(Pixel, Pixel);

bool checkOubVec2f(Vec2f);

Particle *addParticle(Vec2f);

bool Particle_checkOub(Particle *);

void initPlayer(Vec2f);

void Level_init(Level *);

void Level_load(Level *);

//levelState.c

void initLevelState();

void levelState();

//editorState.c

void initEditorState();

void editorState();

#endif
