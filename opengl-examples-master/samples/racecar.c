/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing a shaded 3D triangle.
 *
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"

static GLuint program = 0; /**< id value for the GLSL program */
static kuhl_geometry triangle;
static kuhl_geometry quad;
static kuhl_geometry cube;
static kuhl_geometry cylinder;
static int isRotating = 0; //used for spinning wheels
static int turnAngle = 0; //used for steering wheels 
static int modelAngle = 0; //for rotating model with df

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
	if (action == GLFW_PRESS)
		return;

	// For a list of keys, see: https://www.glfw.org/docs/latest/group__keys.html
	if (key == GLFW_KEY_SPACE)
	{
		printf("toggling rotation\n");
		isRotating = !isRotating;
	}
	if (key == GLFW_KEY_A) {
		turnAngle -= 2;
		printf("Pressed a:\n");
	}
	if (key == GLFW_KEY_S) {
		turnAngle += 2;
		printf("Pressed a:\n");
	}
	if (key == GLFW_KEY_D) {
		modelAngle -= 2;
		printf("Pressed a:\n");
	}
	if (key == GLFW_KEY_F) {
		modelAngle += 2;
		printf("Pressed a:\n");
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
	for (int viewportID = 0; viewportID < viewmat_num_viewports(); viewportID++)
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
		glClearColor(.2, .2, .2, 0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		/* Get the view matrix and the projection matrix */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);

		/* Calculate an angle to rotate the object. glfwGetTime() gets
		 * the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
		float angle = fmod((float) glfwGetTime() * 45, 360);
		if (isRotating == 0){
			angle = 0;
		}	

		/* Make sure all computers/processes use the same angle */
		dgr_setget("angle", &angle, sizeof(GLfloat));

		/*Create Translation matricies*/
		float body1translateMat[16];
		mat4f_translate_new(body1translateMat, 0,0,0);
		float body2translateMat[16];
		mat4f_translate_new(body2translateMat, 0, -.5, -1);
		float body3translateMat[16];
		mat4f_translate_new(body3translateMat, 0, -.5, 1);
		float wheel1translateMat[16];
		mat4f_translate_new(wheel1translateMat, -1.25, -1, 1.5);
		float wheel2translateMat[16];
		mat4f_translate_new(wheel2translateMat, 1.25, -1, 1.5);
		float wheel3translateMat[16];
		mat4f_translate_new(wheel3translateMat, 1, -5.5, -1.5);
		float wheel4translateMat[16];
		mat4f_translate_new(wheel4translateMat, 1, 5.5, -1.5);

		/* Create a 4x4 rotation matrix based on the angle we computed. */
		float body1rotateMat[16];
		mat4f_rotateAxis_new(body1rotateMat, modelAngle, 0,1,0);
		float body2rotateMat[16];
		mat4f_rotateAxis_new(body2rotateMat, modelAngle, 0, 1, 0);
		float body3rotateMat[16];
		mat4f_rotateAxis_new(body3rotateMat, modelAngle, 0, 1, 0);
		float wheel90Mat[16];
		mat4f_rotateAxis_new(wheel90Mat, -90, 0, 0, 1); //fr laying the cylinder horizontally
		
		//For turning wheels with a and s
		float wheelturnRMat[16];
		mat4f_rotateAxis_new(wheelturnRMat, -turnAngle, 0, 1, 0);
		//For rotating whels with d and f
		float wheelturnrotateMat[16];
		mat4f_rotateAxis_new(wheelturnrotateMat, modelAngle, 0, 1, 0);
		//rotation matrix for spining wheel
		float wheelspinMat[16];
		mat4f_rotateAxis_new(wheelspinMat, angle, 0, 1, 0);


		/* Create a scale matrix. */
		float body1scaleMat[16];
		mat4f_scale_new(body1scaleMat, 2, 2, 2);
		float body2scaleMat[16];
		mat4f_scale_new(body2scaleMat, 2, 1, 2);
		float body3scaleMat[16];
		mat4f_scale_new(body3scaleMat, 2, 1, 2);
		float wheel1scaleMat[16];
		mat4f_scale_new(wheel1scaleMat, 1, .25, 1);
		float wheel2scaleMat[16];
		mat4f_scale_new(wheel2scaleMat, 1, .25, 1);
		float wheel3scaleMat[16];
		mat4f_scale_new(wheel3scaleMat, .25, 1, 1);
		float wheel4scaleMat[16];
		mat4f_scale_new(wheel4scaleMat, .25, 1, 1);

		/* Combine the scale and rotation matrices into a single model matrix.
		   modelMat = scaleMat * rotateMat
		*/
		float body1modelMat[16];
		mat4f_mult_mat4f_new(body1modelMat, body1scaleMat, body1rotateMat);
		mat4f_mult_mat4f_new(body1modelMat, body1modelMat, body1translateMat);
		float body2modelMat[16];
		mat4f_mult_mat4f_new(body2modelMat, body2scaleMat, body2rotateMat);
		mat4f_mult_mat4f_new(body2modelMat, body2modelMat, body2translateMat);
		float body3modelMat[16];
		mat4f_mult_mat4f_new(body3modelMat, body3scaleMat, body3rotateMat);
		mat4f_mult_mat4f_new(body3modelMat, body3modelMat, body3translateMat);
		float wheel1modelMat[16];
		mat4f_mult_mat4f_new(wheel1modelMat, wheelturnrotateMat, wheel1translateMat); //turn based on world turning with a/d and lastly translate
		mat4f_mult_mat4f_new(wheel1modelMat, wheel1modelMat, wheelturnRMat); //turn wheel based on steering
		mat4f_mult_mat4f_new(wheel1modelMat, wheel1modelMat, wheel90Mat); //lay wheels horizontally
		mat4f_mult_mat4f_new(wheel1modelMat, wheel1modelMat, wheelspinMat); //turn wheels for if they are turned on
		mat4f_mult_mat4f_new(wheel1modelMat, wheel1modelMat, wheel1scaleMat); //Scale, resolves first
		float wheel2modelMat[16];
		mat4f_mult_mat4f_new(wheel2modelMat, wheelturnrotateMat, wheel2translateMat);
		mat4f_mult_mat4f_new(wheel2modelMat, wheel2modelMat, wheelturnRMat);
		mat4f_mult_mat4f_new(wheel2modelMat, wheel2modelMat, wheel90Mat);
		mat4f_mult_mat4f_new(wheel2modelMat, wheel2modelMat, wheelspinMat);
		mat4f_mult_mat4f_new(wheel2modelMat, wheel2modelMat, wheel2scaleMat);

		float wheel3modelMat[16];
		mat4f_mult_mat4f_new(wheel3modelMat, wheelturnrotateMat, wheel3scaleMat);
		mat4f_mult_mat4f_new(wheel3modelMat, wheel3modelMat, wheel90Mat);
		mat4f_mult_mat4f_new(wheel3modelMat, wheel3modelMat, wheel3translateMat);
		mat4f_mult_mat4f_new(wheel3modelMat, wheel3modelMat, wheelspinMat);
		float wheel4modelMat[16];
		mat4f_mult_mat4f_new(wheel4modelMat, wheelturnrotateMat, wheel4scaleMat);
		mat4f_mult_mat4f_new(wheel4modelMat, wheel4modelMat, wheel90Mat);
		mat4f_mult_mat4f_new(wheel4modelMat, wheel4modelMat, wheel4translateMat);
		mat4f_mult_mat4f_new(wheel4modelMat, wheel4modelMat, wheelspinMat);

		/* Construct a modelview matrix: modelview = viewMat * modelMat */
		float body1modelview[16];
		mat4f_mult_mat4f_new(body1modelview, viewMat, body1modelMat);
		float body2modelview[16];
		mat4f_mult_mat4f_new(body2modelview, viewMat, body2modelMat);
		float body3modelview[16];
		mat4f_mult_mat4f_new(body3modelview, viewMat, body3modelMat);
		float wheel1modelview[16];
		mat4f_mult_mat4f_new(wheel1modelview, viewMat, wheel1modelMat);
		float wheel2modelview[16];
		mat4f_mult_mat4f_new(wheel2modelview, viewMat, wheel2modelMat);
		float wheel3modelview[16];
		mat4f_mult_mat4f_new(wheel3modelview, viewMat, wheel3modelMat);
		float wheel4modelview[16];
		mat4f_mult_mat4f_new(wheel4modelview, viewMat, wheel4modelMat);

		/* Tell OpenGL which GLSL program the subsequent
		 * glUniformMatrix4fv() calls are for. */
		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();
		


		///////////////Start Drawing Objects//////////////////////

		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   body1modelview); // value
		
		/* Generate an appropriate normal matrix based on the model view matrix:
		  normalMat = transpose(inverse(modelview))
		*/
		float normalMat[9];
		mat3f_from_mat4f(normalMat, body1modelview);
		mat3f_invert(normalMat);
		mat3f_transpose(normalMat);
		glUniformMatrix3fv(kuhl_get_uniform("NormalMat"),
		                   1, // number of 3x3 float matrices
		                   0, // transpose
		                   normalMat); // value

		kuhl_errorcheck();
		/* Draw the geometry using the matrices that we sent to the
		 * vertex programs immediately above */
		kuhl_geometry_draw(&cube);
		
		//////////////////////////////

		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			body2modelview); // value

		/* Generate an appropriate normal matrix based on the model view matrix:
		normalMat = transpose(inverse(modelview))
		*/
		float normalMat2[9];
		mat3f_from_mat4f(normalMat2, body2modelview);
		mat3f_invert(normalMat2);
		mat3f_transpose(normalMat2);
		glUniformMatrix3fv(kuhl_get_uniform("NormalMat"),
			1, // number of 3x3 float matrices
			0, // transpose
			normalMat2); // value
		kuhl_geometry_draw(&cube);

		////////////////////////////////////

		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			body3modelview); // value

		/* Generate an appropriate normal matrix based on the model view matrix:
		normalMat = transpose(inverse(modelview))
		*/
		float normalMat3[9];
		mat3f_from_mat4f(normalMat3, body2modelview);
		mat3f_invert(normalMat3);
		mat3f_transpose(normalMat3);
		glUniformMatrix3fv(kuhl_get_uniform("NormalMat"),
			1, // number of 3x3 float matrices
			0, // transpose
			normalMat3); // value
		kuhl_geometry_draw(&cube);

		//////////////////////

		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			wheel1modelview); // value

		/* Generate an appropriate normal matrix based on the model view matrix:
		 normalMat = transpose(inverse(modelview))
		*/
		float normalMat4[9];
		mat3f_from_mat4f(normalMat4, wheel1modelview);
		mat3f_invert(normalMat4);
		mat3f_transpose(normalMat4);
		glUniformMatrix3fv(kuhl_get_uniform("NormalMat"),
			1, // number of 3x3 float matrices
			0, // transpose
			normalMat4); // value

		kuhl_errorcheck();
		kuhl_geometry_draw(&cylinder);

		/////////////////////////////

		/*Send the perspective projection matrix to the vertex program.*/
			glUniformMatrix4fv(kuhl_get_uniform("Projection"),
				1, // number of 4x4 float matrices
				0, // transpose
				perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			wheel2modelview); // value

		/* Generate an appropriate normal matrix based on the model view matrix:
		  normalMat = transpose(inverse(modelview))
		*/
		float normalMat5[9];
		mat3f_from_mat4f(normalMat5, wheel1modelview);
		mat3f_invert(normalMat5);
		mat3f_transpose(normalMat5);
		glUniformMatrix3fv(kuhl_get_uniform("NormalMat"),
			1, // number of 3x3 float matrices
			0, // transpose
			normalMat5); // value

		kuhl_errorcheck();
		kuhl_geometry_draw(&cylinder);

		////////////////////////////////

		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			wheel3modelview); // value

		/* Generate an appropriate normal matrix based on the model view matrix:
		  normalMat = transpose(inverse(modelview))
		*/
		float normalMat6[9];
		mat3f_from_mat4f(normalMat6, wheel3modelview);
		mat3f_invert(normalMat6);
		mat3f_transpose(normalMat6);
		glUniformMatrix3fv(kuhl_get_uniform("NormalMat"),
			1, // number of 3x3 float matrices
			0, // transpose
			normalMat4); // value

		kuhl_errorcheck();
		kuhl_geometry_draw(&cylinder);

		////////////////////////////////////////

		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			wheel4modelview); // value

		/* Generate an appropriate normal matrix based on the model view matrix:
		  normalMat = transpose(inverse(modelview))
		*/
		float normalMat7[9];
		mat3f_from_mat4f(normalMat7, wheel4modelview);
		mat3f_invert(normalMat7);
		mat3f_transpose(normalMat7);
		glUniformMatrix3fv(kuhl_get_uniform("NormalMat"),
			1, // number of 3x3 float matrices
			0, // transpose
			normalMat7); // value

		kuhl_errorcheck();
		kuhl_geometry_draw(&cylinder);

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

