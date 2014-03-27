#include <iostream>
#include<stdio.h>
#include <cmath>
#include <string.h>
#include <vector>
#include <ctime>
#include <GL/glut.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <unistd.h>
using namespace std;

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define NUM_BUFFERS 2 // Maximum data buffers we will need.
#define NUM_SOURCES 2 // Maximum emissions we will need.

// Function Declarations
void drawWorld();
void update(int value);
void drawBox(float width,float height);
void drawSpiders();
void drawCannon(float cannon_x);
void drawGreenBasket(float greenbasket_x);
void drawRedBasket(float redbasket_x);
void addLaser();
void fireLaser();
void writeScore(float x, float y, char* str); 
void initRendering();
void handleResize(int w, int h);
void handleKeypress1(unsigned char key, int x, int y);
void handleKeypress2(int key, int x, int y);
void handleMouseclick(int button, int state, int x, int y);
void handleMouseDrag(int x, int y);
float RandomFloat(float a, float b);

// Global Variables
bool paused=false;
bool gameover=false;
time_t t1=0,t2,l1=0,l2;
float width = 6.0f;
float height = 4.0f;
float cannon_x;
float size = 0.20f;
float velocity = 0.1f;
float spider_size = 0.05f;
float redbasket_x;
float greenbasket_x;
float gun_length = 0.6f;
float laser_length;
float gun_angle = 90.0f; 
float theta = 0.0f; 
float baseline;
int selected=0;	//0 - canon, 1 - red basket, 2 - green basket 
int no_spi=0;
int no_lasers=0;
int cannon_rotate=0;
int score=0;	//FINAL SCORE
int windowWidth;
int windowHeight;
float laser_speed=0.10f;
float minSpeed=0.01f;
float maxSpeed=0.02f;
float cannon_leftlimit;
float cannon_rightlimit;
float red_leftlimit;
float red_rightlimit;
float green_rightlimit;
float green_leftlimit;
float small_hline=0.1f;
float side_line=0.3f;
typedef struct spider
{	float spider_x,spider_y,speed,dead;	//COLORS 0 - black, 1 - red, 2 - green 
	int color;
}spider;
typedef struct laser
{	float laser_x,laser_y,laser_angle;
}laser;
vector < spider > spi;
vector < laser > las;

ALuint Buffers[NUM_BUFFERS];	// Buffers hold sound data.
ALuint Sources[NUM_SOURCES];	// Sources are points of emitting sound.
ALfloat SourcesPos[NUM_SOURCES][3];	// Position of the source sounds.
ALfloat SourcesVel[NUM_SOURCES][3];	// Velocity of the source sounds.
ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };	// Position of the listener.
ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };	// Velocity of the listener.
ALfloat ListenerOri[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };	// Orientation of the listener.

ALboolean LoadALData()
{	// Variables to load into.
	ALenum format;
	ALsizei size;
	ALvoid* data;
	ALsizei freq;
	ALboolean loop;
	// Load wav data into buffers.
	alGenBuffers(NUM_BUFFERS, Buffers);

	if(alGetError() != AL_NO_ERROR)	// Do error check
		return AL_FALSE;

	alutLoadWAVFile((ALbyte*)"audio/music.wav", &format, &data, &size, &freq, &loop);
	alBufferData(Buffers[0], format, data, size, freq);
	alutUnloadWAV(format, data, size, freq);

	alutLoadWAVFile((ALbyte*)"audio/laser.wav", &format, &data, &size, &freq, &loop);
	alBufferData(Buffers[1], format, data, size, freq);
	alutUnloadWAV(format, data, size, freq);

	// Bind buffers into audio sources.
	alGenSources(NUM_SOURCES, Sources);
	if(alGetError() != AL_NO_ERROR)	// Do error check
		return AL_FALSE;

	alSourcei (Sources[0], AL_BUFFER,   Buffers[0]   );
	alSourcef (Sources[0], AL_PITCH,    1.0f         );
	alSourcef (Sources[0], AL_GAIN,     1.0f         );
	alSourcefv(Sources[0], AL_POSITION, SourcesPos[0]);
	alSourcefv(Sources[0], AL_VELOCITY, SourcesVel[0]);
	alSourcei (Sources[0], AL_LOOPING,  AL_TRUE      );

	alSourcei (Sources[1], AL_BUFFER,   Buffers[1]   );
	alSourcef (Sources[1], AL_PITCH,    1.0f         );
	alSourcef (Sources[1], AL_GAIN,     1.0f         );
	alSourcefv(Sources[1], AL_POSITION, SourcesPos[1]);
	alSourcefv(Sources[1], AL_VELOCITY, SourcesVel[1]);
	alSourcei (Sources[1], AL_LOOPING,  AL_FALSE     );
	
	if(alGetError() != AL_NO_ERROR)	// Do another error check and return.
		return AL_FALSE;

	return AL_TRUE;
}

