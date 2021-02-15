
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "graphics.h"
#include "functions.h"
#include "perlin.h"

extern GLubyte world[WORLDX][WORLDY][WORLDZ];

/* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

/* initialize graphics library */
extern void graphicsInit(int *, char **);

/* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat *getLightPosition();

/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setOldViewPosition(float, float, float);
extern void setViewOrientation(float, float, float);
extern void getViewOrientation(float *, float *, float *);

/* add cube to display list so it will be drawn */
extern void addDisplayList(int, int, int);

/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

/* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

/* tube controls */
extern void createTube(int, float, float, float, float, float, float, int);
extern void hideTube(int);
extern void showTube(int);

/* 2D drawing functions */
extern void draw2Dline(int, int, int, int, int);
extern void draw2Dbox(int, int, int, int);
extern void draw2Dtriangle(int, int, int, int, int, int);
extern void set2Dcolour(float[]);

/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
/* flag used to indicate that the test world should be used */
extern int testWorld;
/* flag to print out frames per second */
extern int fps;
/* flag to indicate the space bar has been pressed */
extern int space;
/* flag indicates the program is a client when set = 1 */
extern int netClient;
/* flag indicates the program is a server when set = 1 */
extern int netServer;
/* size of the window in pixels */
extern int screenWidth, screenHeight;
/* flag indicates if map is to be printed */
extern int displayMap;
/* flag indicates use of a fixed viewpoint */
extern int fixedVP;

/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/* allows users to define colours */
extern int setUserColour(int, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat,
                         GLfloat, GLfloat, GLfloat);
void unsetUserColour(int);
extern void getUserColour(int, GLfloat *, GLfloat *, GLfloat *, GLfloat *,
                          GLfloat *, GLfloat *, GLfloat *, GLfloat *);

/********* end of extern variable declarations **************/

/********* self-created functions/Structures **************/

//Structure for the array of doors for each room
typedef struct doorCoords
{
   //The coords of the door hole
   int x;
   int y;
   int z;
   //Extension coords of the door to hallways
   int hallwayBeginningX;
   int hallwayBeginningY;
   int hallwayBeginningZ;
} doorCoords;

typedef struct blockCoords
{
   //The coords of the random block in each room
   int x;
   int y;
   int z;
} blockCoords;

typedef struct stairCoords
{
   //The coords of the random block in each room
   int x;
   int y;
   int z;
   //The coords of the location to teleport the view point too
   //int travelToX;
   //int travelToY;
   //int travelToZ;
} stairCoords;

//Global structure to store all the information about level
typedef struct Room
{
   //Starting coords of the room anchor
   int startingX;
   int startingY;
   int startingZ;
   //The dimensions of the room
   int xLength;
   int yLength;
   int zLength;

   //I changed the way implemented storage so I think this is not needed
   //==================================
   //Colours
   int wallColour;      //6
   int floorShade;      //69
   int floorColour;     //4
   int cubeColour;      //3
   int upStairColour;   //1
   int downStairColour; //1
   //Height of hallway
   int hallwayHeight;
   //==================================

   //The coords of the stairs
   //stairCoords stairs[1];
   //The coords of the random blocks
   blockCoords randBlocks[2];
   //The coords of the coords
   doorCoords Doors[4];

} Room;

Room storage[9];

typedef struct savedViewPoint
{
   float x;
   float y;
   float z;
} savedViewPoint;

typedef struct worldLevels
{
   int savedWorld[100][50][100];
   stairCoords stairGoingUp;
   stairCoords stairGoingDown;
   //savedViewPoint playersLastPointInWorld;
   //Room level[9];

} worldLevels;

worldLevels database[2];

void initWorld();
void showWorldGrid();
void generateDungeonLevel(worldLevels database[2]);

