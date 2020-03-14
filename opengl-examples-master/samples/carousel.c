/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing textured geometry.
 *
 * @author Scott Kuhl
 */

#include "libkuhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>

static GLuint program = 0; /**< id value for the GLSL program */
static GLuint program2 = 1;

static kuhl_geometry quad[5];
static kuhl_geometry* duckmodel;

static float model[16] = { 1,0,0,0,0,1,0,0,0,0,1,0,-.081,-.525,.022,1 };
static float duckmodelview[16];

static float modelview1[16]; //used for storing model views for calculating which to draw first
static float modelview2[16];
static float modelview3[16];
static float modelview4[16];
static float modelview5[16];

static float aspectratios[5]; //used to save the aspect ratio of all quads
static float quaddistance[5][2]; //here we will store the z values to the center of each quad to know which quad to draw first and the second column is for marking if quad has been drawn
static float vec4[4] = { 0,0,0,0 }; //used for calculating camera distance to quads

static int maxindex = 0; // used to keep track of farthes quad in disance drawing
static int texcount = 0; //keeps the count of how many quads to draw
static char* texture[5] = {NULL,NULL,NULL,NULL,NULL}; //these are used to store the file locations of the input texture files

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/* If the library handles this keypress, return */
	if (kuhl_keyboard_handler(window, key, scancode, action, mods))
		return;
	
	if (action == GLFW_PRESS)
		return;

	// For a list of keys, see: https://www.glfw.org/docs/latest/group__keys.html
	if (key == GLFW_KEY_SPACE)
	{
		printf("toggling rotation\n");
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

		/* Turn on blending (note, if you are using transparent textures,
		   the transparency may not look correct unless you draw further
		   items before closer items. This program always draws the
		   geometry in the same order.). */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		/* Get the view or camera matrix; update the frustum values if needed. */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);

		/* Calculate an angle to rotate the object. glfwGetTime() gets
		 * the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
		float angle = fmod(glfwGetTime()*45, 360);

		/* Make sure all computers/processes use the same angle */
		dgr_setget("angle", &angle, sizeof(GLfloat));

		///////////////////////Start ofmy matrices
		int temp = 0;
		temp = 360 / texcount; //This sets a spacing between the quads based on hw amny there are


		mat4f_mult_mat4f_new(duckmodelview, viewMat, model);

		for (int i = 0; i < texcount; i++) {
			float quadtranslateMat[16]; //used for offsettin the quads after rotation
			mat4f_translate_new(quadtranslateMat, 0, 0, 2);

			float rotateMat1[16]; //for rotating the quads aroud the origin making them face center
			mat4f_rotateAxis_new(rotateMat1, angle + (temp * i), 0, 1, 0);

			float scaleMatrix1[16], scaleMatrix2[16];
			mat4f_scale_new(scaleMatrix1, aspectratios[i], 1, 1);

			// Modelview = (viewMatrix * scaleMatrix) * rotationMatrix
			float modelview[16];
			mat4f_mult_mat4f_new(modelview, viewMat, scaleMatrix1);
			mat4f_mult_mat4f_new(modelview, modelview, rotateMat1);
			mat4f_mult_mat4f_new(modelview, modelview, quadtranslateMat);

			if (i == 0) {
				mat4f_copy(modelview1, modelview);
			}
			else if (i == 1) {
				mat4f_copy(modelview2, modelview);
			}
			else if (i == 2) {
				mat4f_copy(modelview3, modelview);
			}
			else if (i == 3) {
				mat4f_copy(modelview4, modelview);
			}
			else if (i == 4) {
				mat4f_copy(modelview5, modelview);
			}
		}

		//now all quads modelview matrices have been calculated, now we need to see which order to draw the quad farthest to closest
		vec4f_set(vec4, .5, .5, 0, 0);
		mat4f_mult_vec4f(vec4, modelview1); //multiply the center of the quad agains modelview to get camera coordinates
		quaddistance[0][0] = vec4[2]; //save the z component f the resulting vector
		//repeat for rest of the quads
		vec4f_set(vec4, .5, .5, 0, 0);
		mat4f_mult_vec4f(vec4, modelview2);
		quaddistance[1][0] = vec4[2];
		vec4f_set(vec4, .5, .5, 0, 0);
		mat4f_mult_vec4f(vec4, modelview3);
		quaddistance[2][0] = vec4[2];
		vec4f_set(vec4, .5, .5, 0, 0);
		mat4f_mult_vec4f(vec4, modelview4);
		quaddistance[3][0] = vec4[2];
		vec4f_set(vec4, .5, .5, 0, 0);
		mat4f_mult_vec4f(vec4, modelview5);
		quaddistance[4][0] = vec4[2];

		/*for (int k = 0; k < 5; k++) { //print out the z distances we saved
			printf("%f", quaddistance[k][0]);
		}*/

		//We want to draw the duck before the quads because it is not transparent
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
			duckmodelview); // value
		kuhl_geometry_draw(duckmodel);

		for (int l = 0; l < 5; l++) { //before each frame of drawing all quads reset which have been drawn
			quaddistance[l][1] = 0;;
		}

		for (int i = 0; i < texcount; i++) {
			for (int k = 0; k < texcount; k++) { //find a quad that hasnt been drawn and set as max
				if (quaddistance[k][1] != 1) {
					//printf("\n maxindex starting as %d", k);
					maxindex = k;
					break;
				}
			}
			for (int j = 0; j < texcount; j++) {
				if (quaddistance[j][0] < quaddistance[maxindex][0] && quaddistance[j][1] == 0) {
					//printf("\n maxindex switched from %d to %d", maxindex, j);
					maxindex = j;
				}
			}
			//max index now points to farthest quad, mark that it is being drawn and draw it
			quaddistance[maxindex][1] = 1;
			//printf("\n Drawing quad at index %d", maxindex);

			kuhl_errorcheck();
			glUseProgram(program);
			kuhl_errorcheck();

			switch (maxindex) {
				case 0:
					/* Send the perspective projection matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("Projection"),
						1, // number of 4x4 float matrices
						0, // transpose
						perspective); // value
					/* Send the modelview matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
						1, // number of 4x4 float matrices
						0, // transpose
						modelview1); // value
					//printf("\n sending model1 \n");
					break;
				case 1:
					/* Send the perspective projection matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("Projection"),
						1, // number of 4x4 float matrices
						0, // transpose
						perspective); // value
					/* Send the modelview matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
						1, // number of 4x4 float matrices
						0, // transpose
						modelview2); // value
					//printf("\n sending model2 \n");
					break;
				case 2:
					/* Send the perspective projection matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("Projection"),
						1, // number of 4x4 float matrices
						0, // transpose
						perspective); // value
					/* Send the modelview matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
						1, // number of 4x4 float matrices
						0, // transpose
						modelview3); // value
					//printf("\n sending model3 \n");
					break;
				case 3:
					/* Send the perspective projection matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("Projection"),
						1, // number of 4x4 float matrices
						0, // transpose
						perspective); // value
					/* Send the modelview matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
						1, // number of 4x4 float matrices
						0, // transpose
						modelview4); // value
					//printf("\n sending model4 \n");
					break;
				case 4:
					/* Send the perspective projection matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("Projection"),
						1, // number of 4x4 float matrices
						0, // transpose
						perspective); // value
					/* Send the modelview matrix to the vertex program. */
					glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
						1, // number of 4x4 float matrices
						0, // transpose
						modelview5); // value
					//printf("\n sending model5 \n");
					break;
				}
				kuhl_errorcheck();
				/* the geometry using the matrices that we sent to the vertex programs immediately above */
				kuhl_geometry_draw(&quad[maxindex]);
			}
		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

