# helloworld
this is getting really out-of-hand, huh?

basically a showcase of my favorite makefile setup, and a proposed way to organize parts of a game (Or similar applications).

Insofar as the makefile, it compiles all the .c files recursively in the "src" directory(Take note, this is not the disadvantaged recursive making that builds partial trees, this simply takes every c file it finds in a recursive search as targets, excluding files and directories marked "!IGNORE") The point of this is that I was fed up with the idea of always having to add source files to the make file, and I also wished to be able to organize the code into directories in a simple way. And sofar as the game organization, it's simply that a 'module' of the game may(all optional) have three functions, init, update, and cleanup. (In my application of SDL, so many modules would use event handling that I introduced a fourth given by the SDL module, "eventhandle") of course, init would be called at the startup of the program, cleanup at the end, and 'update' repeatedly.
