#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <termios.h> // B115200, CS8 등 상수 정의
#include <termio.h>
#include <term.h>
#include <fcntl.h> // O_RDWR , O_NOCTTY 등의 상수 정의
#include <curses.h>


// keyboard input
static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard();
void close_keyboard();
int kbhit();
int readch();
int mygetch();
char getkey();

//serial communication function
int main(void)
{       
	int    fd;
	//int    ndx;
	int    cnt;
	int ch = 0; // integer for keyboard function
	char   buf[1024];
	char key;
	struct termios    newtio;
	struct pollfd     poll_events;      // 체크할 event 정보를 갖는 struct
	int    poll_state;

	// 시리얼 포트를 open
	fd = open( "/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK ); 

	//error 설정
	if ( 0 > fd)
	{        
		printf("open error\n");
		return -1;
	}

	// 시리얼 포트 통신 환경 설정

	memset( &newtio, 0, sizeof(newtio) );
	newtio.c_cflag       = B9600 | CS8 | CLOCAL | CREAD;
	newtio.c_oflag       = 0;
	newtio.c_lflag       = 0;
	newtio.c_cc[VTIME]   = 0;
	newtio.c_cc[VMIN]    = 1;
   
	tcflush  (fd, TCIFLUSH );
	tcsetattr(fd, TCSANOW, &newtio );
	fcntl(fd, F_SETFL, FNDELAY); 

	// poll 사용을 위한 준비
	poll_events.fd        = fd;
	poll_events.events    = POLLIN | POLLERR; //수신된 자료가 있는지, 에러가 있는지
	poll_events.revents   = 0;

	
	// keyboard initialize
	init_keyboard();
	

	// 자료 송수신 시작
	//write(fd, "forum.falinux.com", 17);
	write(fd,"serial communication start",26);
	write(fd,"\n\r",2);

	while (1)
	{
		key = getkey();
		if(key != '\0')
			printf("%c",key);
		
		//keyboard 입력linux2win
		//printf("looping\n\r"); //terminal에 출력
		//sleep(1);
		/*
		key = getkey();
		if(key != '\0')
		{
			printf("%c",key);
			write(fd,key,1);
		}
		*/

		if(kbhit())
		{
			ch = readch();
			printf("you hit %c",ch);
			printf("\n\r");
			write(fd,"linux2win %c",ch);
			printf("\n\r");
		}
		

		poll_state = poll( // poll()을 호출하여 event 발생 여부 확인     
				(struct pollfd*)&poll_events, // event 등록 변수
				1, // 체크할 pollfd 개수
				1000   // time out 시간
				);

		// win2linux
		if ( 0 < poll_state) // 발생한 event 가 있음
		{
			if(poll_events.revents & POLLIN) // event 가 자료 수신?
			{
				cnt = read(fd, buf, 1024); //terminal에서 입력한
				write( fd, buf, cnt);
				printf("data received - %d : %s", cnt, buf);
				printf("\r");
			}

			// 여긴 발생할 일 별로 없음
			if( poll_events.revents & POLLERR) // event 가 에러?
			{
				printf("통신 에러 발생, 프로그램 종료\n\r");
				break;
			}
		}

		// 프로그램 종료
		if(ch=='q')// terminal에서 q를 입력하면 프로그램 종료
		break;
	}
	close(fd);
	close_keyboard();
	exit(0);
	return 0;
}

//keyboard input function
void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}
void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

int kbhit()
{
    char ch;
    int nread;

    if(peek_character != -1)
        return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);

    if(nread == 1) {
        peek_character = ch;
        return 1;
    }
    return 0;
}
int readch()
{
    char ch;

    if(peek_character != -1) {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

int mygetch()
{
	struct termios oldt,
	newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

char getkey()
{
	if(kbhit())
	{
		return mygetch();
	}
	return '\0';
}
