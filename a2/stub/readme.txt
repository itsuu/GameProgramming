Name: Henry Wong
Email: hwong10@uoguelph.ca
ID: 1057822

To compile the file, type "make" Please note that the makefile is currently
set to compile on linux, and that if you are using mac, you have to uncomment/comment out the path line
in the makefile.

After the file is compiled, just type the normal ./a1 and spawn you outside in the overworld near the stairs,
becareful when walking around as you might accidentally walking into the stairs and be teleported.


I have added 4 extra files called:

functions.c
functions.h
perlin.c
perlin.h

functions.c and .h are just placeholders for A3 when I decided to move helper functions over there. Currently it
is empty but it used in the makefile, to make sure the you dont touch the make file

perlin.c and .h are the files used for the perlin noise off and online source. They are also currently used in
the makefile, so make sure you dont touch the make file

perlin noise source: https://gist.github.com/nowl/828013
All credits to the perlin noise source goes to the creator in the above link. He has made it open source
by others in projects.


That should be all.