#include "engine/igui.h"

#include "engine/renderer2d.h"
#include "engine/engine.h"
#include "engine/geometry.h"
#include "engine/text.h"
#include "engine/array.h"
#include "engine/strings.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

typedef struct TextButton{
	char text[STRING_SIZE];
	int fontSize;
	Vec2f pos;
	Vec2f size;
	int paddingX;
	Renderer2D_Color textColor;
	Renderer2D_Color buttonColor;
}TextButton;

typedef struct Slider{
	Vec2f pos;
	Vec2f size;
	float value;
	Renderer2D_Color color;
}Slider;

typedef struct TextInput{
	Vec2f pos;
	Vec2f size;
	Renderer2D_Color color;
	char text[STRING_SIZE];
	int cursorPosition;
	bool focused;
}TextInput;

bool IGUI_hoveringOverGUI = false;

Renderer2D_Color textColor = { 0.0, 0.0, 0.0 };
Renderer2D_Color buttonColor = { 0.8, 0.8, 0.5 };

Renderer2D_Color buttonColorHover = { 0.7, 0.7, 0.6 };
Renderer2D_Color buttonColorSelected = { 0.6, 0.6, 0.5 };

Array textButtons;
Array sliders;
Array textInputs;

Font font;

void IGUI_init(){
	
	Array_init(&textButtons, sizeof(TextButton));
	Array_init(&sliders, sizeof(Slider));
	Array_init(&textInputs, sizeof(TextInput));

	font = getFont("assets/fonts/times.ttf", 100);

}

void IGUI_render(Renderer2D_Renderer *renderer_p){

	Renderer2D_Color color = { 0.8, 0.8, 0.5 };
	float alpha = 1.0;

	for(int i = 0; i < textButtons.length; i++){

		TextButton *textButton_p = Array_getItemPointerByIndex(&textButtons, i);

		Renderer2D_drawColoredRectangle(renderer_p, textButton_p->pos.x - renderer_p->offset.x, textButton_p->pos.y - renderer_p->offset.y, textButton_p->size.x, textButton_p->size.y, textButton_p->buttonColor, alpha);

		Renderer2D_drawText(renderer_p, textButton_p->text, textButton_p->pos.x + textButton_p->paddingX - renderer_p->offset.x, textButton_p->pos.y - renderer_p->offset.y, textButton_p->fontSize, font, alpha);
	
	}

	for(int i = 0; i < sliders.length; i++){

		Slider *slider_p = Array_getItemPointerByIndex(&sliders, i);

		int knobWidth = 10;
		int knobHeight = 20;
		int knobY = slider_p->pos.y - 5 - renderer_p->offset.y;

		int knobX = slider_p->pos.x + (float)(slider_p->size.x - knobWidth) * slider_p->value - renderer_p->offset.x;

		Renderer2D_drawColoredRectangle(renderer_p, slider_p->pos.x - renderer_p->offset.x, slider_p->pos.y - renderer_p->offset.y, slider_p->size.x, slider_p->size.y, slider_p->color, alpha);

		Renderer2D_drawColoredRectangle(renderer_p, knobX, knobY, knobWidth, knobHeight, slider_p->color, alpha);
	
	}

	//render text inputs
	for(int i = 0; i < textInputs.length; i++){
		
		TextInput *textInput_p = Array_getItemPointerByIndex(&textInputs, i);

		Renderer2D_drawColoredRectangle(renderer_p, textInput_p->pos.x - renderer_p->offset.x, textInput_p->pos.y - renderer_p->offset.y, textInput_p->size.x, textInput_p->size.y, textInput_p->color, alpha);

		Renderer2D_drawText(renderer_p, textInput_p->text, textInput_p->pos.x - renderer_p->offset.x, textInput_p->pos.y - renderer_p->offset.y, textInput_p->size.y, font, alpha);

	}

	Array_clear(&textButtons);
	Array_clear(&sliders);
	Array_clear(&textInputs);

	IGUI_hoveringOverGUI = false;

}

bool checkPointInRect(Vec2f point, Vec2f rectPos, Vec2f rectSize){

	if(point.x > rectPos.x && point.x < rectPos.x + rectSize.x
	&& point.y > rectPos.y && point.y < rectPos.y + rectSize.y){
		return true;
	}

	return false;

}