/* This code sets up a hexagonal prism which approximates a cylinder. */
void init_geometryCylinder(kuhl_geometry* cylinder, GLuint program)
{
	GLfloat vertices[512];
	GLfloat normals[512];
	GLuint indices[512];
	int verticesIndex = 0;
	int normalsIndex = 0;
	int indicesIndex = 0;
	// Bottom middle
	vertices[verticesIndex++] = 0;
	vertices[verticesIndex++] = -.5;
	vertices[verticesIndex++] = -0;
	normals[normalsIndex++] = 0;
	normals[normalsIndex++] = -1;
	normals[normalsIndex++] = 0;
	// For each vertex around bottom perimeter
	for (int i = 0; i < 6; i++)
	{
		vertices[verticesIndex++] = .5 * sin(i * M_PI / 3);
		vertices[verticesIndex++] = -.5;
		vertices[verticesIndex++] = .5 * cos(i * M_PI / 3);
		normals[normalsIndex++] = 0;
		normals[normalsIndex++] = -1;
		normals[normalsIndex++] = 0;
	}
	// For each face/triangle on bottom, what are the 3 vertices?
	for (int i = 0; i < 6; i++)
	{
		indices[indicesIndex++] = 0;    // center point
		indices[indicesIndex++] = i + 1;
		if (i + 2 >= 7)
			indices[indicesIndex++] = 1;
		else
			indices[indicesIndex++] = i + 2;
	}

	//top middle
	vertices[verticesIndex++] = 0;
	vertices[verticesIndex++] = .5;
	vertices[verticesIndex++] = -0;
	normals[normalsIndex++] = 0;
	normals[normalsIndex++] = 1;
	normals[normalsIndex++] = 0;

	//For each vert around top parimeter
	for (int i = 0; i < 6; i++) {
		vertices[verticesIndex++] = .5 * sin(i * M_PI / 3);
		vertices[verticesIndex++] = .5;
		vertices[verticesIndex++] = .5 * cos(i * M_PI / 3);
		normals[normalsIndex++] = 0;
		normals[normalsIndex++] = 1;
		normals[normalsIndex++] = 0;
	}

	//For each face/ triangle on the top
	for (int i = 8; i < 14; i++)
	{
		indices[indicesIndex++] = 8;    // center point
		indices[indicesIndex++] = i + 1;
		if (i + 2 >= 14)
			indices[indicesIndex++] = 8;
		else
			indices[indicesIndex++] = i + 2;
	}

	//squares for sides
	for (int i = 0; i < 6; i++)
	{
		if (i != 5) {
			vertices[verticesIndex++] = .5 * sin(i * M_PI / 3);
			vertices[verticesIndex++] = -.5;
			vertices[verticesIndex++] = .5 * cos(i * M_PI / 3);
			normals[normalsIndex++] = .5 * sin(i * M_PI / 3);
			normals[normalsIndex++] = -.5;
			normals[normalsIndex++] = .5 * cos(i * M_PI / 3);
			vertices[verticesIndex++] = .5 * sin((i + 1) * M_PI / 3);
			vertices[verticesIndex++] = -.5;
			vertices[verticesIndex++] = .5 * cos((i + 1) * M_PI / 3);
			normals[normalsIndex++] = .5 * sin((i + 1) * M_PI / 3);
			normals[normalsIndex++] = -.5;
			normals[normalsIndex++] = .5 * cos((i + 1) * M_PI / 3);
			vertices[verticesIndex++] = .5 * sin(i * M_PI / 3);
			vertices[verticesIndex++] = .5;
			vertices[verticesIndex++] = .5 * cos(i * M_PI / 3);
			normals[normalsIndex++] = .5 * sin(i * M_PI / 3);
			normals[normalsIndex++] = .5;
			normals[normalsIndex++] = .5 * cos(i * M_PI / 3);
			vertices[verticesIndex++] = .5 * sin((i + 1) * M_PI / 3);
			vertices[verticesIndex++] = .5;
			vertices[verticesIndex++] = .5 * cos((i + 1) * M_PI / 3);
			normals[normalsIndex++] = .5 * sin((i + 1) * M_PI / 3);
			normals[normalsIndex++] = .5;
			normals[normalsIndex++] = .5 * cos((i + 1) * M_PI / 3);
		}
		else if (i == 5) {
			vertices[verticesIndex++] = .5 * sin(i * M_PI / 3);
			vertices[verticesIndex++] = -.5;
			vertices[verticesIndex++] = .5 * cos(i * M_PI / 3);
			normals[normalsIndex++] = .5 * sin(i * M_PI / 3);
			normals[normalsIndex++] = -.5;
			normals[normalsIndex++] = .5 * cos(i * M_PI / 3);
			vertices[verticesIndex++] = .5 * sin((i + 1) * M_PI / 3);
			vertices[verticesIndex++] = -.5;
			vertices[verticesIndex++] = .5 * cos((0) * M_PI / 3);
			normals[normalsIndex++] = .5 * sin((0) * M_PI / 3);
			normals[normalsIndex++] = -.5;
			normals[normalsIndex++] = .5 * cos((0) * M_PI / 3);
			vertices[verticesIndex++] = .5 * sin(i * M_PI / 3);
			vertices[verticesIndex++] = .5;
			vertices[verticesIndex++] = .5 * cos(i * M_PI / 3);
			normals[normalsIndex++] = .5 * sin(i * M_PI / 3);
			normals[normalsIndex++] = .5;
			normals[normalsIndex++] = .5 * cos(i * M_PI / 3);
			vertices[verticesIndex++] = .5 * sin((0) * M_PI / 3);
			vertices[verticesIndex++] = .5;
			vertices[verticesIndex++] = .5 * cos((0) * M_PI / 3);
			normals[normalsIndex++] = .5 * sin((0) * M_PI / 3);
			normals[normalsIndex++] = .5;
			normals[normalsIndex++] = .5 * cos((0) * M_PI / 3);
		}
	}
	
	for (int i = 14; i < 36; i++) {
		indices[indicesIndex++] = i;
		indices[indicesIndex++] = i + 1;
		indices[indicesIndex++] = i + 2;
	}
	


	kuhl_geometry_new(cylinder, program, verticesIndex / 3, GL_TRIANGLES);
	kuhl_geometry_attrib(cylinder, vertices, 3, "in_Position", 1);
	kuhl_geometry_attrib(cylinder, normals, 3, "in_Normal", 1);
	kuhl_geometry_indices(cylinder, indices, indicesIndex);
}

