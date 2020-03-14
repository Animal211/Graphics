/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing a colored 3D triangle.
 *
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"
void init_geometryBuilding(kuhl_geometry* b_building, kuhl_geometry* b_window, kuhl_geometry* t_building, kuhl_geometry* t_window, GLuint program);

static GLuint program = 0; /**< id value for the GLSL program */
static GLuint program2 = 0;
static float cameraMovment = 0;
static int newrowindicator = 0; //used for tracking when we move 1 unit
static int terrainsidelength = 11;
static kuhl_geometry ground;

static kuhl_geometry buildingbottom[10][10];
static kuhl_geometry windowbottom[10][10];
static kuhl_geometry buildingtop[10][10];
static kuhl_geometry windowtop[10][10];




/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/* If the library handles this keypress, return */
	if (kuhl_keyboard_handler(window, key, scancode, action, mods))
		return;

	/* custom key handling code here */
	if (action == GLFW_PRESS) {
		return;
	}

	/* custom key handling code here */
	//if (action == GLFW_REPEAT) {
	//	return;
	//}

	// For a list of keys, see: https://www.glfw.org/docs/latest/group__keys.html
	if (key == GLFW_KEY_SPACE)
	{
		cameraMovment += .1;
		//printf("fps=%f\n", bufferswap_fps());
	}
	if (key == GLFW_KEY_B) {
		if (cameraMovment > 0) {
			cameraMovment -= .1;
		}
	}
}

/** Draws the 3D scene. */
void display()
{
	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right). */
	viewmat_begin_frame();
	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		viewmat_begin_eye(viewportID);

		/* Where is the viewport that we are drawing onto and what is its size? */
		int viewport[4]; // x,y of lower left corner, width, height
		viewmat_get_viewport(viewport, viewportID);
		/* Tell OpenGL the area of the window that we will be drawing in. */
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Clear the current viewport. Without glScissor(), glClear()
		 * clears the entire screen. We could call glClear() before
		 * this viewport loop---but in order for all variations of
		 * this code to work (Oculus support, etc), we can only draw
		 * after viewmat_begin_eye(). */
		glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(.2,.2,.2,0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		/* Get the view matrix and the projection matrix */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);
		mat4f_lookat_new(viewMat, 5, 2.5, 0 - cameraMovment, 5, 0, -3 - cameraMovment, 0, 1, 0); //*******************************************Forlocking camera


		float vec1[2] = { 123.456, 9876.543 }; //random numbers for making seed
		float vec2[2] = { 0,0 };
		long seed = 0;
		int tempjindex = 0;
		if (newrowindicator != (int)cameraMovment) {
			newrowindicator = (int)cameraMovment;

			for (int j = 0; j < 10; j++) {
				for (int i = 0; i < 10; i++) {

					kuhl_geometry_delete(&buildingtop[i][j]);
					kuhl_geometry_delete(&windowtop[i][j]);
					kuhl_geometry_delete(&buildingbottom[i][j]);
					kuhl_geometry_delete(&windowbottom[i][j]);

				}
			}

			for (int j = newrowindicator; j < newrowindicator + 10; j++) {
				for (int i = 0; i < 10; i++) {
					vec2[0] = i;
					vec2[1] = j;
					seed = (long)(100 * vecNf_dot(vec1, vec2, 2));
					srand48(seed); //each new building gets its own seed based on coordinates
					init_geometryBuilding(&buildingbottom[i][tempjindex], &windowbottom[i][tempjindex], &buildingtop[i][tempjindex], &windowtop[i][tempjindex], program, i, tempjindex);
				}
				tempjindex++;
			}

		}

		float groundtranslateMat[16];
		mat4f_translate_new(groundtranslateMat, 0, 0, - (int) cameraMovment);
		float scaleMat[16];
		mat4f_scale_new(scaleMat, 1, 1, 1);
		float modelMat[16];
		mat4f_mult_mat4f_new(modelMat, scaleMat, groundtranslateMat);
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, modelMat);

		kuhl_errorcheck();
		glUseProgram(program2);
		kuhl_errorcheck();

		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			modelview); // value
		kuhl_errorcheck();
		kuhl_geometry_draw(&ground);
		glUseProgram(0); // stop using a GLSL program.


		//--------Draw buildings-------------------//
		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();
		//mat4f_mult_mat4f_new(modelMat, scaleMat, scaleMat);
		//mat4f_mult_mat4f_new(modelview, viewMat, modelMat);

		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			modelview); // value
		kuhl_errorcheck();
		/* Draw the geometry using the matrices that we sent to the
		 * vertex programs immediately above */
		int jindex = 0;
		for (int j = newrowindicator; j < newrowindicator + 10; j++) {
			for (int i = 0; i < 10; i++) {
				float buildingrotateMat[16];
				mat4f_rotateAxis_new(buildingrotateMat, 90, 0, 1, 0);
				float buildingtranslateMat[16];
				mat4f_translate_new(buildingtranslateMat, i + .8, 0, 0 - j * 1 - .05);

				if (i >= 5) {
					mat4f_rotateAxis_new(buildingrotateMat, 0, 0, 1, 0);
					mat4f_translate_new(buildingtranslateMat, i + .3, 0, 0 - j * 1 - .05);
				}
				
				float modelMat[16];
				mat4f_mult_mat4f_new(modelMat, buildingtranslateMat, buildingrotateMat);
				float modelview[16];
				mat4f_mult_mat4f_new(modelview, viewMat, modelMat);

				glUniformMatrix4fv(kuhl_get_uniform("Projection"),
					1, // number of 4x4 float matrices
					0, // transpose
					perspective); // value
				/* Send the modelview matrix to the vertex program. */
				glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
					1, // number of 4x4 float matrices
					0, // transpose
					modelview); // value
				kuhl_errorcheck();

				kuhl_geometry_draw(&buildingbottom[i][jindex]);
				kuhl_geometry_draw(&windowbottom[i][jindex]);
				kuhl_geometry_draw(&buildingtop[i][jindex]);
				kuhl_geometry_draw(&windowtop[i][jindex]);
			}
			jindex++;
		}

		glUseProgram(0);

		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