//Helper functions to generateDungeonLevel - builds vertical bridges
void buildHallwayFromRoom0ToRoom3(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom1ToRoom4(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom2ToRoom5(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom3ToRoom6(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom4ToRoom7(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom5ToRoom8(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);

//Helper functions to generateDungeonLevel - builds horizontal bridges
void buildHallwayFromRoom0ToRoom1(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom1ToRoom2(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom3ToRoom4(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom4ToRoom5(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom6ToRoom7(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);
void buildHallwayFromRoom7ToRoom8(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade);

//Helper functions
void setPlayerSpawnLocation();
float inverseValue(float valueToInverse);
bool checkAreaAroundViewPoint(float x, float y, float z);

//A2 functions
void generateOutsideLevel(worldLevels database[2]);

void goDownStairs(worldLevels database[2], float playerX, float playerY, float playerZ);
void goUpStairs(worldLevels database[2], float playerX, float playerY, float playerZ);

void saveCurrentWorld(int saveWorldToThis[100][50][100]);
void regenerateWorld(int regenerateThisWorld[100][50][100]);

void spawnBesideBlock(worldLevels database[2]);

/*** collisionResponse() ***/
/* -performs collision detection and response */
/*  sets new xyz  to position of the viewpoint after collision */
/* -can also be used to implement gravity by updating y position of vp*/
/* note that the world coordinates returned from getViewPosition()
	   will be the negative value of the array indices */
void collisionResponse()
{
   //Declaration of variables
   float viewOffset = 0.2;
   bool isWall;
   bool isCube;
   bool check;
   float newX;
   float newY;
   float newZ;
   float oldX;
   float oldY;
   float oldZ;
   float viewX;
   float viewY;
   float viewZ;
   float temptX;
   float temptY;
   float temptZ;

   if (flycontrol == 0)
   {
      //Get the view orientation everytime the keyboard is pressed
      getViewOrientation(&viewX, &viewY, &viewZ);

      //Gets the position infront of the player
      getViewPosition(&temptX, &temptY, &temptZ);

      //Gets the position of the player
      getOldViewPosition(&oldX, &oldY, &oldZ);

      newX = inverseValue(temptX);
      newY = inverseValue(temptY);
      newZ = inverseValue(temptZ);

      isWall = checkAreaAroundViewPoint(newX, newY, newZ);
      if (isWall)
      {
         isCube = checkAreaAroundViewPoint(newX, newY + 1, newZ);

         if (isCube == false)
         {
            if (world[(int)newX][(int)newY][(int)newZ] == 13)
            {
               //Moves the view point "1 cube" higher
               setViewPosition((-1 * newX), (-1 * (newY + 2.0)), (-1 * newZ));

               initWorld();
               generateDungeonLevel(database);
               saveCurrentWorld(database[1].savedWorld);
               //regenerateWorld(database[1].savedWorld);
               //Go downstairs
               goDownStairs(database, (-1 * newX), (-1 * newY), (-1 * newZ));
            }
            else if (world[(int)newX][(int)newY][(int)newZ] == 5)
            {
               //Moves the view point "1 cube" higher
               setViewPosition((-1 * newX), (-1 * (newY + 2.0)), (-1 * newZ));

               initWorld();
               //generateDungeonLevel(database);
               regenerateWorld(database[0].savedWorld);
               //Go upstairs
               goUpStairs(database, (-1 * newX), (-1 * newY), (-1 * newZ));
            }
            else
            {
               //Moves the view point "1 cube" higher
               setViewPosition((-1 * newX), (-1 * (newY + 2.0)), (-1 * newZ));
            }
         }
         else
         {
            //Implement wall sliding
            //Literally looking straight forward any direction
            if (fmod(viewY, 90.0) == 0.0)
            {
               setViewPosition(oldX, oldY, oldZ);
            }
            else
            {
               //Looking at all other possible angles, angles literally get bigger if the screen is bigger so it becomes harder and I couldn't think of a way to do it with math
               if (viewY > 270.0 && viewY < 360.0 || viewY > 630.0 && viewY < 720.0 || viewY > 990.0 && viewY < 1080.0 || viewY > 1350.0 && viewY < 1440.0 || viewY > 1710.0 && viewY < 1800.0 || viewY > 2070.0 && viewY < 2160.0)
               {
                  //Looking Q1
                  temptX = newX;
                  temptY = inverseValue(oldY);
                  temptZ = inverseValue(oldZ);
                  check = checkAreaAroundViewPoint(temptX, temptY, temptZ);

                  if (check == false)
                  {
                     //Move towards direction
                     setViewPosition((-1 * newX), oldY, oldZ);
                  }
                  else
                  {
                     temptX = inverseValue(oldX);
                     temptY = inverseValue(oldY);
                     temptZ = newZ;
                     check = checkAreaAroundViewPoint(temptX, temptY, temptZ);
                     if (check == false)
                     {
                        //Move towards other direction
                        setViewPosition(oldX, oldY, (-1 * newZ));
                     }
                     else
                     {
                        //Stop
                        setViewPosition(oldX, oldY, oldZ);
                     }
                  }
               }

               if (viewY > 0.0 && viewY < 90.0 || viewY > 360.0 && viewY < 450.0 || viewY > 720.0 && viewY < 910.0 || viewY > 1080.0 && viewY < 1170 || viewY > 1440.0 && viewY < 1530.0 || viewY > 1800.0 && viewY < 1890.0)
               {
                  //Looking Q2
                  temptX = newX;
                  temptY = inverseValue(oldY);
                  temptZ = inverseValue(oldZ);
                  check = checkAreaAroundViewPoint(temptX, temptY, temptZ);
                  if (check == false)
                  {
                     //Move towards direction
                     setViewPosition((-1 * newX), oldY, oldZ);
                  }
                  else
                  {
                     temptX = inverseValue(oldX);
                     temptY = inverseValue(oldY);
                     temptZ = newZ;
                     check = checkAreaAroundViewPoint(temptX, temptY, temptZ);
                     if (check == false)
                     {
                        //Move towards other direction
                        setViewPosition(oldX, oldY, (-1 * newZ));
                     }
                     else
                     {
                        //Stop
                        setViewPosition(oldX, oldY, oldZ);
                     }
                  }
               }

               if (viewY > 180.0 && viewY < 270.0 || viewY > 540.0 && viewY < 630.0 || viewY > 900.0 && viewY < 990.0 || viewY > 1260.0 && viewY < 1350.0 || viewY > 1620.0 && viewY < 1720.0 || viewY > 1980.0 && viewY < 2070.0)
               {
                  //Looking Q3
                  temptX = inverseValue(oldX);
                  temptY = inverseValue(oldY);
                  temptZ = newZ;
                  check = checkAreaAroundViewPoint(temptX, temptY, temptZ);
                  if (check == false)
                  {
                     //Move towards direction
                     setViewPosition(oldX, oldY, (-1 * newZ));
                  }
                  else
                  {
                     temptX = newX;
                     temptY = inverseValue(oldY);
                     temptZ = inverseValue(oldZ);
                     check = checkAreaAroundViewPoint(temptX, temptY, temptZ);
                     if (check == false)
                     {
                        //Move towards other direction
                        setViewPosition((-1 * newX), oldY, oldZ);
                     }
                     else
                     {
                        //Stop
                        setViewPosition(oldX, oldY, oldZ);
                     }
                  }
               }

               if (viewY > 90.0 && viewY < 180.0 || viewY > 450.0 && viewY < 540.0 || viewY > 810.0 && viewY < 900.0 || viewY > 1170.0 && viewY < 1260.0 || viewY > 1530.0 && viewY < 1620.0 || viewY > 1890.0 && viewY < 1980.0)
               {
                  //Looking Q4
                  temptX = inverseValue(oldX);
                  temptY = inverseValue(oldY);
                  temptZ = newZ;
                  check = checkAreaAroundViewPoint(temptX, temptY, temptZ);
                  if (check == false)
                  {
                     //Move towards direction
                     setViewPosition(oldX, oldY, (-1 * newZ));
                  }
                  else
                  {
                     temptX = newX;
                     temptY = inverseValue(oldY);
                     temptZ = inverseValue(oldZ);
                     check = checkAreaAroundViewPoint(temptX, temptY, temptZ);
                     if (check == false)
                     {
                        //Move towards other direction
                        setViewPosition((-1 * newX), oldY, oldZ);
                     }
                     else
                     {
                        //Stop
                        setViewPosition(oldX, oldY, oldZ);
                     }
                  }
               }
            }
         }
      }
   }
}

//Helper function used in collision response to tests the area around the view point for extra acuracy
bool checkAreaAroundViewPoint(float x, float y, float z)
{
   //Could be cleaner, but rounding to the float isnt needed.
   if (world[(int)x][(int)y][(int)z] != 0)
   {
      return true;
   }
   else if (world[(int)(x + 0.2)][(int)y][(int)(z - 0.2)] != 0)
   {
      return true;
   }
   else if (world[(int)(x - 0.2)][(int)y][(int)(z + 0.2)] != 0)
   {
      return true;
   }
   else if (world[(int)(x - 0.2)][(int)y][(int)(z - 0.2)] != 0)
   {
      return true;
   }
   else if (world[(int)(x + 0.2)][(int)y][(int)(z + 0.2)] != 0)
   {
      return true;
   }
   return false;
}

//Helper function to invert float values from negative to positive and vice versa
float inverseValue(float valueToInverse)
{
   float inverse = -1.0;
   valueToInverse = (valueToInverse * inverse);
   return valueToInverse;
}

/******* draw2D() *******/
/* draws 2D shapes on screen */
/* use the following functions: 			*/
/*	draw2Dline(int, int, int, int, int);		*/
/*	draw2Dbox(int, int, int, int);			*/
/*	draw2Dtriangle(int, int, int, int, int, int);	*/
/*	set2Dcolour(float []); 				*/
/* colour must be set before other functions are called	*/
void draw2D()
{

   if (testWorld)
   {
      /* draw some sample 2d shapes */
      if (displayMap == 1)
      {
         GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
         set2Dcolour(green);
         draw2Dline(0, 0, 500, 500, 15);
         draw2Dtriangle(0, 0, 200, 200, 0, 200);

         GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
         set2Dcolour(black);
         draw2Dbox(500, 380, 524, 388);
      }
   }
   else
   {

      /* your code goes here */
   }
}

int cloud_check = 1;
int runOnce = 1;
int cloudCounter = 0;

/*** update() ***/
/* background process, it is called when there are no other events */
/* -used to control animations and perform calculations while the  */
/*  system is running */
/* -gravity must also implemented here, duplicate collisionResponse */
void update()
{
   int i, j, k;
   float *la;
   float x, y, z;

   /* sample animation for the testworld, don't remove this code */
   /* demo of animating mobs */
   if (testWorld)
   {

      /* update old position so it contains the correct value */
      /* -otherwise view position is only correct after a key is */
      /*  pressed and keyboard() executes. */
#if 0
// Fire a ray in the direction of forward motion
float xx, yy, zz;
getViewPosition(&x, &y, &z);
getOldViewPosition(&xx, &yy, &zz);
printf("%f %f %f %f %f %f\n", xx, yy, zz, x, y, z);
printf("%f %f %f\n",  -xx+((x-xx)*25.0), -yy+((y-yy)*25.0), -zz+((z-zz)*25.0));
createTube(2, -xx, -yy, -zz, -xx-((x-xx)*25.0), -yy-((y-yy)*25.0), -zz-((z-zz)*25.0), 5);
#endif

      getViewPosition(&x, &y, &z);
      setOldViewPosition(x, y, z);

      /* sample of rotation and positioning of mob */
      /* coordinates for mob 0 */
      static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
      static float mob0ry = 0.0;
      static int increasingmob0 = 1;
      /* coordinates for mob 1 */
      static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
      static float mob1ry = 0.0;
      static int increasingmob1 = 1;
      /* counter for user defined colour changes */
      static int colourCount = 0;
      static GLfloat offset = 0.0;

      /* move mob 0 and rotate */
      /* set mob 0 position */
      setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

      /* move mob 0 in the x axis */
      if (increasingmob0 == 1)
         mob0x += 0.2;
      else
         mob0x -= 0.2;
      if (mob0x > 50)
         increasingmob0 = 0;
      if (mob0x < 30)
         increasingmob0 = 1;

      /* rotate mob 0 around the y axis */
      mob0ry += 1.0;
      if (mob0ry > 360.0)
         mob0ry -= 360.0;

      /* move mob 1 and rotate */
      setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

      /* move mob 1 in the z axis */
      /* when mob is moving away it is visible, when moving back it */
      /* is hidden */
      if (increasingmob1 == 1)
      {
         mob1z += 0.2;
         showMob(1);
      }
      else
      {
         mob1z -= 0.2;
         hideMob(1);
      }
      if (mob1z > 72)
         increasingmob1 = 0;
      if (mob1z < 52)
         increasingmob1 = 1;

      /* rotate mob 1 around the y axis */
      mob1ry += 1.0;
      if (mob1ry > 360.0)
         mob1ry -= 360.0;

      /* change user defined colour over time */
      if (colourCount == 1)
         offset += 0.05;
      else
         offset -= 0.01;
      if (offset >= 0.5)
         colourCount = 0;
      if (offset <= 0.0)
         colourCount = 1;
      setUserColour(9, 0.7, 0.3 + offset, 0.7, 1.0, 0.3, 0.15 + offset, 0.3, 1.0);

      /* sample tube creation  */
      /* draws a purple tube above the other sample objects */
      createTube(1, 45.0, 30.0, 45.0, 50.0, 30.0, 50.0, 6);

      /* end testworld animation */
   }
   else
   {
      /* your code goes here */
      float newX;
      float newY;
      float newZ;
      float temptX;
      float temptY;
      float temptZ;
      int convertedX;
      int convertedY;
      int convertedZ;

      int check = 0;

      //Gravity
      if (flycontrol == 0)
      {
         getViewPosition(&newX, &newY, &newZ);

         temptX = newX;
         temptY = newY;
         temptZ = newZ;

         temptX = inverseValue(temptX);
         temptY = inverseValue(temptY);
         temptZ = inverseValue(temptZ);

         convertedX = (int)temptX;
         convertedY = (int)temptY;
         convertedZ = (int)temptZ;

         if (world[convertedX][convertedY - 1][convertedZ] == 0)
         {
            //Constantly accelerates the character down to the ground
            setViewPosition(newX, newY + 0.3, newZ);
         }

         if (world[convertedX][convertedY - 1][convertedZ] == 5)
         {
            initWorld();
            regenerateWorld(database[0].savedWorld);
            goUpStairs(database, (-1 * newX), (-1 * newY), (-1 * newZ));
         }

         if (world[convertedX][convertedY - 1][convertedZ] == 13)
         {
            //Constantly accelerates the character down to the ground
            setViewPosition(newX, newY + 0.3, newZ);

            for (int i = 0; i < WORLDX; i++)
            {
               for (int j = 0; j < WORLDY; j++)
               {
                  for (int k = 0; k < WORLDZ; k++)
                  {

                     if (database[1].savedWorld[i][j][k] != 0)
                     {
                        check = 1;
                        break;
                     }
                     else
                     {
                        check = 0;
                     }
                  }
               }
            }

            if (check == 0)
            {
               //Generate dungeon
               initWorld();
               generateDungeonLevel(database);
               //Saved Dungeon
               saveCurrentWorld(database[1].savedWorld);
            }
            else
            {
               initWorld();
               regenerateWorld(database[1].savedWorld);
            }
            //regenerateWorld(database[1].savedWorld);
            //Go downstairs
            goDownStairs(database, (-1 * newX), (-1 * newY), (-1 * newZ));
         }
      }

      float tempt;
      int cloud_y = 0;
      int cloudArray1[100];
      int cloudArray2[100];
      int cloudArray3[100];
      //int cloudCounter = 0;
      float speed = 0;

      //Clouds
      /*
      if (cloud_check == 1)
      {
         if (runOnce == 1)
         {
            runOnce = 0;

            for (int i = 0; i < WORLDX; i++)
            {
               for (int j = 0; j < WORLDZ; j++)
               {
                  tempt = perlin2d(i, j, 0.05, 4);
                  tempt = (tempt * 20)+40;
                  cloud_y = (int)round(tempt);

                  if (cloudCounter < 100 && cloud_y > 50 && cloud_y < 100)
                  {
                     cloudArray1[cloudCounter] = i;
                     cloudArray2[cloudCounter] = cloud_y;
                     cloudArray3[cloudCounter] = j;
                     cloudCounter++;
                  }
               }
            }

         }
         speed += 0.05;
         for (int k = 0; j < cloudCounter; k++)
         {
            float tempt1 = (cloudArray1[k] + speed);
            int tempt2 = (int)fmod(tempt1, 100);

            float tempt3 = ((cloudArray1[k] + speed) - 1);
            int tempt4 = (int)fmod(tempt3, 100);

            world[tempt2][cloudArray2[k]][cloudArray3[k]] = 10;
            world[tempt4][cloudArray2[k]][cloudArray3[k]] = 0;
         }

         if (speed >= 100)
         {
            speed = 0;
         }
      }
      */
   }
}

/* called by GLUT when a mouse button is pressed or released */
/* -button indicates which button was pressed or released */
/* -state indicates a button down or button up event */
/* -x,y are the screen coordinates when the mouse is pressed or */
/*  released */
void mouse(int button, int state, int x, int y)
{

   if (button == GLUT_LEFT_BUTTON)
      printf("left button - ");
   else if (button == GLUT_MIDDLE_BUTTON)
      printf("middle button - ");
   else
      printf("right button - ");

   if (state == GLUT_UP)
      printf("up - ");
   else
      printf("down - ");

   printf("%d %d\n", x, y);
}

int main(int argc, char **argv)
{
   int i, j, k;
   /* initialize the graphics system */
   graphicsInit(&argc, argv);

   /* the first part of this if statement builds a sample */
   /* world which will be used for testing */
   /* DO NOT remove this code. */
   /* Put your code in the else statment below */
   /* The testworld is only guaranteed to work with a world of
		with dimensions of 100,50,100. */
   if (testWorld == 1)
   {
      /* initialize world to empty */
      for (i = 0; i < WORLDX; i++)
         for (j = 0; j < WORLDY; j++)
            for (k = 0; k < WORLDZ; k++)
               world[i][j][k] = 0;

      /* some sample objects */
      /* build a red platform */
      for (i = 0; i < WORLDX; i++)
      {
         for (j = 0; j < WORLDZ; j++)
         {
            world[i][24][j] = 3;
         }
      }
      /* create some green and blue cubes */
      world[50][25][50] = 1;
      world[49][25][50] = 1;
      world[49][26][50] = 1;
      world[52][25][52] = 2;
      world[52][26][52] = 2;

      /* create user defined colour and draw cube */
      setUserColour(9, 0.7, 0.3, 0.7, 1.0, 0.3, 0.15, 0.3, 1.0);
      world[54][25][50] = 9;

      /* blue box shows xy bounds of the world */
      for (i = 0; i < WORLDX - 1; i++)
      {
         world[i][25][0] = 2;
         world[i][25][WORLDZ - 1] = 2;
      }
      for (i = 0; i < WORLDZ - 1; i++)
      {
         world[0][25][i] = 2;
         world[WORLDX - 1][25][i] = 2;
      }

      /* create two sample mobs */
      /* these are animated in the update() function */
      createMob(0, 50.0, 25.0, 52.0, 0.0);
      createMob(1, 50.0, 25.0, 52.0, 0.0);

      /* create sample player */
      createPlayer(0, 52.0, 27.0, 52.0, 0.0);
   }
   else
   {
      /* your code to build the world goes here */
      int WORLD_Y = 24;
      //Pinkish white colour = 69
      //0.909, 0.470, 0.737
      setUserColour(69, 0.909, 0.470, 0.737, 1.0, 0.909, 0.470, 0.737, 1.0);
      //White colour = 10
      //0.752, 0.725, 0.725
      setUserColour(10, 0.752, 0.725, 0.725, 1.0, 0.752, 0.725, 0.725, 1.0);
      //Green colour = 11
      //0.078, 0.180, 0.086
      setUserColour(11, 0.078, 0.180, 0.086, 1.0, 0.078, 0.180, 0.086, 1.0);
      //Brown colour = 12
      //0.250, 0.109, 0.109
      setUserColour(12, 0.250, 0.109, 0.109, 1.0, 0.250, 0.109, 0.109, 1.0);
      //Gray colour = 13
      //0.160, 0.160, 0.160
      setUserColour(13, 0.160, 0.160, 0.160, 1.0, 0.160, 0.160, 0.160, 1.0);

      initWorld();
      //showWorldGrid();
      //generateDungeonLevel(database[0]);

      //REMEBER TO UNCOMMENT THESE / UPDATE THEM
      //setPlayerSpawnLocation();
      //flycontrol = 0;

      //THESE ARE TEST LINES
      //saveCurrentWorld(database[0].savedWorld);
      //initWorld();
      //regenerateWorld(database[0].savedWorld);

      generateOutsideLevel(database);
      saveCurrentWorld(database[0].savedWorld);
      spawnBesideBlock(database);
      flycontrol = 0;
      //initWorld();

      //generateDungeonLevel(database);
      //saveCurrentWorld(database[1].savedWorld);
      //initWorld();

      //regenerateWorld(database[0].savedWorld);

      /*
      printf("0 up X: %d Y: %d Z: %d \n", database[0].stairGoingUp.x, database[0].stairGoingUp.y, database[0].stairGoingUp.z);
      printf("0 down X: %d Y: %d Z: %d \n", database[0].stairGoingDown.x, database[0].stairGoingDown.y, database[0].stairGoingDown.z);

      printf("1 up X: %d Y: %d Z: %d \n", database[1].stairGoingUp.x, database[1].stairGoingUp.y, database[1].stairGoingUp.z);
      printf("1 down X: %d Y: %d Z: %d \n", database[1].stairGoingDown.x, database[1].stairGoingDown.y, database[1].stairGoingDown.z);
      */
   }

   /* starts the graphics processing loop */
   /* code after this will not run until the program exits */
   glutMainLoop();
   return 0;
}

void initWorld()
{
   int i, j, k;
   /* initialize world to empty */
   for (i = 0; i < WORLDX; i++)
   {
      for (j = 0; j < WORLDY; j++)
      {
         for (k = 0; k < WORLDZ; k++)
         {
            world[i][j][k] = 0;
         }
      }
   }

   /* some sample objects */
   /* build a red platform */

   /*
   for (i = 0; i < WORLDX; i++)
   {
      for (j = 0; j < WORLDZ; j++)
      {
         world[i][WORLD_Y-1][j] = 3;
      }
   }
   */
}

void showWorldGrid()
{
   int i, j;

   //Create 3x3 grid
   //Show 3x3 grid marks
   for (i = 0; i < 100; i++)
   {
      //X
      world[0][23][i] = 2;
      world[33][23][i] = 2;
      world[66][23][i] = 2;
      world[99][23][i] = 2;
   }

   for (j = 0; j < 100; j++)
   {
      //Z
      world[j][23][0] = 1;
      world[j][23][33] = 1;
      world[j][23][66] = 1;
      world[j][23][99] = 1;
   }

   world[0][25][0] = 4;
   world[99][25][99] = 8;
}

void generateDungeonLevel(worldLevels database[2])
{
   //This function generate one single level, can be called more than once. (Made heading into A2)

   /* Notes: Max room size dimensions are 33x2x33 (X-Y-Z) inclusive
    * Minimum X: 5 units high 
    * Minimum Y: two units high 
    * Minimum Z: 5 units high 
    * 
    * Max X: 28 units high 
    * Max Y: N/A
    * Max Z: 28 units high 
    */

   //Choose starting point in grid
   /* Ranges for grid
    * 0x24x0 - 33x24x33
    * Min X and Z: 3 units
    * Max X and Z: 15 units
    */

   //Level in the world where the dungeon should be generated at
   int WORLD_Y = 24;

   int i, j, k;
   int minStart = 6;
   int maxStart = 15;
   int incrementalValue = 33; //This is the size of each square across the grid

   //Colour of blocks
   int wallColour = 6;
   int floorShade = 69;
   int floorColour = 4;
   int cubeColour = 3;

   //These need to be different colours later
   int upStairColour = 1;
   int downStairColour = 1;

   //Choose room dimensions
   //This the max and min lengths of rooms
   int maxDim = 15; //15 is max
   int minDim = 9;  //9 is min

   //This is the universal offset of the wall for the doors
   int offsetFromWall = 2;
   int beginningHallLength = 3;

   /* This is the reasion why offsetFromWall exists
    *
    *     |d|
    *     |d|
    *     |d|
    * |/|1|2|3|4|/|
    * |/|       |/|
    * |/|       |/|
    * 
    * |2| = represents the ammount of blocks away from the walls the doors are
    * |d| = door hallway, also represents the the number of blocks for the beginning hallway length
    * |/| = wall 
    */

   //This is the values for rooms heights
   int setHallWayWallHeight = 5; // 3 units is the actual space between the floor and the ceiling since the walls are 5 high (3 = 5-2)
   int setRoomHeight = 10;       // 8 units is the actual space of the set heights of the room since the room height is 10 high (8 = 10-2)

   //This is maybe for the future if have time for it
   int minRoomHeight = 5; // Implement rng room height later...
   int maxRoomHeight = 11;

   //Put constant variables inside the storage
   for (i = 0; i < 9; i++)
   {
      storage[i].wallColour = wallColour;
      storage[i].floorShade = floorShade;
      storage[i].floorColour = floorColour;
      storage[i].cubeColour = cubeColour;
      storage[i].upStairColour = upStairColour;
      storage[i].downStairColour = downStairColour;
      storage[i].hallwayHeight = setHallWayWallHeight;
      storage[i].yLength = setRoomHeight;
   }

   srand(time(NULL)); //Include this at the beginning of using rand()

   //Generate the 9 starting positions of the rooms in the grid
   for (i = 0; i < 9; i++)
   {
      int startRandX = (rand() % (maxStart - minStart + 1)) + minStart;
      int startRandZ = (rand() % (maxStart - minStart + 1)) + minStart;

      /* Grid order ID // ID = index of the struct array
       * 0, 1, 2
       * 3, 4, 5
       * 6, 7, 8
       * 
       * Also, the ID's represents the rooms and order they are in the struct array for storage system
       */

      //Set the Y for the storage
      storage[i].startingY = WORLD_Y;

      //set the X for the storage
      if (i == 0 || i == 3 || i == 6)
      {
         storage[i].startingX = startRandX;
      }
      else if (i == 1 || i == 4 || i == 7)
      {
         storage[i].startingX = startRandX + incrementalValue;
      }
      else if (i == 2 || i == 5 || i == 8)
      {
         storage[i].startingX = startRandX + incrementalValue + incrementalValue;
      }

      //Set the Z for the storage
      if (i == 0 || i == 1 || i == 2)
      {
         storage[i].startingZ = startRandZ;
      }
      else if (i == 3 || i == 4 || i == 5)
      {
         storage[i].startingZ = startRandZ + incrementalValue;
      }
      else if (i == 6 || i == 7 || i == 8)
      {
         storage[i].startingZ = startRandZ + incrementalValue + incrementalValue;
      }

      //THIS IS A TEST LINE TO PRINT OUT THE ROOM ANCHORS
      world[storage[i].startingX][WORLD_Y + setRoomHeight][storage[i].startingZ] = floorColour;
      world[storage[i].startingX][WORLD_Y + setRoomHeight + 1][storage[i].startingZ] = floorShade;
   }

   //Create the random room sizes
   for (i = 0; i < 9; i++)
   {
      int xDim = (rand() % (maxDim - minDim + 1)) + minDim;
      int zDim = (rand() % (maxDim - minDim + 1)) + minDim;
      //Setting the room dimensions into the storage system
      storage[i].xLength = xDim;
      storage[i].zLength = zDim;
   }

   for (i = 0; i < 9; i++)
   {
      int randCubeX = (rand() % ((storage[i].xLength - offsetFromWall) - (offsetFromWall + 1) + 1)) + (offsetFromWall + 1);
      int randCubeZ = (rand() % ((storage[i].zLength - offsetFromWall) - (offsetFromWall + 1) + 1)) + (offsetFromWall + 1);
      int randCubeX2 = (rand() % ((storage[i].xLength - offsetFromWall) - (offsetFromWall + 1) + 1)) + (offsetFromWall + 1);
      int randCubeZ2 = (rand() % ((storage[i].zLength - offsetFromWall) - (offsetFromWall + 1) + 1)) + (offsetFromWall + 1);

      //This loop is for the 3x3 grid
      for (j = 0; j < storage[i].xLength; j++) //This loop is for X
      {
         for (k = 0; k < storage[i].zLength; k++) //This loop is for Z
         {
            //Colour randomizer
            int randColour = (rand() % ((2 - 1) + 1)) + 1;
            //This generates the area of the room
            if (randColour == 1)
            {
               world[storage[i].startingX + j][WORLD_Y][storage[i].startingZ + k] = floorShade;
            }
            else
            {
               world[storage[i].startingX + j][WORLD_Y][storage[i].startingZ + k] = floorColour;
            }
            for (int wallHeight = 1; wallHeight < setRoomHeight; wallHeight++)
            {
               world[storage[i].startingX + j][WORLD_Y + wallHeight][storage[i].startingZ + k] = wallColour;
            }
         }
      }

      //These loops removes the inside of the room and leaves only the walls and floors
      for (j = 1; j < storage[i].xLength - 1; j++)
      {
         //This loop is for X
         for (k = 1; k < storage[i].zLength - 1; k++)
         {
            //This loop is for Z

            //This generates the area of the room
            for (int wallHeight = 1; wallHeight < setRoomHeight - 1; wallHeight++)
            {
               world[storage[i].startingX + j][WORLD_Y + wallHeight][storage[i].startingZ + k] = 0;
            }
         }
      }
      //Puts a one or two cubes in each room depends on rng if there is 1 cube or two
      world[storage[i].startingX + randCubeX][WORLD_Y + 1][storage[i].startingZ + randCubeZ] = cubeColour;
      world[storage[i].startingX + randCubeX2][WORLD_Y + 1][storage[i].startingZ + randCubeZ2] = cubeColour;

      storage[i].randBlocks[0].x = storage[i].startingX + randCubeX;
      storage[i].randBlocks[0].y = WORLD_Y + 1;
      storage[i].randBlocks[0].z = storage[i].startingZ + randCubeZ;

      storage[i].randBlocks[1].x = storage[i].startingX + randCubeX2;
      storage[i].randBlocks[1].y = WORLD_Y + 1;
      storage[i].randBlocks[1].z = storage[i].startingZ + randCubeZ2;
   }

   //Literally the dumbest but FULLY ERROR catching way to spawn the stairs without it EVER being missing
   int chooseRoomForStairs = (rand() % 9);
   int stairX = (rand() % ((storage[chooseRoomForStairs].xLength - offsetFromWall) - (offsetFromWall + 1) + 1)) + (offsetFromWall + 1);
   int stairZ = (rand() % ((storage[chooseRoomForStairs].zLength - offsetFromWall) - (offsetFromWall + 1) + 1)) + (offsetFromWall + 1);

   //printf("DOES THIS RUN??\n");

   if (world[storage[chooseRoomForStairs].startingX + stairX][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }
   else if (world[storage[chooseRoomForStairs].startingX + stairX + 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX + 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX + 1;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }
   else if (world[storage[chooseRoomForStairs].startingX + stairX][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ + 1] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ + 1] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ + 1;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }
   else if (world[storage[chooseRoomForStairs].startingX + stairX - 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX - 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX - 1;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }
   else if (world[storage[chooseRoomForStairs].startingX + stairX][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ - 1] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ - 1] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ - 1;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }
   else if (world[storage[chooseRoomForStairs].startingX + stairX - 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ - 1] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX - 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ - 1] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX - 1;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ - 1;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }
   else if (world[storage[chooseRoomForStairs].startingX + stairX + 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ + 1] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX + 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ + 1] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX + 1;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ + 1;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }
   else if (world[storage[chooseRoomForStairs].startingX + stairX - 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ + 1] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX - 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ + 1] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX - 1;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ + 1;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }
   else if (world[storage[chooseRoomForStairs].startingX + stairX + 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ - 1] == 0)
   {
      world[storage[chooseRoomForStairs].startingX + stairX + 1][WORLD_Y + 1][storage[chooseRoomForStairs].startingZ + stairZ - 1] = 5;

      database[1].stairGoingUp.x = storage[chooseRoomForStairs].startingX + stairX + 1;
      database[1].stairGoingUp.y = WORLD_Y + 1;
      database[1].stairGoingUp.z = storage[chooseRoomForStairs].startingZ + stairZ - 1;

      database[1].stairGoingDown.x = -1;
      database[1].stairGoingDown.y = -1;
      database[1].stairGoingDown.z = -1;
   }

   //THIS IS A TEST BLOCK FOR A2 PLEASE DONT MIND IT :((
   //=====================================================================================================================================================================================

   /*
   for (i = 0; i < 9; i++)
   {

      if (i == 0)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX, storage[i].startingZ);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }
      else if (i == 1)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX - incrementalValue, storage[i].startingZ);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }
      else if (i == 2)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX - incrementalValue - incrementalValue, storage[i].startingZ);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }
      else if (i == 3)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX, storage[i].startingZ - incrementalValue);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }
      else if (i == 4)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX - incrementalValue, storage[i].startingZ - incrementalValue);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }
      else if (i == 5)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX - incrementalValue - incrementalValue, storage[i].startingZ - incrementalValue);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }
      else if (i == 6)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX, storage[i].startingZ - incrementalValue - incrementalValue);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }
      else if (i == 7)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX - incrementalValue, storage[i].startingZ - incrementalValue - incrementalValue);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }
      else if (i == 8)
      {
         printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX - incrementalValue - incrementalValue, storage[i].startingZ - incrementalValue - incrementalValue);
         printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
      }

      //Prints out the starting positions and x and z ranges
      //printf("Room %d, X:%d - Z:%d\n", i, storage[i].startingX, storage[i].startingZ);
      //printf("Room %d, xLength:%d - zLength:%d\n\n", i, storage[i].xLength, storage[i].zLength);
   }
   */
   //=====================================================================================================================================================================================

   //rng formula
   //int rng = (rand() % (max - min + 1)) + min;

   for (i = 0; i < 9; i++)
   {
      int randX = (rand() % ((storage[i].xLength - offsetFromWall) - (offsetFromWall) + 1)) + (offsetFromWall);
      int randZ = (rand() % ((storage[i].zLength - offsetFromWall) - (offsetFromWall) + 1)) + (offsetFromWall);
      int randX2 = (rand() % ((storage[i].xLength - offsetFromWall) - (offsetFromWall) + 1)) + (offsetFromWall);
      int randZ2 = (rand() % ((storage[i].zLength - offsetFromWall) - (offsetFromWall) + 1)) + (offsetFromWall);

      /* Grid order ID // ID = index of the struct array
       * 0, 1, 2
       * 3, 4, 5
       * 6, 7, 8
       */

      /* Important note: 
       * All the "Door" if statements literally all do the same thing, I just couldn't think of a way to make it into a single function YET
       * because the all the doors per room literally have a single value change the is mandatory depending on where it is facing when generated
       * into the world. 
       * 
       * I will try to spend time when doing A2 to make this into a function, for now everything works. Sorry for the messy code since its in one block
       * 
       * P.S: All wall building, floor building, ceiling building for hallways are all done modularly where if you comment out a single "build wall" line of code, it that wall would disappear.
       *      I chose to do it this way to easy make changes and control the values in change wall heights, width, colours, and etc
       */

      //Rooms with only 2 doors
      if (i == 0)
      {
         for (j = 0; j < 2; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + storage[i].zLength - 1;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positive Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z + k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z + beginningHallLength;

               //Testing the new coordinate of the door
               //world[storage[i].Doors[j].x][storage[i].Doors[j].y + 5][storage[i].Doors[j].z] = 4;
            }
            else
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX + storage[i].xLength - 1;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positive X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //This is a test line
               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x + beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;

               //Testing the new coordinate of the door
               //world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z] = 4;
            }
         }
      }
      else if (i == 2)
      {
         for (j = 0; j < 2; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + storage[i].zLength - 1;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positive Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z + k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z + beginningHallLength;
            }
            else
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x - beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
         }
      }
      else if (i == 6)
      {
         for (j = 0; j < 2; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z - k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z - beginningHallLength;
            }
            else
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX + storage[i].xLength - 1;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positive X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x + beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
         }
      }
      else if (i == 8)
      {
         for (j = 0; j < 2; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z - k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z - beginningHallLength;
            }
            else
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x - beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
         }
      }

      //Rooms with only 3 doors
      if (i == 1)
      {
         for (j = 0; j < 3; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + storage[i].zLength - 1;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positve Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z + k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z + beginningHallLength;
            }
            else if (j == 1)
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX + storage[i].xLength - 1;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positve X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x + beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
            else
            {
               //Door 3 - Z2
               storage[i].Doors[j].x = storage[i].startingX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ2;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x - beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
         }
      }
      else if (i == 3)
      {
         for (j = 0; j < 3; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z - k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z - beginningHallLength;
            }
            else if (j == 1)
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX + storage[i].xLength - 1;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - postive X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x + beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
            else
            {
               //Door 3 - X2
               storage[i].Doors[j].x = storage[i].startingX + randX2;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + storage[i].zLength - 1;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - postive Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z + k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z + beginningHallLength;
            }
         }
      }
      else if (i == 5)
      {
         for (j = 0; j < 3; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z - k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z - beginningHallLength;
            }
            else if (j == 1)
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x - beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
            else
            {
               //Door 3 - X2
               storage[i].Doors[j].x = storage[i].startingX + randX2;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + storage[i].zLength - 1;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positive Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z + k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z + beginningHallLength;
            }
         }
      }
      else if (i == 7)
      {
         for (j = 0; j < 3; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z - k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z - beginningHallLength;
            }
            else if (j == 1)
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX + storage[i].xLength - 1;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - postive X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x + beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
            else
            {
               //Door 3 - Z2
               storage[i].Doors[j].x = storage[i].startingX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ2;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x - beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
         }
      }

      //Rooms with only 4 doors
      if (i == 4)
      {
         for (j = 0; j < 4; j++)
         {
            if (j == 0)
            {
               //Door 1 - X
               storage[i].Doors[j].x = storage[i].startingX + randX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z - k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z - k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z - beginningHallLength;
            }
            else if (j == 1)
            {
               //Door 2 - Z
               storage[i].Doors[j].x = storage[i].startingX;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - negative X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x - k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x - beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
            else if (j == 2)
            {
               //Door 3 - X2
               storage[i].Doors[j].x = storage[i].startingX + randX2;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + storage[i].zLength - 1;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positive Z direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x][storage[i].Doors[j].y][storage[i].Doors[j].z + k] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z + k] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x - 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                     world[storage[i].Doors[j].x + 1][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + k] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z + beginningHallLength;
            }
            else
            {
               //Door 4 - Z2
               storage[i].Doors[j].x = storage[i].startingX + storage[i].xLength - 1;
               storage[i].Doors[j].y = WORLD_Y;
               storage[i].Doors[j].z = storage[i].startingZ + randZ2;

               //printf("Room %d Door %d: X: %d Y: %d Z: %d\n\n", i, j + 1, storage[i].Doors[j].x, storage[i].Doors[j].y, storage[i].Doors[j].z);

               //Need a line to destroy the wall at the door's location
               for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
               {
                  world[storage[i].Doors[j].x][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z] = 0;
               }

               //Start adding the hallway path - positive X direction

               for (k = 0; k <= beginningHallLength; k++)
               {
                  //Colour randomizer
                  int randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorShade;
                  }
                  else
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y][storage[i].Doors[j].z] = floorColour;
                  }

                  //Make the ceiling of the hallway
                  world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + setHallWayWallHeight - 1][storage[i].Doors[j].z] = wallColour;

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z - 1] = wallColour;
                     world[storage[i].Doors[j].x + k][storage[i].Doors[j].y + wallHeight][storage[i].Doors[j].z + 1] = wallColour;
                  }
               }

               //Adds the beginningHallLength to the X/Z coordinate of the door to move the coordinate to the tip of the hallway
               storage[i].Doors[j].hallwayBeginningX = storage[i].Doors[j].x + beginningHallLength;
               storage[i].Doors[j].hallwayBeginningY = storage[i].Doors[j].y;
               storage[i].Doors[j].hallwayBeginningZ = storage[i].Doors[j].z;
            }
         }
      }
   }

   /* Generating Hallways
    *
    * These call specific function to generate the hallways between the doors of certain rooms.
    * The hallways built from which room to which room is mentioned in the function names
    * 
    * Literally the function similar to the door random generation that was in a single block of code you saw above. These functions are all the same but
    * are just specific to each room singel value changes. I decided to make these into function so I can specifiy / generate the hallways procedurally.
    * 
    * The hallways generates the vertical hallways first then generates the horizontal hallways second
    */

   //Builds vertical bridges/hallways
   buildHallwayFromRoom0ToRoom3(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom1ToRoom4(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom2ToRoom5(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom3ToRoom6(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom4ToRoom7(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom5ToRoom8(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);

   //Build horizontal bridges/hallways
   buildHallwayFromRoom0ToRoom1(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom1ToRoom2(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom3ToRoom4(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom4ToRoom5(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom6ToRoom7(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
   buildHallwayFromRoom7ToRoom8(storage, floorColour, wallColour, setHallWayWallHeight, floorShade);
}

//THIS FUCKING WORKS //Helper functions for all 12 bridges/hallways
void buildHallwayFromRoom0ToRoom3(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room0 to Room3
   //Detection of a path across from the door
   for (int check1 = storage[0].Doors[0].hallwayBeginningZ + 1; check1 < storage[3].Doors[0].hallwayBeginningZ + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 3
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[3].startingX; check2 < storage[3].startingX + storage[3].xLength; check2++) //This should be the range of the starting X and xLength of the connecting door which is the door of Room 3
      {
         //This is a test line
         //world[check2][storage[0].Doors[0].hallwayBeginningY + 6][check1] = 4;

         if (world[check2][storage[0].Doors[0].hallwayBeginningY][check1] != 0)
         {
            int stopX = check2; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[0].Doors[0].hallwayBeginningY;
            int stopZ = check1;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[0].Doors[0].hallwayBeginningX - 1][storage[0].Doors[0].hallwayBeginningY + wallHeight][storage[0].Doors[0].hallwayBeginningZ + 1] = wallColour;
               world[storage[0].Doors[0].hallwayBeginningX + 1][storage[0].Doors[0].hallwayBeginningY + wallHeight][storage[0].Doors[0].hallwayBeginningZ + 1] = wallColour;
               world[storage[0].Doors[0].hallwayBeginningX - 1][storage[0].Doors[0].hallwayBeginningY + wallHeight][storage[0].Doors[0].hallwayBeginningZ + 2] = wallColour;
               world[storage[0].Doors[0].hallwayBeginningX + 1][storage[0].Doors[0].hallwayBeginningY + wallHeight][storage[0].Doors[0].hallwayBeginningZ + 2] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[0].Doors[0].hallwayBeginningX < fixedStopX)
            {
               //Rightside case
               //build postive x
               for (int build = storage[0].Doors[0].hallwayBeginningX; build <= fixedStopX; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else if (storage[0].Doors[0].hallwayBeginningX > fixedStopX)
            {
               //Leftside case
               //build negative x
               for (int build = storage[0].Doors[0].hallwayBeginningX; build >= fixedStopX; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[0].Doors[0].hallwayBeginningX][storage[0].Doors[0].hallwayBeginningY][storage[0].Doors[0].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[0].Doors[0].hallwayBeginningX][storage[0].Doors[0].hallwayBeginningY + 1 + wallHeight][storage[0].Doors[0].hallwayBeginningZ] = 0;

               //This is a test line
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[0].Doors[0].hallwayBeginningX][storage[0].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[0].Doors[0].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[0].Doors[0].hallwayBeginningZ = storage[0].Doors[0].hallwayBeginningZ + 1;
      //world[storage[0].Doors[0].hallwayBeginningX][storage[0].Doors[0].hallwayBeginningY][storage[0].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[0].Doors[0].hallwayBeginningX][storage[0].Doors[0].hallwayBeginningY][storage[0].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[0].Doors[0].hallwayBeginningX][storage[0].Doors[0].hallwayBeginningY][storage[0].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[0].Doors[0].hallwayBeginningX][storage[0].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[0].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[0].Doors[0].hallwayBeginningX - 1][storage[0].Doors[0].hallwayBeginningY + wallHeight][storage[0].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[0].Doors[0].hallwayBeginningX + 1][storage[0].Doors[0].hallwayBeginningY + wallHeight][storage[0].Doors[0].hallwayBeginningZ] = wallColour;

         //world[storage[0].Doors[0].hallwayBeginningX][storage[0].Doors[0].hallwayBeginningY + 1 + wallHeight][storage[0].Doors[0].hallwayBeginningZ] = 0;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[3].Doors[0].hallwayBeginningZ = storage[3].Doors[0].hallwayBeginningZ - 1;
      //world[storage[3].Doors[0].hallwayBeginningX][storage[3].Doors[0].hallwayBeginningY][storage[3].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[3].Doors[0].hallwayBeginningX][storage[3].Doors[0].hallwayBeginningY][storage[3].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[3].Doors[0].hallwayBeginningX][storage[3].Doors[0].hallwayBeginningY][storage[3].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[3].Doors[0].hallwayBeginningX][storage[3].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[3].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[3].Doors[0].hallwayBeginningX - 1][storage[3].Doors[0].hallwayBeginningY + wallHeight][storage[3].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[3].Doors[0].hallwayBeginningX + 1][storage[3].Doors[0].hallwayBeginningY + wallHeight][storage[3].Doors[0].hallwayBeginningZ] = wallColour;

         //world[storage[3].Doors[0].hallwayBeginningX][storage[3].Doors[0].hallwayBeginningY + 1 + wallHeight][storage[3].Doors[0].hallwayBeginningZ] = 0;
      }
   }
}

void buildHallwayFromRoom1ToRoom4(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room1 to Room4
   //Detection of a path across from the door
   for (int check1 = storage[1].Doors[0].hallwayBeginningZ + 1; check1 < storage[4].Doors[0].hallwayBeginningZ + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 4
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[4].startingX; check2 < storage[4].startingX + storage[4].xLength; check2++) //This should be the range of the starting X and xLength of the connecting door which is the door of Room 4
      {
         //This is a test line
         //world[check2][storage[0].Doors[0].hallwayBeginningY + 6][check1] = 4;

         if (world[check2][storage[1].Doors[0].hallwayBeginningY][check1] != 0)
         {
            int stopX = check2; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[1].Doors[0].hallwayBeginningY;
            int stopZ = check1;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[1].Doors[0].hallwayBeginningX - 1][storage[1].Doors[0].hallwayBeginningY + wallHeight][storage[1].Doors[0].hallwayBeginningZ + 1] = wallColour;
               world[storage[1].Doors[0].hallwayBeginningX + 1][storage[1].Doors[0].hallwayBeginningY + wallHeight][storage[1].Doors[0].hallwayBeginningZ + 1] = wallColour;
               world[storage[1].Doors[0].hallwayBeginningX - 1][storage[1].Doors[0].hallwayBeginningY + wallHeight][storage[1].Doors[0].hallwayBeginningZ + 2] = wallColour;
               world[storage[1].Doors[0].hallwayBeginningX + 1][storage[1].Doors[0].hallwayBeginningY + wallHeight][storage[1].Doors[0].hallwayBeginningZ + 2] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[1].Doors[0].hallwayBeginningX < fixedStopX)
            {
               //Rightside case
               //build postive x
               for (int build = storage[1].Doors[0].hallwayBeginningX; build <= fixedStopX; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde

                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else if (storage[1].Doors[0].hallwayBeginningX > fixedStopX)
            {
               //Leftside case
               //build negative x
               for (int build = storage[1].Doors[0].hallwayBeginningX; build >= fixedStopX; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[1].Doors[0].hallwayBeginningX][storage[1].Doors[0].hallwayBeginningY][storage[1].Doors[0].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[1].Doors[0].hallwayBeginningX][storage[1].Doors[0].hallwayBeginningY + 1 + wallHeight][storage[1].Doors[0].hallwayBeginningZ] = 0;

               //This is a test line
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[1].Doors[0].hallwayBeginningX][storage[1].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[1].Doors[0].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[1].Doors[0].hallwayBeginningZ = storage[1].Doors[0].hallwayBeginningZ + 1;
      //world[storage[1].Doors[0].hallwayBeginningX][storage[1].Doors[0].hallwayBeginningY][storage[1].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[1].Doors[0].hallwayBeginningX][storage[1].Doors[0].hallwayBeginningY][storage[1].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[1].Doors[0].hallwayBeginningX][storage[1].Doors[0].hallwayBeginningY][storage[1].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[1].Doors[0].hallwayBeginningX][storage[1].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[1].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[1].Doors[0].hallwayBeginningX - 1][storage[1].Doors[0].hallwayBeginningY + wallHeight][storage[1].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[1].Doors[0].hallwayBeginningX + 1][storage[1].Doors[0].hallwayBeginningY + wallHeight][storage[1].Doors[0].hallwayBeginningZ] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[4].Doors[0].hallwayBeginningZ = storage[4].Doors[0].hallwayBeginningZ - 1;
      //world[storage[4].Doors[0].hallwayBeginningX][storage[4].Doors[0].hallwayBeginningY][storage[4].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[4].Doors[0].hallwayBeginningX][storage[4].Doors[0].hallwayBeginningY][storage[4].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[4].Doors[0].hallwayBeginningX][storage[4].Doors[0].hallwayBeginningY][storage[4].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[4].Doors[0].hallwayBeginningX][storage[4].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[4].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[4].Doors[0].hallwayBeginningX - 1][storage[4].Doors[0].hallwayBeginningY + wallHeight][storage[4].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[4].Doors[0].hallwayBeginningX + 1][storage[4].Doors[0].hallwayBeginningY + wallHeight][storage[4].Doors[0].hallwayBeginningZ] = wallColour;
      }
   }
}

void buildHallwayFromRoom2ToRoom5(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room2 to Room5
   //Detection of a path across from the door
   for (int check1 = storage[2].Doors[0].hallwayBeginningZ + 1; check1 < storage[5].Doors[0].hallwayBeginningZ + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 5
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[5].startingX; check2 < storage[5].startingX + storage[5].xLength; check2++) //This should be the range of the starting X and xLength of the connecting door which is the door of Room 5
      {
         //This is a test line
         //world[check2][storage[0].Doors[0].hallwayBeginningY + 6][check1] = 4;

         if (world[check2][storage[2].Doors[0].hallwayBeginningY][check1] != 0)
         {
            int stopX = check2; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[2].Doors[0].hallwayBeginningY;
            int stopZ = check1;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[2].Doors[0].hallwayBeginningX - 1][storage[2].Doors[0].hallwayBeginningY + wallHeight][storage[2].Doors[0].hallwayBeginningZ + 1] = wallColour;
               world[storage[2].Doors[0].hallwayBeginningX + 1][storage[2].Doors[0].hallwayBeginningY + wallHeight][storage[2].Doors[0].hallwayBeginningZ + 1] = wallColour;
               world[storage[2].Doors[0].hallwayBeginningX - 1][storage[2].Doors[0].hallwayBeginningY + wallHeight][storage[2].Doors[0].hallwayBeginningZ + 2] = wallColour;
               world[storage[2].Doors[0].hallwayBeginningX + 1][storage[2].Doors[0].hallwayBeginningY + wallHeight][storage[2].Doors[0].hallwayBeginningZ + 2] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[2].Doors[0].hallwayBeginningX < fixedStopX)
            {
               //Rightside case
               //build postive x
               for (int build = storage[2].Doors[0].hallwayBeginningX; build <= fixedStopX; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde

                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else if (storage[2].Doors[0].hallwayBeginningX > fixedStopX)
            {
               //Leftside case
               //build negative x
               for (int build = storage[2].Doors[0].hallwayBeginningX; build >= fixedStopX; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[2].Doors[0].hallwayBeginningX][storage[2].Doors[0].hallwayBeginningY][storage[2].Doors[0].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[2].Doors[0].hallwayBeginningX][storage[2].Doors[0].hallwayBeginningY + 1 + wallHeight][storage[2].Doors[0].hallwayBeginningZ] = 0;

               //This is a test line
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[2].Doors[0].hallwayBeginningX][storage[2].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[2].Doors[0].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[2].Doors[0].hallwayBeginningZ = storage[2].Doors[0].hallwayBeginningZ + 1;
      //world[storage[2].Doors[0].hallwayBeginningX][storage[2].Doors[0].hallwayBeginningY][storage[2].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[2].Doors[0].hallwayBeginningX][storage[2].Doors[0].hallwayBeginningY][storage[2].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[2].Doors[0].hallwayBeginningX][storage[2].Doors[0].hallwayBeginningY][storage[2].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[2].Doors[0].hallwayBeginningX][storage[2].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[2].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[2].Doors[0].hallwayBeginningX - 1][storage[2].Doors[0].hallwayBeginningY + wallHeight][storage[2].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[2].Doors[0].hallwayBeginningX + 1][storage[2].Doors[0].hallwayBeginningY + wallHeight][storage[2].Doors[0].hallwayBeginningZ] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[5].Doors[0].hallwayBeginningZ = storage[5].Doors[0].hallwayBeginningZ - 1;
      //world[storage[5].Doors[0].hallwayBeginningX][storage[5].Doors[0].hallwayBeginningY][storage[5].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[5].Doors[0].hallwayBeginningX][storage[5].Doors[0].hallwayBeginningY][storage[5].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[5].Doors[0].hallwayBeginningX][storage[5].Doors[0].hallwayBeginningY][storage[5].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[5].Doors[0].hallwayBeginningX][storage[5].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[5].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[5].Doors[0].hallwayBeginningX - 1][storage[5].Doors[0].hallwayBeginningY + wallHeight][storage[5].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[5].Doors[0].hallwayBeginningX + 1][storage[5].Doors[0].hallwayBeginningY + wallHeight][storage[5].Doors[0].hallwayBeginningZ] = wallColour;
      }
   }
}

void buildHallwayFromRoom3ToRoom6(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room3 to Room6
   //Detection of a path across from the door
   for (int check1 = storage[3].Doors[2].hallwayBeginningZ + 1; check1 < storage[6].Doors[0].hallwayBeginningZ + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 6
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[6].startingX; check2 < storage[6].startingX + storage[6].xLength; check2++) //This should be the range of the starting X and xLength of the connecting door which is the door of Room 6
      {
         //This is a test line
         //world[check2][storage[0].Doors[0].hallwayBeginningY + 6][check1] = 4;

         if (world[check2][storage[3].Doors[2].hallwayBeginningY][check1] != 0)
         {
            int stopX = check2; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[3].Doors[2].hallwayBeginningY;
            int stopZ = check1;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[3].Doors[2].hallwayBeginningX - 1][storage[3].Doors[2].hallwayBeginningY + wallHeight][storage[3].Doors[2].hallwayBeginningZ + 1] = wallColour;
               world[storage[3].Doors[2].hallwayBeginningX + 1][storage[3].Doors[2].hallwayBeginningY + wallHeight][storage[3].Doors[2].hallwayBeginningZ + 1] = wallColour;
               world[storage[3].Doors[2].hallwayBeginningX - 1][storage[3].Doors[2].hallwayBeginningY + wallHeight][storage[3].Doors[2].hallwayBeginningZ + 2] = wallColour;
               world[storage[3].Doors[2].hallwayBeginningX + 1][storage[3].Doors[2].hallwayBeginningY + wallHeight][storage[3].Doors[2].hallwayBeginningZ + 2] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[3].Doors[2].hallwayBeginningX < fixedStopX)
            {
               //Rightside case
               //build postive x
               for (int build = storage[3].Doors[2].hallwayBeginningX; build <= fixedStopX; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde

                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else if (storage[3].Doors[2].hallwayBeginningX > fixedStopX)
            {
               //Leftside case
               //build negative x
               for (int build = storage[3].Doors[2].hallwayBeginningX; build >= fixedStopX; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[3].Doors[2].hallwayBeginningX][storage[3].Doors[2].hallwayBeginningY][storage[3].Doors[2].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[3].Doors[2].hallwayBeginningX][storage[3].Doors[2].hallwayBeginningY + 1 + wallHeight][storage[3].Doors[2].hallwayBeginningZ] = 0;

               //This is a test line
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[3].Doors[2].hallwayBeginningX][storage[3].Doors[2].hallwayBeginningY + setHallWayWallHeight - 1][storage[3].Doors[2].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[3].Doors[2].hallwayBeginningZ = storage[3].Doors[2].hallwayBeginningZ + 1;
      //world[storage[3].Doors[2].hallwayBeginningX][storage[3].Doors[2].hallwayBeginningY][storage[3].Doors[2].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[3].Doors[2].hallwayBeginningX][storage[3].Doors[2].hallwayBeginningY][storage[3].Doors[2].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[3].Doors[2].hallwayBeginningX][storage[3].Doors[2].hallwayBeginningY][storage[3].Doors[2].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[3].Doors[2].hallwayBeginningX][storage[3].Doors[2].hallwayBeginningY + setHallWayWallHeight - 1][storage[3].Doors[2].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[3].Doors[2].hallwayBeginningX - 1][storage[3].Doors[2].hallwayBeginningY + wallHeight][storage[3].Doors[2].hallwayBeginningZ] = wallColour;
         world[storage[3].Doors[2].hallwayBeginningX + 1][storage[3].Doors[2].hallwayBeginningY + wallHeight][storage[3].Doors[2].hallwayBeginningZ] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[6].Doors[0].hallwayBeginningZ = storage[6].Doors[0].hallwayBeginningZ - 1;
      //world[storage[6].Doors[0].hallwayBeginningX][storage[6].Doors[0].hallwayBeginningY][storage[6].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[6].Doors[0].hallwayBeginningX][storage[6].Doors[0].hallwayBeginningY][storage[6].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[6].Doors[0].hallwayBeginningX][storage[6].Doors[0].hallwayBeginningY][storage[6].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[6].Doors[0].hallwayBeginningX][storage[6].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[6].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[6].Doors[0].hallwayBeginningX - 1][storage[6].Doors[0].hallwayBeginningY + wallHeight][storage[6].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[6].Doors[0].hallwayBeginningX + 1][storage[6].Doors[0].hallwayBeginningY + wallHeight][storage[6].Doors[0].hallwayBeginningZ] = wallColour;
      }
   }
}

void buildHallwayFromRoom4ToRoom7(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room4 to Room7
   //Detection of a path across from the door
   for (int check1 = storage[4].Doors[2].hallwayBeginningZ + 1; check1 < storage[7].Doors[0].hallwayBeginningZ + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 3
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[7].startingX; check2 < storage[7].startingX + storage[7].xLength; check2++) //This should be the range of the starting X and xLength of the connecting door which is the door of Room 3
      {
         //This is a test line
         //world[check2][storage[0].Doors[0].hallwayBeginningY + 6][check1] = 4;

         if (world[check2][storage[4].Doors[2].hallwayBeginningY][check1] != 0)
         {
            int stopX = check2; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[4].Doors[2].hallwayBeginningY;
            int stopZ = check1;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[4].Doors[2].hallwayBeginningX - 1][storage[4].Doors[2].hallwayBeginningY + wallHeight][storage[4].Doors[2].hallwayBeginningZ + 1] = wallColour;
               world[storage[4].Doors[2].hallwayBeginningX + 1][storage[4].Doors[2].hallwayBeginningY + wallHeight][storage[4].Doors[2].hallwayBeginningZ + 1] = wallColour;
               world[storage[4].Doors[2].hallwayBeginningX - 1][storage[4].Doors[2].hallwayBeginningY + wallHeight][storage[4].Doors[2].hallwayBeginningZ + 2] = wallColour;
               world[storage[4].Doors[2].hallwayBeginningX + 1][storage[4].Doors[2].hallwayBeginningY + wallHeight][storage[4].Doors[2].hallwayBeginningZ + 2] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[4].Doors[2].hallwayBeginningX < fixedStopX)
            {
               //Rightside case
               //build postive x
               for (int build = storage[4].Doors[2].hallwayBeginningX; build <= fixedStopX; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde

                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else if (storage[4].Doors[2].hallwayBeginningX > fixedStopX)
            {
               //Leftside case
               //build negative x
               for (int build = storage[4].Doors[2].hallwayBeginningX; build >= fixedStopX; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[4].Doors[2].hallwayBeginningX][storage[4].Doors[2].hallwayBeginningY][storage[4].Doors[2].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[4].Doors[2].hallwayBeginningX][storage[4].Doors[2].hallwayBeginningY + 1 + wallHeight][storage[4].Doors[2].hallwayBeginningZ] = 0;

               //This is a test line
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[4].Doors[2].hallwayBeginningX][storage[4].Doors[2].hallwayBeginningY + setHallWayWallHeight - 1][storage[4].Doors[2].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[4].Doors[2].hallwayBeginningZ = storage[4].Doors[2].hallwayBeginningZ + 1;
      //world[storage[4].Doors[2].hallwayBeginningX][storage[4].Doors[2].hallwayBeginningY][storage[4].Doors[2].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[4].Doors[2].hallwayBeginningX][storage[4].Doors[2].hallwayBeginningY][storage[4].Doors[2].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[4].Doors[2].hallwayBeginningX][storage[4].Doors[2].hallwayBeginningY][storage[4].Doors[2].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[4].Doors[2].hallwayBeginningX][storage[4].Doors[2].hallwayBeginningY + setHallWayWallHeight - 1][storage[4].Doors[2].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[4].Doors[2].hallwayBeginningX - 1][storage[4].Doors[2].hallwayBeginningY + wallHeight][storage[4].Doors[2].hallwayBeginningZ] = wallColour;
         world[storage[4].Doors[2].hallwayBeginningX + 1][storage[4].Doors[2].hallwayBeginningY + wallHeight][storage[4].Doors[2].hallwayBeginningZ] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[7].Doors[0].hallwayBeginningZ = storage[7].Doors[0].hallwayBeginningZ - 1;
      //world[storage[7].Doors[0].hallwayBeginningX][storage[7].Doors[0].hallwayBeginningY][storage[7].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[7].Doors[0].hallwayBeginningX][storage[7].Doors[0].hallwayBeginningY][storage[7].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[7].Doors[0].hallwayBeginningX][storage[7].Doors[0].hallwayBeginningY][storage[7].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[7].Doors[0].hallwayBeginningX][storage[7].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[7].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[7].Doors[0].hallwayBeginningX - 1][storage[7].Doors[0].hallwayBeginningY + wallHeight][storage[7].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[7].Doors[0].hallwayBeginningX + 1][storage[7].Doors[0].hallwayBeginningY + wallHeight][storage[7].Doors[0].hallwayBeginningZ] = wallColour;
      }
   }
}

void buildHallwayFromRoom5ToRoom8(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room5 to Room8
   //Detection of a path across from the door
   for (int check1 = storage[5].Doors[2].hallwayBeginningZ + 1; check1 < storage[8].Doors[0].hallwayBeginningZ + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 3
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[8].startingX; check2 < storage[8].startingX + storage[8].xLength; check2++) //This should be the range of the starting X and xLength of the connecting door which is the door of Room 8
      {
         //This is a test line
         //world[check2][storage[0].Doors[0].hallwayBeginningY + 6][check1] = 4;

         if (world[check2][storage[5].Doors[2].hallwayBeginningY][check1] != 0)
         {
            int stopX = check2; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[5].Doors[2].hallwayBeginningY;
            int stopZ = check1;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[5].Doors[2].hallwayBeginningX - 1][storage[5].Doors[2].hallwayBeginningY + wallHeight][storage[5].Doors[2].hallwayBeginningZ + 1] = wallColour;
               world[storage[5].Doors[2].hallwayBeginningX + 1][storage[5].Doors[2].hallwayBeginningY + wallHeight][storage[5].Doors[2].hallwayBeginningZ + 1] = wallColour;
               world[storage[5].Doors[2].hallwayBeginningX - 1][storage[5].Doors[2].hallwayBeginningY + wallHeight][storage[5].Doors[2].hallwayBeginningZ + 2] = wallColour;
               world[storage[5].Doors[2].hallwayBeginningX + 1][storage[5].Doors[2].hallwayBeginningY + wallHeight][storage[5].Doors[2].hallwayBeginningZ + 2] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
               world[fixedStopX + 1][fixedStopY + wallHeight][fixedStopZ - 2] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[5].Doors[2].hallwayBeginningX < fixedStopX)
            {
               //Rightside case
               //build postive x
               for (int build = storage[5].Doors[2].hallwayBeginningX; build <= fixedStopX; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde

                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else if (storage[5].Doors[2].hallwayBeginningX > fixedStopX)
            {
               //Leftside case
               //build negative x
               for (int build = storage[5].Doors[2].hallwayBeginningX; build >= fixedStopX; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[build][stopY][stopZ] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[build][stopY][stopZ] = floorShade;
                  }
                  else
                  {
                     world[build][stopY][stopZ] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[build][stopY + wallHeight][stopZ - 1] = wallColour;
                     world[build][stopY + wallHeight][stopZ + 1] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[build][stopY + 1 + wallHeight][stopZ] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[build][stopY + setHallWayWallHeight - 1][stopZ] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[5].Doors[2].hallwayBeginningX][storage[5].Doors[2].hallwayBeginningY][storage[5].Doors[2].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[5].Doors[2].hallwayBeginningX][storage[5].Doors[2].hallwayBeginningY + 1 + wallHeight][storage[5].Doors[2].hallwayBeginningZ] = 0;

               //This is a test line
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[5].Doors[2].hallwayBeginningX][storage[5].Doors[2].hallwayBeginningY + setHallWayWallHeight - 1][storage[5].Doors[2].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[5].Doors[2].hallwayBeginningZ = storage[5].Doors[2].hallwayBeginningZ + 1;
      //world[storage[5].Doors[2].hallwayBeginningX][storage[5].Doors[2].hallwayBeginningY][storage[5].Doors[2].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[5].Doors[2].hallwayBeginningX][storage[5].Doors[2].hallwayBeginningY][storage[5].Doors[2].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[5].Doors[2].hallwayBeginningX][storage[5].Doors[2].hallwayBeginningY][storage[5].Doors[2].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[5].Doors[2].hallwayBeginningX][storage[5].Doors[2].hallwayBeginningY + setHallWayWallHeight - 1][storage[5].Doors[2].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[5].Doors[2].hallwayBeginningX - 1][storage[5].Doors[2].hallwayBeginningY + wallHeight][storage[5].Doors[2].hallwayBeginningZ] = wallColour;
         world[storage[5].Doors[2].hallwayBeginningX + 1][storage[5].Doors[2].hallwayBeginningY + wallHeight][storage[5].Doors[2].hallwayBeginningZ] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[8].Doors[0].hallwayBeginningZ = storage[8].Doors[0].hallwayBeginningZ - 1;
      //world[storage[8].Doors[0].hallwayBeginningX][storage[8].Doors[0].hallwayBeginningY][storage[8].Doors[0].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[8].Doors[0].hallwayBeginningX][storage[8].Doors[0].hallwayBeginningY][storage[8].Doors[0].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[8].Doors[0].hallwayBeginningX][storage[8].Doors[0].hallwayBeginningY][storage[8].Doors[0].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[8].Doors[0].hallwayBeginningX][storage[8].Doors[0].hallwayBeginningY + setHallWayWallHeight - 1][storage[8].Doors[0].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[8].Doors[0].hallwayBeginningX - 1][storage[8].Doors[0].hallwayBeginningY + wallHeight][storage[8].Doors[0].hallwayBeginningZ] = wallColour;
         world[storage[8].Doors[0].hallwayBeginningX + 1][storage[8].Doors[0].hallwayBeginningY + wallHeight][storage[8].Doors[0].hallwayBeginningZ] = wallColour;
      }
   }
}

void buildHallwayFromRoom0ToRoom1(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room0 to Room1
   //Detection of a path across from the door
   for (int check1 = storage[0].Doors[1].hallwayBeginningX + 1; check1 < storage[1].Doors[2].hallwayBeginningX + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 1
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[1].startingZ; check2 < storage[1].startingZ + storage[1].zLength; check2++) //Scans Z //This should be the range of the starting X and xLength of the connecting door which is the door of Room 1
      {
         //This is a test line
         //world[check1][storage[0].Doors[0].hallwayBeginningY + 6][check2] = 4;

         if (world[check1][storage[0].Doors[1].hallwayBeginningY][check2] != 0)
         {
            int stopX = check1; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[0].Doors[1].hallwayBeginningY;
            int stopZ = check2;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[0].Doors[1].hallwayBeginningX + 1][storage[0].Doors[1].hallwayBeginningY + wallHeight][storage[0].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[0].Doors[1].hallwayBeginningX + 1][storage[0].Doors[1].hallwayBeginningY + wallHeight][storage[0].Doors[1].hallwayBeginningZ + 1] = wallColour;
               world[storage[0].Doors[1].hallwayBeginningX + 2][storage[0].Doors[1].hallwayBeginningY + wallHeight][storage[0].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[0].Doors[1].hallwayBeginningX + 2][storage[0].Doors[1].hallwayBeginningY + wallHeight][storage[0].Doors[1].hallwayBeginningZ + 1] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[0].Doors[1].hallwayBeginningZ < fixedStopZ)
            {
               //Rightside case
               //build postive x
               for (int build = storage[0].Doors[1].hallwayBeginningZ; build <= fixedStopZ; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else if (storage[0].Doors[1].hallwayBeginningZ > fixedStopZ)
            {
               //Leftside case
               //build negative x
               for (int build = storage[0].Doors[1].hallwayBeginningZ; build >= fixedStopZ; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY][storage[0].Doors[1].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY + 1 + wallHeight][storage[0].Doors[1].hallwayBeginningZ] = 0;

               //This is a test line
               //world[stopX][stopY + 5 + wallHeight][stopZ] = 3;
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[0].Doors[1].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[0].Doors[1].hallwayBeginningX = storage[0].Doors[1].hallwayBeginningX + 1;
      //world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY][storage[0].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY][storage[0].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY][storage[0].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[0].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY + wallHeight][storage[0].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[0].Doors[1].hallwayBeginningX][storage[0].Doors[1].hallwayBeginningY + wallHeight][storage[0].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[1].Doors[2].hallwayBeginningX = storage[1].Doors[2].hallwayBeginningX - 1;
      //world[storage[1].Doors[2].hallwayBeginningX][storage[1].Doors[2].hallwayBeginningY][storage[1].Doors[2].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[1].Doors[2].hallwayBeginningX][storage[1].Doors[2].hallwayBeginningY][storage[1].Doors[2].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[1].Doors[2].hallwayBeginningX][storage[1].Doors[2].hallwayBeginningY][storage[1].Doors[2].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[1].Doors[2].hallwayBeginningX][storage[1].Doors[2].hallwayBeginningY + setHallWayWallHeight - 1][storage[1].Doors[2].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[1].Doors[2].hallwayBeginningX][storage[1].Doors[2].hallwayBeginningY + wallHeight][storage[1].Doors[2].hallwayBeginningZ - 1] = wallColour;
         world[storage[1].Doors[2].hallwayBeginningX][storage[1].Doors[2].hallwayBeginningY + wallHeight][storage[1].Doors[2].hallwayBeginningZ + 1] = wallColour;
      }
   }
}

void buildHallwayFromRoom1ToRoom2(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room1 to Room2
   //Detection of a path across from the door
   for (int check1 = storage[1].Doors[1].hallwayBeginningX + 1; check1 < storage[2].Doors[1].hallwayBeginningX + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 2
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[2].startingZ; check2 < storage[2].startingZ + storage[2].zLength; check2++) //Sacns Z //This should be the range of the starting X and xLength of the connecting door which is the door of Room 2
      {
         //This is a test line
         //world[check1][storage[0].Doors[0].hallwayBeginningY + 6][check2] = 4;

         if (world[check1][storage[1].Doors[1].hallwayBeginningY][check2] != 0)
         {
            int stopX = check1; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[1].Doors[1].hallwayBeginningY;
            int stopZ = check2;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[1].Doors[1].hallwayBeginningX + 1][storage[1].Doors[1].hallwayBeginningY + wallHeight][storage[1].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[1].Doors[1].hallwayBeginningX + 1][storage[1].Doors[1].hallwayBeginningY + wallHeight][storage[1].Doors[1].hallwayBeginningZ + 1] = wallColour;
               world[storage[1].Doors[1].hallwayBeginningX + 2][storage[1].Doors[1].hallwayBeginningY + wallHeight][storage[1].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[1].Doors[1].hallwayBeginningX + 2][storage[1].Doors[1].hallwayBeginningY + wallHeight][storage[1].Doors[1].hallwayBeginningZ + 1] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[1].Doors[1].hallwayBeginningZ < fixedStopZ)
            {
               //Rightside case
               //build postive x
               for (int build = storage[1].Doors[1].hallwayBeginningZ; build <= fixedStopZ; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else if (storage[1].Doors[1].hallwayBeginningZ > fixedStopZ)
            {
               //Leftside case
               //build negative x
               for (int build = storage[1].Doors[1].hallwayBeginningZ; build >= fixedStopZ; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY][storage[1].Doors[1].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY + 1 + wallHeight][storage[1].Doors[1].hallwayBeginningZ] = 0;

               //This is a test line
               //world[stopX][stopY + 5 + wallHeight][stopZ] = 3;
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[1].Doors[1].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[1].Doors[1].hallwayBeginningX = storage[1].Doors[1].hallwayBeginningX + 1;
      //world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY][storage[1].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY][storage[1].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY][storage[1].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[1].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY + wallHeight][storage[1].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[1].Doors[1].hallwayBeginningX][storage[1].Doors[1].hallwayBeginningY + wallHeight][storage[1].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[2].Doors[1].hallwayBeginningX = storage[2].Doors[1].hallwayBeginningX - 1;
      //world[storage[2].Doors[1].hallwayBeginningX][storage[2].Doors[1].hallwayBeginningY][storage[2].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[2].Doors[1].hallwayBeginningX][storage[2].Doors[1].hallwayBeginningY][storage[2].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[2].Doors[1].hallwayBeginningX][storage[2].Doors[1].hallwayBeginningY][storage[2].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[2].Doors[1].hallwayBeginningX][storage[2].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[2].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[2].Doors[1].hallwayBeginningX][storage[2].Doors[1].hallwayBeginningY + wallHeight][storage[2].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[2].Doors[1].hallwayBeginningX][storage[2].Doors[1].hallwayBeginningY + wallHeight][storage[2].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }
   }
}

void buildHallwayFromRoom3ToRoom4(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room3 to Room4
   //Detection of a path across from the door
   for (int check1 = storage[3].Doors[1].hallwayBeginningX + 1; check1 < storage[4].Doors[1].hallwayBeginningX + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 4
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;
      for (int check2 = storage[4].startingZ; check2 < storage[4].startingZ + storage[4].zLength; check2++) //Scans Z //This should be the range of the starting X and xLength of the connecting door which is the door of Room 4
      {
         //This is a test line
         //world[check1][storage[0].Doors[0].hallwayBeginningY + 6][check2] = 4;

         if (world[check1][storage[3].Doors[1].hallwayBeginningY][check2] != 0)
         {
            int stopX = check1; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[3].Doors[1].hallwayBeginningY;
            int stopZ = check2;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[3].Doors[1].hallwayBeginningX + 1][storage[3].Doors[1].hallwayBeginningY + wallHeight][storage[3].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[3].Doors[1].hallwayBeginningX + 1][storage[3].Doors[1].hallwayBeginningY + wallHeight][storage[3].Doors[1].hallwayBeginningZ + 1] = wallColour;
               world[storage[3].Doors[1].hallwayBeginningX + 2][storage[3].Doors[1].hallwayBeginningY + wallHeight][storage[3].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[3].Doors[1].hallwayBeginningX + 2][storage[3].Doors[1].hallwayBeginningY + wallHeight][storage[3].Doors[1].hallwayBeginningZ + 1] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[3].Doors[1].hallwayBeginningZ < fixedStopZ)
            {
               //Rightside case
               //build postive x
               for (int build = storage[3].Doors[1].hallwayBeginningZ; build <= fixedStopZ; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else if (storage[3].Doors[1].hallwayBeginningZ > fixedStopZ)
            {
               //Leftside case
               //build negative x
               for (int build = storage[3].Doors[1].hallwayBeginningZ; build >= fixedStopZ; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY][storage[3].Doors[1].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY + 1 + wallHeight][storage[3].Doors[1].hallwayBeginningZ] = 0;

               //This is a test line
               //world[stopX][stopY + 5 + wallHeight][stopZ] = 3;
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[3].Doors[1].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[3].Doors[1].hallwayBeginningX = storage[3].Doors[1].hallwayBeginningX + 1;
      //world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY][storage[3].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY][storage[3].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY][storage[3].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[3].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY + wallHeight][storage[3].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[3].Doors[1].hallwayBeginningX][storage[3].Doors[1].hallwayBeginningY + wallHeight][storage[3].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[4].Doors[1].hallwayBeginningX = storage[4].Doors[1].hallwayBeginningX - 1;
      //world[storage[4].Doors[1].hallwayBeginningX][storage[4].Doors[1].hallwayBeginningY][storage[4].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[4].Doors[1].hallwayBeginningX][storage[4].Doors[1].hallwayBeginningY][storage[4].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[4].Doors[1].hallwayBeginningX][storage[4].Doors[1].hallwayBeginningY][storage[4].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[4].Doors[1].hallwayBeginningX][storage[4].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[4].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[4].Doors[1].hallwayBeginningX][storage[4].Doors[1].hallwayBeginningY + wallHeight][storage[4].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[4].Doors[1].hallwayBeginningX][storage[4].Doors[1].hallwayBeginningY + wallHeight][storage[4].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }
   }
}

void buildHallwayFromRoom4ToRoom5(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room4 to Room5
   //Detection of a path across from the door
   for (int check1 = storage[4].Doors[3].hallwayBeginningX + 1; check1 < storage[5].Doors[1].hallwayBeginningX + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 5
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;
      for (int check2 = storage[5].startingZ; check2 < storage[5].startingZ + storage[5].zLength; check2++) //Scans Z //This should be the range of the starting X and xLength of the connecting door which is the door of Room 5
      {
         //This is a test line
         //world[check1][storage[0].Doors[0].hallwayBeginningY + 6][check2] = 4;

         if (world[check1][storage[4].Doors[3].hallwayBeginningY][check2] != 0)
         {
            int stopX = check1; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[4].Doors[3].hallwayBeginningY;
            int stopZ = check2;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[4].Doors[3].hallwayBeginningX + 1][storage[4].Doors[3].hallwayBeginningY + wallHeight][storage[4].Doors[3].hallwayBeginningZ - 1] = wallColour;
               world[storage[4].Doors[3].hallwayBeginningX + 1][storage[4].Doors[3].hallwayBeginningY + wallHeight][storage[4].Doors[3].hallwayBeginningZ + 1] = wallColour;
               world[storage[4].Doors[3].hallwayBeginningX + 2][storage[4].Doors[3].hallwayBeginningY + wallHeight][storage[4].Doors[3].hallwayBeginningZ - 1] = wallColour;
               world[storage[4].Doors[3].hallwayBeginningX + 2][storage[4].Doors[3].hallwayBeginningY + wallHeight][storage[4].Doors[3].hallwayBeginningZ + 1] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[4].Doors[3].hallwayBeginningZ < fixedStopZ)
            {
               //Rightside case
               //build postive x
               for (int build = storage[4].Doors[3].hallwayBeginningZ; build <= fixedStopZ; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else if (storage[4].Doors[3].hallwayBeginningZ > fixedStopZ)
            {
               //Leftside case
               //build negative x
               for (int build = storage[4].Doors[3].hallwayBeginningZ; build >= fixedStopZ; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY][storage[4].Doors[3].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY + 1 + wallHeight][storage[4].Doors[3].hallwayBeginningZ] = 0;

               //This is a test line
               //world[stopX][stopY + 5 + wallHeight][stopZ] = 3;
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY + setHallWayWallHeight - 1][storage[4].Doors[3].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[4].Doors[3].hallwayBeginningX = storage[4].Doors[3].hallwayBeginningX + 1;
      //world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY][storage[4].Doors[3].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY][storage[4].Doors[3].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY][storage[4].Doors[3].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY + setHallWayWallHeight - 1][storage[4].Doors[3].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY + wallHeight][storage[4].Doors[3].hallwayBeginningZ - 1] = wallColour;
         world[storage[4].Doors[3].hallwayBeginningX][storage[4].Doors[3].hallwayBeginningY + wallHeight][storage[4].Doors[3].hallwayBeginningZ + 1] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[5].Doors[1].hallwayBeginningX = storage[5].Doors[1].hallwayBeginningX - 1;
      //world[storage[5].Doors[1].hallwayBeginningX][storage[5].Doors[1].hallwayBeginningY][storage[5].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[5].Doors[1].hallwayBeginningX][storage[5].Doors[1].hallwayBeginningY][storage[5].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[5].Doors[1].hallwayBeginningX][storage[5].Doors[1].hallwayBeginningY][storage[5].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[5].Doors[1].hallwayBeginningX][storage[5].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[5].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[5].Doors[1].hallwayBeginningX][storage[5].Doors[1].hallwayBeginningY + wallHeight][storage[5].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[5].Doors[1].hallwayBeginningX][storage[5].Doors[1].hallwayBeginningY + wallHeight][storage[5].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }
   }
}

void buildHallwayFromRoom6ToRoom7(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room6 to Room7
   //Detection of a path across from the door
   for (int check1 = storage[6].Doors[1].hallwayBeginningX + 1; check1 < storage[7].Doors[2].hallwayBeginningX + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 7
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;
      for (int check2 = storage[7].startingZ; check2 < storage[7].startingZ + storage[7].zLength; check2++) //Scans Z //This should be the range of the starting X and xLength of the connecting door which is the door of Room 7
      {
         //This is a test line
         //world[check1][storage[0].Doors[0].hallwayBeginningY + 6][check2] = 4;

         if (world[check1][storage[6].Doors[1].hallwayBeginningY][check2] != 0)
         {
            int stopX = check1; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[6].Doors[1].hallwayBeginningY;
            int stopZ = check2;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[6].Doors[1].hallwayBeginningX + 1][storage[6].Doors[1].hallwayBeginningY + wallHeight][storage[6].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[6].Doors[1].hallwayBeginningX + 1][storage[6].Doors[1].hallwayBeginningY + wallHeight][storage[6].Doors[1].hallwayBeginningZ + 1] = wallColour;
               world[storage[6].Doors[1].hallwayBeginningX + 2][storage[6].Doors[1].hallwayBeginningY + wallHeight][storage[6].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[6].Doors[1].hallwayBeginningX + 2][storage[6].Doors[1].hallwayBeginningY + wallHeight][storage[6].Doors[1].hallwayBeginningZ + 1] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[6].Doors[1].hallwayBeginningZ < fixedStopZ)
            {
               //Rightside case
               //build postive x
               for (int build = storage[6].Doors[1].hallwayBeginningZ; build <= fixedStopZ; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else if (storage[6].Doors[1].hallwayBeginningZ > fixedStopZ)
            {
               //Leftside case
               //build negative x
               for (int build = storage[6].Doors[1].hallwayBeginningZ; build >= fixedStopZ; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY][storage[6].Doors[1].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY + 1 + wallHeight][storage[6].Doors[1].hallwayBeginningZ] = 0;

               //This is a test line
               //world[stopX][stopY + 5 + wallHeight][stopZ] = 3;
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[6].Doors[1].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[6].Doors[1].hallwayBeginningX = storage[6].Doors[1].hallwayBeginningX + 1;
      //world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY][storage[6].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY][storage[6].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY][storage[6].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[6].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY + wallHeight][storage[6].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[6].Doors[1].hallwayBeginningX][storage[6].Doors[1].hallwayBeginningY + wallHeight][storage[6].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[7].Doors[2].hallwayBeginningX = storage[7].Doors[2].hallwayBeginningX - 1;
      //world[storage[7].Doors[2].hallwayBeginningX][storage[7].Doors[2].hallwayBeginningY][storage[7].Doors[2].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[7].Doors[2].hallwayBeginningX][storage[7].Doors[2].hallwayBeginningY][storage[7].Doors[2].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[7].Doors[2].hallwayBeginningX][storage[7].Doors[2].hallwayBeginningY][storage[7].Doors[2].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[7].Doors[2].hallwayBeginningX][storage[7].Doors[2].hallwayBeginningY + setHallWayWallHeight - 1][storage[7].Doors[2].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[7].Doors[2].hallwayBeginningX][storage[7].Doors[2].hallwayBeginningY + wallHeight][storage[7].Doors[2].hallwayBeginningZ - 1] = wallColour;
         world[storage[7].Doors[2].hallwayBeginningX][storage[7].Doors[2].hallwayBeginningY + wallHeight][storage[7].Doors[2].hallwayBeginningZ + 1] = wallColour;
      }
   }
}

void buildHallwayFromRoom7ToRoom8(Room storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room7 to Room8
   //Detection of a path across from the door
   for (int check1 = storage[7].Doors[1].hallwayBeginningX + 1; check1 < storage[8].Doors[1].hallwayBeginningX + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 8
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;
      for (int check2 = storage[8].startingZ; check2 < storage[8].startingZ + storage[8].zLength; check2++) //Sacns Z //This should be the range of the starting X and xLength of the connecting door which is the door of Room 8
      {
         //This is a test line
         //world[check1][storage[0].Doors[0].hallwayBeginningY + 6][check2] = 4;

         if (world[check1][storage[7].Doors[1].hallwayBeginningY][check2] != 0)
         {
            int stopX = check1; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[7].Doors[1].hallwayBeginningY;
            int stopZ = check2;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Builds the wall around the hallway - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[7].Doors[1].hallwayBeginningX + 1][storage[7].Doors[1].hallwayBeginningY + wallHeight][storage[7].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[7].Doors[1].hallwayBeginningX + 1][storage[7].Doors[1].hallwayBeginningY + wallHeight][storage[7].Doors[1].hallwayBeginningZ + 1] = wallColour;
               world[storage[7].Doors[1].hallwayBeginningX + 2][storage[7].Doors[1].hallwayBeginningY + wallHeight][storage[7].Doors[1].hallwayBeginningZ - 1] = wallColour;
               world[storage[7].Doors[1].hallwayBeginningX + 2][storage[7].Doors[1].hallwayBeginningY + wallHeight][storage[7].Doors[1].hallwayBeginningZ + 1] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[7].Doors[1].hallwayBeginningZ < fixedStopZ)
            {
               //Rightside case
               //build postive x
               for (int build = storage[7].Doors[1].hallwayBeginningZ; build <= fixedStopZ; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else if (storage[7].Doors[1].hallwayBeginningZ > fixedStopZ)
            {
               //Leftside case
               //build negative x
               for (int build = storage[7].Doors[1].hallwayBeginningZ; build >= fixedStopZ; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway with random colours
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Builds the wall around the hallway
                  for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
                  {
                     //Build the walls beside the brigde
                     world[stopX - 1][stopY + wallHeight][build] = wallColour;
                     world[stopX + 1][stopY + wallHeight][build] = wallColour;

                     //This line deletes everything above the bridge to clear it
                     world[stopX][stopY + 1 + wallHeight][build] = 0;
                  }

                  //Builds the ceiling of the bridge to the otherside of the hallway
                  world[stopX][stopY + setHallWayWallHeight - 1][build] = wallColour;
               }
            }
            else
            {
               //equal case, do nothing since the doors are into the same X or Z position across each other
            }

            //Fixing the door entrance to the bridge
            world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY][storage[7].Doors[1].hallwayBeginningZ] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY + 1 + wallHeight][storage[7].Doors[1].hallwayBeginningZ] = 0;

               //This is a test line
               //world[stopX][stopY + 5 + wallHeight][stopZ] = 3;
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[7].Doors[1].hallwayBeginningZ] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[7].Doors[1].hallwayBeginningX = storage[7].Doors[1].hallwayBeginningX + 1;
      //world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY][storage[7].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY][storage[7].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY][storage[7].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[7].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY + wallHeight][storage[7].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[7].Doors[1].hallwayBeginningX][storage[7].Doors[1].hallwayBeginningY + wallHeight][storage[7].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[8].Doors[1].hallwayBeginningX = storage[8].Doors[1].hallwayBeginningX - 1;
      //world[storage[8].Doors[1].hallwayBeginningX][storage[8].Doors[1].hallwayBeginningY][storage[8].Doors[1].hallwayBeginningZ] = floorColour;

      //Make the floor of the hallway with random colours
      if (randColour == 1)
      {
         world[storage[8].Doors[1].hallwayBeginningX][storage[8].Doors[1].hallwayBeginningY][storage[8].Doors[1].hallwayBeginningZ] = floorShade;
      }
      else
      {
         world[storage[8].Doors[1].hallwayBeginningX][storage[8].Doors[1].hallwayBeginningY][storage[8].Doors[1].hallwayBeginningZ] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[8].Doors[1].hallwayBeginningX][storage[8].Doors[1].hallwayBeginningY + setHallWayWallHeight - 1][storage[8].Doors[1].hallwayBeginningZ] = wallColour;

      //Builds the wall around the hallway
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[8].Doors[1].hallwayBeginningX][storage[8].Doors[1].hallwayBeginningY + wallHeight][storage[8].Doors[1].hallwayBeginningZ - 1] = wallColour;
         world[storage[8].Doors[1].hallwayBeginningX][storage[8].Doors[1].hallwayBeginningY + wallHeight][storage[8].Doors[1].hallwayBeginningZ + 1] = wallColour;
      }
   }
}

void setPlayerSpawnLocation()
{
   //int rng = (rand() % (max - min + 1)) + min; //rng formula

   //Randomly choose a room
   int randRoomID = (rand() % 9);

   //offset from the wall
   int wallOffset = 3; //3 is the minium value
   float spawnX;
   float spawnY;
   float spawnZ;

   int minX = storage[randRoomID].startingX + wallOffset;
   int maxX = storage[randRoomID].startingX + storage[randRoomID].xLength - wallOffset;
   int minZ = storage[randRoomID].startingZ + wallOffset;
   int maxZ = storage[randRoomID].startingZ + storage[randRoomID].zLength - wallOffset;

   srand(time(NULL)); //Include this at the beginning of using rand()

   //Randomly choose a location in an area of a room
   int randX = (rand() % (maxX - minX + 1)) + minX;
   int randZ = (rand() % (maxZ - minZ + 1)) + minZ;

   //Inverse the world locations so they work with the player location
   spawnX = inverseValue((float)randX);
   spawnY = inverseValue((float)storage[randRoomID].startingY + 2);
   spawnZ = inverseValue((float)randZ);

   //Set the spawn location of the player/viewer
   setViewPosition(spawnX, spawnY, spawnZ);
   setOldViewPosition(spawnX, spawnY, spawnZ);
}

void saveCurrentWorld(int saveWorldToThis[100][50][100])
{
   for (int i = 0; i < WORLDX; i++)
   {
      for (int j = 0; j < WORLDY; j++)
      {
         for (int k = 0; k < WORLDZ; k++)
         {
            saveWorldToThis[i][j][k] = world[i][j][k];
         }
      }
   }
}

void regenerateWorld(int regenerateThisWorld[100][50][100])
{
   for (int i = 0; i < WORLDX; i++)
   {
      for (int j = 0; j < WORLDY; j++)
      {
         for (int k = 0; k < WORLDZ; k++)
         {
            world[i][j][k] = regenerateThisWorld[i][j][k];
         }
      }
   }
}

void generateOutsideLevel(worldLevels database[2])
{
   //i = X
   //k = Z
   int whiteColour = 10;
   int greenColour = 11;
   int brownColour = 12;
   int downStairColour = 13;

   float tempt = 0;
   int perlin_Y;

   srand(time(NULL));
   //rng formula
   //int rng = (rand() % (max - min + 1)) + min;
   int stairX = (rand() % (80 - 20 + 1)) + 20;
   int stairZ = (rand() % (80 - 20 + 1)) + 20;

   //printf("RANDX: %d\n", randX);
   //printf("RANDZ: %d\n", randZ);

   for (int i = 0; i < WORLDX; i++)
   {
      for (int k = 0; k < WORLDZ; k++)
      {
         //perlin2d(X, Z, FREQUENCY, DEPTH) * HEIGHT
         tempt = perlin2d(i, k, 0.05, 2);
         tempt = (tempt * 25);
         perlin_Y = (int)round(tempt);
         if (perlin_Y >= 20)
         {
            world[i][perlin_Y][k] = whiteColour;
            world[i][perlin_Y - 1][k] = whiteColour;
            world[i][perlin_Y - 2][k] = whiteColour;
         }
         else if (perlin_Y >= 10)
         {
            world[i][perlin_Y][k] = greenColour;
            world[i][perlin_Y - 1][k] = greenColour;
            world[i][perlin_Y - 2][k] = greenColour;
         }
         else
         {
            world[i][perlin_Y][k] = brownColour;
            world[i][perlin_Y - 1][k] = brownColour;
            world[i][perlin_Y - 2][k] = brownColour;
         }
      }
   }

   for (int y = 0; y < WORLDY; y++)
   {
      if (world[stairX][y][stairZ] != 0 && world[stairX][y + 1][stairZ] == 0)
      {
         world[stairX][y + 1][stairZ] = downStairColour;
         database[0].stairGoingDown.x = stairX;
         database[0].stairGoingDown.y = y + 1;
         database[0].stairGoingDown.z = stairZ;

         database[0].stairGoingUp.x = -1;
         database[0].stairGoingUp.y = -1;
         database[0].stairGoingUp.z = -1;
         break;
      }
   }
}

void goDownStairs(worldLevels database[2], float playerX, float playerY, float playerZ)
{
   int lengthOfDatabase = 2;

   float newX;
   float newY;
   float newZ;

   playerX = inverseValue(playerX);
   playerY = inverseValue(playerY);
   playerZ = inverseValue(playerZ);

   //printf("TEST TEST TEST\n");

   //Size is not set/determined and needs to be changed, haven't made a function to find the length of struct array yet
   for (int i = 0; i < lengthOfDatabase - 1; i++)
   {
      //Check all values of player's x,y (+/-1), and z with the stair location in database

      //printf("TEST TEST TEST420\n");
      if ((database[i + 1].stairGoingDown.x != -1) && (database[i + 1].stairGoingDown.y != -1) && (database[i + 1].stairGoingDown.z != -1))
      {
         //printf("TEST TEST TEST69\n");
         if (world[database[i + 1].stairGoingDown.x + 1][database[i + 1].stairGoingDown.y][database[i + 1].stairGoingDown.z] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingDown.x + 1));
            newY = inverseValue((float)(database[i + 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingDown.z));
            //printf("X: %0.2f Y: %0.2f Z: %0.2f \n", newX, newY, newZ);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST1\n");
            break;
         }
         else if (world[database[i + 1].stairGoingDown.x - 1][database[i + 1].stairGoingDown.y][database[i + 1].stairGoingDown.z] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingDown.x - 1));
            newY = inverseValue((float)(database[i + 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingDown.z));
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST2\n");
            break;
         }
         else if (world[database[i + 1].stairGoingDown.x][database[i + 1].stairGoingDown.y][database[i + 1].stairGoingDown.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingDown.x));
            newY = inverseValue((float)(database[i + 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingDown.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST3\n");
            break;
         }
         else if (world[database[i + 1].stairGoingDown.x][database[i + 1].stairGoingDown.y][database[i + 1].stairGoingDown.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingDown.x));
            newY = inverseValue((float)(database[i + 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingDown.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST4\n");
            break;
         }
         else if (world[database[i + 1].stairGoingDown.x + 1][database[i + 1].stairGoingDown.y][database[i + 1].stairGoingDown.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingDown.x + 1));
            newY = inverseValue((float)(database[i + 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingDown.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST5\n");
            break;
         }
         else if (world[database[i + 1].stairGoingDown.x - 1][database[i + 1].stairGoingDown.y][database[i + 1].stairGoingDown.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingDown.x - 1));
            newY = inverseValue((float)(database[i + 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingDown.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST6\n");
            break;
         }
         else if (world[database[i + 1].stairGoingDown.x + 1][database[i + 1].stairGoingDown.y][database[i + 1].stairGoingDown.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingDown.x + 1));
            newY = inverseValue((float)(database[i + 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingDown.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST7\n");
            break;
         }
         else if (world[database[i + 1].stairGoingDown.x - 1][database[i + 1].stairGoingDown.y][database[i + 1].stairGoingDown.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingDown.x - 1));
            newY = inverseValue((float)(database[i + 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingDown.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST7\n");
            break;
         }
      }
      else
      {
         if (world[database[i + 1].stairGoingUp.x + 1][database[i + 1].stairGoingUp.y][database[i + 1].stairGoingUp.z] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingUp.x + 1));
            newY = inverseValue((float)(database[i + 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingUp.z));
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST8\n");
            break;
         }
         else if (world[database[i + 1].stairGoingUp.x - 1][database[i + 1].stairGoingUp.y][database[i + 1].stairGoingUp.z] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingUp.x - 1));
            newY = inverseValue((float)(database[i + 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingUp.z));
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST8\n");
            break;
         }
         else if (world[database[i + 1].stairGoingUp.x][database[i + 1].stairGoingUp.y][database[i + 1].stairGoingUp.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingUp.x));
            newY = inverseValue((float)(database[i + 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingUp.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST9\n");
            break;
         }
         else if (world[database[i + 1].stairGoingUp.x][database[i + 1].stairGoingUp.y][database[i + 1].stairGoingUp.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingUp.x));
            newY = inverseValue((float)(database[i + 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingUp.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST10\n");
            break;
         }
         else if (world[database[i + 1].stairGoingUp.x + 1][database[i + 1].stairGoingUp.y][database[i + 1].stairGoingUp.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingUp.x + 1));
            newY = inverseValue((float)(database[i + 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingUp.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST11\n");
            break;
         }
         else if (world[database[i + 1].stairGoingUp.x - 1][database[i + 1].stairGoingUp.y][database[i + 1].stairGoingUp.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingUp.x - 1));
            newY = inverseValue((float)(database[i + 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingUp.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST12\n");
            break;
         }
         else if (world[database[i + 1].stairGoingUp.x + 1][database[i + 1].stairGoingUp.y][database[i + 1].stairGoingUp.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingUp.x + 1));
            newY = inverseValue((float)(database[i + 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingUp.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST13\n");
            break;
         }
         else if (world[database[i + 1].stairGoingUp.x - 1][database[i + 1].stairGoingUp.y][database[i + 1].stairGoingUp.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i + 1].stairGoingUp.x - 1));
            newY = inverseValue((float)(database[i + 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i + 1].stairGoingUp.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST14\n");
            break;
         }
      }
   }
}

void goUpStairs(worldLevels database[2], float playerX, float playerY, float playerZ)
{
   int lengthOfDatabase = 2;

   float newX;
   float newY;
   float newZ;

   playerX = inverseValue(playerX);
   playerY = inverseValue(playerY);
   playerZ = inverseValue(playerZ);

   //printf("TEST TEST TEST\n");

   //Size is not set/determined and needs to be changed, haven't made a function to find the length of struct array yet
   for (int i = lengthOfDatabase - 1; i > 0; i--)
   {
      //Check all values of player's x,y (+/-1), and z with the stair location in database

      //printf("TEST TEST TEST420\n");
      if ((database[i - 1].stairGoingUp.x != -1) && (database[i - 1].stairGoingUp.y != -1) && (database[i - 1].stairGoingUp.z != -1))
      {
         //printf("TEST TEST TEST69\n");
         if (world[database[i - 1].stairGoingUp.x + 1][database[i - 1].stairGoingUp.y][database[i - 1].stairGoingUp.z] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingUp.x + 1));
            newY = inverseValue((float)(database[i - 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingUp.z));

            //printf("X: %0.2f Y: %0.2f Z: %0.2f \n", newX, newY, newZ);

            setViewPosition(newX, newY, newZ);
            printf("TEST TEST TEST1\n");
            break;
         }
         else if (world[database[i - 1].stairGoingUp.x - 1][database[i - 1].stairGoingUp.y][database[i - 1].stairGoingUp.z] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingUp.x - 1));
            newY = inverseValue((float)(database[i - 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingUp.z));
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST2\n");
            break;
         }
         else if (world[database[i - 1].stairGoingUp.x][database[i - 1].stairGoingUp.y][database[i - 1].stairGoingUp.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingUp.x));
            newY = inverseValue((float)(database[i - 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingUp.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST3\n");
            break;
         }
         else if (world[database[i - 1].stairGoingUp.x][database[i - 1].stairGoingUp.y][database[i - 1].stairGoingUp.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingUp.x));
            newY = inverseValue((float)(database[i - 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingUp.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST4\n");
            break;
         }
         else if (world[database[i - 1].stairGoingUp.x + 1][database[i - 1].stairGoingUp.y][database[i - 1].stairGoingUp.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingUp.x + 1));
            newY = inverseValue((float)(database[i - 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingUp.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST5\n");
            break;
         }
         else if (world[database[i - 1].stairGoingUp.x - 1][database[i - 1].stairGoingUp.y][database[i - 1].stairGoingUp.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingUp.x - 1));
            newY = inverseValue((float)(database[i - 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingUp.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST6\n");
            break;
         }
         else if (world[database[i - 1].stairGoingUp.x + 1][database[i - 1].stairGoingUp.y][database[i - 1].stairGoingUp.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingUp.x + 1));
            newY = inverseValue((float)(database[i - 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingUp.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST7\n");
            break;
         }
         else if (world[database[i - 1].stairGoingUp.x - 1][database[i - 1].stairGoingUp.y][database[i - 1].stairGoingUp.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingUp.x - 1));
            newY = inverseValue((float)(database[i - 1].stairGoingUp.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingUp.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST7\n");
            break;
         }
      }
      else
      {
         if (world[database[i - 1].stairGoingDown.x + 1][database[i - 1].stairGoingDown.y][database[i - 1].stairGoingDown.z] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingDown.x + 1));
            newY = inverseValue((float)(database[i - 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingDown.z));
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST8\n");
            break;
         }
         else if (world[database[i - 1].stairGoingDown.x - 1][database[i - 1].stairGoingDown.y][database[i - 1].stairGoingDown.z] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingDown.x - 1));
            newY = inverseValue((float)(database[i - 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingDown.z));
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST8\n");
            break;
         }
         else if (world[database[i - 1].stairGoingDown.x][database[i - 1].stairGoingDown.y][database[i - 1].stairGoingDown.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingDown.x));
            newY = inverseValue((float)(database[i - 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingDown.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST9\n");
            break;
         }
         else if (world[database[i - 1].stairGoingDown.x][database[i - 1].stairGoingDown.y][database[i - 1].stairGoingDown.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingDown.x));
            newY = inverseValue((float)(database[i - 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingDown.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST10\n");
            break;
         }
         else if (world[database[i - 1].stairGoingDown.x + 1][database[i - 1].stairGoingDown.y][database[i - 1].stairGoingDown.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingDown.x + 1));
            newY = inverseValue((float)(database[i - 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingDown.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST11\n");
            break;
         }
         else if (world[database[i - 1].stairGoingDown.x - 1][database[i - 1].stairGoingDown.y][database[i - 1].stairGoingDown.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingDown.x - 1));
            newY = inverseValue((float)(database[i - 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingDown.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST12\n");
            break;
         }
         else if (world[database[i - 1].stairGoingDown.x + 1][database[i - 1].stairGoingDown.y][database[i - 1].stairGoingDown.z - 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingDown.x + 1));
            newY = inverseValue((float)(database[i - 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingDown.z) - 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST13\n");
            break;
         }
         else if (world[database[i - 1].stairGoingDown.x - 1][database[i - 1].stairGoingDown.y][database[i - 1].stairGoingDown.z + 1] == 0)
         {
            newX = inverseValue((float)(database[i - 1].stairGoingDown.x - 1));
            newY = inverseValue((float)(database[i - 1].stairGoingDown.y + 1));
            newZ = inverseValue((float)(database[i - 1].stairGoingDown.z) + 1);
            setViewPosition(newX, newY, newZ);
            //printf("TEST TEST TEST14\n");
            break;
         }
      }
   }
}

void spawnBesideBlock(worldLevels database[2])
{
   if (world[database[0].stairGoingDown.x + 1][database[0].stairGoingDown.y][database[0].stairGoingDown.z] == 0)
   {
      setViewPosition((float)(-1 * database[0].stairGoingDown.x + 1), ((float)(-1 * database[0].stairGoingDown.y - 2)), ((float)(-1 * database[0].stairGoingDown.z)));
   }
   else if (world[database[0].stairGoingDown.x][database[0].stairGoingDown.y][database[0].stairGoingDown.z + 1] == 0)
   {
      setViewPosition((float)(-1 * database[0].stairGoingDown.x), ((float)(-1 * database[0].stairGoingDown.y - 2)), ((float)(-1 * database[0].stairGoingDown.z + 1)));
   }
   else if (world[database[0].stairGoingDown.x - 1][database[0].stairGoingDown.y][database[0].stairGoingDown.z] == 0)
   {
      setViewPosition((float)(-1 * database[0].stairGoingDown.x - 1), ((float)(-1 * database[0].stairGoingDown.y - 2)), ((float)(-1 * database[0].stairGoingDown.z)));
   }
   else if (world[database[0].stairGoingDown.x][database[0].stairGoingDown.y][database[0].stairGoingDown.z - 1] == 0)
   {
      setViewPosition((float)(-1 * database[0].stairGoingDown.x), ((float)(-1 * database[0].stairGoingDown.y - 2)), ((float)(-1 * database[0].stairGoingDown.z - 1)));
   }
   else if (world[database[0].stairGoingDown.x - 1][database[0].stairGoingDown.y][database[0].stairGoingDown.z - 1] == 0)
   {
      setViewPosition((float)(-1 * database[0].stairGoingDown.x - 1), ((float)(-1 * database[0].stairGoingDown.y - 2)), ((float)(-1 * database[0].stairGoingDown.z - 1)));
   }
   else if (world[database[0].stairGoingDown.x + 1][database[0].stairGoingDown.y][database[0].stairGoingDown.z - 1] == 0)
   {
      setViewPosition((float)(-1 * database[0].stairGoingDown.x + 1), ((float)(-1 * database[0].stairGoingDown.y - 2)), ((float)(-1 * database[0].stairGoingDown.z - 1)));
   }
   else if (world[database[0].stairGoingDown.x - 1][database[0].stairGoingDown.y][database[0].stairGoingDown.z + 1] == 0)
   {
      setViewPosition((float)(-1 * database[0].stairGoingDown.x - 1), ((float)(-1 * database[0].stairGoingDown.y - 2)), ((float)(-1 * database[0].stairGoingDown.z + 1)));
   }
   else if (world[database[0].stairGoingDown.x + 1][database[0].stairGoingDown.y][database[0].stairGoingDown.z - 1] == 0)
   {
      setViewPosition((float)(-1 * database[0].stairGoingDown.x + 1), ((float)(-1 * database[0].stairGoingDown.y - 2)), ((float)(-1 * database[0].stairGoingDown.z - 1)));
   }
}