bool IGUI_textButton_click(char *text, Vec2f pos, int fontSize, bool selected){

	TextButton *textButton_p = Array_addItem(&textButtons);

	textButton_p->fontSize = fontSize;
	textButton_p->pos = pos;

	String_set(textButton_p->text, text, STRING_SIZE);

	int width, height;

	char *data = getImageDataFromFontAndString_mustFree(font, text, &width, &height);
	free(data);

	width *= (float)fontSize / (float)height;
	height = fontSize;

	textButton_p->size = getVec2f(width, height);

	textButton_p->paddingX = 5;
	textButton_p->size.x += textButton_p->paddingX * 2;

	textButton_p->buttonColor = buttonColor;

	bool hover = false;
	bool clicked = false;
	bool hasBeenDowned = false;

	//check status
	if(checkPointInRect(Engine_pointer.pos, textButton_p->pos, textButton_p->size)){
		hover = true;
		IGUI_hoveringOverGUI = true;
	}
	if(checkPointInRect(Engine_pointer.lastDownedPos, textButton_p->pos, textButton_p->size)){
		hasBeenDowned = true;
		IGUI_hoveringOverGUI = true;
	}

	if(selected){
		textButton_p->buttonColor = buttonColorSelected;
	}

	if(hover){
		textButton_p->buttonColor = buttonColorHover;

		if(Engine_pointer.upped
		&& hasBeenDowned){
			clicked = true;
		}
	
	}

	return clicked;

}

void IGUI_SliderData_init(IGUI_SliderData *sliderData_p, float value){
	sliderData_p->value = value;
}

void IGUI_slider(Vec2f pos, IGUI_SliderData *sliderData_p){

	Slider *slider_p = Array_addItem(&sliders);

	slider_p->pos = pos;
	
	slider_p->size = getVec2f(150, 10);

	slider_p->color = buttonColor;

	//check status

	bool hover = false;
	bool clicked = false;
	bool hasBeenDowned = false;
	bool hoverX = false;

	//check status
	if(checkPointInRect(Engine_pointer.pos, slider_p->pos, slider_p->size)){
		hover = true;
		IGUI_hoveringOverGUI = true;
	}
	if(checkPointInRect(Engine_pointer.lastDownedPos, slider_p->pos, slider_p->size)){
		hasBeenDowned = true;
		IGUI_hoveringOverGUI = true;
	}
	if(Engine_pointer.pos.x > slider_p->pos.x && Engine_pointer.pos.x < slider_p->pos.x + slider_p->size.x){
		hoverX = true;
	}

	if(hover){
		slider_p->color = buttonColorHover;
	}

	if(hasBeenDowned
	&& Engine_pointer.down){

		if(hoverX){
			sliderData_p->value = (Engine_pointer.pos.x - slider_p->pos.x) / slider_p->size.x;
		}

		if(Engine_pointer.pos.x > slider_p->pos.x + slider_p->size.x){
			sliderData_p->value = 1.0;
		}
		if(Engine_pointer.pos.x < slider_p->pos.x){
			sliderData_p->value = 0.0;
		}

		slider_p->color = buttonColorHover;

	}

	//set value
	slider_p->value = sliderData_p->value;

}

void IGUI_TextInputData_init(IGUI_TextInputData *textInputData_p, char *startText, int startTextLength){

	String_set(textInputData_p->text, "", STRING_SIZE);

	if(startText != NULL){
		String_set(textInputData_p->text, startText, startTextLength);
	
	}

	textInputData_p->cursorPosition = 0;
	textInputData_p->focused = false;

}

void IGUI_textInput(Vec2f pos, IGUI_TextInputData *textInputData_p){

	TextInput *textInput_p = Array_addItem(&textInputs);

	textInput_p->pos = pos;
	textInput_p->size = getVec2f(150, 25);

	//textInput_p->color = Renderer2D_getColor(1.0, 1.0, 1.0);
	textInput_p->color = buttonColor;

	if(Engine_pointer.downed){

		if(checkPointInRect(Engine_pointer.pos, textInput_p->pos, textInput_p->size)){
			textInputData_p->focused = true;
		}else{
			textInputData_p->focused = false;
		}

	}

	if(checkPointInRect(Engine_pointer.pos, textInput_p->pos, textInput_p->size)){
		textInput_p->color = buttonColorHover;
	}

	textInput_p->focused = textInputData_p->focused;
	textInput_p->cursorPosition = textInputData_p->cursorPosition;

	if(textInputData_p->focused){

		for(int i = 0; i < Engine_textInput.length; i++){

			char *text = Array_getItemPointerByIndex(&Engine_textInput, i);

			if(text[0] == 8){//handle backspace
				if(strlen(textInputData_p->text) > 0){
					textInputData_p->text[strlen(textInputData_p->text) - 1] = *"\0";
				}
			}else{
				String_append(textInputData_p->text, text);
			}
		
		}

		textInput_p->color = buttonColorSelected;

	}

	String_set(textInput_p->text, textInputData_p->text, STRING_SIZE);

	//String_set(textInput_p->text, "hello", STRING_SIZE);

	textInput_p->cursorPosition = 0;

}
