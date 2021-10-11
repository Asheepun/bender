#ifndef IGUI_H_
#define IGUI_H_

#include "engine/geometry.h"
#include "engine/renderer2d.h"
#include "stdbool.h"

typedef struct IGUI_SliderData{
	float value;
}IGUI_SliderData;

void IGUI_init();

void IGUI_render(Renderer2D_Renderer *);

bool IGUI_textButton_click(char *, Vec2f, int);

void IGUI_SliderData_init(IGUI_SliderData *, float);

void IGUI_slider(Vec2f, IGUI_SliderData *);

#endif