/* This illustrates how to draw 3 sides of a cube. */
void init_geometryCube(kuhl_geometry* cube, GLuint program)
{
	// 4*3 is the number of vertices. If you add more, you should increase this number!
	kuhl_geometry_new(cube, program, 4 * 6, GL_TRIANGLES);
	/* The data that we want to draw. Each set of three numbers is a
	 * position. */
	GLfloat vertexData[] = { -.5, -.5, -.5, // -Z face
							.5,  -.5, -.5,
							.5,   .5, -.5,
							-.5,  .5, -.5,
							-.5, -.5, .5, // +Z face
							.5,  -.5, .5,
							.5,   .5, .5,
							-.5,  .5, .5,
							-.5, -.5, -.5,  // -X face
							-.5, .5,  -.5,
							-.5, .5,   .5,
							-.5, -.5,  .5,
							.5,	-.5, -.5,  //+X face
							.5, .5, -.5,
							.5, .5, .5,
							.5, -.5, .5,
							-.5, -.5, .5,  //-Y face
							.5, -.5, .5,
							.5, -.5, -.5,
							-.5, -.5,-.5,
							-.5, .5, .5,  //+Y face
							.5, .5, .5,
							.5, .5, -.5,
							-.5, .5,-.5
	};
	kuhl_geometry_attrib(cube, vertexData, 3, "in_Position", 1);
	/* The normals for each vertex. Each set of three numbers is a
	 * normal vector. The first normal vector corresponds with the
	 * first point listed above. */
	GLfloat normalData[] = { 0, 0, -1,
							0, 0, -1,
							0, 0, -1,
							0, 0, -1,
							0, 0, 1,
							0, 0, 1,
							0, 0, 1,
							0, 0, 1,
							-1, 0, 0,
							-1, 0, 0,
							-1, 0, 0,
							-1, 0, 0,
							1, 0, 0,
							1, 0, 0,
							1, 0, 0,
							1, 0, 0,
							0, -1, 0,
							0, -1, 0,
							0, -1, 0,
							0, -1, 0,
							0, 1, 0,
							0, 1, 0,
							0, 1, 0,
							0, 1, 0
	};
	kuhl_geometry_attrib(cube, normalData, 3, "in_Normal", 1);
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
	kuhl_geometry_indices(cube, indexData, 3 * 12);
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

	/* The normals for each vertex */
	GLfloat normalData[] = {0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1};
	kuhl_geometry_attrib(geom, normalData, 3, "in_Normal", KG_WARN);
}


