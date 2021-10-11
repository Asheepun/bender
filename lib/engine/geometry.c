#include "engine/geometry.h"

#include "stdbool.h"
#include "math.h"
#include "stdio.h"

//GENERAL MATH FUNCTIONS
float normalize(float x){
	return x / fabs(x);
}

float getSquared(float x){
	return x * x;
}

int ___x = 1;
int ___a = 65521;
int ___c = 0;
int ___m = 17364;

float getRandom(){

	___x = (___a * ___x + ___c) % ___m;
	float r = (float)___x;
	r /= ___m;

	return r;
	
}

//VEC2F FUNCTIONS

Vec2f getVec2f(float x, float y){
	Vec2f v = { x, y };
	return v;
}

void Vec2f_set(Vec2f *v, float x, float y){
	v->x = x;
	v->y = y;
}

void Vec2f_log(Vec2f v){
	printf("{ x: %f, y: %f }\n", v.x, v.y);
}

void Vec2f_add(Vec2f *v1, Vec2f v2){
	v1->x += v2.x;
	v1->y += v2.y;
}

void Vec2f_sub(Vec2f *v1, Vec2f v2){
	v1->x -= v2.x;
	v1->y -= v2.y;
}

void Vec2f_mul(Vec2f *v1, Vec2f v2){
	v1->x *= v2.x;
	v1->y *= v2.y;
}

void Vec2f_div(Vec2f *v1, Vec2f v2){
	v1->x /= v2.x;
	v1->y /= v2.y;
}

void Vec2f_mulByFloat(Vec2f *v1, float a){
	v1->x *= a;
	v1->y *= a;
}

void Vec2f_divByFloat(Vec2f *v1, float a){
	v1->x /= a;
	v1->y /= a;
}

void Vec2f_normalize(Vec2f *v){
	float mag = getMagVec2f(*v);
	if(mag != 0){
		v->x /= mag;
		v->y /= mag;
	}
}

void Vec2f_invert(Vec2f *v){
	v->x *= -1;
	v->y *= -1;
}

float getMagVec2f(Vec2f v){
	return sqrt(v.x * v.x + v.y * v.y);
}

Vec2f getAddVec2f(Vec2f v1, Vec2f v2){
	return getVec2f(v1.x + v2.x, v1.y + v2.y);
}

Vec2f getSubVec2f(Vec2f v1, Vec2f v2){
	return getVec2f(v1.x - v2.x, v1.y - v2.y);
}

Vec2f getMulVec2fFloat(Vec2f v, float a){
	return getVec2f(v.x * a, v.y * a);
}

Vec2f getDivVec2fFloat(Vec2f v, float a){
	return getVec2f(v.x / a, v.y / a);
}

