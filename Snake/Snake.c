#include "Snake.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

static pthread_mutex_t change_mutex;
static pthread_cond_t change_cond = PTHREAD_COND_INITIALIZER;
int poison = 1;
int change = 0;
char **field;
int length;
int food_x,food_y;
Snake *snake;
int speed;

// 0 = up, 1 = down, 2 = left, 3 = right

int next_command;
int old_command;
int kbhit()
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}
 
void inputThread(void *arg)
{
	//Don't echo input
	//http://stackoverflow.com/questions/558009/ansi-c-no-echo-keyboard-input
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &t);	
	int ch = 0;
	
	while(1)
	{
		while(!(ch = kbhit()))
			continue;

	ch = getchar();

	
	if(ch == 127)	       
		break;
	else if(ch == 119)
		next_command = 0;
	else if(ch == 115)
		next_command = 1;
	else if(ch == 97)
		next_command = 3;
	else if(ch == 100)
		next_command = 4;
	else if(ch == 43)
	{
		if(speed > 20000)
			speed -= 10000;
	}
	else if(ch == 45)
		if(speed < 10000000)
			speed += 10000;
	
	ch = 0;
	}
	poison = 0;
	
	//Echo input
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag = ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &t);	
}

void tui(void *arg)
{
	
	while(poison)
	{
		pthread_mutex_lock( &change_mutex );
		while(change == 0)
			pthread_cond_wait( &change_cond, &change_mutex );
		pthread_mutex_unlock( &change_mutex );
		system("clear");
		
		for(int i = 0;i < DEFAULT_X;i++)
		{
			for(int j=0;j < DEFAULT_Y;j++)
			{
				putc(field[i][j],stdout);
			}
			if( i == 10)
				printf("Length : %i ", length);
			if( i == 12 )
				printf("Speed : %i", speed);
			putchar('\n');
		}
		change = 0;
		
		printf("\n Press + to speed up, - to slow down \n");
		printf(" Press Del to Exit \n");
	}
}

char **init_field(int x, int y)
{
	char **field;
	field = malloc(x * sizeof(char*));
	for(int i=0;i < y; i++)
	{
		field[i] = malloc(y * sizeof(char));
	}
	return field;
}
void add_snake(Snake *snake)
{
	Snake *tmp = snake;
	
	tmp = (Snake *)realloc(snake,(length + 1) * sizeof(Snake));
	
	if(tmp[length].x != DEFAULT_X)
		tmp[length].x = tmp[length-1].x+1;
	else if(tmp[length].y != DEFAULT_Y)
		tmp[length].y = tmp[length-1].y+1;
	tmp[length].sign = "B";
	
	length++;
	printf("3: %p , %i \n",(void *)snake,length);
	snake = tmp;
}
void move_Snake(Snake *snake, int max)
{

	for(int i=max-1;i>0;i--)
	{
		snake[i].x = snake[i-1].x;
		snake[i].y = snake[i-1].y;
	}
	
	if(next_command+1 != old_command && next_command-1 != old_command)
		old_command = next_command;
		if(old_command == 0)
			snake[0].x += -1;
		
		if(old_command == 1)
			snake[0].x += +1;
	
			
		if(old_command == 3)
			snake[0].y += -1;
	
		if(old_command == 4)
			snake[0].y += +1;
	
	
	if(snake[0].x > DEFAULT_X-1)
		snake[0].x = 1;
		
	if(snake[0].y > DEFAULT_Y-1)
		snake[0].y = 1;
	
	if(snake[0].x == 0)
		snake[0].x = DEFAULT_X-1;
		
	if(snake[0].y == 0)
		snake[0].y = DEFAULT_Y-1;
	
	if(snake[0].x == food_x && snake[0].y == food_y)
	{
		Snake *tmp = snake;
		food_x = RESET;
		food_y = RESET;
		
		add_snake(tmp);
		snake = tmp;
		
	}
}

void spawn_food(Snake *snake, int length)
{
	int x,y;
	x = rand() % DEFAULT_X;
	y = rand() % DEFAULT_Y;
	
	for(int i=0;i<length;i++)
	{
		if(snake[i].x == x && snake[i].y == y)
		{
			x++;
			y++;
		}
	}
	
	food_x = x;
	food_y = y;
	
	
}

void print_inField(Snake *snake, char **field, int x, int y, int max)
{
	for(int i=0; i < x; i++)
	{
		for(int j=0; j < y; j++)
		{
			field[i][j] = ' ';
			if(j == 0 || j == DEFAULT_Y-1)
				field[i][j] = '|';
			if(i == 0 || i == DEFAULT_X-1)
				field[i][j] = '_';

		}
	}
	if(food_x != -1 && food_y != -1)
		field[food_x][food_y] = '#';

	for(int i=0;i<length;i++)
		field[snake[i].x][snake[i].y] = *snake[i].sign;
	
}

void routine(void *arg)
{
	Snake *snake = (Snake *)arg;
	while(poison)
	{

		usleep(speed);
		
		pthread_mutex_lock( &change_mutex );
		
		print_inField(snake, field, DEFAULT_X, DEFAULT_Y, length);
		
		move_Snake(snake,length);
		
		pthread_mutex_unlock( &change_mutex );
		
		change = 1;
		pthread_cond_signal( &change_cond );
		if(food_x == -1 || food_y == -1)
			spawn_food(snake,length);
	}
}



void init()
{
	

	
	int size_x = DEFAULT_X;
	int size_y = DEFAULT_Y;
	void *status;
	int erg=0;
	speed = 100000;
	srand(time(NULL));
	
	food_x = RESET;
	food_y = RESET;
	pthread_t tui_t,rou_t,inp_t;
	length = DEFAULT_LEN;
	snake = (Snake *)malloc(DEFAULT_X* DEFAULT_Y * sizeof(Snake));
	
	setvbuf(stdout, NULL, _IOLBF, DEFAULT_Y);
	
	
	for(int i=0;i<length;i++)
	{
		snake[i].x = (size_x/2);
		snake[i].y = (size_y/2)+i;
		snake[i].sign = "O";
	}
	
	field = init_field(size_x,size_y);
	
	for(int i=0;i<size_x;i++)
		for(int j=0;j<size_y;j++)
			field[i][j] = ' ';
		
	
	
	for(int i=0;i<length;i++)
			field[snake[i].x][snake[i].y] = *snake[i].sign;
			
	pthread_create(&tui_t, NULL,(void *(*)(void *)) tui, (void *) field);
	pthread_create(&rou_t, NULL,(void *(*)(void *)) routine, (void *) snake);
	pthread_create(&inp_t, NULL,(void *(*)(void *)) inputThread, NULL);
	
	erg = pthread_join(rou_t, &status ); 	
	erg = pthread_join(tui_t, &status );
	erg = pthread_join(inp_t, &status );
 	
 	free(snake);
}

int main()
{
	init();
	
}
