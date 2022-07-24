#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/igui.h"
#include "engine/array.h"
#include "engine/files.h"

#include "stdio.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"
#include "dirent.h"

enum DrawingTools{
	DRAWING_TOOL_NONE,
	DRAWING_TOOL_PEN,
	DRAWING_TOOL_RECTANGLE,
	DRAWING_TOOL_ENTITY,
	DRAWING_TOOL_WIDTH,
	NUMBER_OF_DRAWING_TOOLS,
};

enum DrawingModes{
	DRAWING_MODE_PLACING,
	DRAWING_MODE_REMOVING,
	NUMBER_OF_DRWAING_MODES,
};

int drawingRadius = 20;
bool displayGUI = true;

IGUI_SliderData drawingRadiusSlider;
IGUI_TextInputData levelTextInput;

enum DrawingTools currentDrawingTool = DRAWING_TOOL_RECTANGLE;
enum EntityType currentDrawingEntityType = ENTITY_TYPE_PLAYER;
enum DrawingModes currentDrawingMode = DRAWING_MODE_PLACING;

Vec2f rectanglePos;
Vec2f rectangleSize;
bool holdingRectangle;

Pixel currentColor;

bool openingLevel = false;

void World_initEditorState(World *world_p){

	//for(int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++){
		//staticParticlesBuffer[i] = backgroundColor;
	//}

	IGUI_SliderData_init(&drawingRadiusSlider, 0.5);

	IGUI_TextInputData_init(&levelTextInput, world_p->currentLevel.name, strlen(world_p->currentLevel.name));

	currentColor = rockColor;

}