void init_geometryActualTriangle(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog, 3, GL_TRIANGLES);

	GLfloat texcoordData[] = {0, 0,
	                          1, 0,
	                          1, 1 };
	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);
	// The 2 parameter above means each texture coordinate is a 2D coordinate.


	/* The data that we want to draw */
	GLfloat vertexData[] = {0, 0, 0,
	                        1, 0, 0,
	                        1, 1, 0};
	kuhl_geometry_attrib(geom, vertexData, 3, "in_Position", KG_WARN);
	// The 3 parameter above means that each vertex position is a 3D coordinate.

	/* Load the texture. It will be bound to texId */	
	GLuint texId = 0;
	float aspectRatio = kuhl_read_texture_file("../images/rainbow.png", &texId);
	msg(MSG_DEBUG, "Aspect ratio of image is %f\n", aspectRatio); // write message to log.txt
	
	/* Tell this piece of geometry to use the texture we just loaded. */
	kuhl_geometry_texture(geom, texId, "tex", KG_WARN);

	kuhl_errorcheck();
}

/*I was having an issue renamng this to quad so temp it is triangle*/
void init_geometryTriangle(kuhl_geometry* geom, GLuint prog, const char* textureFilename, int index)
{
	kuhl_geometry_new(geom, prog, 4, GL_TRIANGLES); //draw triangles using 4 vertices

	GLfloat texcoordData[] = { 0, 0, //bottom left of texture
							  1, 0, //bottom right
							  1, 1, //top right
							  0, 1}; //top left
	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);
	// The 2 parameter above means each texture coordinate is a 2D coordinate.


	/* The data that we want to draw */
	GLfloat vertexData[] = { 0, 0, 0, //bottom left of quad in 3d
							1, 0, 0, //bottom right
							1, 1, 0, //top right
							0, 1, 0, //top left
	};
	kuhl_geometry_attrib(geom, vertexData, 3, "in_Position", KG_WARN);
	// The 3 parameter above means that each vertex position is a 3D coordinate.

	  /*use index of each point to pass two triangles to make quad*/
	GLuint indexData[] = { 0, 1, 2,
						   0, 2, 3 };
	kuhl_geometry_indices(geom, indexData, 6);

	/* Load the texture. It will be bound to texId */
	GLuint texId = 0;
	float aspectRatio = kuhl_read_texture_file(textureFilename, &texId); //../images/rainbow.png
	msg(MSG_DEBUG, "\n Aspect ratio of image is %f", aspectRatio); // write message to log.txt
	aspectratios[index] = aspectRatio;

	/* Tell this piece of geometry to use the texture we just loaded. */
	kuhl_geometry_texture(geom, texId, "tex", KG_WARN);

	kuhl_errorcheck();
}

