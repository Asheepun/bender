#ifndef GAME_H_
#define GAME_H_

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

//STRUCTS

typedef struct Particle{
	Vec2f pos;
	Vec2f velocity;
	Vec2f acceleration;
	bool isBended;
}Particle;

typedef struct Pixel{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
}Pixel;

typedef struct Collision{
	int index;
}Collision;

typedef struct Player{
	Vec2f pos;
	Vec2f size;
	Vec2f velocity;
	Vec2f acceleration;
	float walkForce;
	float jumpForce;
	bool onGround;
}Player;

//GLOBAL VARIABLES

static Pixel backgroundColor = { 0, 0, 0, 255 };
static Pixel rockColor = { 255, 255, 255, 255 };

static int WIDTH = 800;
static int HEIGHT = 450;

static float BENDING_RADIUS = 25;
static float BENDING_MAGNITUDE = 0.05;

static float GRAVITY = 0.05;
static float AIR_FRICTION = 0.98;
static float COLLISION_DAMPING = 0.9;

static float PLAYER_JUMP_ACCELERATION = 4.5;
static float PLAYER_GRAVITY = 0.20;
static float PLAYER_SIDE_ACCELERATION = 0.35;
static float PLAYER_SIDE_RESITANCE = 0.90;

//WORLD VARIABLES
Array particles;
Pixel *staticParticlesBuffer;
Collision *collisionBuffer;
Collision *clearedCollisionBuffer;
Player player;

//RENDER VARIABLES

Renderer2D_Renderer renderer;
Pixel *screenBuffer;
Renderer2D_Texture screenTexture;
int screenTextureSize;

//FUNCTIONS

int getBufferIndex(float, float);

bool checkPixelEquals(Pixel, Pixel);

void addParticle(Vec2f);

#endif