float getDistanceVec2f(Vec2f v1, Vec2f v2){
	return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

Vec2f getNormalizedVec2f(Vec2f v){
	return getVec2f(v.x / getMagVec2f(v), v.y / getMagVec2f(v));
}

Vec2f getInverseVec2f(Vec2f v){
	return getVec2f(1 / v.x, 1 / v.y);
}

//VEC3F FUNCTIONS
Vec3f getVec3f(float x, float y, float z){
	Vec3f v = { x, y, z };
	return v;
}

void Vec3f_log(Vec3f v){
	printf("{ x: %f, y: %f, z: %f }\n", v.x, v.y, v.z);
}

void Vec3f_add(Vec3f *v1, Vec3f v2){
	v1->x += v2.x;
	v1->y += v2.y;
	v1->z += v2.z;
}

void Vec3f_sub(Vec3f *v1, Vec3f v2){
	v1->x -= v2.x;
	v1->y -= v2.y;
	v1->z -= v2.z;
}

void Vec3f_mulByFloat(Vec3f *v, float f){
	v->x *= f;
	v->y *= f;
	v->z *= f;
}

void Vec3f_divByFloat(Vec3f *v, float x){
	v->x /= x;
	v->y /= x;
	v->z /= x;
}

void Vec3f_normalize(Vec3f *v){
	Vec3f_divByFloat(v, getMagVec3f(*v));
}

void Vec3f_inverse(Vec3f *v){
	v->x *= -1;
	v->y *= -1;
	v->z *= -1;
}

void Vec3f_rotate(Vec3f *point, float rotX, float rotY, float rotZ){

	//x axis
	float mag = sqrt(point->y * point->y + point->z * point->z);

	float deg = atan2(point->z, point->y);

	deg += rotX;

	point->y = mag * cos(deg);
	point->z = mag * sin(deg);

	//y axis
	mag = sqrt(point->x * point->x + point->z * point->z);

	deg = atan2(point->x, point->z);

	deg += rotY;

	point->x = mag * sin(deg);
	point->z = mag * cos(deg);

	//z axis
	mag = sqrt(point->x * point->x + point->y * point->y);

	deg = atan2(point->y, point->x);

	deg += rotZ;

	point->x = mag * cos(deg);
	point->y = mag * sin(deg);

}

float getMagVec3f(Vec3f v){
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3f getAddVec3f(Vec3f v1, Vec3f v2){
	return getVec3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vec3f getSubVec3f(Vec3f v1, Vec3f v2){
	return getVec3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vec3f getMulVec3fFloat(Vec3f v, float a){
	return getVec3f(v.x * a, v.y * a, v.z * a);
}

Vec3f getDivVec3fFloat(Vec3f v, float a){
	return getVec3f(v.x / a, v.y / a, v.z / a);
}

float getDistanceVec3f(Vec3f v1, Vec3f v2){
	return getMagVec3f(getSubVec3f(v1, v2));
}

float getDotVec3f(Vec3f v1, Vec3f v2){
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vec3f getCrossVec3f(Vec3f v1, Vec3f v2){
	Vec3f v = {
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x,
	};

	return v;
}

float getAngleBetweenVec3f(Vec3f v1, Vec3f v2){
	float a = getMagVec3f(getSubVec3f(v1, v2));
	float b = getMagVec3f(v1);
	float c = getMagVec3f(v2);

	return acos((a * a - b * b - c * c) / (-2 * b * c));
}

float getAreaFromTriangleVec3f(Vec3f v1, Vec3f v2, Vec3f v3){
	return getMagVec3f(getCrossVec3f(getSubVec3f(v1, v2), getSubVec3f(v1, v3))) / 2;
	//float a = getMagVec3f(getSubVec3f(v1, v2));
	//float b = getMagVec3f(getSubVec3f(v1, v3));
	//float c = getMagVec3f(getSubVec3f(v2, v3));

	//return (b * c / 2) * sqrt(1 - getSquare((b * b + c * c - a * a) / (2 * b * c)));
}

Vec3f getNormalFromTriangleVec3f(Vec3f v1, Vec3f v2, Vec3f v3){
	Vec3f N = getCrossVec3f(getSubVec3f(v2, v1), getSubVec3f(v3, v1));

	Vec3f_normalize(&N);

	return N;
}

int test = 0;

//ONLY WORKS IF l1 IS 0, 0, 0 MUST FIX!!!
Vec3f getLineToTriangleIntersectionVec3f(Vec3f l1, Vec3f l2, Vec3f v1, Vec3f v2, Vec3f v3){

	Vec3f O = l1;
	Vec3f P = v1;

	Vec3f dl = getSubVec3f(l2, O);

	Vec3f N = getNormalFromTriangleVec3f(v1, v2, v3);

	float a1 = getAngleBetweenVec3f(getSubVec3f(N, O), getSubVec3f(P, O));

	if(a1 > M_PI / 2){
		a1 = M_PI - a1;
		Vec3f_inverse(&N);
	}

	Vec3f_mulByFloat(&N, getDistanceVec3f(O, P) * cos(a1));

	float a2 = getAngleBetweenVec3f(dl, N);

	if(a2 > M_PI / 2){
		a2 = M_PI - a2;
		Vec3f_inverse(&dl);
	}

	Vec3f_normalize(&dl);
	Vec3f_mulByFloat(&dl, getMagVec3f(N) / cos(a2));

	if(test % 1000 == 0){

		/*
		printf("---GETINTERSECTIONCALL---\n");

		printf("a1: %f\n", a1);
		printf("a2: %f\n", a2);

		Vec3f_log(O);
		Vec3f_log(dl);

		Vec3f_log(getAddVec3f(O, dl));
		*/
	
	}

	test++;

	return getAddVec3f(O, dl);

}