void init_geometryGround(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog,
		terrainsidelength * terrainsidelength, // number of vertices *3xyz
		GL_TRIANGLES); // type of thing to draw

/* Vertices that we want to form triangles out of. Every 3 numbers
 * is a vertex position. Below, we provide indices to form
 * triangles out of these vertices. */
 //GLfloat verticesPos[3 * 50 * 50]; //xy3* sidelength * sidelength
 //GLint indices1[2401 * 6]; //6 indices per quad * how many quads (sidelenth-1 squared)  (9 for our testing 4x4 having 3x3 quads)
	int temp = 3 * terrainsidelength * terrainsidelength;
	int temp1 = 6 * (terrainsidelength - 1) * (terrainsidelength - 1);
	GLfloat* verticesPos;
	GLuint* indices1;
	verticesPos = (GLfloat*)malloc(temp * sizeof(GLfloat));
	indices1 = (GLuint*)malloc(temp1 * sizeof(GLuint));
	int vertIndex = 0;
	int indIndex = 0;

	for (int j = 0; j < terrainsidelength; j++) { //For each row
		for (int i = 0; i < terrainsidelength; i++) { //for each each vertex (3 because xyz)
			verticesPos[vertIndex++] = i; //x
			verticesPos[vertIndex++] = 0; //y zero because quad starts flat
			verticesPos[vertIndex++] = -j; //z set based on what row
		}
	}

	kuhl_geometry_attrib(geom, verticesPos, //vertexPositions,
		3, // number of components x,y,z
		"in_Position", // GLSL variable
		KG_WARN); // warn if attribute is missing in GLSL program?

	//every 3 indices draws a triangle. These loops draw a quad at a time, 2 tris to a quad each tri has 3 vert indexes = 6
	for (int j = 0; j < (terrainsidelength - 1); j++) { //for each row of quads
		for (int i = 0; i < (terrainsidelength - 1); i++) { //for each quad

			indices1[indIndex++] = i + j * terrainsidelength; //left tri [X\]
			indices1[indIndex++] = 1 + i + j * terrainsidelength;
			indices1[indIndex++] = i + terrainsidelength + j * terrainsidelength;

			indices1[indIndex++] = 1 + i + j * terrainsidelength; //right tri [\X]
			indices1[indIndex++] = i + terrainsidelength + j * terrainsidelength;
			indices1[indIndex++] = 1 + i + terrainsidelength + j * terrainsidelength;
		}
	}

	GLfloat* texcoordData;
	int temp3 = 2 * terrainsidelength * terrainsidelength; // x and y for 1000x1000 verts
	texcoordData = (GLfloat*)malloc(temp3 * sizeof(GLfloat));
	int texIndex = 0;

	for (float j = 0; j < terrainsidelength; j++) { //For each row
		for (float i = 0; i < terrainsidelength; i++) {
			if (i == 0) {
				texcoordData[texIndex++] = 0;
			}
			else {
				texcoordData[texIndex++] = i;
			}

			if (j == 0) {
				texcoordData[texIndex++] = 0;
			}
			else {
				texcoordData[texIndex++] = j;
			}
		}
	}

	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);
	// The 2 parameter above means each texture coordinate is a 2D coordinate.

	kuhl_geometry_indices(geom, indices1, indIndex);

	/* Load the texture. It will be bound to texId */
	GLuint texId = 0;
	float aspectRatio = kuhl_read_texture_file_wrap("../images/Streets.png", &texId, GL_REPEAT, GL_REPEAT);
	msg(MSG_DEBUG, "\n Aspect ratio of image is %f", aspectRatio); // write message to log.txt

	/* Tell this piece of geometry to use the texture we just loaded. */
	kuhl_geometry_texture(geom, texId, "tex", KG_WARN);

	kuhl_errorcheck();
	free(texcoordData);
	free(verticesPos);
	free(indices1);
}

