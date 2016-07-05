#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termio.h>
#include <termios.h>
#include <term.h>
#include <string.h>
#include <curses.h>

static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard();
void close_keyboard();
int kbhit();
int readch();
 
int main(void)
{
	//keyboard 입력 인식
	int ch = 0;
	init_keyboard();


    int fd_send;
    //fd_send=open("/dev/ttyUSB0", O_RDWR | O_NOCTTY );  // 컨트롤 c 로 취소안되게 하기 | O_NOCTTY
    fd_send=open("/dev/ttyUSB0", O_RDWR ); //device 열기
    
    assert(fd_send != -1);
    
    struct termios newtio;//serial port setting하기
    // newtio <-- serial port setting.
    memset(&newtio, 0, sizeof(struct termios));
    newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;//속도
    newtio.c_iflag    = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG);
    
    tcflush(fd_send, TCIFLUSH);
    tcsetattr(fd_send, TCSANOW, &newtio);
 
    //
    const char *str = "serial program. sending text from linux to window \r\n";
    write(fd_send, str, strlen(str)+1);

	//while(1)
	while(ch != 'q')
	{
		//printf("looping\n");
		//sleep(1);
		if(kbhit())
		{
			ch = readch();
			printf("you hit %c\n",ch);
			write(fd_send, &ch ,2);
		}
	}

	close_keyboard();
	exit(0);
    close(fd_send);
    return 0;
}

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


