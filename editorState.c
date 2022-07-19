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
	DRAWING_TOOL_PLAYER,
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

Vec2f rectanglePos;
Vec2f rectangleSize;
bool holdingRectangle;

Pixel currentColor;

bool openingLevel = false;

void initEditorState(){

	//for(int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++){
		//staticParticlesBuffer[i] = backgroundColor;
	//}

	IGUI_SliderData_init(&drawingRadiusSlider, 0.5);

	IGUI_TextInputData_init(&levelTextInput, currentLevel.name, strlen(currentLevel.name));

	currentColor = rockColor;

}

void editorState(){

	//move offset
	if(Engine_keys[ENGINE_KEY_D].down){
		renderer.offset.x -= 3;
	}
	if(Engine_keys[ENGINE_KEY_A].down){
		renderer.offset.x += 3;
	}

	if(renderer.offset.x > 0){
		renderer.offset.x = 0;
	}
	if(renderer.offset.x < -MAX_WIDTH + WIDTH){
		renderer.offset.x = -MAX_WIDTH + WIDTH;
	}

	Vec2f offsetPointerPos = getSubVec2f(Engine_pointer.pos, renderer.offset);

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

		if(IGUI_textButton_click("Player", getVec2f(posX, posY), 20, currentDrawingTool == DRAWING_TOOL_PLAYER)){
			currentDrawingTool = DRAWING_TOOL_PLAYER;
		}


		if(currentDrawingTool == DRAWING_TOOL_PEN){
			IGUI_slider(getVec2f(130, 50), &drawingRadiusSlider);
		}

		drawingRadius = drawingRadiusSlider.value * 40;

		posX = WIDTH - 130;
		posY = 10;

		if(IGUI_textButton_click("Play Level", getVec2f(posX, posY), 20, false)){
			initLevelState();
			currentGameState = GAME_STATE_LEVEL;	
		}
		posY += 30;

		if(IGUI_textButton_click("New Level", getVec2f(posX, posY), 20, false)){

			Level_init(&currentLevel);

			String_set(levelTextInput.text, currentLevel.name, STRING_SIZE);

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
			
			writeDataToFile(path, (char *)&currentLevel, sizeof(Level));

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

						memcpy(&currentLevel, data, sizeof(Level));

						String_set(levelTextInput.text, currentLevel.name, STRING_SIZE);

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

				if(!checkOubVec2f(pos)
				&& getMagVec2f(getSubVec2f(pos, offsetPointerPos)) < drawingRadius){
					
					currentLevel.staticParticlesBuffer[index] = currentColor;

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

					if(!checkOubVec2f(pos)){
						currentLevel.staticParticlesBuffer[index] = currentColor;
					}
				
				}
			}

			holdingRectangle = false;

		}

		addSprite(rectanglePos, rectangleSize, Renderer2D_getColor(0.0, 0.0, 1.0), alpha);
		
	}

	if(currentDrawingTool == DRAWING_TOOL_PLAYER
	&& !IGUI_hoveringOverGUI){

		if(Engine_pointer.down){
			currentLevel.playerPos = offsetPointerPos;
		}
	
	}

	String_set(currentLevel.name, levelTextInput.text, STRING_SIZE);

	Level_load(&currentLevel);

	//update screen texture
	memcpy(screenBuffer, staticParticlesBuffer, sizeof(Pixel) * MAX_WIDTH * MAX_HEIGHT);

	Renderer2D_Texture_free(&screenTexture);

	Renderer2D_Texture_init(&screenTexture, "screen-texture", (unsigned char *)screenBuffer, MAX_WIDTH, MAX_HEIGHT);

	/*
	for(int i = 0; i < Engine_textInput.length; i++){
		char *text = Array_getItemPointerByIndex(&Engine_textInput, i);
		printf("%s\n", text);
	}
	*/

}
