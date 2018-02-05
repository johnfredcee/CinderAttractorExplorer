#version 150

uniform sampler2D uTex0;

in lowp  vec4	Color;
out lowp  vec4 	oColor;

void main(void)
{
	oColor = texture(uTex0, gl_PointCoord) * Color;
}


