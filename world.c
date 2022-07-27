#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"
#include "engine/files.h"

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

	for(int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++){
		world_p->clearedCollisionBuffer[i].ID = -1;
	}

	world_p->currentLevelIndex = 0;

	World_restore(world_p);

}

void World_restore(World *world_p){

	memcpy(world_p->collisionBuffer, world_p->clearedCollisionBuffer, sizeof(Collision) * MAX_WIDTH * MAX_HEIGHT);

	Array_clear(&world_p->entities);
	Array_clear(&world_p->sprites);
	Array_clear(&world_p->particles);
	
	world_p->levelWidth = WIDTH;
	world_p->levelHeight = HEIGHT;

	world_p->playerDied = false;
	world_p->completedLevel = false;

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

	EntityHeader_init(&particle_p->entityHeader);
	////particle_p->ID = currentID;
	//currentID++;

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

	Array_init(&level_p->enemyPoses, sizeof(Vec2f));

	String_set(level_p->name, "Untitled", STRING_SIZE);

	level_p->width = WIDTH;

}

void Level_clear(Level *level_p){

	for(int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++){
		level_p->staticParticlesBuffer[i] = backgroundColor;
	}

	level_p->playerPos = getVec2f(-100, -100);

	Array_clear(&level_p->enemyPoses);

	String_set(level_p->name, "Untitled", STRING_SIZE);

	level_p->width = WIDTH;

}

void Level_loadFromFile(Level *level_p, char *fileName){

	Level_clear(level_p);

	int pixelDataSize = sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT;
	
	long int fileSize;
	char *data = getFileData_mustFree(fileName, &fileSize);

	memcpy(level_p->staticParticlesBuffer, data, pixelDataSize);

	int numberOfRows = 0;
	int currentChar = 0;
	char rows[64][100];
	memset(rows, 0, 64 * 100);
	for(int i = 0; i < fileSize - pixelDataSize; i++){

		if(*(data + pixelDataSize + i) == *"\n"){
			numberOfRows++;
			currentChar = 0;
		}else{
			rows[numberOfRows][currentChar] = *(data + pixelDataSize + i);
			currentChar++;
		}

	}

	char *ptr;

	for(int i = 0; i < numberOfRows; i++){

		if(strcmp(rows[i], ":name") == 0){
			String_set(level_p->name, rows[i + 1], STRING_SIZE);
			printf("%s\n", rows[i + 1]);
		}

		if(strcmp(rows[i], ":width") == 0){
			level_p->width = (int)strtol(rows[i + 1], &ptr, 10);
		}

		if(strcmp(rows[i], ":playerPos") == 0){
			level_p->playerPos = getVec2f(
					(float)strtol(rows[i + 1], &ptr, 10),
					(float)strtol(rows[i + 2], &ptr, 10)
			);
		}

		if(strcmp(rows[i], ":enemyPos") == 0){

			Vec2f *pos_p = Array_addItem(&level_p->enemyPoses);

			*pos_p = getVec2f(
					(float)strtol(rows[i + 1], &ptr, 10),
					(float)strtol(rows[i + 2], &ptr, 10)
			);

		}

	}

	free(data);

}

void Level_writeToFile(Level *level_p){

	int pixelDataSize = sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT;
	int otherDataSize =  100 * (sizeof(Vec2f) + 64);

	char *data = malloc(pixelDataSize + otherDataSize);

	memset(data, 0, pixelDataSize + otherDataSize);

	memcpy(data, level_p->staticParticlesBuffer, pixelDataSize);

	String_append(data + pixelDataSize, ":name\n");
	String_append(data + pixelDataSize, level_p->name);
	String_append(data + pixelDataSize, "\n");

	String_append(data + pixelDataSize, ":width\n");
	String_append_int(data + pixelDataSize, level_p->width);
	String_append(data + pixelDataSize, "\n");

	String_append(data + pixelDataSize, ":playerPos\n");
	String_append_int(data + pixelDataSize, (int)level_p->playerPos.x);
	String_append(data + pixelDataSize, "\n");
	String_append_int(data + pixelDataSize, (int)level_p->playerPos.y);
	String_append(data + pixelDataSize, "\n");

	for(int i = 0; i < level_p->enemyPoses.length; i++){

		Vec2f *pos_p = Array_getItemPointerByIndex(&level_p->enemyPoses, i);

		String_append(data + pixelDataSize, ":enemyPos\n");
		String_append_int(data + pixelDataSize, (int)pos_p->x);
		String_append(data + pixelDataSize, "\n");
		String_append_int(data + pixelDataSize, (int)pos_p->y);
		String_append(data + pixelDataSize, "\n");
		
	}

	otherDataSize = strlen(data + pixelDataSize);

	//printf("%i\n", otherDataSize);

	//printf("%s", data + pixelDataSize);

	char path[STRING_SIZE];
	String_set(path, "levels/", STRING_SIZE);
	String_append(path, level_p->name);
	String_append(path, ".level");
			
	writeDataToFile(path, data, pixelDataSize + otherDataSize);

}

void World_Level_load(World *world_p, Level *level_p){

	World_restore(world_p);

	memcpy(world_p->staticParticlesBuffer, level_p->staticParticlesBuffer, sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);

	//Array_clear(&world_p->entities);

	World_addPlayer(world_p, level_p->playerPos);

	for(int i = 0; i < level_p->enemyPoses.length; i++){

		World_addEnemy(world_p, *((Vec2f *)Array_getItemPointerByIndex(&level_p->enemyPoses, i)));

	}

	world_p->levelWidth = level_p->width;

}

Entity *World_addEnemy(World *world_p, Vec2f pos){

	Entity *entity_p = Array_addItem(&world_p->entities);

	entity_p->type = ENTITY_TYPE_ENEMY;

	EntityHeader_init(&entity_p->entityHeader);

	Body_init(&entity_p->body, pos, getVec2f(15, 20));

	Physics_init(&entity_p->physics);

	return entity_p;

}

bool checkBodyBodyCollision(Body body1, Body body2){
	return body1.pos.x < body2.pos.x + body2.size.x
		&& body1.pos.x + body1.size.x > body2.pos.x
		&& body1.pos.y < body2.pos.y + body2.size.y
		&& body1.pos.y + body1.size.y > body2.pos.y;
}
