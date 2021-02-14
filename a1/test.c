void buildHallwayFromRoom0ToRoom1(Storage storage[9], int floorColour, int wallColour, int setHallWayWallHeight, int floorShade)
{
   //ID: Room0 to Room1
   //Detection of a path across from the door
   for (int check1 = storage[0].Doors[1].x + 1; check1 < storage[1].Doors[2].x + 2; check1++) //The +33 should be replace with the Z location of the connecting door which is the door of Room 1
   {
      //Colour randomizer
      int randColour = (rand() % ((2 - 1) + 1)) + 1;

      for (int check2 = storage[1].startingZ; check2 < storage[1].startingZ + storage[1].zLength; check2++) //Scans Z //This should be the range of the starting X and xLength of the connecting door which is the door of Room 1
      {
         //This is a test line
         //world[check1][storage[0].Doors[0].y + 6][check2] = 4;

         if (world[check1][storage[0].Doors[1].y][check2] != 0)
         {
            int stopX = check1; // + 1? Need a + 1 to accunt for the new walls added
            int stopY = storage[0].Doors[1].y;
            int stopZ = check2;

            //Fixed the position of the other door entrance to the bridge
            int fixedStopX = stopX + 1;
            int fixedStopY = stopY;
            int fixedStopZ = stopZ + 1;

            //Test wall creation - extends the walls by two from the starting end
            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               world[storage[0].Doors[1].x + 1][storage[0].Doors[1].y + wallHeight][storage[0].Doors[1].z - 1] = wallColour;
               world[storage[0].Doors[1].x + 1][storage[0].Doors[1].y + wallHeight][storage[0].Doors[1].z + 1] = wallColour;
               world[storage[0].Doors[1].x + 2][storage[0].Doors[1].y + wallHeight][storage[0].Doors[1].z - 1] = wallColour;
               world[storage[0].Doors[1].x + 2][storage[0].Doors[1].y + wallHeight][storage[0].Doors[1].z + 1] = wallColour;

               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 1][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ - 1] = wallColour;
               world[fixedStopX - 2][fixedStopY + wallHeight][fixedStopZ + 1] = wallColour;
            }

            //Build the sideways bridge
            //Leftside, rightside or equal case check
            if (storage[0].Doors[1].z < fixedStopZ)
            {
               //Rightside case
               //build postive x
               for (int build = storage[0].Doors[1].z; build <= fixedStopZ; build++)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Test wall creation
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
            else if (storage[0].Doors[1].z > fixedStopZ)
            {
               //Leftside case
               //build negative x
               for (int build = storage[0].Doors[1].z; build >= fixedStopZ; build--)
               {
                  //Builds the bridge to the otherside of the hallway
                  //world[stopX][stopY][build] = floorColour;

                  randColour = (rand() % ((2 - 1) + 1)) + 1;

                  //Make the floor of the hallway
                  if (randColour == 1)
                  {
                     world[stopX][stopY][build] = floorShade;
                  }
                  else
                  {
                     world[stopX][stopY][build] = floorColour;
                  }

                  //Test wall creation
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
               //equal case, do nothing
            }

            //Fixing the door entrance to the bridge
            world[storage[0].Doors[1].x][storage[0].Doors[1].y][storage[0].Doors[1].z] = floorColour;
            world[fixedStopX][fixedStopY][fixedStopZ] = floorColour;

            for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
            {
               //Clears the entrance to the bridge
               world[storage[0].Doors[1].x][storage[0].Doors[1].y + 1 + wallHeight][storage[0].Doors[1].z] = 0;

               //This is a test line
               //world[stopX][stopY + 5 + wallHeight][stopZ] = 3;
               //world[fixedStopX][fixedStopY + 5 + wallHeight][fixedStopZ] = 4;

               world[fixedStopX][fixedStopY + 1 + wallHeight][fixedStopZ] = 0;
            }

            //Build the ceiling to the hallway's bridge
            world[storage[0].Doors[1].x][storage[0].Doors[1].y + setHallWayWallHeight - 1][storage[0].Doors[1].z] = wallColour;
            world[fixedStopX][fixedStopY + setHallWayWallHeight - 1][fixedStopZ] = wallColour;

            return;
         }
      }

      //Increases the hallway length by one
      storage[0].Doors[1].x = storage[0].Doors[1].x + 1;
      //world[storage[0].Doors[1].x][storage[0].Doors[1].y][storage[0].Doors[1].z] = floorColour;

      //Make the floor of the hallway
      if (randColour == 1)
      {
         world[storage[0].Doors[1].x][storage[0].Doors[1].y][storage[0].Doors[1].z] = floorShade;
      }
      else
      {
         world[storage[0].Doors[1].x][storage[0].Doors[1].y][storage[0].Doors[1].z] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[0].Doors[1].x][storage[0].Doors[1].y + setHallWayWallHeight - 1][storage[0].Doors[1].z] = wallColour;

      //Test wall creation
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[0].Doors[1].x][storage[0].Doors[1].y + wallHeight][storage[0].Doors[1].z - 1] = wallColour;
         world[storage[0].Doors[1].x][storage[0].Doors[1].y + wallHeight][storage[0].Doors[1].z + 1] = wallColour;
      }

      //Increases the opposite hallway of the top door by one towards the top door
      storage[1].Doors[2].x = storage[1].Doors[2].x - 1;
      //world[storage[1].Doors[2].x][storage[1].Doors[2].y][storage[1].Doors[2].z] = floorColour;

      //Make the floor of the hallway
      if (randColour == 1)
      {
         world[storage[1].Doors[2].x][storage[1].Doors[2].y][storage[1].Doors[2].z] = floorShade;
      }
      else
      {
         world[storage[1].Doors[2].x][storage[1].Doors[2].y][storage[1].Doors[2].z] = floorColour;
      }

      //Build the ceiling of the hallway
      world[storage[1].Doors[2].x][storage[1].Doors[2].y + setHallWayWallHeight - 1][storage[1].Doors[2].z] = wallColour;

      //Test wall creation
      for (int wallHeight = 0; wallHeight < setHallWayWallHeight; wallHeight++)
      {
         //Need a check for block the middle on the hallway
         world[storage[1].Doors[2].x][storage[1].Doors[2].y + wallHeight][storage[1].Doors[2].z - 1] = wallColour;
         world[storage[1].Doors[2].x][storage[1].Doors[2].y + wallHeight][storage[1].Doors[2].z + 1] = wallColour;
      }
   }
}