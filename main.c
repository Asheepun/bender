#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

bool Vec2f_checkOub(Vec2f v){
	if(v.x < 0
	|| v.y < 0
	|| v.x >= WIDTH
	|| v.y >= HEIGHT){
		return true;
	}

	return false;
}

bool Particle_checkOub(Particle *particle_p){
	if(particle_p->pos.x < 0
	|| particle_p->pos.y < 0
	|| particle_p->pos.x >= WIDTH
	|| particle_p->pos.y >= HEIGHT){
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
		clearedCollisionBuffer[i].index = -1;
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

	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		if(Particle_checkOub(particle_p)){
			continue;
		}

		int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

		//check and handle if particle collides with static particle
		if(checkPixelEquals(staticParticlesBuffer[index], rockColor)){

			int n = 0;
			bool oub = false;

			bool foundSomething = false;

			while(n < HEIGHT){

				n++;

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y + n);

				if((int)particle_p->pos.y + n < HEIGHT
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					foundSomething = true;
					break;
				}

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y - n);

				if((int)particle_p->pos.y - n > 0
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

		//check and handle if particle collides with moving particle
		if(collisionBuffer[index].index != -1){

			int movementDirection = 1;
			if(particle_p->velocity.y < 0){
				movementDirection = -1;
			}

			int n = 0;

			while(n < HEIGHT){

				n++;

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y + n);

				if(particle_p->pos.y + n < HEIGHT
				&& collisionBuffer[index].index == -1
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					break;
				}

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y - n);

				if(particle_p->pos.y - n >= 0
				&& collisionBuffer[index].index == -1
				&& checkPixelEquals(staticParticlesBuffer[index], backgroundColor)){
					n = -n;
					break;
				}
			
			}

			particle_p->pos.y = (int)particle_p->pos.y + n;

			particle_p->velocity.y *= COLLISION_DAMPING;

		}

		collisionBuffer[index].index = i;

	}

	/*
	//handle player particle col y
	{
		bool col = false;
		int highestCol = 0;
		int lowestCol = HEIGHT;

		for(int x = 0; x < player.size.x; x++){
			for(int y = 0; y < player.size.y; y++){
				
				Vec2f pos = getVec2f(player.pos.x + x, player.pos.y + y);

				int index = getBufferIndex(pos.x, pos.y);

				if(checkPixelEquals(staticParticlesBuffer[index], rockColor)
				|| collisionBuffer[index].index != -1){
					col = true;

					if(pos.y > highestCol){
						highestCol = pos.y;
					}
					if(pos.y < lowestCol){
						lowestCol = pos.y;
					}

				}

			}
		
		}

		player.onGround = false;

		if(col){

			float playerCenterY = player.pos.y + player.size.y / 2;
			float centerCol = (highestCol + lowestCol) / 2;

			if(playerCenterY < centerCol){
				player.pos.y = lowestCol - player.size.y;
				player.onGround = true;
			}
			if(playerCenterY > centerCol){
				player.pos.y = highestCol;
			}

			player.velocity.y = 0;

		}
	}
	*/

	/*
	//handle moving particles col y
	{

		for(int i = 0; i < particles.length; i++){

			Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

			if(Particle_checkOub(particle_p)){
				continue;
			}

			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			while(collisionBuffer[index].index != -1){
				
				if(particle_p->velocity.y < 0){
					particle_p->pos.y++;
				}
				if(particle_p->velocity.y >= 0){
					particle_p->pos.y--;
				}

				particle_p->velocity.y *= COLLISION_DAMPING;

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);
				
			}

			collisionBuffer[index].index = i;
			
		}
		
	}

	//handle static particles col y
	{
		bool anyCols = true;

		while(anyCols){

			anyCols = false;

			for(int i = 0; i < particles.length; i++){
				
				Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

				if(Particle_checkOub(particle_p)){
					continue;
				}

				bool col = false;

				int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

				while(checkPixelEquals(staticParticlesBuffer[index], rockColor)){

					if(particle_p->velocity.y < 0){
						particle_p->pos.y++;
					}
					if(particle_p->velocity.y >= 0){
						particle_p->pos.y--;
					}

					index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);
				
					col = true;
					anyCols = true;
				
				}

				if(col){
					
					staticParticlesBuffer[index] = rockColor;

					Array_removeItemByIndex(&particles, i);

					i--;

					continue;

				}

			}
		
		}
	}
*/
	
	/*
	//handle player col y
	{
		bool col = false;
		int highestCol = 0;
		int lowestCol = HEIGHT;

		for(int x = 0; x < player.size.x; x++){
			for(int y = 0; y < player.size.y; y++){
				
				Vec2f pos = getVec2f(player.pos.x + x, player.pos.y + y);

				int index = getBufferIndex(pos.x, pos.y);

				if(checkPixelEquals(staticParticlesBuffer[index], rockColor)
				|| collisionBuffer[index].index != -1){
					col = true;

					if(pos.y > highestCol){
						highestCol = pos.y;
					}
					if(pos.y < lowestCol){
						lowestCol = pos.y;
					}

				}

			}
		
		}

		player.onGround = false;

		if(col){

			float playerCenterY = player.pos.y + player.size.y / 2;
			float centerCol = (highestCol + lowestCol) / 2;

			if(playerCenterY < centerCol){
				player.pos.y = lowestCol - player.size.y;
				player.onGround = true;
			}
			if(playerCenterY > centerCol){
				player.pos.y = highestCol;
			}

			player.velocity.y = 0;

		}
	}
	*/

	/*
	//move particles x
	for(int i = 0; i < particles.length; i++){

		Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

		particle_p->pos.x += particle_p->velocity.x;

	}

	//move player x
	{
		player.pos.x += player.velocity.x;
	}

	//handle col x
	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);

	//handle moving particles col x
	memcpy(collisionBuffer, clearedCollisionBuffer, sizeof(Collision) * WIDTH * HEIGHT);
	{
		for(int i = 0; i < particles.length; i++){

			Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

			if(Particle_checkOub(particle_p)){
				continue;
			}

			int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

			while(collisionBuffer[index].index != -1){
				
				if(particle_p->velocity.x < 0){
					particle_p->pos.x++;
				}
				if(particle_p->velocity.x >= 0){
					particle_p->pos.x--;
				}

				particle_p->velocity.x *= COLLISION_DAMPING;

				index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);
				
			}

			collisionBuffer[index].index = i;
			
		}
		
	}

	//handle static particles col x
	{
		bool anyCols = true;

		while(anyCols){

			anyCols = false;

			for(int i = 0; i < particles.length; i++){
				
				Particle *particle_p = Array_getItemPointerByIndex(&particles, i);

				if(Particle_checkOub(particle_p)){
					continue;
				}

				bool col = false;

				int index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);

				while(checkPixelEquals(staticParticlesBuffer[index], rockColor)){

					if(particle_p->velocity.x < 0){
						particle_p->pos.x++;
					}
					if(particle_p->velocity.x >= 0){
						particle_p->pos.x--;
					}

					index = getBufferIndex(particle_p->pos.x, particle_p->pos.y);
				
					col = true;
					anyCols = true;
				
				}

				if(col){
					
					staticParticlesBuffer[index] = rockColor;

					Array_removeItemByIndex(&particles, i);

					i--;

					continue;

				}

			}
		
		}
	}
	
	//handle player col x
	{
		bool col = false;
		int highestCol = 0;
		int lowestCol = WIDTH;

		for(int x = 0; x < player.size.x; x++){
			for(int y = 0; y < player.size.y; y++){
				
				Vec2f pos = getVec2f(player.pos.x + x, player.pos.y + y);

				int index = getBufferIndex(pos.x, pos.y);

				if(checkPixelEquals(staticParticlesBuffer[index], rockColor)
				|| collisionBuffer[index].index != -1){
					col = true;

					if(pos.x > highestCol){
						highestCol = pos.x;
					}
					if(pos.x < lowestCol){
						lowestCol = pos.x;
					}

				}

			}
		
		}

		if(col){

			float playerCenterX = player.pos.x + player.size.x / 2;
			float centerCol = (highestCol + lowestCol) / 2;

			if(playerCenterX < centerCol){
				player.pos.x = lowestCol - player.size.x;
			}
			if(playerCenterX > centerCol){
				player.pos.x = highestCol;
			}

			player.velocity.x = 0;

		}
	}
	*/

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