void init_geometryComplex(kuhl_geometry* t_building, kuhl_geometry* t_window, GLuint prog, int isComplex, float depth, float width, float height) {
	float depth2 = depth * drand48();
	float width2 = width * drand48();
	float height2 = height * drand48();
	kuhl_geometry_new(t_building, program, 4 * 6, GL_TRIANGLES);
	/* The data that we want to draw. Each set of three numbers is a
	 * position. */
	GLfloat vertexData[] = { 0, 0+height, -depth2, // -Z face
							width2,  0 + height, -depth2,
							width2,   height2 + height, -depth2,
							0,  height2 + height, -depth2,
							0, 0 + height, 0, // +Z face
							width2,  0 + height, 0,
							width2,   height2 + height, 0,
							0,  height2 + height, 0,
							0, 0 + height, -depth2,  // -X face
							0, height2 + height,  -depth2,
							0, height2 + height,   0,
							0, 0 + height,  0,
							width2,	0 + height, -depth2,  //+X face
							width2, height2 + height, -depth2,
							width2, height2 + height, 0,
							width2, 0 + height, 0,
							0, 0 + height, 0,  //-Y face
							width2, 0 + height, 0,
							width2, 0 + height, -depth2,
							0, 0 + height,-depth2,
							0, height2 + height, 0,  //+Y face
							width2, height2 + height, 0,
							width2, height2 + height, -depth2,
							0, height2 + height,-depth2
	};
	kuhl_geometry_attrib(t_building, vertexData, 3, "in_Position", 1);

	// Since we can't draw quads in OpenGL 3.0+, we'll make quads out of our vertices : Two triangles form a quad.
	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
						   0, 2, 3,
						   4, 5, 6,  // second quad
						   4, 6, 7,
						   8, 9, 10, // third quad
						   8, 10, 11,
						   12, 13, 14, //fourth quad
						   12, 14, 15,
						   16, 17, 18, //fifth quad
						   16, 18, 19,
						   20, 21, 22, //sixth quad
						   20, 22, 23 };

	// 3*6 is the length of the indices array. Since we are drawing
	// triangles, it should be a multiple of 3.
	kuhl_geometry_indices(t_building, indexData, 3 * 12);

	GLfloat colorData[] = { .6,.6,.6, //
							.6,.6,.6,
						   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6 };
	kuhl_geometry_attrib(t_building, colorData, 3, "in_Color", KG_WARN);

	//--------------------Windows created below-------------------------//
	//1. calculate how many windows can fit on width
	//2. calculate windows can fit on depth
	//3. how many window rows fit into height
	//4. create geometry that loops using above info
	int windowsOnWidth = 0;
	int windowsOnDepth = 0;
	int windowRows = 0;
	int vertIndex = 0;
	int indIndex = 0;
	int colorIndex = 0;


	//say windows take up .05 with .01 gap on top and right.
	windowsOnWidth = (int)(width2 / .06);
	windowsOnDepth = (int)(depth2 / .06);
	windowRows = (int)(height2 / .06);

	int verts = 4 * (windowsOnWidth * windowRows) + 4 * (windowsOnDepth * windowRows); //4 corners * per window * per row
	int indis = 6 * (windowsOnWidth * windowRows) + 6 * (windowsOnDepth * windowRows);
	kuhl_geometry_new(t_window, program, verts, GL_TRIANGLES);
	GLfloat* verticesPos;
	GLuint* indices;
	GLfloat* windowColor;
	verticesPos = (GLfloat*)malloc(3 * verts * sizeof(GLfloat));
	indices = (GLuint*)malloc(indis * sizeof(GLuint));
	windowColor = (GLfloat*)malloc(3 * verts * sizeof(GLfloat));

	//verts for front face
	for (int j = 0; j < windowRows; j++) {
		for (int i = 0; i < windowsOnWidth; i++) {
			verticesPos[vertIndex++] = i * .06; //bottom left
			verticesPos[vertIndex++] = j * .06 + height;
			verticesPos[vertIndex++] = .02; //*******************This iswhere we should offset front windows from building to stop zfighting

			verticesPos[vertIndex++] = (i * .06) + .05; //bottom right
			verticesPos[vertIndex++] = j * .06 + height;
			verticesPos[vertIndex++] = .02;

			verticesPos[vertIndex++] = i * .06; //top left
			verticesPos[vertIndex++] = j * .06 + .05 + height;
			verticesPos[vertIndex++] = .02;

			verticesPos[vertIndex++] = (i * .06) + .05; //top right
			verticesPos[vertIndex++] = j * .06 + .05 + height;
			verticesPos[vertIndex++] = .02;

			//Now I need to send 4 RGB signals for each vertex.
			if (drand48() > .5) { //turn the lights on 50% of the time
				//yellow if R=1 G=1 B=0
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0; //end of vert1
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
			}
			else { //else off
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;//end of vert1
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
			}
		}

	}

	//indicesfor the front face
	//fore eah row do each window, giving 3 indicies for the 2 triangles to make the quad
	for (int j = 0; j < (windowRows); j++) { //for each row of quads
		for (int i = 0; i < (windowsOnWidth); i++) { //for each quad
			indices[indIndex++] = (i * 4) + (j * windowsOnWidth * 4);  //left tri [X\]
			indices[indIndex++] = (i * 4 + 1) + (j * windowsOnWidth * 4);
			indices[indIndex++] = (i * 4 + 2) + (j * windowsOnWidth * 4);

			indices[indIndex++] = (i * 4 + 1) + (j * windowsOnWidth * 4); //right tri [\X]
			indices[indIndex++] = (i * 4 + 2) + (j * windowsOnWidth * 4);
			indices[indIndex++] = (i * 4 + 3) + (j * windowsOnWidth * 4);
		}
	}

	//verts for the side faces
	for (int j = 0; j < windowRows; j++) {
		for (int i = 0; i < windowsOnDepth; i++) {
			//printf("On row %d position %d. Rows: %d Depth: %d \n",j,i,windowRows, windowsOnDepth);
			verticesPos[vertIndex++] = -.02; //bottom right *******************This iswhere we should offset front windows from building to stop zfighting
			verticesPos[vertIndex++] = j * .06 + height;
			verticesPos[vertIndex++] = -i * .06;

			verticesPos[vertIndex++] = -.02;
			verticesPos[vertIndex++] = j * .06 +height; //bottom left
			verticesPos[vertIndex++] = -(i * .06) - .05;

			verticesPos[vertIndex++] = -.02; //top right
			verticesPos[vertIndex++] = j * .06 + .05 + height;
			verticesPos[vertIndex++] = -i * .06;

			verticesPos[vertIndex++] = -.02; //top left
			verticesPos[vertIndex++] = (j * .06) + .05 + height;
			verticesPos[vertIndex++] = -(i * .06) - .05;

			//Now I need to send 4 RGB signals for each vertex.
			if (drand48() > .5) { //turn the lights on 50% of the time
				//yellow if R=1 G=1 B=0
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0; //end of vert1
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
			}
			else { //else off
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;//end of vert1
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
			}
		}
	}

	//indicesfor the side face
	//fore eah row do each window, giving 3 indicies for the 2 triangles to make the quad
	for (int j = 0; j < (windowRows); j++) { //for each row of quads
		for (int i = 0; i < (windowsOnDepth); i++) { //for each quad
			indices[indIndex++] = (i * 4) + (j * windowsOnDepth * 4) + (4 * (windowsOnWidth * windowRows));  //left tri [X\]
			indices[indIndex++] = (i * 4 + 1) + (j * windowsOnDepth * 4) + (4 * (windowsOnWidth * windowRows));
			indices[indIndex++] = (i * 4 + 2) + (j * windowsOnDepth * 4) + (4 * (windowsOnWidth * windowRows));

			indices[indIndex++] = (i * 4 + 1) + (j * windowsOnDepth * 4) + (4 * (windowsOnWidth * windowRows)); //right tri [\X]
			indices[indIndex++] = (i * 4 + 2) + (j * windowsOnDepth * 4) + (4 * (windowsOnWidth * windowRows));
			indices[indIndex++] = (i * 4 + 3) + (j * windowsOnDepth * 4) + (4 * (windowsOnWidth * windowRows));

		}
	}

	//--------------END of troubled section-----------


	kuhl_geometry_attrib(t_window, verticesPos, 3, "in_Position", 1);
	kuhl_geometry_attrib(t_window, windowColor, 3, "in_Color", KG_WARN);
	kuhl_geometry_indices(t_window, indices, indIndex);

	free(indices);
	free(verticesPos);
	free(windowColor);
}

