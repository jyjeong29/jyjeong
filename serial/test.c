//변수 통합 이후 버전
//fd -> fd_get 은 실패함
	//pollfd event 등등으로 인해...

// polling 에서 timeout을 0으로 하면 되긴 하는데
// 중간 중간 data flow가 끊겨서 프로그램이 먹통됨....

// 7월 4일 기준 timeout을 0으로 하면 문제 없음을 확인 함
// 문제는 가끔씩 프로그램이 먹통되는 문제 발생
// 원인을 모르겠기때문에 좀 더 오류를 해결하는 방안에대해서 찾아볼 필욕 있음

//각각의 header file이 어떤 함수를 포함하는지 정리 해 둘 것

//7월14일 termios구조체 분석
//terminal <-> teraterm 간 통신 중 freezing
//왜 이런 문제 생기는지 모르겠음 : 항상 teraterm만 멈춤
//teraterm 설정은 Auto//cr 로 설정하면 됨
 

// teraterm에서 enter를 입력하면, linux terminal에서 get 출력시 enter 자동 적용
//-> 해당 문제는 아래에서 해결 가능하지만 terminal에서 spacebar랑 enter 구분 안되는 문제 발생

#include <stdio.h> //standard input output
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>

#include <fcntl.h> // O_RDWR , O_NOCTTY 등의 상수 정의
#include <term.h>
#include <termio.h>
#include <termios.h> // B115200, CS8 등 상수 정의
#include <curses.h>
#include <assert.h>



//keyboard input 설정
static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard();// keyboard initialization
void close_keyboard();// stop using keyboard
int kbhit();// start to use keyboard 
int readch();// read data from keyboard input 


int main( void)
{

//############### keyboard init(키보드 초기화)
	init_keyboard();

//########################### //serial port open

	int fd_send,fd;
	fd_send = fd = open("/dev/ttyUSB0", O_RDWR ); //device 열기
	//fd = open( "/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK );
	// O_RDWR와 같은 option의 의미를 모르겠다.


//########################### //의미를 모르겠는 data들


//########################### //port configure

	//keyboard 입력 인식
	int ch = 0;

	//assert(fd_send != -1);//이건 왜 있는 건가

	// 구조체
	struct termios newtio;//serial port setting하기
	// termios구조체에는 장치 파일 설정에 관한 것이 저장 되어 있다.

	//c_iflag : input -> 모든 입력 처리 정의(read() 함수로 인해 시리얼 포트로 들어온 데이터)

	//c_oflag : output -> 출력 처리 방법 정의
	//c_cflag : configuration -> port setting 정의
	//c_lflag : echo 설정
	//c_cc 배열 : EOF, STOP 등의 제어 동작을 어떤 문자로 할지 정의 termios.h 에 저장 되어 있음

	//newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;//NO-rts/cts
	//newtio.c_iflag = IGNPAR | ICRNL;//IGNPAR=non-parity
	//newtio.c_oflag = 0;
	//newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG);


//##########################
//get data from teraterm configure
	//int    ndx;
	int    cnt;
	char   buf[1024]="\0";//

	memset( &newtio, 0, sizeof(newtio) );
	newtio.c_cflag      = B115200 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag		= IGNPAR | ICRNL;//IGNPAR=non-parity
	newtio.c_oflag      = 0;
	//newtio.c_lflag      = 0;
	newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG);//해당 설정을 하지 않으면 teraterm에서 enter 인식이 안됨
	newtio.c_cc[VTIME]  = 0;//inter-character timer unused
	newtio.c_cc[VMIN]   = 0;//blocking read until 1 character arrives
					//read configuration - 해당 설정을 통해서, 1문자만 입력 받던
					//teraterm -> terminal 1문자만 입력받던 문제를 해결할 수 있다.


	//clean all data
	tcflush(fd_send|fd, TCIFLUSH);
     //tcflush  (fd, TCIFLUSH );

	//port setting 종료
	tcsetattr(fd_send|fd, TCSANOW, &newtio);
     //tcsetattr(fd, TCSANOW, &newtio);
	// fcntl(fd, F_SETFL, FNDELAY);


//############################
// poll 사용을 위한 configuration

	struct pollfd     poll_events;      // 체크할 event 정보를 갖는 struct
	int    poll_state;
	poll_events.fd        = fd;
	poll_events.events    = POLLIN | POLLERR; //수신된 자료가 있는지, 에러가
	poll_events.revents   = 0;




//############################
//inform communication started by sending or getting text below

// send
// text from linux -> teraterm
// 두가지 방법이 존재
 
	const char *str = "serial program. sending text from linux to window \r";
	write(fd_send, str, strlen(str)+1);

	write(fd,"serial communication start",27);
	write(fd,"\r",1);

//############################
// error message
	if ( 0 > fd) {
		printf("open error\n");
		return -1; }
//############################
// when linux input data is 'q', program is over

	while (ch != 'q')
	{

//############################
// send
// linux -> teraterm
// 이게 interrupt 방식 아닌가?
// 아니네 잘 생각해보면 이것도 결국엔 interrupt가 아니다
// interrupt는 interrupt handler에 저장을 해 놓으면
// 자동으로 실행되는 개념이다.

		if(kbhit())
		{
			ch = readch();
			printf("send : %c\n\r",ch);
			write(fd_send, &ch ,4);
		}
//send data end

//#############################################################
// start polling
// get
// teraterm -> linux
                poll_state = poll( // poll()을 호출하여 event 발생 여부 확인     
                                (struct pollfd*)&poll_events, // event 등록 변수
                                1, // 체크할 pollfd 개수
				1
                                );

                if ( 0 < poll_state) // 발생한 event 가 있음
                {
                        if(poll_events.revents & POLLIN) // event 가 자료 수신?
                        {
                                cnt = read(fd, buf, 1024); //terminal에서 입력한 data
                                write( fd, buf, cnt);
                                printf( " get : %d %s\r\n", cnt, buf);
                        }

                        // 여긴 발생할 일 별로 없음
                        if( poll_events.revents & POLLERR) // event 가 에러?
                        {
                                printf( "통신 라인에 에러가 발생, 프로그램 종료");
                                break;
                        }
                }
        }

//##################### //program 종료 안내
	str = "\rprogram is over\r";//str에 문자열 저장
	write(fd_send, str, strlen(str)+1);// teraterm에 str 출력

//##################### //keyboard랑 port 닫아주기
	close_keyboard();//keyboard 닫기 : 안해주면 키보드 먹통 됨
	close(fd_send);
	close(fd);//fd_send, fd mmemory?? 닫아주기
	exit(0);
	return 0;
}

//keyboard 설정 초기화
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

//keyboard port 종료
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





