tetris: tetris.c
	gcc -Wall tetris.c `sdl-config --cflags --libs` -o tetris -I/usr/include/