int main(int argc, char** argv)
{
	texcount = argc; //save the number of textures passed and subtract one for counting name
	texcount = texcount - 1;
	/*Save the arguments*/
	printf("Executing %s:", argv[0]);
	if (argc == 1) {
		printf("\nNo texture files passed. Please re-execute with 2-5 texture files as aruments.");
		exit(1);
	}
	if (argc > 6 || argc == 2) { //if there are more than 5 args it cant support that
		printf("\n This program can only support 2-5 arguments. Re-execute with an acceptable number of arguments");
		exit(1);
	}
	if (argc >= 2)
	{
		printf("\n%d Textures passed.", texcount);
		printf("\n----Texture File Locations----");

		/*save the input args for later use*/
		if (argv[1] != NULL && argc >= 2) {
			texture[0] = argv[1];
		}
		if (argv[2] != NULL && argc >= 3) {
			texture[1] = argv[2];
		}
		if (argv[3] != NULL && argc >= 4) {
			texture[2] = argv[3];
		}
		if (argv[4] != NULL && argc >= 5) {
			texture[3] = argv[4];
		}
		if (argv[5] != NULL && argc >= 6) {
			texture[4] = argv[5];
		}
		printf("\ntexture1 = %s", texture[0]);
		printf("\ntexture2 = %s", texture[1]);
		printf("\ntexture3 = %s", texture[2]);
		printf("\ntexture4 = %s", texture[3]);
		printf("\ntexture5 = %s", texture[4]);

	}

	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	program2 = kuhl_create_program("carousel.vert", "carousel.frag"); // triangle-shade
	glUseProgram(program2);
	duckmodel = kuhl_load_model("../models/duck/duck.dae", NULL, program2, NULL);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program("texture.vert", "texture.frag");
	glUseProgram(program);
	kuhl_errorcheck();

	//init_geometryTriangle(&triangle, program);
	for (int i = 0; i < texcount; i++) {
		if (texture[i] != NULL) {
			init_geometryTriangle(&quad[i], program, texture[i], i);
		}
	}

	/*print the saved aspect ratios
	printf("/n Aspect ratios: {");
	for (int i = 0; i < texcount; i++) {
		printf("%f, ", aspectratios[i]);
	}
	printf("} /n");
	*/

	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,0,10}; // location of camera
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