/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuad(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog,
	                  4, // number of vertices
	                  GL_TRIANGLES); // type of thing to draw

	/* Vertices that we want to form triangles out of. Every 3 numbers
	 * is a vertex position. Below, we provide indices to form
	 * triangles out of these vertices. */
	GLfloat vertexPositions[] = {0+1.1, 0, 0,
	                             1+1.1, 0, 0,
	                             1+1.1, 1, 0,
	                             0+1.1, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions,
	                     3, // number of components x,y,z
	                     "in_Position", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?

	/* The normals for each vertex */
	GLfloat normalData[] = {0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1,
	                        0, 0, 1};
	kuhl_geometry_attrib(geom, normalData, 3, "in_Normal", KG_WARN);
	
	/* A list of triangles that we want to draw. "0" refers to the
	 * first vertex in our list of vertices. Every three numbers forms
	 * a single triangle. */
	GLuint indexData[] = { 0, 1, 2,  
	                       0, 2, 3 };
	kuhl_geometry_indices(geom, indexData, 6);

	kuhl_errorcheck();
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
	program = kuhl_create_program("triangle-shade.vert", "triangle-shade.frag");

	/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
	   the correct program. */
	glUseProgram(program);
	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	glUniform1i(kuhl_get_uniform("red"), 1);
	kuhl_errorcheck();
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);


	/* Create kuhl_geometry structs for the objects that we want to
	 * draw. */
	init_geometryCube(&cube, program);
	init_geometryCylinder(&cylinder, program);

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,3,10}; // location of camera
	float initCamLook[3] = {0,1,0}; // a point the camera is facing at
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
