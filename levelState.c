#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

#include "stdio.h"
#include "math.h"
#include "string.h"


Vec2f forcePoint = { 0, 0 };

void initLevelState(){

	//restore world
	for(int i = 0; i < WIDTH * HEIGHT; i++){
		clearedCollisionBuffer[i].ID = -1;
	}

	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);

	//init player
	{
		player.pos = getVec2f(600, 100);
		player.size = getVec2f(15, 20);
		player.velocity = getVec2f(0, 0);
		player.acceleration = getVec2f(0, 0);
		player.walkForce = 0;
		player.jumpForce = 0;
		player.onGround = false;
		player.collidedWithMovingParticle = false;
		player.collidedWithStaticParticle = false;
	}

	//add obstacles
	for(int x = 0; x < 200; x++){
		for(int y = 0; y < 100; y++){

			Vec2f pos = getVec2f(200 + x - y * 1, 100 + y);

			addParticle(pos);

		}
	}

	for(int i = 0; i < WIDTH * HEIGHT; i++){
		staticParticlesBuffer[i] = backgroundColor;
	}

	for(int x = 0; x < WIDTH; x++){
		for(int y = 0; y < 100; y++){

			int index = getBufferIndex(x, y + HEIGHT - 100);

			staticParticlesBuffer[index] = rockColor;

		}
	}

	for(int x = 0; x < 100; x++){
		for(int y = 0; y < 100; y++){

			int index = getBufferIndex(x, y);

			staticParticlesBuffer[index] = metalColor;

		}
	}

}