void SetListenerValues()	//We already defined certain values for the listener, but we need to tell OpenAL to use that data.
{	alListenerfv(AL_POSITION,    ListenerPos);	
	alListenerfv(AL_VELOCITY,    ListenerVel);
	alListenerfv(AL_ORIENTATION, ListenerOri);
}

void KillALData()  	//We have allocated memory for our buffers and sources which needs 
{	alDeleteBuffers(NUM_BUFFERS, Buffers);	//to be returned to the system. This function frees that memory.
	alDeleteSources(NUM_SOURCES, Sources);
	alutExit();
}

int main(int argc, char **argv) 
{	alutInit(NULL, 0);	// Initialize OpenAL and clear the error bit.
	alGetError();
	if(LoadALData() == AL_FALSE)	// Load the wav data.
		return 0;
	SetListenerValues();
	atexit(KillALData);	// Setup an exit procedure.
	alSourcePlay(Sources[0]);	// Play background Music.
	
	// Initialize GLUT and other variables
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	baseline=-0.4*height;
	laser_length=0.1*height;
	cannon_x=0.0f;
	redbasket_x=2*size-0.5*width;
	greenbasket_x=0.5*width-2*size;
	int w = glutGet(GLUT_SCREEN_WIDTH);
	int h = glutGet(GLUT_SCREEN_HEIGHT);
	windowWidth = w * 2 / 3;
	windowHeight = h * 2 / 3;
	cannon_leftlimit=-width/2;
	cannon_rightlimit=width/2;
	red_leftlimit=-width/2;
	red_rightlimit=width/2;
	green_rightlimit=width/2;
	green_leftlimit=-width/2;

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition((w - windowWidth) / 2, (h - windowHeight) / 2);

	glutCreateWindow("Arachnophobia");  // Setup the window
	initRendering();	// functions we need to call in the start
	// Register callbacks
	glutDisplayFunc(drawWorld);	//asking glut to use the func. to use it draw on screen
	glutIdleFunc(drawWorld);	//use this when nothing is happening on the screen: called as per frame rate
	glutKeyboardFunc(handleKeypress1);	//use when keyboard input
	glutSpecialFunc(handleKeypress2);
	glutMouseFunc(handleMouseclick);	//use with mouse input
	glutMotionFunc(handleMouseDrag);	//use with mouse motion (drag)
	glutReshapeFunc(handleResize);
	glutTimerFunc(10, update, 0);
	
	glutMainLoop();	//goes into infinite loop - each of the above functions called as per needs
	return 0;
}

// Initializing some openGL 3D rendering options
void initRendering() 
{	glEnable(GL_DEPTH_TEST);        // Enable objects to be drawn ahead/behind one another
	glEnable(GL_COLOR_MATERIAL);    // Enable coloring
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);   // Setting a background color
}

// Function called when the window is resized
void handleResize(int w, int h) 
{	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)w / (float)h, 0.1f, 200.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void handleKeypress1(unsigned char key, int x, int y) 
{	if (key == 27 || key == (int)'q') 
	exit(0);     // escape key is pressed
	else if ( key == (int)'b' )
		selected=0;
	else if ( key == (int)'r' )
		selected=1;
	else if ( key == (int)'g' )
		selected=2;
	else if ( key == 32 )
	{	addLaser();
		alSourcePlay(Sources[1]);
	}
	else if ( key == (int)'p' && !gameover)
		paused=!paused;
}

