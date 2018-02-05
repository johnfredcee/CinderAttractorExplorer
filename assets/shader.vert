#version 150

uniform mat4	ciModelViewProjection;

in vec4		ciPosition;
in vec4		ciColor;
in float    particleRadius;

out lowp vec4	Color;

void main(void)
{
    gl_Position = ciModelViewProjection * ciPosition;
    gl_PointSize = particleRadius;
	Color = ciColor;
};
   