void levelState(){

	//control player
	{
		player.walkForce = 0;
		player.jumpForce = 0;

		if(Engine_keys[ENGINE_KEY_A].down){
			player.walkForce = -PLAYER_SIDE_ACCELERATION;
		}
		if(Engine_keys[ENGINE_KEY_D].down){
			player.walkForce = PLAYER_SIDE_ACCELERATION;
		}

		if(Engine_keys[ENGINE_KEY_W].down
		&& player.onGround){
			player.jumpForce = -PLAYER_JUMP_ACCELERATION;
		}

		player.onGround = false;

	}

	forcePoint = Engine_pointer.pos;

	//check if static particles are bended
	if(Engine_pointer.downed){

		//check that it is not buried
		bool buried = true;
		for(int x = 0; x < BENDING_RADIUS * 2; x++){
			for(int y = 0; y < BENDING_RADIUS * 2; y++){

				Vec2f pos = getVec2f(Engine_pointer.pos.x - BENDING_RADIUS + x, Engine_pointer.pos.y - BENDING_RADIUS + y);

				int index = getBufferIndex(pos.x, pos.y);

				if(!checkPixelEquals(staticParticlesBuffer[index], rockColor)){
					buried = false;	
				}

			}
		}

		if(!buried){
			for(int x = 0; x < BENDING_RADIUS * 2; x++){
				for(int y = 0; y < BENDING_RADIUS * 2; y++){

					Vec2f pos = getVec2f(Engine_pointer.pos.x - BENDING_RADIUS + x, Engine_pointer.pos.y - BENDING_RADIUS + y);

					int index = getBufferIndex(pos.x, pos.y);

					if(!checkOubVec2f(pos)
					&& checkPixelEquals(staticParticlesBuffer[index], rockColor)
					&& getMagVec2f(getSubVec2f(pos, forcePoint)) < BENDING_RADIUS){
						
						staticParticlesBuffer[index] = backgroundColor;

						Particle *particle_p = addParticle(pos);

					}
				
				}
			}
		}
		
	}

	//apply physics particles
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

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

	//apply physics player
	{
		player.acceleration = getVec2f(0, 0);

		player.acceleration.y += PLAYER_GRAVITY;

		player.acceleration.y += player.jumpForce;

		player.acceleration.x += player.walkForce;

		Vec2f_add(&player.velocity, player.acceleration);

		player.velocity.x *= PLAYER_SIDE_RESITANCE;
	}

	//move and collide particles

	//move particles y
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		particle_p->pos.y += particle_p->velocity.y;

	}

	//handle col y
	
	//put particles into collision buffer y
	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);

	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		if(!Particle_checkOub(particle_p)){

			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			collisionBuffer[index].ID = particle_p->ID;

		}
	
	}

	//check and handle if particles collides with static particles and if they should become static particles y
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		if(Particle_checkOub(particle_p)){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		if(!checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){

			int n = 0;
			bool oub = false;

			bool foundSomething = false;

			while(n < HEIGHT){

				n++;

				index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y + n);

				if((int)particle_p->pos.y + n < HEIGHT
				&& (int)particle_p->pos.y + n >= 0
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					break;
				}

				index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y - n);

				if((int)particle_p->pos.y - n < HEIGHT
				&& (int)particle_p->pos.y - n > 0
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					n = -n;
					break;
				}
			
			}

			particle_p->pos.y = (int)particle_p->pos.y + n;

			float distanceToForcePoint = getMagVec2f(getSubVec2f(particle_p->pos, forcePoint));
			
			bool inForce = distanceToForcePoint < BENDING_RADIUS + BENDING_RADIUS_MARGIN && Engine_pointer.down;

			if(!inForce){

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);
			
				staticParticlesBuffer[index] = rockColor;

				Array_removeItemByIndex(&particles, i);
				i--;

				continue;
			
			}
			
		}

	}

	//check and handle if particles collide and move them out of the way y
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		if((int)particle_p->pos.y < 0
		|| (int)particle_p->pos.y >= HEIGHT
		|| collisionBuffer[index].ID != -1
		&& collisionBuffer[index].ID != particle_p->ID
		|| !checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){

			int n = 0;
			bool foundSomething = false;

			while(n < HEIGHT){

				n++;

				index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y + n);

				if((int)particle_p->pos.y + n < HEIGHT
				&& (int)particle_p->pos.y + n >= 0
				&& collisionBuffer[index].ID == -1
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					break;
				}

				index = getBufferIndex(particle_p->pos.x, (int)particle_p->pos.y - n);

				if((int)particle_p->pos.y - n < HEIGHT
				&& (int)particle_p->pos.y - n >= 0
				&& collisionBuffer[index].ID == -1
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
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

		}

		index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		collisionBuffer[index].ID = particle_p->ID;

	}

	//check player col y against moving particles
	for(int y = 0; y < player.size.y; y++){
		for(int x = 0; x < player.size.x; x++){

			int index = getBufferIndex((int)player.pos.x + x, (int)player.pos.y + y);

			if(collisionBuffer[index].ID != -1){
				if(y > player.size.y / 2){
					player.pos.y = (int)player.pos.y - (player.size.y - y);
					player.velocity.y = 0;
					player.onGround = true;
				}else{
					player.pos.y = (int)player.pos.y + y + 1;
					player.velocity.y = 0;
				}
			}

		}
	}

	//check player col y against static particles
	for(int y = 0; y < player.size.y; y++){
		for(int x = 0; x < player.size.x; x++){

			if(checkOubVec2f(getVec2f((int)player.pos.x + x, (int)player.pos.y + y))){
				continue;
			}

			int index = getBufferIndex((int)player.pos.x + x, (int)player.pos.y + y);

			if(!checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
				if(y > player.size.y / 2){
					player.pos.y = (int)player.pos.y - (player.size.y - y);
					player.velocity.y = 0;
					player.onGround = true;
				}else{
					player.pos.y = (int)player.pos.y + y + 1;
					player.velocity.y = 0;
				}
			}

		}
	}

	//check player col y against moving particles second time
	for(int y = 0; y < player.size.y; y++){
		for(int x = 0; x < player.size.x; x++){

			if(checkOubVec2f(getVec2f((int)player.pos.x + x, (int)player.pos.y + y))){
				continue;
			}

			int index = getBufferIndex((int)player.pos.x + x, (int)player.pos.y + y);

			if(collisionBuffer[index].ID != -1){

				Particle *particle_p = Array_getItemPointerByID(&particles, collisionBuffer[index].ID);
				collisionBuffer[index].ID = -1;

				if(y > player.size.y / 2){
					for(int i = (int)player.pos.y + y; i < HEIGHT; i++){

						int index = getBufferIndex((int)player.pos.x + x, (int)i);

						if(collisionBuffer[index].ID == -1
						&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)
						&& i > player.pos.y + player.size.y){
							collisionBuffer[index].ID = particle_p->ID;
							particle_p->pos.y = (int)i;
							particle_p->velocity.y = 0;
							break;
						}

						if(i == HEIGHT - 1){
							Array_removeItemByID(&particles, collisionBuffer[index].ID);
						}

					}
				}else{
					for(int i = (int)player.pos.y + y; i >= 0; i--){

						int index = getBufferIndex((int)player.pos.x + x, (int)i);

						if(collisionBuffer[index].ID == -1
						&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)
						&& i < player.pos.y){
							collisionBuffer[index].ID = particle_p->ID;
							particle_p->pos.y = (int)i;
							particle_p->velocity.y = 0;
							break;
						}

						if(i == 0){
							Array_removeItemByID(&particles, collisionBuffer[index].ID);
						}

					}
				}

			}

		}
	}

	//move particles x
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		particle_p->pos.x += particle_p->velocity.x;

	}

	//handle col x
	
	//put particles into collision buffer x
	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);

	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		if(!Particle_checkOub(particle_p)){

			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			collisionBuffer[index].ID = particle_p->ID;

		}
	
	}

	//check and handle if particles collides with static particles and should become static particles x
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		if(Particle_checkOub(particle_p)){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		if(!checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){

			int n = 0;
			bool oub = false;

			bool foundSomething = false;

			while(n < WIDTH){

				n++;

				index = getBufferIndex((int)particle_p->pos.x + n, particle_p->pos.y);

				if((int)particle_p->pos.x + n < WIDTH
				&& (int)particle_p->pos.x + n >= 0
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					break;
				}

				index = getBufferIndex((int)particle_p->pos.x - n, particle_p->pos.y);

				if((int)particle_p->pos.x - n < WIDTH
				&& (int)particle_p->pos.x - n >= 0
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					n = -n;
					break;
				}
			
			}

			particle_p->pos.x = (int)particle_p->pos.x + n;

			float distanceToForcePoint = getMagVec2f(getSubVec2f(particle_p->pos, forcePoint));
			
			bool inForce = distanceToForcePoint < BENDING_RADIUS + BENDING_RADIUS_MARGIN && Engine_pointer.down;

			if(!inForce){

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);
			
				staticParticlesBuffer[index] = rockColor;

				Array_removeItemByIndex(&particles, i);
				i--;

				continue;
			
			}
			
		}

	}

	//check if particles collide and move them out of the way x
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		if((int)particle_p->pos.x < 0
		|| (int)particle_p->pos.x >= WIDTH
		|| collisionBuffer[index].ID != -1
		&& collisionBuffer[index].ID != particle_p->ID
		|| !checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){

			int n = 0;

			while(n < WIDTH){

				n++;

				index = getBufferIndex((int)particle_p->pos.x + n, particle_p->pos.y);

				if(particle_p->pos.x + n < WIDTH
				&& particle_p->pos.x + n >= 0
				&& collisionBuffer[index].ID == -1
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					break;
				}

				index = getBufferIndex((int)particle_p->pos.x - n, particle_p->pos.y);

				if(particle_p->pos.x - n < WIDTH
				&& particle_p->pos.x - n >= 0
				&& collisionBuffer[index].ID == -1
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					n = -n;
					break;
				}
			
			}

			particle_p->pos.x = (int)particle_p->pos.x + n;

			particle_p->velocity.x *= COLLISION_DAMPING;

		}

		index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		collisionBuffer[index].ID = particle_p->ID;

	}

	//check player col x against moving particles
	for(int x = 0; x < player.size.x; x++){
		for(int y = 0; y < player.size.y; y++){

			if(checkOubVec2f(getVec2f((int)player.pos.x + x, (int)player.pos.y + y))){
				continue;
			}

			int index = getBufferIndex((int)player.pos.x + x, (int)player.pos.y + y);

			if(collisionBuffer[index].ID != -1){

				if(x > player.size.x / 2){
					player.pos.x = (int)player.pos.x - (player.size.x - x);
					player.velocity.x = 0;
				}else{
					player.pos.x = (int)player.pos.x + x + 1;
					player.velocity.x = 0;
				}
				
			}

		}
	}
	//check player col x against static particles
	for(int x = 0; x < player.size.x; x++){
		for(int y = 0; y < player.size.y; y++){

			if(checkOubVec2f(getVec2f((int)player.pos.x + x, (int)player.pos.y + y))){
				continue;
			}

			int index = getBufferIndex((int)player.pos.x + x, (int)player.pos.y + y);

			if(!checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){

				if(x > player.size.x / 2){
					player.pos.x = (int)player.pos.x - (player.size.x - x);
					player.velocity.x = 0;
				}else{
					player.pos.x = (int)player.pos.x + x + 1;
					player.velocity.x = 0;
				}
				
			}

		}
	}

	//check player col x against moving particles second time
	for(int x = 0; x < player.size.x; x++){
		for(int y = 0; y < player.size.y; y++){

			if(checkOubVec2f(getVec2f((int)player.pos.x + x, (int)player.pos.y + y))){
				continue;
			}

			int index = getBufferIndex((int)player.pos.x + x, (int)player.pos.y + y);

			if(collisionBuffer[index].ID != -1){

				//Array_removeItemByID(&particles, collisionBuffer[index].ID);

				Particle *particle_p = Array_getItemPointerByID(&particles, collisionBuffer[index].ID);
				collisionBuffer[index].ID = -1;

				if(x > player.size.x / 2){
					for(int i = (int)player.pos.x + x; i < WIDTH; i++){

						int index = getBufferIndex((int)i, (int)player.pos.y + y);

						if(collisionBuffer[index].ID == -1
						&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)
						&& i > player.pos.x + player.size.x){
							collisionBuffer[index].ID = particle_p->ID;
							particle_p->pos.x = (int)i;
							particle_p->velocity.x = 0;
							break;
						}

						if(i == WIDTH - 1){
							Array_removeItemByID(&particles, collisionBuffer[index].ID);
						}

					}
				}else{
					for(int i = (int)player.pos.x + x; i >= 0; i--){

						int index = getBufferIndex((int)i, (int)player.pos.y + y);

						if(collisionBuffer[index].ID == -1
						&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)
						&& i < player.pos.x){
							collisionBuffer[index].ID = particle_p->ID;
							particle_p->pos.x = (int)i;
							particle_p->velocity.x = 0;
							break;
						}

						if(i == 0){
							Array_removeItemByID(&particles, collisionBuffer[index].ID);
						}

					}
				}
				
			}

		}
	}

	//move and collide player
	
	//put particles into collision buffer
	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);

	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		if(!Particle_checkOub(particle_p)){

			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			collisionBuffer[index].ID = particle_p->ID;

		}
	
	}

	//move player y
	{
		player.pos.y += player.velocity.y;
	}

	//check player col y
	for(int y = 0; y < player.size.y; y++){
		for(int x = 0; x < player.size.x; x++){

			if(checkOubVec2f(getVec2f((int)player.pos.x + x, (int)player.pos.y + y))){
				continue;
			}

			int index = getBufferIndex((int)player.pos.x + x, (int)player.pos.y + y);

			if(collisionBuffer[index].ID != -1
			|| !checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){

				if(y > player.size.y / 2){
					player.pos.y = (int)player.pos.y - (player.size.y - y);
					player.velocity.y = 0;
					player.onGround = true;
				}else{
					player.pos.y = (int)player.pos.y + y + 1;
					player.velocity.y = 0;
				}
				
			}

		}
	}

	//move player x
	{
		player.pos.x += player.velocity.x;
	}

	//check player col x
	for(int x = 0; x < player.size.x; x++){
		for(int y = 0; y < player.size.y; y++){

			if(checkOubVec2f(getVec2f((int)player.pos.x + x, (int)player.pos.y + y))){
				continue;
			}

			int index = getBufferIndex((int)player.pos.x + x, (int)player.pos.y + y);

			if(collisionBuffer[index].ID != -1
			|| !checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){

				if(x > player.size.x / 2){
					player.pos.x = (int)player.pos.x - (player.size.x - x);
					player.velocity.x = 0;
				}else{
					player.pos.x = (int)player.pos.x + x + 1;
					player.velocity.x = 0;
				}
				
			}

		}
	}

	//update screen texture
	memcpy(screenBuffer, staticParticlesBuffer, sizeof(Pixel) * WIDTH * HEIGHT);

	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		if(particle_p->pos.x >= WIDTH
		|| particle_p->pos.y >= HEIGHT
		|| particle_p->pos.x < 0
		|| particle_p->pos.y < 0){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		screenBuffer[index] = rockColor;

	}

	Renderer2D_Texture_free(&screenTexture);

	Renderer2D_Texture_init(&screenTexture, "screen-texture", (unsigned char *)screenBuffer, WIDTH, HEIGHT);


}
