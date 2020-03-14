#version 150 // GLSL 150 = OpenGL 3.2

out vec4 fragColor;
in float percentTraveled;

uniform sampler2D tex1;
uniform sampler2D tex2;

void main() 
{
	if(true)
	{
		fragColor = (1-percentTraveled)*texture(tex1, gl_PointCoord) + percentTraveled*texture(tex2, gl_PointCoord)/3;
		//fragColor = texture(tex1, gl_PointCoord);
	}
}
