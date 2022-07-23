#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

void World_init(World *world_p){

	//init screen rendering
	world_p->screenBuffer = malloc(sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);

	Renderer2D_Texture_init(&world_p->screenTexture, "screen-texture", (unsigned char *)world_p->screenBuffer, MAX_WIDTH, MAX_HEIGHT);

	//init world
	Array_init(&world_p->particles, sizeof(Particle));
	world_p->staticParticlesBuffer = malloc(sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);
	world_p->collisionBuffer = malloc(sizeof(Collision) * MAX_WIDTH * MAX_HEIGHT);
	world_p->clearedCollisionBuffer = malloc(sizeof(Collision) * MAX_WIDTH * MAX_HEIGHT);

	Array_init(&world_p->entities, sizeof(Entity));

	Array_init(&world_p->sprites, sizeof(Sprite));
	
	world_p->levelWidth = WIDTH;
	world_p->levelHeight = HEIGHT;

}

void World_restore(World *world_p){

	Array_clear(&world_p->entities);

	Array_clear(&world_p->sprites);

}

int getBufferIndex(float x, float y){
	return (int)y * MAX_WIDTH + (int)x;
}

bool checkPixelEquals(Pixel p1, Pixel p2){
	return p1.r == p2.r
		&& p1.g == p2.g
		&& p1.b == p2.b
		&& p1.a == p2.a;
}

static size_t currentID = 0;

Sprite *World_addSprite(World *world_p, Vec2f pos, Vec2f size, Renderer2D_Color color, float alpha){

	Sprite *sprite_p = Array_addItem(&world_p->sprites);

	sprite_p->pos = pos;
	sprite_p->size = size;
	sprite_p->color = color;
	sprite_p->alpha = alpha;

}

Particle *World_addParticle(World *world_p, Vec2f pos){

	Particle *particle_p = Array_addItem(&world_p->particles);

	particle_p->ID = currentID;
	currentID++;

	particle_p->pos = pos;
	particle_p->lastPos = pos;

	particle_p->velocity = getVec2f(0, 0);
	particle_p->acceleration = getVec2f(0, 0);

	particle_p->isBended = false;

	return particle_p;

}


bool World_checkOubVec2f(World *world_p, Vec2f v){
	if((int)v.x < 0
	|| (int)v.y < 0
	|| (int)v.x >= world_p->levelWidth
	|| (int)v.y >= world_p->levelHeight){
		return true;
	}

	return false;
}

bool World_Particle_checkOub(World *world_p, Particle *particle_p){
	if((int)particle_p->pos.x < 0
	|| (int)particle_p->pos.y < 0
	|| (int)particle_p->pos.x >= world_p->levelWidth
	|| (int)particle_p->pos.y >= world_p->levelHeight){
		return true;
	}

	return false;
}

void Body_init(Body *body_p, Vec2f pos, Vec2f size){

	body_p->pos = pos;
	body_p->size = size;

}

void Physics_init(Physics *physics_p){

	physics_p->velocity = getVec2f(0, 0);
	physics_p->acceleration = getVec2f(0, 0);
	physics_p->resistance = getVec2f(1, 1);
	physics_p->onGround = false;

}

/*
Body *addBody(Vec2f pos, Vec2f size){

	Body *body_p = Array_addItem(&bodies);

	EntityHeader_init(&body_p->entityHeader);

	body_p->pos = pos;
	body_p->size = size;
	body_p->lastPos = pos;
	body_p->velocity = getVec2f(0, 0);
	body_p->acceleration = getVec2f(0, 0);
	body_p->onGround = false;

	return body_p;
}

Body *getBodyByID(size_t ID){
	return Array_getItemPointerByID(&bodies, ID);
}
*/

Entity *World_addPlayer(World *world_p, Vec2f pos){

	Entity *entity_p = Array_addItem(&world_p->entities);

	entity_p->type = ENTITY_TYPE_PLAYER;

	EntityHeader_init(&entity_p->entityHeader);

	Body_init(&entity_p->body, pos, getVec2f(15, 20));

	Physics_init(&entity_p->physics);

	return entity_p;

}

/*
void initPlayer(Vec2f pos){

	player.bodyID = addBody(pos, getVec2f(15, 20))->entityHeader.ID;
	//player.pos = pos;
	//player.size = getVec2f(15, 20);
	//player.velocity = getVec2f(0, 0);
	//player.acceleration = getVec2f(0, 0);
	player.walkForce = 0;
	player.jumpForce = 0;
	player.collidedWithMovingParticle = false;
	player.collidedWithStaticParticle = false;

}
*/

void Level_init(Level *level_p){

	for(int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++){
		level_p->staticParticlesBuffer[i] = backgroundColor;
	}

	level_p->playerPos = getVec2f(-100, -100);

	String_set(level_p->name, "Untitled", STRING_SIZE);

	level_p->width = WIDTH;

}

void World_Level_load(World *world_p, Level *level_p){

	memcpy(world_p->staticParticlesBuffer, level_p->staticParticlesBuffer, sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);

	Array_clear(&world_p->entities);

	World_addPlayer(world_p, level_p->playerPos);

	world_p->levelWidth = level_p->width;

	//initPlayer(level_p->playerPos);

}

Entity *World_addEnemy(World *world_p, Vec2f pos){

	Entity *entity_p = Array_addItem(&world_p->entities);

	entity_p->type = ENTITY_TYPE_ENEMY;

	EntityHeader_init(&entity_p->entityHeader);

	Body_init(&entity_p->body, pos, getVec2f(15, 20));

	Physics_init(&entity_p->physics);

	return entity_p;

}
