#version 100

attribute vec2 vertexPosition_attribute;
attribute vec2 textureVertex_attribute;

varying vec2 textureCoord;

uniform float posX;
uniform float posY;

uniform float width;
uniform float height;

vec2 vertexPosition;
vec2 textureVertex;

void main(){

	vertexPosition = vertexPosition_attribute;
	textureVertex = textureVertex_attribute;

	gl_Position = vec4(
		posX + width + -1.0 + vertexPosition.x * width,
		-posY + -height + 1.0 + vertexPosition.y * height,
		0.0,
		1.0
	);

	textureCoord = textureVertex;

}
