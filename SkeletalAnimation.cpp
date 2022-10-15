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
#include "loadTGA.h"

//----------Globals----------------------------
const aiScene* scene = NULL;
aiVector3D scene_min, scene_max, scene_center;
float scene_scale;
int tDuration;
int timeStep;
bool toggleAnimation = false;
bool toggleBackground = true;
float PI = 3.14159265;
float CDR = PI / 180.0;
float lightPosn[4] = { -5, 10, 10, 1 };
float shadowMat[16] = { lightPosn[1],0,0,0, -lightPosn[0],0,-lightPosn[2],-1,0,0,lightPosn[1],0, 0,0,0,lightPosn[1] };
aiVector3D left_foot_vec;
int tick = 0;
int tick_step = 1;

// ball
float ball_x = 0.6;
float ball_y = 0.09;
float ball_z = 0.4;

float init_ball = true;
float reset_ball = false;
float isKicked = false;


float ball_time = 0;
float ball_Veloctiy = 10;
float ball_Vx = ball_Veloctiy * cos(45 * CDR) * cos(45 * CDR);;
float ball_Vy = ball_Veloctiy * sin(45 * CDR) * cos(45 * CDR);;
float ball_Vz = ball_Veloctiy * cos(45 * CDR);
float gravity = 9.81;

//	View Variables
float cam_y = 1;
float cam_x = 0;
float cam_z = 10;
float camera_movement_speed = 1;
float camera_rotation_speed = 3;
float view_angle = 0;

// texture
const int TEXTURES_NUM = 2;
GLuint texID[TEXTURES_NUM]; // init textures number
string texture_path = "./src/textures/";
const char* textures[TEXTURES_NUM] = { "floor.tga", "background_3.tga" };


GLUquadric* q;
float theta = 20.0;  //walking,leg,arm,and tails'rotating angle


// upload texture
void loadTexture()
{
	glGenTextures(TEXTURES_NUM, texID);


	glBindTexture(GL_TEXTURE_2D, texID[0]);
	loadTGA(texture_path + textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texID[1]);
	loadTGA(texture_path + textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);




	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//GL_REPLACE  GL_MODULATE

}


// Rendering a skeleton mesh
void renderSkeleton(const aiNode* node) {
	aiMesh* mesh;
	int meshIndex;
	float materialCol[4] = { 0, 0, 1, 1 };
	aiFace* face;

	meshIndex = node->mMeshes[0];          //Get the mesh indices from the current node
	mesh = scene->mMeshes[meshIndex];    //Using mesh index, get the mesh object

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

// render Chest
void renderChest() {
	glPushMatrix();
	glTranslatef(0, 7, 0);
	glScalef(14, 20, 4);
	glutSolidCube(1);
	glPopMatrix();
}

// render Hip
void renderHips() {
	glPushMatrix();
	glScalef(14, 4, 4);
	glutSolidCube(1);
	glPopMatrix();
}

// render Collar
void renderCollar(int side) {
	glPushMatrix();
	glTranslatef(side * 7, 0, 0);
	glutSolidSphere(2.5, 20, 20);
	glPopMatrix();
}

// render up leg
void renderUpLeg() {
	glPushMatrix();
	glTranslatef(0, -9, 0);
	glScalef(3, 18, 3);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -18, 0);
	glutSolidSphere(2.5, 20, 20);
	glPopMatrix();
}

// render low leg
void renderLowLeg() {
	glPushMatrix();
	glTranslatef(0, -9, 0);
	glScalef(3, 18, 3);
	glutSolidCube(1);
	glPopMatrix();
}

// reder right foot
void renderFoot() {
	glPushMatrix();
	glTranslatef(0, -1.5, 2);
	glScalef(3, 3, 7);
	glutSolidCube(1);
	glPopMatrix();
}

//  reder Up Arm
void renderUpArm() {
	glPushMatrix();
	glTranslatef(0, -6.5, 0);
	glScalef(2.5, 13, 2.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -13, 0);
	glutSolidSphere(2.5, 20, 20);
	glPopMatrix();
}

