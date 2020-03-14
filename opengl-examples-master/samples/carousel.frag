#version 150 // Specify which version of GLSL we are using.

out vec4 fragColor;

in vec4 out_VertexPos; //location of the frag in camera coords
in vec3 out_Normal; 
in vec4 out_LightCords; //light location in camera coords

float diffuseScalar(vec3 normal, vec3 lightDir, bool frontBackSame)
{
	float diffuse = dot(normalize(lightDir), normalize(normal.xyz));

	if(frontBackSame)
		diffuse = abs(diffuse);
	else
		diffuse = clamp(diffuse, 0, 1);

	diffuse = diffuse/2 + .5;

	return diffuse;
}

void main() 
{
	/* Get position of light in camera coordinates. When we do
	 * headlight style rendering, the light will be at the position of
	 * the camera! */
	vec3 lightPos = vec3(0,0,0);

	/* Calculate a vector pointing from our current position (in
	 * camera coordinates) to the light position. */
	vec3 lightDir = lightPos - out_VertexPos.xyz;

	/* Calculate diffuse shading */
	float diffuse = diffuseScalar(out_Normal.xyz, lightDir, true);

	/*SPECULAR SHADING*/
	vec3 camPos = vec3(0,0,0); //camera position in camera cords
	vec3 fragToCamVec =  vec3(camPos.x - out_VertexPos.x, camPos.y - out_VertexPos.y, camPos.z - out_VertexPos.z); //get vector from frag to camera
	vec3 fragToLightVec =  vec3(out_LightCords.x - out_VertexPos.x, out_LightCords.y - out_VertexPos.y, out_LightCords.z - out_VertexPos.z);
	vec3 normFTCV = normalize(fragToCamVec);
	vec3 normFTLV = normalize(fragToLightVec);
	vec3 halfVec = vec3((normFTCV.x + normFTLV.x)/2, (normFTCV.y + normFTLV.y)/2, (normFTCV.z + normFTLV.z)/2);
	float specular = dot(out_Normal, halfVec);

	if (specular < 0){
		specular = 0;
	} else {
		specular = pow(specular,10);
	}

	fragColor.xyz = (specular + diffuse) * vec3(.5, .5, .5); 
	fragColor.a = 1;  // make sure it is opaque; don't let diffuse variable scale alpha.
}