void init_geometryBuilding(kuhl_geometry* b_building, kuhl_geometry* b_window, kuhl_geometry* t_building, kuhl_geometry* t_window, GLuint program){
	float depth = .6; //these are their max values, aka smaller that the 1x1 city blocks
	float width = .55;
	float height = 1.5;

	depth = depth * drand48() + .2;
	width = width * drand48() + .2;
	height = height * drand48() + .5;
	if (drand48() > .5) {
		init_geometryComplex(t_building, t_window, program, 1, depth, width, height); //send over the demensions used for bottom half so top half is smaller
	}
	else {
		init_geometryComplex(t_building, t_window, program, 0, depth, width, height);
	}

	// 4*3 is the number of vertices. If you add more, you should increase this number!
	kuhl_geometry_new(b_building, program, 4 * 6, GL_TRIANGLES);
	/* The data that we want to draw. Each set of three numbers is a
	 * position. */
	GLfloat vertexData[] = { 0, 0, -depth, // -Z face
							width,  0, -depth,
							width,   height, -depth,
							0,  height, -depth,
							0, 0, 0, // +Z face
							width,  0, 0,
							width,   height, 0,
							0,  height, 0,
							0, 0, -depth,  // -X face
							0, height,  -depth,
							0, height,   0,
							0, 0,  0,
							width,	0, -depth,  //+X face
							width, height, -depth,
							width, height, 0,
							width, 0, 0,
							0, 0, 0,  //-Y face
							width, 0, 0,
							width, 0, -depth,
							0, 0,-depth,
							0, height, 0,  //+Y face
							width, height, 0,
							width, height, -depth,
							0, height,-depth
	};
	kuhl_geometry_attrib(b_building, vertexData, 3, "in_Position", 1);

	// Since we can't draw quads in OpenGL 3.0+, we'll make quads out of our vertices : Two triangles form a quad.
	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
						   0, 2, 3,
						   4, 5, 6,  // second quad
						   4, 6, 7,
						   8, 9, 10, // third quad
						   8, 10, 11,
						   12, 13, 14, //fourth quad
						   12, 14, 15,
						   16, 17, 18, //fifth quad
						   16, 18, 19,
						   20, 21, 22, //sixth quad
						   20, 22, 23 };

	// 3*6 is the length of the indices array. Since we are drawing
	// triangles, it should be a multiple of 3.
	kuhl_geometry_indices(b_building, indexData, 3 * 12);

	GLfloat colorData[] = { .6,.6,.6, //
							.6,.6,.6,
						   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6,
							   .6,.6,.6};
	kuhl_geometry_attrib(b_building, colorData, 3, "in_Color", KG_WARN);

	//--------------------Windows created below-------------------------//
	//1. calculate how many windows can fit on width
	//2. calculate windows can fit on depth
	//3. how many window rows fit into height
	//4. create geometry that loops using above info
	int windowsOnWidth = 0;
	int windowsOnDepth = 0;
	int windowRows = 0;
	int vertIndex = 0;
	int indIndex = 0;
	int colorIndex = 0;


	//say windows take up .05 with .01 gap on top and right.
	windowsOnWidth =  (int)(width / .06);
	windowsOnDepth =  (int)(depth / .06);
	windowRows =  (int)(height / .06);

	int verts = 4 * (windowsOnWidth * windowRows) +4 *2* (windowsOnDepth * windowRows); //4 corners * per window * per row
	int indis = 6 * (windowsOnWidth * windowRows) +6 *2* (windowsOnDepth * windowRows);
	kuhl_geometry_new(b_window, program, verts, GL_TRIANGLES);
	GLfloat* verticesPos;
	GLuint* indices;
	GLfloat* windowColor;
	verticesPos = (GLfloat*)malloc(3 * verts * sizeof(GLfloat));
	indices = (GLuint*)malloc( indis * sizeof(GLuint));
	windowColor = (GLfloat*)malloc(3 * verts * sizeof(GLfloat));

	//verts for front face
	for (int j = 0; j < windowRows; j++) {
		for (int i = 0; i < windowsOnWidth; i++) {
			verticesPos[vertIndex++] = i * .06; //bottom left
			verticesPos[vertIndex++] = j * .06;
			verticesPos[vertIndex++] = .03; //*******************This iswhere we should offset front windows from building to stop zfighting

			verticesPos[vertIndex++] = (i * .06) + .05; //bottom right
			verticesPos[vertIndex++] = j * .06;
			verticesPos[vertIndex++] = .03;

			verticesPos[vertIndex++] = i * .06; //top left
			verticesPos[vertIndex++] = j * .06 + .05;
			verticesPos[vertIndex++] = .03;

			verticesPos[vertIndex++] = (i * .06) + .05; //top right
			verticesPos[vertIndex++] = j * .06 + .05;
			verticesPos[vertIndex++] = .03;

			//Now I need to send 4 RGB signals for each vertex.
			if (drand48() >.5) { //turn the lights on 50% of the time
				//yellow if R=1 G=1 B=0
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0; //end of vert1
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
			}
			else { //else off
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;//end of vert1
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
			}
		}

	}

	//indicesfor the front face
	//fore eah row do each window, giving 3 indicies for the 2 triangles to make the quad
	for (int j = 0; j < (windowRows); j++) { //for each row of quads
		for (int i = 0; i < (windowsOnWidth); i++) { //for each quad
			indices[indIndex++] = (i * 4) + (j * windowsOnWidth * 4);  //left tri [X\]
			indices[indIndex++] = (i * 4 + 1) + (j * windowsOnWidth * 4);
			indices[indIndex++] = (i * 4 + 2) + (j * windowsOnWidth * 4);

			indices[indIndex++] = (i * 4 + 1) + (j * windowsOnWidth * 4); //right tri [\X]
			indices[indIndex++] = (i * 4 + 2) + (j * windowsOnWidth * 4);
			indices[indIndex++] = (i * 4 + 3) + (j * windowsOnWidth * 4);
		}
	}

	//////////begining of troubled section-----------//
	//verts for the side faces
	for (int j = 0; j < windowRows; j++) {
		for (int i = 0; i < windowsOnDepth; i++) {
			//printf("On row %d position %d. Rows: %d Depth: %d \n",j,i,windowRows, windowsOnDepth);
			verticesPos[vertIndex++] = -.03; //bottom right *******************This iswhere we should offset front windows from building to stop zfighting
			verticesPos[vertIndex++] = j * .06;
			verticesPos[vertIndex++] = -i * .06;

			verticesPos[vertIndex++] = -.03;
			verticesPos[vertIndex++] = j * .06; //bottom left
			verticesPos[vertIndex++] = -(i * .06) - .05;

			verticesPos[vertIndex++] = -.03; //top right
			verticesPos[vertIndex++] = j * .06 + .05;
			verticesPos[vertIndex++] = -i * .06;

			verticesPos[vertIndex++] = -.03; //top left
			verticesPos[vertIndex++] = (j * .06) + .05;
			verticesPos[vertIndex++] = -(i * .06) - .05;

			//Now I need to send 4 RGB signals for each vertex.
			if (drand48() > .5) { //turn the lights on 50% of the time
				//yellow if R=1 G=1 B=0
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0; //end of vert1
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 1;
				windowColor[colorIndex++] = 0;
			}
			else { //else off
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;//end of vert1
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
				windowColor[colorIndex++] = 0;
			}
		}
	}

	//indicesfor the side face
	//fore eah row do each window, giving 3 indicies for the 2 triangles to make the quad
	for (int j = 0; j < (windowRows ); j++) { //for each row of quads
		for (int i = 0; i < (windowsOnDepth); i++) { //for each quad
			indices[indIndex++] = (i * 4) + (j * windowsOnDepth * 4) + ( 4 * (windowsOnWidth * windowRows));  //left tri [X\]
			indices[indIndex++] = (i * 4 + 1) + (j * windowsOnDepth * 4) + (4 * (windowsOnWidth * windowRows));
			indices[indIndex++] = (i * 4 + 2) + (j * windowsOnDepth * 4) + ( 4 * (windowsOnWidth * windowRows));

			indices[indIndex++] = (i * 4 + 1) + (j * windowsOnDepth * 4) + ( 4 * (windowsOnWidth * windowRows)); //right tri [\X]
			indices[indIndex++] = (i * 4 + 2) + (j * windowsOnDepth * 4) + ( 4 * (windowsOnWidth * windowRows));
			indices[indIndex++] = (i * 4 + 3) + (j * windowsOnDepth * 4) + ( 4 * (windowsOnWidth * windowRows));

		}
	}

	//--------------END of troubled section-----------

	kuhl_geometry_attrib(b_window, verticesPos, 3, "in_Position", 1);
	kuhl_geometry_attrib(b_window, windowColor, 3, "in_Color", KG_WARN);
	kuhl_geometry_indices(b_window, indices, indIndex);

	free(indices);
	free(verticesPos);
	free(windowColor);
}


int main(int argc, char** argv)
{
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program2 = kuhl_create_program("texture.vert", "texture.frag");
	program = kuhl_create_program("triangle-color.vert", "triangle-color.frag");

	/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
	   the correct program. */
	glUseProgram(program);
	kuhl_errorcheck();

	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	/* Create kuhl_geometry structs for the objects that we want to
	 * draw. */
	init_geometryGround(&ground, program2);

	float vec1[2] = { 123.456, 9876.543 }; //random numbers for making seed
	float vec2[2] = { 0,0 };
	long seed = 0;
	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < 10; i++) {
			vec2[0] = i;
			vec2[1] = j;
			seed = (long) (100 * vecNf_dot(vec1, vec2, 2));
			srand48(seed); //each new building gets its own seed based on coordinates
			init_geometryBuilding(&buildingbottom[i][j], &windowbottom[i][j], &buildingtop[i][j], &windowtop[i][j], program, i,j);

		}
	}

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,0,3}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);

	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