void handleKeypress2(int key, int x, int y) 
{	if(selected==0)	
	{	if (key == GLUT_KEY_LEFT && cannon_x-size-1.5*small_hline>=cannon_leftlimit)
			cannon_x -= velocity;
		if (key == GLUT_KEY_RIGHT && cannon_x+size+1.5*small_hline<=cannon_rightlimit)
			cannon_x += velocity;
		if (key == GLUT_KEY_UP && gun_angle<=160)
			gun_angle +=5;
		if (key == GLUT_KEY_DOWN && gun_angle>=20)
			gun_angle -= 5;
	}
	else if(selected==1)	
	{	if (key == GLUT_KEY_LEFT && redbasket_x-size>=red_leftlimit)
			redbasket_x -= velocity;
		if (key == GLUT_KEY_RIGHT && redbasket_x+size<=red_rightlimit)
			redbasket_x += velocity;
	}
	else if(selected==2)	
	{	if (key == GLUT_KEY_LEFT && greenbasket_x-size>=green_leftlimit)
			greenbasket_x -= velocity;
		if (key == GLUT_KEY_RIGHT && greenbasket_x+size<=green_rightlimit)
			greenbasket_x += velocity;
	}
}

void handleMouseclick(int button, int state, int x, int y) 
{	if(paused)
        	return;
	float x_mouse=((float)x-455)/125;
	float y_mouse=(-(float)y+255)/125;
    	if (state == GLUT_DOWN)
    	{	if (button == GLUT_LEFT_BUTTON)
        	{	cannon_rotate=0;
			if(y_mouse>=-1.8 && y_mouse<=-1.1 && fabs(cannon_x-x_mouse)<0.3)
            			selected=0;
            		else if(y_mouse>=-1.8 && y_mouse<=-1.1 && fabs(x_mouse-redbasket_x)<0.3)
            			selected=1;
            		else if(y_mouse>=-1.8 && y_mouse<=-1.1 && fabs(x_mouse-greenbasket_x)<0.3)
            			selected=2;
            		else
            		{}
		}
		if(button == GLUT_RIGHT_BUTTON)
		{	if(y_mouse>=-1.8 && y_mouse<=-1.1 && fabs(cannon_x-x_mouse)<0.3)
		    		cannon_rotate=1;
		    	else
		        	cannon_rotate=0;
		}
        }   
    	glutPostRedisplay();
}

void handleMouseDrag(int x,int y)
{	if(paused)
        	return;
    	float x_mouse=((float)x-455)/125;
    	float y_mouse=(-(float)y+255)/125;
    	if(selected==1 && fabs(redbasket_x-x_mouse)<=0.3)
	{	if(x_mouse-size>=red_leftlimit && x_mouse+size<=red_rightlimit)
        		redbasket_x=x_mouse;
	}
    	else if(selected==2 && fabs(greenbasket_x-x_mouse)<=0.3)
    	{	if(x_mouse-size>=green_leftlimit && x_mouse+size<=green_rightlimit)
        		greenbasket_x=x_mouse;
	}
    	else if(cannon_rotate==0 && selected==0 && fabs(cannon_x-x_mouse)<=0.3)
    	{	if(x_mouse-size>=cannon_leftlimit && x_mouse+size<=cannon_rightlimit)
        		cannon_x=x_mouse;
	}
    	if(cannon_rotate==1 && fabs(cannon_x-x_mouse)<=0.3)
    	{	if(cannon_x-x_mouse>0)
        	{	if(gun_angle <=160)
                		gun_angle += 5;
		}        	
		else
		{	if(gun_angle >=20)
                		gun_angle -=5;
		}
    	}
}

// Function to draw objects on the screen
void drawWorld() 
{	if(!paused)
	{	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);	
		glLoadIdentity();
		glPushMatrix();
		// Draw Box
		glTranslatef(0.0f, 0.0f, -5.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		drawBox(width,height);
		glPushMatrix();
		drawCannon(cannon_x);
		glPopMatrix();
		drawRedBasket(redbasket_x);
		drawGreenBasket(greenbasket_x);
		drawSpiders();	
		fireLaser();
		glPushMatrix();
		char str[100],go[100],sc[100],qtoe[100];
		glPopMatrix();
		glPushMatrix();
		if(!gameover)
		{	glPushMatrix();	
			glColor3f(0,0,0);
			sprintf(str,"Score = %d",score);
			writeScore(0.55,0.85,str);	//SCORE WRITING	
			glPopMatrix();	
		}		
		else
		{	glPushMatrix();	
			glColor3f(0,0,0);
			sprintf(go,"GAME OVER !!");	//GAME OVER CONDITION
			sprintf(sc,"Score = %d",score);
			sprintf(qtoe,"PRESS Q TO EXIT");
			writeScore(-0.17,0.0,go);
			writeScore(-0.1,-0.1,sc);
			writeScore(-0.2,-0.2,qtoe);
			glPopMatrix();	
			paused=true;		
		}
		glPopMatrix();
		glPopMatrix();
		glutSwapBuffers();
	}
}

