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
}Slider;

Renderer2D_Color textColor = { 0.0, 0.0, 0.0 };
Renderer2D_Color buttonColor = { 0.8, 0.8, 0.5 };

Renderer2D_Color buttonColorHover = { 0.7, 0.7, 0.6 };

Array textButtons;
Array sliders;

Font font;

void IGUI_init(){
	
	Array_init(&textButtons, sizeof(TextButton));
	Array_init(&sliders, sizeof(Slider));

	font = getFont("assets/fonts/times.ttf", 100);

}

void IGUI_render(Renderer2D_Renderer *renderer_p){

	Renderer2D_Color color = { 0.8, 0.8, 0.5 };
	float alpha = 1.0;

	for(int i = 0; i < textButtons.length; i++){

		TextButton *textButton_p = Array_getItemPointerByIndex(&textButtons, i);

		//draw button
		Renderer2D_setShaderProgram(renderer_p, renderer_p->colorShaderProgram);

		Renderer2D_beginRectangle(renderer_p, textButton_p->pos.x, textButton_p->pos.y, textButton_p->size.x, textButton_p->size.y);

		Renderer2D_supplyUniform(renderer_p, &textButton_p->buttonColor, "color", RENDERER2D_UNIFORM_TYPE_COLOR);

		Renderer2D_supplyUniform(renderer_p, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

		Renderer2D_drawRectangle(renderer_p);

		//draw text
		Renderer2D_setShaderProgram(renderer_p, renderer_p->textureShaderProgram);

		Renderer2D_beginText(renderer_p, textButton_p->text, textButton_p->pos.x + textButton_p->paddingX, textButton_p->pos.y, textButton_p->fontSize, font);

		Renderer2D_supplyUniform(renderer_p, &color, "color", RENDERER2D_UNIFORM_TYPE_COLOR);

		Renderer2D_supplyUniform(renderer_p, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

		Renderer2D_drawRectangle(renderer_p);
	
	}

	for(int i = 0; i < sliders.length; i++){

		Slider *slider_p = Array_getItemPointerByIndex(&sliders, i);

		int knobWidth = 10;
		int knobHeight = 20;
		int knobY = slider_p->pos.y - 5;

		int knobX = slider_p->pos.x + (float)(slider_p->size.x - knobWidth) * slider_p->value;

		Renderer2D_setShaderProgram(renderer_p, renderer_p->colorShaderProgram);

		Renderer2D_beginRectangle(renderer_p, slider_p->pos.x, slider_p->pos.y, slider_p->size.x, slider_p->size.y);

		Renderer2D_supplyUniform(renderer_p, &buttonColor, "color", RENDERER2D_UNIFORM_TYPE_COLOR);

		Renderer2D_supplyUniform(renderer_p, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

		Renderer2D_drawRectangle(renderer_p);

		Renderer2D_setShaderProgram(renderer_p, renderer_p->colorShaderProgram);

		Renderer2D_beginRectangle(renderer_p, knobX, knobY, knobWidth, knobHeight);

		Renderer2D_supplyUniform(renderer_p, &buttonColor, "color", RENDERER2D_UNIFORM_TYPE_COLOR);

		Renderer2D_supplyUniform(renderer_p, &alpha, "alpha", RENDERER2D_UNIFORM_TYPE_FLOAT);

		Renderer2D_drawRectangle(renderer_p);
	
	}

	Array_clear(&textButtons);
	Array_clear(&sliders);

}

bool checkPointInRect(Vec2f point, Vec2f rectPos, Vec2f rectSize){

	if(point.x > rectPos.x && point.x < rectPos.x + rectSize.x
	&& point.y > rectPos.y && point.y < rectPos.y + rectSize.y){
		return true;
	}

	return false;

}

bool IGUI_textButton_click(char *text, Vec2f pos, int fontSize){

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
	}
	if(checkPointInRect(Engine_pointer.lastDownedPos, textButton_p->pos, textButton_p->size)){
		hasBeenDowned = true;
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

	//check status

	bool hover = false;
	bool clicked = false;
	bool hasBeenDowned = false;
	bool hoverX = false;

	//check status
	if(checkPointInRect(Engine_pointer.pos, slider_p->pos, slider_p->size)){
		hover = true;
	}
	if(checkPointInRect(Engine_pointer.lastDownedPos, slider_p->pos, slider_p->size)){
		hasBeenDowned = true;
	}
	if(Engine_pointer.pos.x > slider_p->pos.x && Engine_pointer.pos.x < slider_p->pos.x + slider_p->size.x){
		hoverX = true;
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

	}

	//set value
	slider_p->value = sliderData_p->value;

}
