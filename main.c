#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"
#include "engine/igui.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

void Engine_start(){

	Engine_setWindowTitle("Simulation sketches");

	Engine_setWindowSize(WIDTH * 1.5, HEIGHT * 1.5);

	Engine_centerWindow();

	Renderer2D_init(&renderer, WIDTH, HEIGHT);

	IGUI_init();

	//init screen rendering
	screenBuffer = malloc(sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);

	Renderer2D_Texture_init(&screenTexture, "screen-texture", (unsigned char *)screenBuffer, MAX_WIDTH, MAX_HEIGHT);

	//init world
	Array_init(&particles, sizeof(Particle));
	staticParticlesBuffer = malloc(sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);
	collisionBuffer = malloc(sizeof(Collision) * MAX_WIDTH * MAX_HEIGHT);
	clearedCollisionBuffer = malloc(sizeof(Collision) * MAX_WIDTH * MAX_HEIGHT);

	Level_init(&currentLevel);

	Array_init(&sprites, sizeof(Sprite));

	currentGameState = GAME_STATE_LEVEL_EDITOR;

	//initLevelState();
	initEditorState();

}

void Engine_update(float deltaTime){

	if(Engine_keys[ENGINE_KEY_Q].down){
		Engine_quit();
	}

	Engine_setPointerScale(Engine_clientWidth / (float)WIDTH, Engine_clientHeight / (float)HEIGHT);

	if(currentGameState == GAME_STATE_LEVEL){
		levelState();
	}
	if(currentGameState == GAME_STATE_LEVEL_EDITOR){
		editorState();
	}

}

void Engine_draw(){

	Renderer2D_updateDrawSize(&renderer, Engine_clientWidth, Engine_clientHeight);

	float alpha = 1.0;
	Renderer2D_Color color;

	//draw world pixels
	Renderer2D_setShaderProgram(&renderer, renderer.textureShaderProgram);

	Renderer2D_beginRectangle(&renderer, 0, 0, MAX_WIDTH, MAX_HEIGHT);

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

	//draw sprites
	for(int i = 0; i < sprites.length; i++){
		
		Sprite *sprite_p = Array_getItemPointerByIndex(&sprites, i);

		Renderer2D_setShaderProgram(&renderer, renderer.colorShaderProgram);

		Renderer2D_beginRectangle(&renderer, (int)sprite_p->pos.x, (int)sprite_p->pos.y, (int)sprite_p->size.x, (int)sprite_p->size.y);

		Renderer2D_supplyUniform(&renderer, &sprite_p->alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);
		Renderer2D_supplyUniform(&renderer, &sprite_p->color, "color", RENDERER2D_UNIFORM_TYPE_COLOR);

		Renderer2D_drawRectangle(&renderer);

	}

	IGUI_render(&renderer);

}

void Engine_finnish(){

}
