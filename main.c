#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

bool Vec2f_checkOub(Vec2f v){
	if((int)v.x < 0
	|| (int)v.y < 0
	|| (int)v.x >= WIDTH
	|| (int)v.y >= HEIGHT){
		return true;
	}

	return false;
}

bool Particle_checkOub(Particle *particle_p){
	if((int)particle_p->pos.x < 0
	|| (int)particle_p->pos.y < 0
	|| (int)particle_p->pos.x >= WIDTH
	|| (int)particle_p->pos.y >= HEIGHT){
		return true;
	}

	return false;
}

void Engine_start(){

	Engine_setWindowTitle("Simulation sketches");

	Engine_setWindowSize(WIDTH * 1.5, HEIGHT * 1.5);

	Engine_centerWindow();

	Renderer2D_init(&renderer, WIDTH, HEIGHT);

	//init screen rendering
	screenBuffer = malloc(sizeof(Pixel) * WIDTH * HEIGHT);

	Renderer2D_Texture_init(&screenTexture, "screen-texture", (unsigned char *)screenBuffer, WIDTH, HEIGHT);

	//init world
	Array_init(&particles, sizeof(Particle));
	staticParticlesBuffer = malloc(sizeof(Pixel) * WIDTH * HEIGHT);
	collisionBuffer = malloc(sizeof(Collision) * WIDTH * HEIGHT);
	clearedCollisionBuffer = malloc(sizeof(Collision) * WIDTH * HEIGHT);

	{
		player.pos = getVec2f(600, 100);
		player.size = getVec2f(15, 20);
		player.velocity = getVec2f(0, 0);
		player.acceleration = getVec2f(0, 0);
		player.walkForce = 0;
		player.jumpForce = 0;
		player.onGround = false;
	}

	for(int i = 0; i < WIDTH * HEIGHT; i++){
		clearedCollisionBuffer[i].ID = -1;
	}

	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);

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

}

Vec2f forcePoint = { 0, 0 };

void Engine_update(float deltaTime){

	if(Engine_keys[ENGINE_KEY_Q].down){
		Engine_quit();
	}

	Engine_setPointerScale(Engine_clientWidth / (float)WIDTH, Engine_clientHeight / (float)HEIGHT);

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

					if(!Vec2f_checkOub(pos)
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

	//move particles y
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		particle_p->pos.y += particle_p->velocity.y;

	}

	//move player y
	{
		player.pos.y += player.velocity.y;
	}

	//handle col y
	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);

	//put particles into collision buffer y
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

		if(checkPixelEquals(staticParticlesBuffer[index], rockColor)){

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
		|| checkPixelEquals(staticParticlesBuffer[index], rockColor)){

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

			/*
			if(collisionBuffer[index].ID != -1){
				printf("bad\n");
				printf("buffer: %i\n", collisionBuffer[index].ID);
				printf("particle: %i\n", particle_p->ID);
				printf("n: %i\n", n);
				Vec2f_log(particle_p->pos);
			}
			*/

		}

		index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		collisionBuffer[index].ID = particle_p->ID;

		{

			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			if(checkPixelEquals(staticParticlesBuffer[index], rockColor)
			|| collisionBuffer[index].ID != particle_p->ID
			&& collisionBuffer[index].ID != -1){
				printf("STILL COL AFTER Y FIX!\n");
				//Vec2f_log(particle_p->pos);
				//printf("particle: %i\n", particle_p->ID);
				//printf("buffer: %i\n", collisionBuffer[index].ID);
			}
		
		}

	}

	//move particles x
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		if((int)particle_p->pos.y < 0){
			printf("OUB PARTICLE!\n");
		}

		{
			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			if(checkPixelEquals(staticParticlesBuffer[index], rockColor)){
				printf("STILL COL AFTER Y FIX AFTER THE FACT 2!\n");
			}
			if(collisionBuffer[index].ID != particle_p->ID
			&& collisionBuffer[index].ID != -1){
				printf("buffer: %i\n", collisionBuffer[index].ID);
				printf("particle: %i\n", particle_p->ID);
				printf("333STILL COL AFTER Y FIX AFTER THE FACT 3!\n");
			}
		}

		particle_p->pos.x += particle_p->velocity.x;

	}

	//move player x
	{
		player.pos.x += player.velocity.x;
	}

	//handle col x
	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);

	//put particles into collision buffer x
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

		if(checkPixelEquals(staticParticlesBuffer[index], rockColor)){

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
		|| checkPixelEquals(staticParticlesBuffer[index], rockColor)){

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

void Engine_draw(){

	Renderer2D_updateDrawSize(&renderer, Engine_clientWidth, Engine_clientHeight);

	float alpha = 1.0;
	Renderer2D_Color color;

	//draw world pixels
	Renderer2D_setShaderProgram(&renderer, renderer.textureShaderProgram);

	Renderer2D_beginRectangle(&renderer, 0, 0, WIDTH, HEIGHT);

	Renderer2D_setTexture(&renderer, screenTexture);

	Renderer2D_supplyUniform(&renderer, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

	Renderer2D_drawRectangle(&renderer);

	//draw player
	color = Renderer2D_getColor(0.1, 0.2, 0.7);

	Renderer2D_setShaderProgram(&renderer, renderer.colorShaderProgram);

	Renderer2D_beginRectangle(&renderer, (int)player.pos.x, (int)player.pos.y, (int)player.size.x, (int)player.size.y);

	Renderer2D_supplyUniform(&renderer, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);
	Renderer2D_supplyUniform(&renderer, &color, "color", RENDERER2D_UNIFORM_TYPE_COLOR);

	Renderer2D_drawRectangle(&renderer);

}

void Engine_finnish(){

}