// render Hand
void renderHand() {
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glScalef(2.5, 14, 2.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -8, 0);
	glutSolidSphere(3, 20, 20);

	glPopMatrix();


}

void renderNeck() {
	glPushMatrix();
	glTranslatef(0, 3, 0);
	glRotatef(90, 1, 0, 0);
	glutSolidCylinder(1.5, 5, 50, 10);
	glPopMatrix();
}

void renderHead() {
	glPushMatrix();
	glTranslatef(0, 2, 0);
	glutSolidSphere(6, 20, 20);
	glPopMatrix();
}

// ------A recursive function to traverse scene graph and render each mesh----------
// Simplified version for rendering a skeleton mesh
void render(const aiNode* node)
{
	aiMatrix4x4 m = node->mTransformation;

	m.Transpose();   //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float*)&m);   //Multiply by the transformation matrix for this node

	//The scene graph for a skeleton contains at most one mesh per node
	//Skeleton meshes are always triangle meshes
	if (node->mNumMeshes > 0)
	{
		if (node->mName == aiString("Chest"))
		{
			renderChest();
		}
		else if (node->mName == aiString("Hips")) {
			renderHips();
		}
		else if (node->mName == aiString("RightCollar")) {
			renderCollar(-1);
		}
		else if (node->mName == aiString("LeftCollar")) {
			renderCollar(1);
		}
		else if (node->mName == aiString("RightUpLeg") || node->mName == aiString("LeftUpLeg")) {
			renderUpLeg();
		}
		else if (node->mName == aiString("RightLowLeg") || node->mName == aiString("LeftLowLeg")) {
			renderLowLeg();
		}
		else if (node->mName == aiString("RightFoot")) {
			renderFoot();
		}
		else if (node->mName == aiString("LeftFoot")) {
			aiNode* parent = node->mParent;
			aiMatrix4x4 matrices[4];
			matrices[3] = node->mTransformation;
			int index = 2;

			while (parent != NULL) {
				matrices[index] = parent->mTransformation;
				parent = parent->mParent;
				index--;
			}

			aiMatrix4x4 worldMatrix = matrices[0];

			for (int i = 1; i < 4; i++)
			{
				worldMatrix *= matrices[i];
			}

			left_foot_vec = aiVector3D(0, 0, 0);
			left_foot_vec *= worldMatrix;
			renderFoot();
		}
		else if (node->mName == aiString("RightUpArm") || node->mName == aiString("LeftUpArm")) {
			renderUpArm();
		}
		else if (node->mName == aiString("RightHand") || node->mName == aiString("LeftHand")) {
			renderHand();
		}
		else if (node->mName == aiString("Neck")) {
			renderNeck();
		}
		else if (node->mName == aiString("Head")) {
			renderHead();
		}
		else
		{
			renderSkeleton(node);
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
	q = gluNewQuadric();

	loadTexture();

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
	scene = aiImportFile("./src/models/Dance.bvh", aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Debone);
	if (scene == NULL) exit(1);
	//printTreeInfo(scene->mRootNode);
	//printAnimInfo(scene, 0);

	// animation = scene->mAnimations[0];

	tDuration = scene->mAnimations[0]->mDuration;

	float fps = scene->mAnimations[0]->mTicksPerSecond;
	timeStep = 1000.0 / fps; //In milliseconds


	get_bounding_box(scene, &scene_min, &scene_max);
	scene_center = (scene_min + scene_max) * 0.5f;
	aiVector3D scene_diag = scene_max - scene_center;
	scene_scale = 1.0 / scene_diag.Length();
}
void drawFloor()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texID[0]);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	for (int i = -5; i < 5; i++)
	{
		for (int j = -5; j < 5; j++)
		{
			glNormal3f(0, 1, 0);
			glTexCoord2f(0.0, 0.0);
			glVertex3f(i, 0.01, j);
			glTexCoord2f(10.0, 0.0);
			glVertex3f(i, 0.01, j + 1);
			glTexCoord2f(10.0, 10.0);
			glVertex3f(i + 1, 0.01, j + 1);
			glTexCoord2f(0.0, 10.0);
			glVertex3f(i + 1, 0.01, j);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawBackground()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texID[1]);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-5, 10, -5);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-5, 0, -5);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(5, 0, -5);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(5, 10, -5);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawTree()
{
	glPushMatrix();
	glColor4f(0.5, 0., 0., 1.0); //base
	glTranslatef(0, 2, 0);
	glScalef(1, 4, 1);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();               //green 1    
	glColor4f(0., 1., 0., 1.0);
	glTranslatef(0, 4, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(3, 2, 20, 20);
	glPopMatrix();

	glPushMatrix();               //green 2   
	glColor4f(0., 1., 0., 1.0);
	glTranslatef(0, 6, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(3, 2, 20, 20);
	glPopMatrix();
}

void drawPanda()
{

	glPushMatrix();                        //body
		glColor3f(1, 1, 1);
		glTranslatef(30.0, 4, 0.0);
		glRotatef(-90.0, 0., 1., 0.);
		gluCylinder(q, 2.0, 2.0, 4, 20, 5);
		glTranslatef(0.0, 0.0, 4);
		gluDisk(q, 0.0, 2.0, 20, 4);
		glTranslatef(0.0, 0.0, -4);
		gluDisk(q, 0.0, 2.0, 20, 4);
	glPopMatrix();


	glPushMatrix();
		glColor3f(1, 1, 1);
		glTranslatef(25, 4, 0.0);                  //head
		glutSolidSphere(1.5, 20, 20);

		glPushMatrix();
			glColor3f(0, 0, 0);
			glTranslatef(0, 1.2, -1);                  //ear
			glutSolidSphere(0.5, 20, 20);
		glPopMatrix();

		glPushMatrix();
			glColor3f(0, 0, 0);
			glTranslatef(0, 1.2, 1);                  //ear
			glutSolidSphere(0.5, 20, 20);
		glPopMatrix();

		glPushMatrix();
			glColor3f(0, 0, 0);
			glTranslatef(-1.2, 0.8, -0.5);                  //eye
			glutSolidSphere(0.2, 20, 20);
		glPopMatrix();

		glPushMatrix();
			glColor3f(0, 0, 0);
			glTranslatef(-1.2, 0.8, 0.5);                 //eye
			glutSolidSphere(0.2, 20, 20);
		glPopMatrix();

		glPushMatrix();
		glColor3f(0, 0, 0);
			glTranslatef(-1.5, 0.0, 0);                 //eye
			glutSolidSphere(0.15, 20, 20);
		glPopMatrix();

	glPopMatrix();

	
	glPushMatrix();                       //legs
		glColor3f(1, 1, 1);
		glTranslatef(27, 1, 0.5);
		glScalef(0.5, 2, 0.5);
		glutSolidCube(1);
	glPopMatrix();

	
	glPushMatrix();                          //legs
		glColor3f(1, 1, 1);
		glTranslatef(27, 1, -0.5);
		glScalef(0.5, 2, 0.5);
		glutSolidCube(1);
	glPopMatrix();

	
	glPushMatrix();                          //legs
		glColor3f(1, 1, 1);
		glTranslatef(29.5, 1, 0.5);
		glScalef(0.5, 2, 0.5);
		glutSolidCube(1);
	glPopMatrix();

	
	glPushMatrix();                          //legs
		glColor3f(1, 1, 1);
		glTranslatef(29.5, 1, -0.5);
		glScalef(0.5, 2, 0.5);
		glutSolidCube(1);
	glPopMatrix();

	
	glPushMatrix();                          //tail
		glColor3f(1, 1, 1);
		glTranslatef(30, 4, 0);
		glRotatef(theta, 0, 1, 0);
		glRotatef(-90, 0, 0, 1);
		glScalef(0.5, 3, 0.5);
		glutSolidCube(1);
	glPopMatrix();

}

void displayScene()
{

	glPushMatrix();
		glTranslatef(0, 0, 2);
		drawFloor();
		if (toggleBackground) drawBackground();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(-2, 0, 0);
		glScalef(0.7, 0.7, 0.7);
		drawTree();
	glPopMatrix();
	
	glPushMatrix();
		glTranslatef(0, 0, -1);
		glScalef(0.1, 0.1, 0.1);
		drawPanda();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0, 0, 0);
		glScalef(0.1, 0.1, 0.1);
		drawPanda();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0, 0, 1);
		glScalef(0.1, 0.1, 0.1);
		drawPanda();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0, 0, 2);
		glScalef(0.1, 0.1, 0.1);
		drawPanda();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0, 0, 3);
		glScalef(0.1, 0.1, 0.1);
		drawPanda();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0, 0, 4);
		glScalef(0.1, 0.1, 0.1);
		drawPanda();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0, 0, 5);
		glScalef(0.1, 0.1, 0.1);
		drawPanda();
	glPopMatrix();


}

