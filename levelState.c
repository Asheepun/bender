#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

char *levelNames[] = {
	"levels/level1.level",
	"levels/level2.level",
	"levels/level3.level",
	"levels/level4.level",
	"levels/level5.level",
	"levels/level6.level",
	"levels/level7.level",
	"levels/level8.level",
	"levels/level9.level",
	"levels/level10.level",
};

Vec2f forcePoint = { 0, 0 };

void World_initLevelState(World *world_p){

	World_Level_load(world_p, &world_p->currentLevel);

}

void World_levelState(World *world_p){

	Vec2f offsetPointerPos = getSubVec2f(Engine_pointer.pos, world_p->renderer.offset);

	if(world_p->playerDied){

		World_initLevelState(world_p);

		world_p->playerDied = false;

	}

	if(world_p->completedLevel){

		world_p->currentLevelIndex++;

		Level_loadFromFile(&world_p->currentLevel, levelNames[world_p->currentLevelIndex]);

		World_initLevelState(world_p);

		world_p->completedLevel = false;

	}

	if(Engine_keys[ENGINE_KEY_G].downed){
		World_initEditorState(world_p);
		world_p->currentGameState = GAME_STATE_LEVEL_EDITOR;
	}

	//control entities
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->physics.acceleration = getVec2f(0, 0);

		if(entity_p->type == ENTITY_TYPE_PLAYER){

			entity_p->physics.acceleration.y += PLAYER_GRAVITY;

			entity_p->physics.resistance.x = PLAYER_WALK_RESISTANCE;

			if(Engine_keys[ENGINE_KEY_A].down){
				entity_p->physics.acceleration.x = -PLAYER_WALK_ACCELERATION;
			}
			if(Engine_keys[ENGINE_KEY_D].down){
				entity_p->physics.acceleration.x = PLAYER_WALK_ACCELERATION;
			}
			if(Engine_keys[ENGINE_KEY_W].down
			&& entity_p->physics.onGround){
				entity_p->physics.acceleration.y = -PLAYER_JUMP_ACCELERATION;
			}

		}

		if(entity_p->type == ENTITY_TYPE_ENEMY){

			entity_p->physics.acceleration.y += ENEMY_GRAVITY;

			//look for player
			for(int j = 0; j < world_p->entities.length; j++){

				Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

				if(entity2_p->type == ENTITY_TYPE_PLAYER
				&& fabs(entity2_p->body.pos.x - entity_p->body.pos.x) < ENEMY_DETECTION_RANGE){

					if(entity2_p->body.pos.x < entity_p->body.pos.x){
						entity_p->physics.acceleration.x += -ENEMY_WALK_ACCELERATION;
					}else{
						entity_p->physics.acceleration.x += ENEMY_WALK_ACCELERATION;
					}

					if(entity_p->physics.onGround){
						entity_p->physics.acceleration.y -= ENEMY_JUMP_ACCELERATION;
					}
				
				}

			}

			entity_p->physics.resistance.x = ENEMY_WALK_RESISTANCE;

		}

	}

	forcePoint = offsetPointerPos;

	//check if static particles are bended
	if(Engine_pointer.downed){

		//check that it is not buried
		bool buried = true;
		for(int x = 0; x < BENDING_RADIUS * 2; x++){
			for(int y = 0; y < BENDING_RADIUS * 2; y++){

				Vec2f pos = getVec2f(offsetPointerPos.x - BENDING_RADIUS + x, offsetPointerPos.y - BENDING_RADIUS + y);

				int index = getBufferIndex(pos.x, pos.y);

				if(!checkPixelEquals(world_p->staticParticlesBuffer[index], rockColor)){
					buried = false;	
				}

			}
		}

		if(!buried){
			for(int x = 0; x < BENDING_RADIUS * 2; x++){
				for(int y = 0; y < BENDING_RADIUS * 2; y++){

					Vec2f pos = getVec2f(offsetPointerPos.x - BENDING_RADIUS + x, offsetPointerPos.y - BENDING_RADIUS + y);

					int index = getBufferIndex(pos.x, pos.y);

					if(!World_checkOubVec2f(world_p, pos)
					&& checkPixelEquals(world_p->staticParticlesBuffer[index], rockColor)
					&& getMagVec2f(getSubVec2f(pos, forcePoint)) < BENDING_RADIUS){
						
						world_p->staticParticlesBuffer[index] = backgroundColor;

						Particle *particle_p = World_addParticle(world_p, pos);

					}
				
				}
			}
		}
		
	}

	//apply physics particles
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		particle_p->lastPos = particle_p->pos;

		particle_p->acceleration = getVec2f(0, 0);

		particle_p->acceleration.y += GRAVITY;

		if(Engine_pointer.down
		&& getMagVec2f(getSubVec2f(particle_p->pos, forcePoint)) < BENDING_RADIUS){
			particle_p->isBended = true;
		}

		if(Engine_pointer.down
		&& particle_p->isBended){
			Vec2f diff = getSubVec2f(particle_p->pos, forcePoint);
			float dist = getMagVec2f(diff);
			Vec2f_normalize(&diff);
			Vec2f_invert(&diff);
			Vec2f_mulByFloat(&diff, BENDING_MAGNITUDE);
			Vec2f_mulByFloat(&diff, sqrt(dist));

			Vec2f_add(&particle_p->acceleration, diff);
		}

		Vec2f_add(&particle_p->velocity, particle_p->acceleration);

		Vec2f_mulByFloat(&particle_p->velocity, AIR_FRICTION);

	}

	//apply physics entities
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->lastBody = entity_p->body;

		//entity_p->physics.acceleration = getVec2f(0, 0);

		Vec2f_add(&entity_p->physics.velocity, entity_p->physics.acceleration);

		Vec2f_mul(&entity_p->physics.velocity, entity_p->physics.resistance);

		entity_p->physics.onGround = false;

	}

	//move and collide particles and entities

	//move particles y
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		particle_p->pos.y += particle_p->velocity.y;

	}

	//handle particles oub y
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);
		
		if(particle_p->pos.y < 0){
			particle_p->pos.y = 0;
		}

		if(particle_p->pos.y >= HEIGHT - 1){

			//Array_removeItemByIndex(&world_p->particles, i);
			//i--;
			//continue;
			//particle_p->pos.y = HEIGHT - 1;

		}

	}

	//move entities y
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->body.pos.y += entity_p->physics.velocity.y;

	}

	//handle col y
	
	//put particles into collision buffer y
	memcpy(world_p->collisionBuffer, world_p->clearedCollisionBuffer, sizeof(Collision) * MAX_WIDTH * MAX_HEIGHT);

	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		if(!World_Particle_checkOub(world_p, particle_p)){

			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			world_p->collisionBuffer[index].ID = particle_p->entityHeader.ID;

		}
	
	}

	//check and handle if particles collides with static particles and if they should become static particles y
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		if(World_Particle_checkOub(world_p, particle_p)){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		int startIndex = index;

		if(!checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){

			world_p->collisionBuffer[index].ID = -1;

			int n = 0;
			//bool oub = false;

			bool foundSomething = false;

			while(n < HEIGHT){

				n++;

				index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y + n);

				if((int)particle_p->pos.y + n < HEIGHT
				&& (int)particle_p->pos.y + n >= 0
				&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					break;
				}

				index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y - n);

				if((int)particle_p->pos.y - n < HEIGHT
				&& (int)particle_p->pos.y - n > 0
				&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					n = -n;
					break;
				}
			
			}

			particle_p->pos.y = (int)particle_p->pos.y + n;

			float distanceToForcePoint = getMagVec2f(getSubVec2f(particle_p->pos, forcePoint));
			
			bool inForce = distanceToForcePoint < BENDING_RADIUS + BENDING_RADIUS_MARGIN && Engine_pointer.down;

			if(!inForce){
			//&& !checkPixelEquals(world_p->staticParticlesBuffer[startIndex], metalColor)
			//&& Engine_pointer.down){

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);
			
				world_p->staticParticlesBuffer[index] = rockColor;

				Array_removeItemByIndex(&world_p->particles, i);
				i--;

				continue;
			
			}
			
		}

	}

	//check and handle if particles collide and move them out of the way y
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		if(World_Particle_checkOub(world_p, particle_p)){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		if(world_p->collisionBuffer[index].ID != -1
		&& world_p->collisionBuffer[index].ID != particle_p->entityHeader.ID){

			int n = 0;
			bool foundSomething = false;

			while(n < world_p->levelHeight){

				n++;

				index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y + n);

				if((int)particle_p->pos.y + n < world_p->levelHeight
				&& (int)particle_p->pos.y + n >= 0
				&& world_p->collisionBuffer[index].ID == -1
				&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					break;
				}

				index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y - n);

				if((int)particle_p->pos.y - n < world_p->levelHeight
				&& (int)particle_p->pos.y - n >= 0
				&& world_p->collisionBuffer[index].ID == -1
				&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					n = -n;
					break;
				}
			
			}

			if(!foundSomething){
				printf("Found nothing!\n");
			}

			particle_p->pos.y = (int)particle_p->pos.y + n;

			particle_p->velocity.y *= COLLISION_DAMPING;

			index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			world_p->collisionBuffer[index].ID = particle_p->entityHeader.ID;

		}

	}

	//check entities col y against moving particles
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		for(int y = 0; y < entity_p->body.size.y; y++){
			for(int x = 0; x < entity_p->body.size.x; x++){

				if(World_checkOubVec2f(world_p, getVec2f((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y))){
					continue;
				}

				int index = getBufferIndex((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y);

				if(world_p->collisionBuffer[index].ID != -1){

					Particle *particle_p = Array_getItemPointerByID(&world_p->particles, world_p->collisionBuffer[index].ID);

					float entityCenterY = entity_p->lastBody.pos.y + entity_p->body.size.y / 2;
					float particleY = particle_p->lastPos.y;

					if(particleY > entityCenterY){
						entity_p->body.pos.y = (int)entity_p->body.pos.y - (entity_p->body.size.y - y);
						entity_p->physics.velocity.y = 0;
						entity_p->physics.onGround = true;
					}else{
						entity_p->body.pos.y = (int)entity_p->body.pos.y + y + 1;
						entity_p->physics.velocity.y = 0;
					}
				}

			}
		}
	}

	//check entities col y against static particles
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		for(int y = 0; y < entity_p->body.size.y; y++){
			for(int x = 0; x < entity_p->body.size.x; x++){

				if(World_checkOubVec2f(world_p, getVec2f((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y))){
					continue;
				}

				int index = getBufferIndex((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y);

				if(!checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){

					float entityCenterY = entity_p->lastBody.pos.y + entity_p->body.size.y / 2;
					float particleY = entity_p->body.pos.y + y;

					if(particleY > entityCenterY){
						entity_p->body.pos.y = (int)entity_p->body.pos.y - (entity_p->body.size.y - y);
						entity_p->physics.velocity.y = 0;
						entity_p->physics.onGround = true;
					}else{
						entity_p->body.pos.y = (int)entity_p->body.pos.y + y + 1;
						entity_p->physics.velocity.y = 0;
					}
				}

			}
		}
	}

	//handle oub entities y
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);
		
		if(entity_p->body.pos.y < 0){
			entity_p->body.pos.y = 0;
		}

		if(entity_p->body.pos.y > world_p->levelHeight){
			if(entity_p->type == ENTITY_TYPE_PLAYER){
				world_p->playerDied = true;
			}
		}

	}

	//check entities col y against moving particles second time
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		for(int y = 0; y < entity_p->body.size.y; y++){
			for(int x = 0; x < entity_p->body.size.x; x++){

				if(World_checkOubVec2f(world_p, getVec2f((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y))){
					continue;
				}

				int index = getBufferIndex((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y);

				if(world_p->collisionBuffer[index].ID != -1){

					Particle *particle_p = Array_getItemPointerByID(&world_p->particles, world_p->collisionBuffer[index].ID);

					world_p->collisionBuffer[index].ID = -1;

					//printf("removed ID: %i y\n", particle_p->ID);

					//Array_removeItemByID(&particles, particle_p->ID);

					int n = 0;
					bool foundSomething = false;

					while(n < world_p->levelHeight){

						n++;

						index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y + n);

						if((int)particle_p->pos.y + n < world_p->levelHeight
						&& (int)particle_p->pos.y + n >= 0
						&& world_p->collisionBuffer[index].ID == -1
						&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)
						&& (int)particle_p->pos.y + n < (int)entity_p->body.pos.y
						&& (int)particle_p->pos.y + n > (int)entity_p->body.pos.y + (int)entity_p->body.size.y){
							foundSomething = true;
							break;
						}

						index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y - n);

						if((int)particle_p->pos.y - n < world_p->levelHeight
						&& (int)particle_p->pos.y - n >= 0
						&& world_p->collisionBuffer[index].ID == -1
						&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)
						&& !((int)particle_p->pos.y - n >= (int)entity_p->body.pos.y
						&& (int)particle_p->pos.y - n < (int)entity_p->body.pos.y + (int)entity_p->body.size.y)){
							foundSomething = true;
							n = -n;
							break;
						}
			
					}

					particle_p->pos.y = (int)particle_p->pos.y + n;

					particle_p->velocity.y = 0;

					index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

					world_p->collisionBuffer[index].ID = particle_p->entityHeader.ID;

					if(entity_p->type == ENTITY_TYPE_ENEMY){

						world_p->staticParticlesBuffer[index] = rockColor;

						world_p->collisionBuffer[index].ID = -1;

						Array_removeItemByID(&world_p->particles, particle_p->entityHeader.ID);
						
					}

				}

			}
		}
	}

	//move particles x
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		particle_p->pos.x += particle_p->velocity.x;

	}

	//handle oub x
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);
		
		if(particle_p->pos.x < 0){
			particle_p->pos.x = 0;
		}

		if(particle_p->pos.x >= world_p->levelWidth - 1){
			particle_p->pos.x = world_p->levelWidth - 1;
		}

	}

	//move entities x
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		entity_p->body.pos.x += entity_p->physics.velocity.x;

	}

	//handle col x
	
	//put particles into collision buffer x
	memcpy(world_p->collisionBuffer, world_p->clearedCollisionBuffer, sizeof(Collision) * MAX_WIDTH * MAX_HEIGHT);

	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		if(!World_Particle_checkOub(world_p, particle_p)){

			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			world_p->collisionBuffer[index].ID = particle_p->entityHeader.ID;

		}
	
	}

	//check and handle if particles collides with static particles and should become static particles x
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		if(World_Particle_checkOub(world_p, particle_p)){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		int startIndex = index;

		if(!checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){

			world_p->collisionBuffer[index].ID = -1;

			int n = 0;
			bool oub = false;

			bool foundSomething = false;

			while(n < world_p->levelWidth){

				n++;

				index = getBufferIndex((int)particle_p->pos.x + n, particle_p->pos.y);

				if((int)particle_p->pos.x + n < world_p->levelWidth
				&& (int)particle_p->pos.x + n >= 0
				&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					break;
				}

				index = getBufferIndex((int)particle_p->pos.x - n, particle_p->pos.y);

				if((int)particle_p->pos.x - n < world_p->levelWidth
				&& (int)particle_p->pos.x - n >= 0
				&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					n = -n;
					break;
				}
			
			}

			particle_p->pos.x = (int)particle_p->pos.x + n;

			float distanceToForcePoint = getMagVec2f(getSubVec2f(particle_p->pos, forcePoint));
			
			bool inForce = distanceToForcePoint < BENDING_RADIUS + BENDING_RADIUS_MARGIN && Engine_pointer.down;

			if(!inForce){
			//&& !checkPixelEquals(world_p->staticParticlesBuffer[startIndex], metalColor)
			//&& Engine_pointer.down){

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);
			
				world_p->staticParticlesBuffer[index] = rockColor;
				//collisionBuffer[index].ID = -1;

				Array_removeItemByIndex(&world_p->particles, i);
				i--;

				continue;
			
			}
			
		}

	}

	//check if particles collide and move them out of the way x
	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		if(World_Particle_checkOub(world_p, particle_p)){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		if(world_p->collisionBuffer[index].ID != -1
		&& world_p->collisionBuffer[index].ID != particle_p->entityHeader.ID){

			int n = 0;

			while(n < world_p->levelWidth){

				n++;

				index = getBufferIndex((int)particle_p->pos.x + n, particle_p->pos.y);

				if(particle_p->pos.x + n < world_p->levelWidth
				&& particle_p->pos.x + n >= 0
				&& world_p->collisionBuffer[index].ID == -1
				&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
					break;
				}

				index = getBufferIndex((int)particle_p->pos.x - n, particle_p->pos.y);

				if(particle_p->pos.x - n < world_p->levelWidth
				&& particle_p->pos.x - n >= 0
				&& world_p->collisionBuffer[index].ID == -1
				&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
					n = -n;
					break;
				}
			
			}

			particle_p->pos.x = (int)particle_p->pos.x + n;

			particle_p->velocity.x *= COLLISION_DAMPING;

		}

		index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		world_p->collisionBuffer[index].ID = particle_p->entityHeader.ID;

	}

	//check entities col x against moving particles
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		for(int x = 0; x < entity_p->body.size.x; x++){
			for(int y = 0; y < entity_p->body.size.y; y++){

				if(World_checkOubVec2f(world_p, getVec2f((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y))){
					continue;
				}

				int index = getBufferIndex((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y);

				if(world_p->collisionBuffer[index].ID != -1){

					Particle *particle_p = Array_getItemPointerByID(&world_p->particles, world_p->collisionBuffer[index].ID);

					float entityCenterX = entity_p->lastBody.pos.x + entity_p->body.size.x / 2;
					float particleX = particle_p->lastPos.x;

					if(particleX > entityCenterX){
						entity_p->body.pos.x = (int)entity_p->body.pos.x - (entity_p->body.size.x - x);
						entity_p->physics.velocity.x = 0;
					}else{
						entity_p->body.pos.x = (int)entity_p->body.pos.x + x + 1;
						entity_p->physics.velocity.x = 0;
					}
					
				}

			}
		}

	}

	//check entities col x against static particles
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		for(int x = 0; x < entity_p->body.size.x; x++){
			for(int y = 0; y < entity_p->body.size.y; y++){

				if(World_checkOubVec2f(world_p, getVec2f((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y))){
					continue;
				}

				int index = getBufferIndex((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y);

				if(!checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){

					float entityCenterX = entity_p->lastBody.pos.x + entity_p->body.size.x / 2;
					float particleX = entity_p->body.pos.x + x;

					if(particleX > entityCenterX){
						entity_p->body.pos.x = (int)entity_p->body.pos.x - (entity_p->body.size.x - x);
						entity_p->physics.velocity.x = 0;
					}else{
						entity_p->body.pos.x = (int)entity_p->body.pos.x + x + 1;
						entity_p->physics.velocity.x = 0;
					}
					
				}

			}
		}
	}

	//handle oub entities x
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);
		
		if(entity_p->body.pos.x < 0){
			entity_p->body.pos.x = 0;
		}

		if(entity_p->body.pos.x > world_p->levelWidth){
			if(entity_p->type == ENTITY_TYPE_PLAYER){
				world_p->completedLevel = true;
			}
		}

	}

	//check entities col x against moving particles second time
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		for(int x = 0; x < entity_p->body.size.x; x++){
			for(int y = 0; y < entity_p->body.size.y; y++){

				if(World_checkOubVec2f(world_p, getVec2f((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y))){
					continue;
				}

				int index = getBufferIndex((int)entity_p->body.pos.x + x, (int)entity_p->body.pos.y + y);

				if(world_p->collisionBuffer[index].ID != -1){

					//Array_removeItemByID(&particles, collisionBuffer[index].ID);

					Particle *particle_p = Array_getItemPointerByID(&world_p->particles, world_p->collisionBuffer[index].ID);

					world_p->collisionBuffer[index].ID = -1;

					//printf("removed ID: %i x\n", particle_p->ID);

					//Array_removeItemByID(&particles, particle_p->ID);

					int n = 0;
					bool foundNothing = true;

					while(n < world_p->levelWidth){

						n++;

						index = getBufferIndex((int)particle_p->pos.x + n, particle_p->pos.y);

						if(particle_p->pos.x + n < world_p->levelWidth
						&& particle_p->pos.x + n >= 0
						&& world_p->collisionBuffer[index].ID == -1
						&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
							foundNothing = false;
							break;
						}

						index = getBufferIndex((int)particle_p->pos.x - n, particle_p->pos.y);

						if(particle_p->pos.x - n < world_p->levelWidth
						&& particle_p->pos.x - n >= 0
						&& world_p->collisionBuffer[index].ID == -1
						&& checkPixelEquals(world_p->staticParticlesBuffer[index], backgroundColor)){
							n = -n;
							foundNothing = false;
							break;
						}
					
					}

					if(foundNothing){
						printf("found nothing!\n");
						//break;
					}

					particle_p->pos.x = (int)particle_p->pos.x + n;

					particle_p->velocity.x = 0;
					
					index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

					world_p->collisionBuffer[index].ID = particle_p->entityHeader.ID;

					if(entity_p->type == ENTITY_TYPE_ENEMY){
			
						world_p->staticParticlesBuffer[index] = rockColor;

						world_p->collisionBuffer[index].ID = -1;

						Array_removeItemByID(&world_p->particles, particle_p->entityHeader.ID);
						
					}

				}

			}
		}

	}

	//check if player collides with enemies
	for(int i = 0; i < world_p->entities.length; i++){

		Entity *entity1_p = Array_getItemPointerByIndex(&world_p->entities, i);

		for(int j = 0; j < world_p->entities.length; j++){

			Entity *entity2_p = Array_getItemPointerByIndex(&world_p->entities, j);

			if(i != j
			&& entity1_p->type == ENTITY_TYPE_PLAYER
			&& entity2_p->type == ENTITY_TYPE_ENEMY
			&& checkBodyBodyCollision(entity1_p->body, entity2_p->body)){
				world_p->playerDied = true;
			}
		
		}
	
	}

	//update screen texture
	memcpy(world_p->screenBuffer, world_p->staticParticlesBuffer, sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);

	for(int i = 0; i < world_p->particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&world_p->particles, i);

		if(particle_p->pos.x >= MAX_WIDTH
		|| particle_p->pos.y >= MAX_HEIGHT
		|| particle_p->pos.x < 0
		|| particle_p->pos.y < 0){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		world_p->screenBuffer[index] = rockColor;

	}

	Renderer2D_Texture_free(&world_p->screenTexture);

	Renderer2D_Texture_init(&world_p->screenTexture, "screen-texture", (unsigned char *)world_p->screenBuffer, MAX_WIDTH, MAX_HEIGHT);

	//update offset based on player position
	for(int i = 0; i < world_p->entities.length; i++){
		
		Entity *entity_p = Array_getItemPointerByIndex(&world_p->entities, i);

		if(entity_p->type == ENTITY_TYPE_PLAYER){

			world_p->renderer.offset.x = -entity_p->body.pos.x + WIDTH / 2;

			if(world_p->renderer.offset.x > 0){
				world_p->renderer.offset.x = 0;
			}

			if(world_p->renderer.offset.x < -world_p->levelWidth + WIDTH){
				world_p->renderer.offset.x = -world_p->levelWidth + WIDTH;
			}

		}

	}

}