void World_editorState(World *world_p){

	//move offset
	if(Engine_keys[ENGINE_KEY_D].down){
		world_p->renderer.offset.x -= 3;
	}
	if(Engine_keys[ENGINE_KEY_A].down){
		world_p->renderer.offset.x += 3;
	}

	if(world_p->renderer.offset.x > 0){
		world_p->renderer.offset.x = 0;
	}
	if(world_p->renderer.offset.x < -MAX_WIDTH + WIDTH){
		world_p->renderer.offset.x = -MAX_WIDTH + WIDTH;
	}

	Vec2f offsetPointerPos = getSubVec2f(Engine_pointer.pos, world_p->renderer.offset);

	//GUI
	if(Engine_keys[ENGINE_KEY_H].downed){
		displayGUI = !displayGUI;
	}

	if(displayGUI){

		int posX = 30;
		int posY = 10;

		if(currentDrawingTool == DRAWING_TOOL_PEN
		|| currentDrawingTool == DRAWING_TOOL_RECTANGLE){
			if(IGUI_textButton_click("Rock", getVec2f(posX, posY), 20, checkPixelEquals(currentColor, rockColor))){
				currentColor = rockColor;
			}
			posY += 30;

			if(IGUI_textButton_click("Metal", getVec2f(posX, posY), 20, checkPixelEquals(currentColor, metalColor))){
				currentColor = metalColor;
			}
			posY += 30;

			if(IGUI_textButton_click("Air", getVec2f(posX, posY), 20, checkPixelEquals(currentColor, backgroundColor))){
				currentColor = backgroundColor;
			}
			posY += 30;
		}

		if(currentDrawingTool == DRAWING_TOOL_ENTITY){

			if(IGUI_textButton_click("Player", getVec2f(posX, posY), 20, currentDrawingEntityType == ENTITY_TYPE_PLAYER)){
				currentDrawingEntityType = ENTITY_TYPE_PLAYER;
			}
			posY += 30;

			if(IGUI_textButton_click("Enemy", getVec2f(posX, posY), 20, currentDrawingEntityType == ENTITY_TYPE_ENEMY)){
				currentDrawingEntityType = ENTITY_TYPE_ENEMY;
			}
		
		}

		posX = 130;
		posY = 10;

		if(IGUI_textButton_click("Pen", getVec2f(posX, posY), 20, currentDrawingTool == DRAWING_TOOL_PEN)){
			currentDrawingTool = DRAWING_TOOL_PEN;
		}
		posX += 50;

		if(IGUI_textButton_click("Rect", getVec2f(posX, posY), 20, currentDrawingTool == DRAWING_TOOL_RECTANGLE)){
			currentDrawingTool = DRAWING_TOOL_RECTANGLE;
		}

		posX += 50;

		if(IGUI_textButton_click("Entity", getVec2f(posX, posY), 20, currentDrawingTool == DRAWING_TOOL_ENTITY)){
			currentDrawingTool = DRAWING_TOOL_ENTITY;
		}

		posX += 70;

		if(IGUI_textButton_click("Width", getVec2f(posX, posY), 20, currentDrawingTool == DRAWING_TOOL_WIDTH)){
			currentDrawingTool = DRAWING_TOOL_WIDTH;
		}

		if(currentDrawingTool == DRAWING_TOOL_ENTITY
		&& currentDrawingEntityType == ENTITY_TYPE_ENEMY){
			if(IGUI_textButton_click("Place", getVec2f(130, 40), 20, currentDrawingMode == DRAWING_MODE_PLACING)){
				currentDrawingMode = DRAWING_MODE_PLACING;
			}
			if(IGUI_textButton_click("Remove", getVec2f(190, 40), 20, currentDrawingMode == DRAWING_MODE_REMOVING)){
				currentDrawingMode = DRAWING_MODE_REMOVING;
			}
			
		}

		if(currentDrawingTool == DRAWING_TOOL_PEN){
			IGUI_slider(getVec2f(130, 50), &drawingRadiusSlider);
		}

		drawingRadius = drawingRadiusSlider.value * 40;

		posX = WIDTH - 130;
		posY = 10;

		if(IGUI_textButton_click("Play Level", getVec2f(posX, posY), 20, false)){
			World_initLevelState(world_p);
			world_p->currentGameState = GAME_STATE_LEVEL;	
		}
		posY += 30;

		if(IGUI_textButton_click("New Level", getVec2f(posX, posY), 20, false)){

			Level_init(&world_p->currentLevel);

			String_set(levelTextInput.text, world_p->currentLevel.name, STRING_SIZE);

		}
		posY += 30;

		if(IGUI_textButton_click("Open Level", getVec2f(posX, posY), 20, false)){
			openingLevel = true;
		}
		posY += 30;

		if(IGUI_textButton_click("Save Level", getVec2f(posX, posY), 20, false)){

			char path[STRING_SIZE];
			String_set(path, "levels/", STRING_SIZE);
			String_append(path, levelTextInput.text);
			String_append(path, ".level");
			
			writeDataToFile(path, (char *)&world_p->currentLevel, sizeof(Level));

		}
		posY += 30;

		if(IGUI_textButton_click("Delete Level", getVec2f(posX, posY), 20, false)){

		}
		posY += 30;

		if(openingLevel){

			posX = WIDTH - 220;
			posY = 10;

			char dirPath[STRING_SIZE];
			String_set(dirPath, "levels/", STRING_SIZE);

			DIR *dataDir = opendir(dirPath);
			struct dirent* dirEntry;

			while((dirEntry = readdir(dataDir)) != NULL){

				if(strcmp(dirEntry->d_name, ".") != 0
				&& strcmp(dirEntry->d_name, "..") != 0){

					char fileName[STRING_SIZE];
					String_set(fileName, dirEntry->d_name, STRING_SIZE);

					char path[STRING_SIZE];
					String_set(path, dirPath, STRING_SIZE);
					String_append(path, fileName);
					
					char levelName[STRING_SIZE];
					String_set(levelName, fileName, STRING_SIZE);
					memset(strrchr(levelName, *"."), *"\0", 1);

					if(IGUI_textButton_click(levelName, getVec2f(posX, posY), 20, false)){

						String_set(levelTextInput.text, levelName, STRING_SIZE);

						long int fileSize;
						char *data = getFileData_mustFree(path, &fileSize);

						memcpy(&world_p->currentLevel, data, sizeof(Level));

						String_set(levelTextInput.text, world_p->currentLevel.name, STRING_SIZE);

						free(data);

					}
					posY += 30;

				}
			
			}
		
		}

		if(Engine_pointer.downed
		&& !IGUI_hoveringOverGUI){
			openingLevel = false;
		}

		IGUI_textInput(getVec2f(380, 10), &levelTextInput);
		
	}

	//drawing
	if(currentDrawingTool == DRAWING_TOOL_PEN
	&& Engine_pointer.down
	&& !IGUI_hoveringOverGUI){

		for(int x = 0; x < drawingRadius * 2; x++){
			for(int y = 0; y < drawingRadius * 2; y++){

				Vec2f pos = getVec2f(offsetPointerPos.x - drawingRadius + x, offsetPointerPos.y - drawingRadius + y);

				int index = getBufferIndex(pos.x, pos.y);

				if(!World_checkOubVec2f(world_p, pos)
				&& getMagVec2f(getSubVec2f(pos, offsetPointerPos)) < drawingRadius){
					
					world_p->currentLevel.staticParticlesBuffer[index] = currentColor;

				}

			
			}
		}
	
	}

	if(currentDrawingTool == DRAWING_TOOL_RECTANGLE){

		float alpha = 0.0;
	
		if(Engine_pointer.downed
		&& !IGUI_hoveringOverGUI){
			rectanglePos = offsetPointerPos;
			holdingRectangle = true;
		}

		rectangleSize = getVec2f(0, 0);
		if(holdingRectangle){
			rectangleSize = getSubVec2f(offsetPointerPos, rectanglePos);
			alpha = 0.5;
		}

		if(Engine_pointer.upped){

			if(rectangleSize.x < 0){
				rectanglePos.x += rectangleSize.x;
				rectangleSize.x *= -1;
			}
			if(rectangleSize.y < 0){
				rectanglePos.y += rectangleSize.y;
				rectangleSize.y *= -1;
			}

			for(int x = 0; x < rectangleSize.x; x++){
				for(int y = 0; y < rectangleSize.y; y++){

					Vec2f pos = getVec2f((int)rectanglePos.x + x, (int)rectanglePos.y + y);

					int index = getBufferIndex(pos.x, pos.y);

					if(!World_checkOubVec2f(world_p, pos)){
						world_p->currentLevel.staticParticlesBuffer[index] = currentColor;
					}
				
				}
			}

			holdingRectangle = false;

		}

		World_addSprite(world_p, rectanglePos, rectangleSize, Renderer2D_getColor(0.0, 0.0, 1.0), alpha);
		
	}

	if(currentDrawingTool == DRAWING_TOOL_ENTITY
	&& !IGUI_hoveringOverGUI){

		if(currentDrawingEntityType == ENTITY_TYPE_PLAYER
		&& Engine_pointer.down){
			world_p->currentLevel.playerPos = offsetPointerPos;
		}

		if(Engine_pointer.downed){

			if(currentDrawingEntityType == ENTITY_TYPE_ENEMY){

				if(currentDrawingMode == DRAWING_MODE_PLACING){
					world_p->currentLevel.enemyPoses[world_p->currentLevel.enemyPosesLength] = offsetPointerPos;
					world_p->currentLevel.enemyPosesLength++;
				}
				if(currentDrawingMode == DRAWING_MODE_REMOVING){

					for(int i = 0; i < world_p->currentLevel.enemyPosesLength; i++){

						if(offsetPointerPos.x > world_p->currentLevel.enemyPoses[i].x
						&& offsetPointerPos.x < world_p->currentLevel.enemyPoses[i].x + 15
						&& offsetPointerPos.y > world_p->currentLevel.enemyPoses[i].y
						&& offsetPointerPos.y < world_p->currentLevel.enemyPoses[i].y + 20){

							memcpy(world_p->currentLevel.enemyPoses + i, world_p->currentLevel.enemyPoses + i + 1, (world_p->currentLevel.enemyPosesLength - (i + 1)) * sizeof(Vec2f));
							world_p->currentLevel.enemyPosesLength--;
							
							break;
						}

					}

				}

			}

		}
	
	}

	if(currentDrawingTool == DRAWING_TOOL_WIDTH
	&& Engine_pointer.down
	&& !IGUI_hoveringOverGUI){
		
		world_p->currentLevel.width = offsetPointerPos.x;

	}

	World_addSprite(world_p, getVec2f(world_p->currentLevel.width, 0), getVec2f(10, HEIGHT), Renderer2D_getColor(1.0, 0.0, 0.0), 0.5);

	String_set(world_p->currentLevel.name, levelTextInput.text, STRING_SIZE);

	World_Level_load(world_p, &world_p->currentLevel);

	//update screen texture
	memcpy(world_p->screenBuffer, world_p->staticParticlesBuffer, sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);

	Renderer2D_Texture_free(&world_p->screenTexture);

	Renderer2D_Texture_init(&world_p->screenTexture, "screen-texture", (unsigned char *)world_p->screenBuffer, MAX_WIDTH, MAX_HEIGHT);

	/*
	for(int i = 0; i < Engine_textInput.length; i++){
		char *text = Array_getItemPointerByIndex(&Engine_textInput, i);
		printf("%s\n", text);
	}
	*/

}