void drawBall() {
	glTranslatef(ball_x, ball_y, ball_z);
	glutSolidSphere(0.1, 20, 20);
}

void displatBall()
{
	glPushMatrix();
		glColor3f(1, 0, 0);
		drawBall();
	glPopMatrix();
}

void displayModel()
{
	glPushMatrix();
	glTranslatef(0, -0.1, 0);
	glScalef(scene_scale, scene_scale, scene_scale);
	glTranslatef(-scene_center.x, -scene_center.y, -scene_center.z);
	glColor3f(0, 0.7, 0.7);
	render(scene->mRootNode);
	glPopMatrix();
}


void displayModelShadow()
{

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(0, 0.05, 0);
	glMultMatrixf(shadowMat);
	glScalef(scene_scale, scene_scale, scene_scale);
	glTranslatef(-scene_center.x, -scene_center.y, -scene_center.z);
	glColor4f(0.1, 0.1, 0.1, 1);
	render(scene->mRootNode);
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

}

void displayBallShadow()
{

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(0, 0.05, 0);
	glMultMatrixf(shadowMat);
	glColor4f(0.1, 0.1, 0.1, 1);
	drawBall();
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

}

//------The main display function---------
void display()
{

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
	glTranslatef(-scene_center.x, -scene_center.y, -scene_center.z);
	glRotatef(view_angle, 0, 1, 0);
	glTranslatef(scene_center.x, scene_center.y, scene_center.z);

	displayScene();
	displayModel();
	displayModelShadow();
	displatBall();
	displayBallShadow();
	glPopMatrix();

	glutSwapBuffers();
}

