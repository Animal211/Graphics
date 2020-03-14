/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing a 3D triangle.
 *
 * @author Scott Kuhl
 */

#include "libkuhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static GLuint program = 0; /**< id value for the GLSL program */

static kuhl_geometry triangle;
static kuhl_geometry quad;

static int isRotating=0;
static int terrainsidelength = 1000; //4x4 verts and  3x3 quads

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/* If the library handles this keypress, return */
	if (kuhl_keyboard_handler(window, key, scancode, action, mods))
		return;

	/* Custom key handling code below: */

	/* Action can be press (key down), release (key up), or repeat
	   (key is held down, the delay until the first repeat and the
	   rate at which repeats fire may depend on the OS.)
	   
	   Here, we ignore any press events so the event will only
	   happen when the key is released or it will happen repeatedly
	   when the key is held down.
	*/
	if(action == GLFW_PRESS)
		return;

	// For a list of keys, see: https://www.glfw.org/docs/latest/group__keys.html
	if(key == GLFW_KEY_SPACE)
	{
		printf("toggling rotation\n");
		isRotating = !isRotating;
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
		//mat4f_lookat_new(viewMat, 0, 0, 10 +glfwGetTime()*10, 0, 0, 0 +glfwGetTime()*10, 0, 1, 0);
		//mat4f_print(viewMat);

		/* Calculate an angle to rotate the object. glfwGetTime() gets
		 * the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
		float angle = fmodf((float) (glfwGetTime()*45.0), 360);
		if(isRotating == 0)
			angle = 0;

		/* Make sure all computers/processes use the same angle */
		dgr_setget("angle", &angle, sizeof(GLfloat));

		/* Create a 4x4 rotation matrix based on the angle we computed. */
		float rotateMat[16];
		mat4f_rotateAxis_new(rotateMat, angle, 0,1,0);

		/* Create a scale matrix. */
		float scaleMat[16];
		mat4f_scale_new(scaleMat, 3, 3, 3);

		/* Combine the scale and rotation matrices into a single model matrix.
		   modelMat = scaleMat * rotateMat
		*/
		float modelMat[16];
		mat4f_mult_mat4f_new(modelMat, scaleMat, rotateMat);

		/* Construct a modelview matrix: modelview = viewMat * modelMat */
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, modelMat);

		/* Tell OpenGL which GLSL program the subsequent
		 * glUniformMatrix4fv() calls are for. */
		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();
		
		/* Send the perspective projection matrix to the vertex program. */
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
		//kuhl_geometry_draw(&triangle);
		kuhl_geometry_draw(&quad);

		/* If we wanted to draw multiple triangles and quads at
		 * different locations, we could call glUniformMatrix4fv again
		 * to change the ModelView matrix and then call
		 * kuhl_geometry_draw() again to draw that object again using
		 * the new model matrix. */

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

void init_geometryTriangle(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog, 3, // num vertices
	                  GL_TRIANGLES); // primitive type

	/* Vertices that we want to form triangles out of. Every 3 numbers
	 * is a vertex position. Since no indices are provided, every
	 * three vertex positions form a single triangle.*/
	GLfloat vertexPositions[] = {0, 0, 0,
	                             1, 0, 0,
	                             1, 1, 0};
	kuhl_geometry_attrib(geom, vertexPositions, // data
	                     3, // number of components (x,y,z)
	                     "in_Position", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?
}


/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuad(kuhl_geometry* geom, GLuint prog)
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
	verticesPos = (GLfloat*) malloc(temp * sizeof(GLfloat));
	indices1 = (GLuint*) malloc(temp1 * sizeof(GLuint));
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
	for (int j = 0; j < (terrainsidelength-1); j++) { //for each row of quads
		for (int i = 0; i < (terrainsidelength - 1); i++) { //for each quad

			indices1[indIndex++] = i + j*terrainsidelength; //left tri [X\]
			indices1[indIndex++] = 1 + i + j*terrainsidelength;
			indices1[indIndex++] = i + terrainsidelength + j*terrainsidelength;

			indices1[indIndex++] = 1 + i + j*terrainsidelength; //right tri [\X]
			indices1[indIndex++] = i + terrainsidelength + j*terrainsidelength;
			indices1[indIndex++] = 1 + i + terrainsidelength + j * terrainsidelength;
		}
	}

	GLfloat* texcoordData;
	int temp3 = 2 * terrainsidelength * terrainsidelength; // x and y for 1000x1000 verts
	texcoordData = (GLfloat*) malloc(temp3 * sizeof(GLfloat));
	int texIndex = 0;

	for (float j = 0; j < terrainsidelength; j++) { //For each row
		for (float i = 0; i < terrainsidelength; i++) { 
			if (i == 0) {
				texcoordData[texIndex++] = 0;
			}
			else {
				texcoordData[texIndex++] = i / terrainsidelength;
			}

			if (j == 0) {
				texcoordData[texIndex++] = 0; 
			}
			else {
				texcoordData[texIndex++] = j / terrainsidelength;
			}
		}
	}

	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);
	// The 2 parameter above means each texture coordinate is a 2D coordinate.

	kuhl_geometry_indices(geom, indices1, indIndex); //9*6 bc number of indices want to use temp1 but that breaks it for some reason.
	
	/* Load the texture. It will be bound to texId */
	GLuint texId = 0;
	float aspectRatio = kuhl_read_texture_file("../images/color1.png", &texId); 
	msg(MSG_DEBUG, "\n Aspect ratio of image is %f", aspectRatio); // write message to log.txt

	/* Tell this piece of geometry to use the texture we just loaded. */
	kuhl_geometry_texture(geom, texId, "tex", KG_WARN);
	
	kuhl_errorcheck();
	free(texcoordData);
	free(verticesPos);
	free(indices1);
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
	program = kuhl_create_program("terrain.vert", "terrain.frag");

	/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
	   the correct program. */
	glUseProgram(program);
	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	//glUniform1i(kuhl_get_uniform("red"), 0);
	kuhl_errorcheck();
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	/* Create kuhl_geometry structs for the objects that we want to
	 * draw. */
	init_geometryTriangle(&triangle, program);
	init_geometryQuad(&quad, program);

	dgr_init();     /* Initialize DGR based on config file. */

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
