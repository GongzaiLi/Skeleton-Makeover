//  ========================================================================
//  COSC422: Advanced Computer Graphics;  University of Canterbury (2022)
//
//  FILE NAME: Skeleton Animation.cpp
//  See Ex14_SkeletalAnimation.pdf for details 
//  ========================================================================

#include <iostream>
#include <fstream>
#include <GL/freeglut.h>
using namespace std;

#include <assimp/cimport.h>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp_extras.h"

//----------Globals----------------------------
const aiScene* scene = NULL;
aiVector3D scene_min, scene_max, scene_center;
float scene_scale;
int tDuration;
int timeStep;

//	View Variables
float cam_y = 1;
float cam_x = 0;
float cam_z = 10;
float camera_movement_speed = 1;
float angle = 0;
float camera_rotation_speed = 3;
float view_angle = 0;

// ------A recursive function to traverse scene graph and render each mesh----------
// Simplified version for rendering a skeleton mesh
void render(const aiNode* node)
{
	aiMatrix4x4 m = node->mTransformation;
	aiMesh* mesh;
	aiFace* face;
	float materialCol[4] = { 1, 0, 1, 1 };
	int meshIndex;

	m.Transpose();   //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float*)&m);   //Multiply by the transformation matrix for this node

	//The scene graph for a skeleton contains at most one mesh per node
	//Skeleton meshes are always triangle meshes
	if(node->mNumMeshes > 0)
	{
		meshIndex = node->mMeshes[0];          //Get the mesh indices from the current node
		mesh = scene->mMeshes[meshIndex];    //Using mesh index, get the mesh object
		glColor4fv(materialCol);   //Default material colour

		//Draw the mesh in the current node
		for (int k = 0; k < mesh->mNumFaces; k++)
		{
			face = &mesh->mFaces[k];
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < face->mNumIndices; i++) {
				int vertexIndex = face->mIndices[i];
				if (mesh->HasNormals())
					glNormal3fv(&mesh->mNormals[vertexIndex].x);
				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}
			glEnd();
		}
	}

	// Recursively draw all children of the current node
	for (int i = 0; i < node->mNumChildren; i++)
		render(node->mChildren[i]);

	glPopMatrix();
}

//----- Function to update node matrices for each tick ------
// Complete this function
void updateNodeMatrices(int tick)
{
	aiAnimation* anim;   //Animation object
	aiMatrix4x4 matPos, matRot, matProd;  //Position, rotation, product matrices
	aiMatrix3x3 matRot3;
	aiNode* node;

	aiNodeAnim* chnl;
	aiVector3D posn;
	aiQuaternion rotn;

	anim = scene->mAnimations[0];
	for (int i = 0; i < anim->mNumChannels; i++)
	{
		matPos = aiMatrix4x4();   //Identity
		matRot = aiMatrix4x4();
		aiNodeAnim* chnl = anim->mChannels[i];      //Channel

		if (chnl->mNumPositionKeys > 1) {
			posn = chnl->mPositionKeys[tick].mValue;
		}
		else {
			posn = chnl->mPositionKeys[0].mValue;
		}
		if (chnl->mNumRotationKeys > 1) {
			rotn = chnl->mRotationKeys[tick].mValue;
		}
		else {
			rotn = chnl->mRotationKeys[0].mValue;
		}

		matPos.Translation(posn, matPos);
		matRot3 = rotn.GetMatrix();
		matRot = aiMatrix4x4(matRot3);
		matProd = matPos * matRot;


		node = scene->mRootNode->FindNode(chnl->mNodeName);
		node->mTransformation = matProd;
	}
}

//--------------------OpenGL initialization------------------------
void initialise()
{
	float ambient[4] = { 0.2, 0.2, 0.2, 1.0 };  //Ambient light
	float white[4] = { 1, 1, 1, 1 };		    //Light's colour
	float black[4] = { 0, 0, 0, 1 };
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, black);   //Disable specular light
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, 1, 1.0, 500.0);

	//---- Load the model ------
	scene = aiImportFile("./src/models/Boxing.bvh", aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Debone);
	if (scene == NULL) exit(1);
	//printTreeInfo(scene->mRootNode);
	//printAnimInfo(scene, 0);

	tDuration = scene->mAnimations[0]->mDuration;

	float fps = scene->mAnimations[0]->mTicksPerSecond;
	timeStep = 1000.0 / fps; //In milliseconds

	get_bounding_box(scene, &scene_min, &scene_max);
	scene_center = (scene_min + scene_max) * 0.5f;
	aiVector3D scene_diag = scene_max - scene_center;
	scene_scale = 1.0 / scene_diag.Length();
}


//------The main display function---------
void display()
{
	float lightPosn[4] = { -5, 10, 10, 1 };
	aiMatrix4x4 m = scene->mRootNode->mTransformation;
	float xpos = m.a4;   //Root joint's position in world space
	float ypos = m.b4;
	float zpos = m.c4;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cam_x, cam_y, cam_z, 0, cam_y, 0, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosn);

	glPushMatrix();
	   glScalef(scene_scale, scene_scale, scene_scale);
	   glTranslatef(-xpos, 0, -zpos);   //Move model to origin
	   glRotatef(view_angle, 0, 1, 0);
	   render(scene->mRootNode);
	glPopMatrix();

	glutSwapBuffers();
}

//update
void update(int tick) {

	if (tick > tDuration) return;
	//cout << tick << endl;
	updateNodeMatrices(tick);
	tick++;
	glutTimerFunc(timeStep, update, tick);
	glutPostRedisplay();
}

// Camera rotation
void cameraRotation(int direction) {
	view_angle += direction * camera_rotation_speed;
	
	if (view_angle >= 360) {
		view_angle = 0;
	}
	else if (view_angle <= 0) {
		view_angle = 360;
	}
}

// Camera zoom
void cameraZoom(int direction) {
	//float move_x = -cam_x;
	//float move_z = -cam_z;
	//if (abs(move_x) > abs(move_z)) {
	//	move_z /= abs(move_x);
	//	move_x /= abs(move_x);
	//}
	//else {
	//	move_x /= abs(move_z);
	//	move_z /= abs(move_z);
	//}


	//float xDist = scene_center.x - cam_x;
	//float yDist = scene_center.y - cam_y;
	//float zDist = scene_center.z - cam_z;
	//float distance = sqrt(pow(xDist, 2) + pow(yDist, 2) + pow(zDist, 2));
	////if (distance > 1 || distance < 0)
	////{
		cout << cam_z << endl;

		//cam_x += move_x * direction * camera_movement_speed;
		cam_z += direction * camera_movement_speed;

		if (cam_z <= 2) {
			cam_z = 2;
		}

		if (cam_z >= 20) {
			cam_z = 20;
		}
	//}
}

// Callback function for special keyboard events
void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		cameraRotation(1);
		break;
	case GLUT_KEY_RIGHT:
		cameraRotation(-1);
		break;
	case GLUT_KEY_UP:
		cameraZoom(-1);
		break;
	case GLUT_KEY_DOWN:
		cameraZoom(1);
		break;
	}

	glutPostRedisplay();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Skeleton Animation");

	initialise();
	glutDisplayFunc(display);
	glutTimerFunc(timeStep, update, 0);
	glutSpecialFunc(special);


	glutMainLoop();
	aiReleaseImport(scene);
}

