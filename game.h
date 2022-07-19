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

enum CollisionDirection{
	COLLISION_DIRECTION_NONE,
	COLLISION_DIRECTION_UP,
	COLLISION_DIRECTION_DOWN,
	COLLISION_DIRECTION_LEFT,
	COLLISION_DIRECTION_RIGHT,
};

enum EntityType{
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_ENEMY,
};

//STRUCTS
typedef struct Sprite{
	Vec2f pos;
	Vec2f size;
	Renderer2D_Color color;
	float alpha;
}Sprite;

typedef struct Particle{
	size_t ID;
	Vec2f pos;
	Vec2f lastPos;
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

typedef struct Body{
	Vec2f pos;
	Vec2f size;
}Body;

typedef struct Physics{
	Vec2f velocity;
	Vec2f acceleration;
	Vec2f resistance;
	bool onGround;
}Physics;

/*
typedef struct Body{
	EntityHeader entityHeader;
	Vec2f pos;
	Vec2f size;
	Vec2f lastPos;
	Vec2f velocity;
	Vec2f acceleration;
	bool onGround;
	enum CollisionDirection previousCollisionDirection;
}Body;
*/

typedef struct Entity{
	EntityHeader entityHeader;
	enum EntityType type;
	Body body;
	Body lastBody;
	Physics physics;
}Entity;

/*
typedef struct Player{
	size_t bodyID;
	//Vec2f pos;
	//Vec2f size;
	//Vec2f velocity;
	//Vec2f acceleration;
	float walkForce;
	float jumpForce;
	bool collidedWithMovingParticle;
	bool collidedWithStaticParticle;
}Player;

typedef struct Enemy{
	size_t bodyID;
	//Vec2f pos;
	//Vec2f size;
	//Vec2f velocity;
	//Vec2f acceleration;
}Enemy;
*/

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
static float PLAYER_WALK_ACCELERATION = 0.35;
static float PLAYER_WALK_RESISTANCE = 0.90;

static float ENEMY_JUMP_ACCELERATION = 3.5;
static float ENEMY_GRAVITY = 0.15;
static float ENEMY_WALK_ACCELERATION = 0.40;
static float ENEMY_WALK_RESISTANCE = 0.90;
static float ENEMY_DETECTION_RANGE = 200;

enum GameState currentGameState;

Level currentLevel;

//WORLD VARIABLES
Array particles;
Pixel *staticParticlesBuffer;
Collision *collisionBuffer;
Collision *clearedCollisionBuffer;

Array entities;

Array sprites;
//Player player;
//Array enemies;

//Array bodies;

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

Sprite *addSprite(Vec2f, Vec2f, Renderer2D_Color, float);

Entity *addPlayer(Vec2f);
Entity *addEnemy(Vec2f);

//Body *addBody(Vec2f, Vec2f);

//Body *getBodyByID(size_t);

bool Particle_checkOub(Particle *);

//void initPlayer(Vec2f);

void Level_init(Level *);

void Level_load(Level *);

//levelState.c

void initLevelState();

void levelState();

//editorState.c

void initEditorState();

void editorState();

#endif