void writeScore(float x, float y, char *str) 
{ 	//(x,y) is from the bottom left of the window 
	glMatrixMode(GL_PROJECTION); 
	glPushMatrix(); 
	glLoadIdentity(); 
	glMatrixMode(GL_MODELVIEW); 
	glPushMatrix(); 
	glLoadIdentity(); 
	glPushAttrib(GL_DEPTH_TEST); 
	glDisable(GL_DEPTH_TEST); 
	glRasterPos2f(x,y); 
	glColor3f(0,0,0);
	if(!gameover)
		for (unsigned int i=0; i<strlen(str); i++) 
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]); 
	else
		for (unsigned int i=0; i<strlen(str); i++) 
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]); 
	glPopAttrib(); 
	glMatrixMode(GL_PROJECTION); 
	glPopMatrix(); 
	glMatrixMode(GL_MODELVIEW); 
	glPopMatrix(); 
}

void update(int value) 
{	for(unsigned i=0;i<spi.size();i++)
	{	float x1,y2,x2,y1,lasx,lasy;
		x1=spi[i].spider_x-spider_size*2.125;
		x2=spi[i].spider_x+spider_size*2.125;
		y1=spi[i].spider_y-spider_size*3.461;
		y2=spi[i].spider_y+spider_size*3.461;
		if(spi[i].spider_x>cannon_x-size-small_hline && spi[i].spider_x<cannon_x+size+small_hline && baseline+size>y1)
		{	spi.erase(spi.begin()+i);	
			gameover=true;
		}
		for(unsigned j=0;j<las.size();j++)
		{	lasx=las[j].laser_x+laser_length*cos(DEG2RAD(las[j].laser_angle));
			lasy=las[j].laser_y+laser_length*sin(DEG2RAD(las[j].laser_angle));
			if(lasx>x1 && lasx<x2 && lasy>y1 && lasy<y2)
			{	if(spi[i].color==0)
					score+=1;
				spi.erase(spi.begin()+i);	//HERE CHECK COLOR ALSO FOR SCORING
				las.erase(las.begin()+j);
			}
		}
		//SPIDER COLLIDING WITH RED/GREEN BASKETS - CHECK COLOR FOR SCORING
		if(spi[i].spider_x>redbasket_x-size && spi[i].spider_x<redbasket_x+size && baseline+size>y1)
		{	if(spi[i].color==1)
				score+=1;
			else
				score-=1;
			spi.erase(spi.begin()+i);
		}
		else if(spi[i].spider_x>greenbasket_x-size && spi[i].spider_x<greenbasket_x+size && baseline+size>y1)
		{	if(spi[i].color==2)
				score+=1;
			else
				score-=1;
			spi.erase(spi.begin()+i);
		}
		//condition when baskets in same position - score always +1
		else if(spi[i].spider_x>redbasket_x-size && spi[i].spider_x<redbasket_x+size && spi[i].spider_x>greenbasket_x-size && spi[i].spider_x<greenbasket_x+size && baseline+size>y1)
		{	score+=1;
			spi.erase(spi.begin()+i);
		}
	}
	glutTimerFunc(10, update, 0);	
}

void drawBox(float width,float heigth) 
{	glLineWidth(1);	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);	
	glVertex2f(-width/2, -height/2);	
	glVertex2f(width/2, -height/2);
	glVertex2f(width/2, height/2);
	glVertex2f(-width/2, height/2);
	glEnd();
	glBegin(GL_QUADS);	
	glVertex2f(-width/2, -height/2);
	glVertex2f(width/2, -height/2);
	glVertex2f(width/2, baseline);
	glVertex2f(-width/2, baseline);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

