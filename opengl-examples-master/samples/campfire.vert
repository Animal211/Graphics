#version 150 // GLSL 150 = OpenGL 3.2

uniform float time;
in vec3 in_Position;
out float percentTraveled;

uniform mat4 ModelView;
uniform mat4 Projection;


float wiggle(float VID, float percent){
	float temp1 = sin(VID + percent + 5) * .2;
	float temp2 = sin(VID + percent + 12) * .3;
	float temp3 = sin(VID + percent + 32) * .5;
	
	float move = temp1 + temp2 + temp3 +5;
	return move;
}

void main() 
{
	float travelDist = 2;
	float timeToLoop = 2;
	float N = 3333;

	float timesince = 0;
	timesince = mod(time, timeToLoop);
	//percentTraveled = (timesince+gl_VertexID) / (timeToLoop+N); 
	percentTraveled = mod((timesince)+gl_VertexID/N, 2)/2;
	float xmove = 0;
	float zmove = 0;
	xmove = wiggle(gl_VertexID, percentTraveled);
	zmove = wiggle(mod(gl_VertexID,40), percentTraveled);
	vec3 movePoint = vec3(xmove,mod((timesince)+gl_VertexID/N, 2),zmove);

	vec4 pos = vec4(in_Position.xyz + movePoint, 1.0);
	gl_Position = Projection * ModelView * pos;
	gl_PointSize = 30 * (1-percentTraveled);
}
