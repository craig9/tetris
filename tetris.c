#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <SDL/SDL.h>

/* Changes:
 * 24 March 2006:
 * No longer crashes after a few blocks have fallen
 * Now you can hold the down key, and it'll slide down properly
 * The falling block is now in yellow
 * Now it speeds up based on how many lines you have
 */

/* TODO: 
 * Should be able to slide before the brick is laid on the wayy
 * Should speed up based on the number of lines...
 * Wrap shapes, squares and stuff in structs
 */

SDL_Surface* screen = NULL;

typedef struct square {int x, y;} square;
typedef struct shape {square squares[4];} shape;

bool board[10][20];
/* 7 shapes, 4 squares per shape, 2 co-ords per square */
int shapes[7][4][2] = {{{0,0},{0,1},{0,-1},{0,-2}}, /* I */
{{0,0},{0,1},{0,-1},{1,-1}}, /* L */
{{0,0},{0,1},{0,-1},{-1,-1}}, /* backwards L */
{{0,0},{1,0},{-1,0},{0,-1}}, /* T */
{{0,0},{1,0},{0,1},{1,1}}, /* square */
{{0,0},{0,-1},{-1,0},{-1,1}}, /* S */
{{0,0},{0,1},{-1,0},{-1,-1}}}; /* Z */

const int square_size = 20;
int current_shape[4][2];
int xpos = 5;
int ypos = 0;
int frame = 0;
int lines = 0;
bool sliding_down = false;
bool game_over = false;

void clear_board()
{
	int i, j;
	for (i=0; i<10; i++)
		for (j=0; j<20; j++)
			board[i][j] = false;
	game_over = false;
}

bool check_rotate_bounds_ok(int tmp_shape[4][2])
{
	int i;

	for(i=0; i<4; i++)
	{
		int test_x = tmp_shape[i][0] + xpos;
		int test_y = tmp_shape[i][1] + ypos;

		if(board[test_x][test_y] == true) return false;
		if(test_x<0) return false;
		if(test_x>9) return false;
		if(test_y>19) return false;
	}

	return true;
}

void rotate_shape()
{
	/* TODO LATER: squares, i's, s's, z's, need 2-stage rotating... */

	int tmp_shape[4][2];
	int i;

	/* copy current shape into temporary storage and rotate */
	for(i=0; i<4; i++)
	{
		int xold = current_shape[i][0];
		int yold = current_shape[i][1];
		
		int xnew = yold;
		int ynew = -xold;

		tmp_shape[i][0] = xnew;
		tmp_shape[i][1] = ynew;
	}

	/* make sure it doesn't hit anything or hang out (!) */
	if(!check_rotate_bounds_ok(tmp_shape))
		return;

	/* copy it back */
	for(i=0; i<4; i++)
	{
		current_shape[i][0] = tmp_shape[i][0];
		current_shape[i][1] = tmp_shape[i][1];
	}
}


void create_new_shape()
{
	int num = rand() % 7;
	int rotate = rand() % 4;
	int i, j;

    printf("num is %d\n", num);
    printf("rotate is %d\n", rotate);

	for(i=0; i<4; i++)
		for(j=0; j<2; j++)
			current_shape[i][j] = shapes[num][i][j];

	while (rotate--)
		rotate_shape();
}


/* TODO: merge this function with check_rotate_bounds_ok() */
bool check_bounds_ok(int x, int y)
{
	int i;

	for(i=0; i<4; i++)
	{
		int test_x = current_shape[i][0] + x;
		int test_y = current_shape[i][1] + y;

		if (test_y < 0) test_y = 0;

		if(board[test_x][test_y] == true) return false;
		if(test_x<0) return false;
		if(test_x>9) return false;
		if(test_y>19) return false;
	}

	return true;
}

void create_snapshot()
{
	int x, y;
	for(y=0; y<20; y++)
	{
		for(x=0; x<10; x++)
		{
			printf("%s", board[x][y] ? "X" : "O");
		}
		printf("\n");
	}
	printf("\n");
}

void remove_line(int y)
{
	int x;

	for( ; y>0; y--)
		for (x=0; x<10; x++)
			board[x][y] = board[x][y-1];

	lines++;
}

void check_wins()
{
	int y;

	for(y=19; y>=0; y--)
		if(board[0][y] && board[1][y] && 
		   board[2][y] && board[3][y] && 
		   board[4][y] && board[5][y] && 
		   board[6][y] && board[7][y] && 
		   board[8][y] && board[9][y])
		{
			remove_line(y);
		}
}

void print_game_over()
{
	FILE* fp = NULL;
	time_t t = time(NULL);
	char* date_time = asctime(gmtime(&t));

	fp = fopen("scores.txt", "a");
	fprintf(fp, "Someone scored %d at %s", lines, date_time);
	fclose(fp);
}

void lay_brick()
{
	int i, x, y;

	printf("Laying brick at %d, %d\n", xpos, ypos);	
	for (i=0; i<4; i++)
	{
		x = current_shape[i][0]+xpos;
		y = current_shape[i][1]+ypos;
		if (y == 0)
		{
			if (!game_over)
				print_game_over();
			game_over = true;
		}
		board[x][y] = true;
	}

	create_new_shape();
	ypos = 0;
	xpos = 5;
}

void check_brick()
{
	int i, x, y;

	for (i=0; i<4; i++)
	{
		x = current_shape[i][0]+xpos;
		y = current_shape[i][1]+ypos;
		
		if(y >= 19 || board[x][y+1] == true)
		{
			lay_brick();
			return;
		}
	}
}