float RandomFloat(float a, float b) 
{	float random = ((float) rand()) / (float) RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

void drawSpiders()
{	if(t1==0)
		t1=time(0)-2;
	t2=time(0);
	if(t2>=t1+2)
	{	spider s;
		s.spider_x=RandomFloat(-width/2,width/2);
		s.spider_y=height/2+1.0f;
		s.speed=RandomFloat(minSpeed,maxSpeed);
		s.color=((int)RandomFloat(0,10))%3;
		s.dead=0;
		spi.push_back(s);
		t1=t2;
	}
	for(unsigned n=0;n<spi.size();n++)
	{	glPushMatrix();
		glTranslatef(spi[n].spider_x, spi[n].spider_y, 0.0f);
		if(spi[n].color==0)
			glColor3f(0.0f, 0.0f, 0.0f);
		else if(spi[n].color==1)
			glColor3f(1.0f, 0.0f, 0.0f);
		else if(spi[n].color==2)
			glColor3f(0.0f, 1.0f, 0.0f);
		//DRAWING SPIDER
		//upper trapezium
		glBegin(GL_QUADS);
		glVertex2f(-spider_size,1.5*spider_size);	
		glVertex2f(spider_size,1.5*spider_size);	
		glVertex2f(0.6*spider_size,2*spider_size);	
		glVertex2f(-0.6*spider_size,2*spider_size);	
		glEnd();
		//main body
		glBegin(GL_QUADS);
		glVertex2f(-spider_size,-1.5*spider_size);	
		glVertex2f(spider_size,-1.5*spider_size);	
		glVertex2f(spider_size,1.5*spider_size);	
		glVertex2f(-spider_size,1.5*spider_size);	
		glEnd();
		//lower trapezium
		glBegin(GL_QUADS);
		glVertex2f(-0.6*spider_size,-2*spider_size);	
		glVertex2f(0.6*spider_size,-2*spider_size);	
		glVertex2f(spider_size,-1.5*spider_size);	
		glVertex2f(-spider_size,-1.5*spider_size);	
		glEnd();
		//middle 8 lines
		glLineWidth(2);
		glBegin(GL_LINES);
		glVertex2f(spider_size,0.0f);	
		glVertex2f(spider_size*1.866,spider_size*0.5);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(spider_size*1.866,spider_size*0.5);	
		glVertex2f(spider_size*2.125,spider_size*1.466);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(spider_size,0.0f);	
		glVertex2f(spider_size*1.866,-spider_size*0.5);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(spider_size*1.866,-spider_size*0.5);	
		glVertex2f(spider_size*2.125,-spider_size*1.466);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(-spider_size,0.0f);	
		glVertex2f(-spider_size*1.866,spider_size*0.5);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(-spider_size*1.866,spider_size*0.5);	
		glVertex2f(-spider_size*2.125,spider_size*1.466);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(-spider_size,0.0f);	
		glVertex2f(-spider_size*1.866,-spider_size*0.5);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(-spider_size*1.866,-spider_size*0.5);	
		glVertex2f(-spider_size*2.125,-spider_size*1.466);	
		glEnd();
		//upper 4 lines
		glBegin(GL_LINES);
		glVertex2f(0.8*spider_size,spider_size*1.4);	
		glVertex2f(spider_size*1.861,spider_size*2.461);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(spider_size*1.861,spider_size*2.461);	
		glVertex2f(spider_size*1.861,spider_size*3.461);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(-0.8*spider_size,spider_size*1.4);	
		glVertex2f(-spider_size*1.861,spider_size*2.461);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(-spider_size*1.861,spider_size*2.461);	
		glVertex2f(-spider_size*1.861,spider_size*3.461);	
		glEnd();
		//lower 4 lines
		glBegin(GL_LINES);
		glVertex2f(0.8*spider_size,-spider_size*1.4);	
		glVertex2f(spider_size*1.861,-spider_size*2.461);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(spider_size*1.861,-spider_size*2.461);	
		glVertex2f(spider_size*1.861,-spider_size*3.461);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(-0.8*spider_size,-spider_size*1.4);	
		glVertex2f(-spider_size*1.861,-spider_size*2.461);	
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(-spider_size*1.861,-spider_size*2.461);	
		glVertex2f(-spider_size*1.861,-spider_size*3.461);	
		glEnd();
		//SPIDER DRAWING FINISHED
		glPopMatrix();
		if(spi[n].spider_y>=baseline)
			spi[n].spider_y-=spi[n].speed;
		else
		{	if(spi[n].spider_x>cannon_x && cannon_rightlimit>spi[n].spider_x)
				cannon_rightlimit=spi[n].spider_x-spider_size*3.125;
			if(spi[n].spider_x<cannon_x && cannon_leftlimit<spi[n].spider_x)
				cannon_leftlimit=spi[n].spider_x+spider_size*3.125;
			if(spi[n].spider_x>redbasket_x && red_rightlimit>spi[n].spider_x)
				red_rightlimit=spi[n].spider_x-spider_size*3.125;
			if(spi[n].spider_x<redbasket_x && red_leftlimit<spi[n].spider_x)
				red_leftlimit=spi[n].spider_x+spider_size*3.125;
			if(spi[n].spider_x>greenbasket_x && green_rightlimit>spi[n].spider_x)
				green_rightlimit=spi[n].spider_x-spider_size*3.125;
			if(spi[n].spider_x<greenbasket_x && green_leftlimit<spi[n].spider_x)
				green_leftlimit=spi[n].spider_x+spider_size*3.125;
			if(!spi[n].dead)	//score -5 when spider on ground		
			{	spi[n].dead=1;
				score-=5;
			}
		}
	}
}

void drawCannon(float cannon_x) 
{	glPushMatrix();
	glTranslatef(cannon_x, baseline, 0.0f);
	glRotatef(gun_angle-90.0f,0.0f,0.0f,1.0f);
	//main box
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
	glBegin(GL_QUADS);	
	glColor3f(0.0f, 0.0f, 1.0f);	
	glVertex2f(-size,-size);	
	glVertex2f(size,-size);
	glVertex2f(size,size);
	glVertex2f(-size,size);
	glEnd();
	//small box on top of main box
	glBegin(GL_QUADS);	
	glColor3f(0.0f, 0.0f, 1.0f);	
	glVertex2f(-size/3,size);	
	glVertex2f(size/3,size);
	glVertex2f(size/3,5*size/3);
	glVertex2f(-size/3,5*size/3);
	glEnd();
	//small horizontal connector on left
	glLineWidth(60);
	glBegin(GL_LINES);	
	glColor3f(0.0f, 0.0f, 1.0f);	
	glVertex2f(-size-small_hline,0.0f);
	glVertex2f(-size,0.0f);
	glEnd();
	//small horizontal connector on right
	glLineWidth(60);
	glBegin(GL_LINES);	
	glColor3f(0.0f, 0.0f, 1.0f);	
	glVertex2f(size,0.0f);
	glVertex2f(size+small_hline,0.0f);	
	glEnd();
	//vertical line on right
	glLineWidth(60);
	glBegin(GL_LINES);	
	glColor3f(0.0f, 0.0f, 1.0f);	
	glVertex2f(size+small_hline,-side_line);
	glVertex2f(size+small_hline,side_line);
	glEnd();
	//vertical line on left
	glLineWidth(60);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);	
	glVertex2f(-size-small_hline,-side_line);
	glVertex2f(-size-small_hline,side_line);
	glEnd();
	glPopMatrix();
	//main gun
	glLineWidth(60);
	glColor3f(0.0f, 0.0f,1.0f);
	glBegin(GL_LINES);
	glVertex2f(cannon_x,baseline);
	glVertex2f(cannon_x+gun_length*cos(DEG2RAD(gun_angle)),baseline+gun_length*sin(DEG2RAD(gun_angle))); 
	glEnd(); 
}

