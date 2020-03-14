#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position; // vertex position (object coordinates)
in vec3 in_Normal;   // normal vector   (object coordinates)

out vec4 out_VertexPos; // vertex position (camera coordinates)
out vec4 out_Normal;    // normal vector   (camera coordinates)

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 GeomTransform;

uniform int red;

void main() 
{
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	mat3 NormalMat = mat3(transpose(inverse(ModelView * GeomTransform)));
	out_Normal = normalize(NormalMat * in_Normal);

	gl_Position = Projection * ModelView * GeomTransform * pos; // object -> unhomogenized NDC
}

