#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

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

Sprite *addSprite(Vec2f pos, Vec2f size, Renderer2D_Color color, float alpha){

	Sprite *sprite_p = Array_addItem(&sprites);

	sprite_p->pos = pos;
	sprite_p->size = size;
	sprite_p->color = color;
	sprite_p->alpha = alpha;

}

Particle *addParticle(Vec2f pos){

	Particle *particle_p = Array_addItem(&particles);

	particle_p->ID = currentID;
	currentID++;

	particle_p->pos = pos;
	particle_p->lastPos = pos;

	particle_p->velocity = getVec2f(0, 0);
	particle_p->acceleration = getVec2f(0, 0);

	particle_p->isBended = false;

	return particle_p;

}


bool checkOubVec2f(Vec2f v){
	if((int)v.x < 0
	|| (int)v.y < 0
	|| (int)v.x >= levelWidth
	|| (int)v.y >= levelHeight){
		return true;
	}

	return false;
}

bool Particle_checkOub(Particle *particle_p){
	if((int)particle_p->pos.x < 0
	|| (int)particle_p->pos.y < 0
	|| (int)particle_p->pos.x >= levelWidth
	|| (int)particle_p->pos.y >= levelHeight){
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

void addPlayer(Vec2f pos){

	Entity *entity_p = Array_addItem(&entities);

	entity_p->type = ENTITY_TYPE_PLAYER;

	EntityHeader_init(&entity_p->entityHeader);

	Body_init(&entity_p->body, pos, getVec2f(15, 20));

	Physics_init(&entity_p->physics);

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

}

void Level_load(Level *level_p){

	memcpy(staticParticlesBuffer, level_p->staticParticlesBuffer, sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);

	Array_clear(&entities);

	addPlayer(level_p->playerPos);

	//initPlayer(level_p->playerPos);

}

/*
Enemy *addEnemy(Vec2f pos){

	Enemy *enemy_p = Array_addItem(&enemies);
	
	enemy_p->bodyID = addBody(pos, getVec2f(15, 20))->entityHeader.ID;

	//enemy_p->pos = pos;
	//enemy_p->size = getVec2f(15, 20);
	//enemy_p->velocity = getVec2f(0, 0);
	//enemy_p->acceleration = getVec2f(0, 0);

}
*/