void drawRedBasket(float redbasket_x) 
{	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f);	
	glVertex2f(redbasket_x-size,baseline-size);
	glVertex2f(redbasket_x+size,baseline-size);
	glVertex2f(redbasket_x+size,baseline+size);
	glVertex2f(redbasket_x-size,baseline+size);
	glEnd();
}

void drawGreenBasket(float greenbasket_x) 
{	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
	glBegin(GL_QUADS);	
	glColor3f(0.0f, 1.0f, 0.0f);	
	glVertex2f(greenbasket_x-size,baseline-size);
	glVertex2f(greenbasket_x+size,baseline-size);
	glVertex2f(greenbasket_x+size,baseline+size);
	glVertex2f(greenbasket_x-size,baseline+size);
	glEnd();
}

void addLaser()
{	if(l1==0)
		l1=time(0)-1;
	l2=time(0);
	if(l2>=l1+1)
	{	laser l;
		l.laser_x=cannon_x+gun_length*cos(DEG2RAD(gun_angle));
		l.laser_y=baseline+gun_length*sin(DEG2RAD(gun_angle));
		l.laser_angle=gun_angle;
		no_lasers++;
		las.push_back(l);
		l1=l2;
	}
}
void fireLaser()
{	for(unsigned i=0;i<las.size();i++)
	{	glPushMatrix();
		glColor3f(0.0f, 0.0f, 0.0f);
		glLineWidth(5);
		//LASER REFLECTION CODE
		float end1_x,end1_y,end2_x,pt_x,pt_y;
		end1_x=las[i].laser_x;
		end1_y=las[i].laser_y;
		end2_x=las[i].laser_x+laser_length*cos(DEG2RAD(las[i].laser_angle));
		//left wall - transition state (calculate length & draw 2 lines)
		float abcd= 180-las[i].laser_angle;
		if(end1_x>-width/2 && end2_x<-width/2)
		{	pt_x=-width/2;
			pt_y=tan(DEG2RAD(abcd))*(las[i].laser_x+width/2) + las[i].laser_y;
			glBegin(GL_LINES);	//part 1 of line
			glVertex2f(end1_x,end1_y);
			glVertex2f(pt_x,pt_y);
			glEnd();
			float new_len=laser_length - (pt_y - las[i].laser_y)/(sin(DEG2RAD(las[i].laser_angle)));
			glBegin(GL_LINES);	//part 2 of line
			glVertex2f(pt_x,pt_y);
			glVertex2f(pt_x+new_len*cos(DEG2RAD(abcd)),pt_y+new_len*sin(DEG2RAD(abcd)));
			glEnd();
		}
		//right wall - transition state (calculate length & draw 2 lines)
		else if(end1_x<width/2 && end2_x>width/2)	
		{	pt_x=width/2;
			pt_y=tan(DEG2RAD(las[i].laser_angle))*(width/2 - las[i].laser_x) + las[i].laser_y;
			glBegin(GL_LINES);	//part 1 of line
			glVertex2f(end1_x,end1_y);
			glVertex2f(pt_x,pt_y);
			glEnd();
			float new_len=laser_length - (pt_y - las[i].laser_y)/(sin(DEG2RAD(las[i].laser_angle)));
			glBegin(GL_LINES);	//part 2 of line
			glVertex2f(pt_x,pt_y);
			glVertex2f(pt_x+new_len*cos(DEG2RAD(abcd)),pt_y+new_len*sin(DEG2RAD(abcd)));
			glEnd();
		}
		//left wall - later state		
		else if(end1_x<=-width/2)
		{	pt_x=-width/2;
			pt_y=tan(DEG2RAD(abcd))*(las[i].laser_x+width/2) + las[i].laser_y;
			las[i].laser_x=pt_x;
			las[i].laser_y=pt_y;
			las[i].laser_angle=180-las[i].laser_angle;
			glPushMatrix();
			glTranslatef(las[i].laser_x, las[i].laser_y, 0.0f);
			glBegin(GL_LINES);
			glVertex2f(las[i].laser_x,las[i].laser_y);
			glVertex2f(las[i].laser_x+laser_length*cos(DEG2RAD(las[i].laser_angle)),las[i].laser_y+laser_length*sin(DEG2RAD(las[i].laser_angle))); 
			glEnd();
			glPopMatrix();
		}
		//right wall - later state 	
		else if(end1_x>=width/2)	
		{	pt_x=width/2;
			pt_y=tan(DEG2RAD(las[i].laser_angle))*(width/2 - las[i].laser_x) + las[i].laser_y;
			las[i].laser_x=pt_x;
			las[i].laser_y=pt_y;
			las[i].laser_angle=180-las[i].laser_angle;
			glPushMatrix();
			glTranslatef(las[i].laser_x, las[i].laser_y, 0.0f);
			glBegin(GL_LINES);
			glVertex2f(las[i].laser_x,las[i].laser_y);
			glVertex2f(las[i].laser_x+laser_length*cos(DEG2RAD(las[i].laser_angle)),las[i].laser_y+laser_length*sin(DEG2RAD(las[i].laser_angle))); 
			glEnd();
			glPopMatrix();
		}
		else	//NORMAL
		{	glBegin(GL_LINES);
			glVertex2f(las[i].laser_x,las[i].laser_y);
			glVertex2f(las[i].laser_x+laser_length*cos(DEG2RAD(las[i].laser_angle)),las[i].laser_y+laser_length*sin(DEG2RAD(las[i].laser_angle))); 
			glEnd();
		}
		glPopMatrix();				
		las[i].laser_x+=laser_speed*cos(DEG2RAD(las[i].laser_angle));	
		las[i].laser_y+=laser_speed*sin(DEG2RAD(las[i].laser_angle));
		if(las[i].laser_y>=height/2 || las[i].laser_y <=-height/2)
			las.erase(las.begin()+i);
	}
}