void updateBall()
{

	if (reset_ball) {
		ball_x = 0.6;
		ball_y = 0.08;
		ball_z = 0.4;

		isKicked = false;
		init_ball = true;

		ball_Vy = ball_Veloctiy * sin(45 * CDR) * cos(45 * CDR);;
	} {
		ball_x += ball_Vx / 1000 * timeStep;
		ball_y += ball_Vy / 1000 * timeStep;
		ball_z += ball_Vz / 1000 * timeStep;

		ball_Vy -= gravity / 1000 * timeStep;
	}

}

//update
void update(int value) {
	if (tick > tDuration) {
		tick = 0;
		reset_ball = true;
	}
	//if (tick <= 0) tick_step = 1;

	if (toggleAnimation) {
		glutTimerFunc(timeStep, update, 0);
	}
	else {
		//cout << tick << endl;
		updateNodeMatrices(tick);
		tick += tick_step;

		if ((left_foot_vec.x * scene_scale) >= ball_x + 0.2 && (left_foot_vec.z * scene_scale) >= ball_z - 0.2) {
			isKicked = true;
			reset_ball = false;

		}

		if (isKicked) {
			updateBall();
		}
		glutTimerFunc(timeStep, update, tick);
	}


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
	//cout << cam_z << endl;
	cam_z += direction * camera_movement_speed;

	if (cam_z <= 2) {
		cam_z = 2;
	}

	if (cam_z >= 20) {
		cam_z = 20;
	}
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


void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case ' ':
		toggleAnimation = !toggleAnimation;
		break;
	case '1':
		toggleBackground = !toggleBackground;
		break;
	case 'b':
		reset_ball = true;
		break;
	}

	glutPostRedisplay();
}



int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Skeleton Animation-gli65");

	initialise();
	glutDisplayFunc(display);
	glutTimerFunc(timeStep, update, 0);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
	aiReleaseImport(scene);
}