void clear_screen()
{
	SDL_Rect rect;

	rect.w = 10 * square_size;
	rect.h = 20 * square_size;
	rect.x = 0;
	rect.y = 0;

	SDL_FillRect(screen, &rect, 
				 SDL_MapRGB(screen->format, 0, 0, 0));

}

void draw_square(int x, int y)
{
	SDL_Rect rect;

	if(x<0 || x > 9 || y < 0 || y > 19)
		return;

	rect.w = square_size;
	rect.h = square_size;
	rect.x = x*square_size;
	rect.y = y*square_size;

	SDL_FillRect(screen, &rect, 
				 SDL_MapRGB(screen->format, 0, 128, 0));

	rect.w -= 2;
	rect.h -= 2;
	rect.x += 1;
	rect.y += 1;

	SDL_FillRect(screen, &rect, 
				 SDL_MapRGB(screen->format, 0, 255, 0));
}

void draw_square2(int x, int y)
{
	SDL_Rect rect;

	if(x<0 || x > 9 || y < 0 || y > 19)
		return;

	rect.w = square_size;
	rect.h = square_size;
	rect.x = x*square_size;
	rect.y = y*square_size;

	SDL_FillRect(screen, &rect, 
				 SDL_MapRGB(screen->format, 128, 128, 0));

	rect.w -= 2;
	rect.h -= 2;
	rect.x += 1;
	rect.y += 1;

	SDL_FillRect(screen, &rect, 
				 SDL_MapRGB(screen->format, 255, 255, 0));
}

void sleep()
{
	SDL_Delay(50 - (lines/5));

	frame += 1;
	if(frame>10) 
		frame = 0;
}

void update_title()
{
	char title[100];
	if (game_over)
		snprintf(title, sizeof(title), "Game Over - %d", lines);
	else
		snprintf(title, sizeof(title), "Tetris - %d", lines);
	SDL_WM_SetCaption(title, NULL);
}

void animate()
{
	/* Only animate on frame 0 of 5, then we have a chance to move
	 * the brick around left, right and down */
	if(!game_over)
	{
		if(frame == 0 && check_bounds_ok(xpos, ypos+1))
		{
			ypos += 1;
			check_brick();
			check_wins();
		}
		if(sliding_down &&
		   check_bounds_ok(xpos, ypos+1))
		{
			ypos += 1;
			check_brick();
			check_wins();
		}
	}
}

void draw_board()
{
	int x,y;

	for(x=0; x<10; x++)
		for(y=0; y<20; y++)
			if(board[x][y])
				draw_square(x, y);
}

void draw_shape()
{
	int i;

	for(i=0; i<4; i++)
		draw_square2(xpos+current_shape[i][0], 
					 ypos+current_shape[i][1]);
}

void draw()
{
	clear_screen();
	draw_board();
	draw_shape();
	SDL_Flip(screen);
	update_title();
}


void handle_key_down(SDLKey key, SDLMod mod)
{
	switch (key)
	{
	case SDLK_ESCAPE:
		printf("Caught an escape keypress, quitting\n");
		print_game_over();
		exit(EXIT_SUCCESS);

		/* Cheat modes!
	case SDLK_F1:
		printf("Clearing the board\n");
		clear_board();
		break;

	case SDLK_F2:
		printf("Increasing line count by 10\n");
		lines += 10;
		break;

	case SDLK_F5:
		printf("Snapshot\n");
		create_snapshot();
		break;
		*/

	case SDLK_UP:
	case SDLK_w:
		if(!game_over)
			rotate_shape();
		break;

	case SDLK_DOWN:
	case SDLK_s:
		sliding_down = true;
		/*
		if(check_bounds_ok(xpos, ypos+1) && !game_over)
		{
		   ypos += 1;
		   check_brick();
		   check_wins();
		}
		*/
		break;

	case SDLK_LEFT:
	case SDLK_a:
		if(check_bounds_ok(xpos-1, ypos) && !game_over)
		   xpos -= 1;
		break;

	case SDLK_RIGHT:
	case SDLK_d:
		if(check_bounds_ok(xpos+1, ypos) && !game_over)
		   xpos += 1;
		break;

	default:
		break;
	}
}

void handle_key_up(SDLKey key, SDLMod mod)
{
	switch (key)
	{
	case SDLK_DOWN:
	case SDLK_s:
		sliding_down = false;
		break;

	default:
		break;
	}
}

int main(int argc, char** argv)
{
	SDL_Event event;
	srand(time(NULL));

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Couldn't init video\n");
		exit(EXIT_FAILURE);
	}

	atexit(SDL_Quit);
	
	screen = SDL_SetVideoMode(10*square_size, 20*square_size, 
							  8, SDL_SWSURFACE);

	if(screen == NULL)
	{
		fprintf(stderr, "Couldn't set video mode\n");
		exit(EXIT_FAILURE);
	}

	clear_board();
	create_new_shape();
	update_title();

	while(1)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT: 
				exit(EXIT_SUCCESS);
			case SDL_KEYDOWN: 
				handle_key_down(event.key.keysym.sym, event.key.keysym.mod);
				break;
			case SDL_KEYUP: 
				handle_key_up(event.key.keysym.sym, event.key.keysym.mod);
				break;
			}
		}
		animate();
		draw();
		/* TODO: speed it up as the score gets higher */
		sleep();
	}
	exit(EXIT_SUCCESS);
}
