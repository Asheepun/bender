#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"
#include "engine/igui.h"
#include "engine/files.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

World world;

void Engine_start(){

	Engine_setWindowTitle("Simulation sketches");

	Engine_setWindowSize(WIDTH * 1.5, HEIGHT * 1.5);

	Engine_centerWindow();

	Renderer2D_init(&world.renderer, WIDTH, HEIGHT);

	IGUI_init();

	World_init(&world);

	//world.currentLevelIndex = 4;
	//world.currentLevelIndex = 3;
	world.currentLevelIndex = 0;

	Level_init(&world.currentLevel);

	Level_loadFromFile(&world.currentLevel, "levels/Untitled.level");

	Level_loadFromFile(&world.currentLevel, "levels/level1.level");

	World_Level_load(&world, &world.currentLevel);

	//currentGameState = GAME_STATE_LEVEL_EDITOR;
	world.currentGameState = GAME_STATE_LEVEL;

	World_initLevelState(&world);
	//World_initEditorState(&world);

}

void Engine_update(float deltaTime){

	//clear sprites
	world.sprites.length = 0;

	if(Engine_keys[ENGINE_KEY_Q].down){
		Engine_quit();
	}

	Engine_setPointerScale(Engine_clientWidth / (float)WIDTH, Engine_clientHeight / (float)HEIGHT);

	if(world.currentGameState == GAME_STATE_LEVEL){
		World_levelState(&world);
	}
	if(world.currentGameState == GAME_STATE_LEVEL_EDITOR){
		World_editorState(&world);
	}

	//update entity sprites
	for(int i = 0; i < world.entities.length; i++){

		Entity *entity_p = Array_getItemPointerByIndex(&world.entities, i);

		if(entity_p->type == ENTITY_TYPE_PLAYER){
			World_addSprite(&world, entity_p->body.pos, entity_p->body.size, Renderer2D_getColor(0.1, 0.2, 0.7), 1.0);
		}

		if(entity_p->type == ENTITY_TYPE_ENEMY){
			World_addSprite(&world, entity_p->body.pos, entity_p->body.size, Renderer2D_getColor(1.0, 0.0, 0.0), 1.0);
		}
	
	}

}

void Engine_draw(){

	Renderer2D_updateDrawSize(&world.renderer, Engine_clientWidth, Engine_clientHeight);

	float alpha = 1.0;
	Renderer2D_Color color;

	//draw world pixels
	Renderer2D_setShaderProgram(&world.renderer, world.renderer.textureShaderProgram);

	Renderer2D_beginRectangle(&world.renderer, 0, 0, MAX_WIDTH, MAX_HEIGHT);

	Renderer2D_setTexture(&world.renderer, world.screenTexture);

	Renderer2D_supplyUniform(&world.renderer, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

	Renderer2D_drawRectangle(&world.renderer);

	//draw sprites
	for(int i = 0; i < world.sprites.length; i++){
		
		Sprite *sprite_p = Array_getItemPointerByIndex(&world.sprites, i);

		Renderer2D_setShaderProgram(&world.renderer, world.renderer.colorShaderProgram);

		Renderer2D_beginRectangle(&world.renderer, (int)sprite_p->pos.x, (int)sprite_p->pos.y, (int)sprite_p->size.x, (int)sprite_p->size.y);

		Renderer2D_supplyUniform(&world.renderer, &sprite_p->alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);
		Renderer2D_supplyUniform(&world.renderer, &sprite_p->color, "color", RENDERER2D_UNIFORM_TYPE_COLOR);

		Renderer2D_drawRectangle(&world.renderer);

	}

	IGUI_render(&world.renderer);

}

void Engine_finnish(){

}
