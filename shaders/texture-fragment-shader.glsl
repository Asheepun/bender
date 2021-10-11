#version 100

precision mediump float;

varying vec2 textureCoord;

uniform sampler2D tex;

uniform float alpha;

void main(){

	gl_FragColor = texture2D(tex, textureCoord) * vec4(1.0, 1.0, 1.0, alpha);

}
