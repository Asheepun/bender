#include "game.h"

#include "engine/engine.h"
#include "engine/renderer2d.h"
#include "engine/array.h"

#include "stdio.h"
#include "math.h"
#include "string.h"

int getBufferIndex(float x, float y){
	return (int)y * WIDTH + (int)x;
}

bool checkPixelEquals(Pixel p1, Pixel p2){
	return p1.r == p2.r
		&& p1.g == p2.g
		&& p1.b == p2.b
		&& p1.a == p2.a;
}

Particle *addParticle(Vec2f pos){

	Particle *particle_p = Array_addItem(&particles);

	particle_p->pos = pos;

	particle_p->velocity = getVec2f(0, 0);
	particle_p->acceleration = getVec2f(0, 0);

	particle_p->isBended = false;

	return particle_p;

}
