#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>

struct termios tio; // 터미널 구조체

int main(int argc, char *argv[]) {
	int fd, result; //fd 파일 디스크립터, result : 파일 RW 결과값
	long baud = B38400; //통신속도
	unsigned char buf; // 데이터 받을 buf -> unsigned를 안붙이면 값이 이상하게 나온다.
	char sBuffer[7]={0x02, 0x03, 0x49, 0x56, 0x00, 0x03, 0x1C}; // 보낼 데이터 (현재 뚜리가 사용하고 있는 키트의 정의된 프로토콜 형식


	if((fd = open("/dev/ttyUSB0", O_RDWR|O_NDELAY|O_NOCTTY)) < 0) { // READ, WRITE로 Serial0 포트 열기
		exit(1);
	}

	////////////////// 보낼 옵션 설정

	tio.c_cflag = baud|CS8|CREAD|CLOCAL; // baud 통신 속도, CS8 (8bit, No Parity, 1 Stop Bit)설정
	// CREAD 문자 수신가능하게 함, CLOCAL, Local Connection 모뎀제어 안함..
	//처음엔 CRTSCTS를 같이 c_cflag에 줬었다. 그래서 정상적으로 작동이 안되서 상당히 얘를 먹었다.
	// CRCTSCTS는 하드웨어 흐름제어, 직렬 케이블의 모든 선이 연결되어 있을 때 사용 -> 보통 모든선을 다 안쓰므로. ^^
	tio.c_cflag &= ~HUPCL;
	tio.c_lflag = 0;  // Local Mode 설정
	tio.c_iflag = IGNPAR; // Parity 오류가 있는 문자 무시
	tio.c_oflag = 0; // 출력처리 설정 0이면 아무것도 안함

	///////////////// 옵션 설정 끝 (물론 추가적인 옵션도 많이 있으나 찾아보기 바란다.)

	tcflush(fd, TCIFLUSH); // 설정을 초기화
	tcsetattr(fd, TCSANOW, &tio); // tio 터미널 구조체의 속정으로 설정을 적용시킨다.
	
	fcntl(fd, F_SETFL, FNDELAY); // 열려있는 파일 제어를 위해 사용

	result = write(fd, sBuffer, 7); // 실제적으로 시리얼로 데이터를 보낸다. sBuffer값의 7개만 시리얼0으로 보낸다.

	if(result < 0) { // 에러냐?
		printf("write error\n");
		close(fd); 
		exit(1);
	}

	usleep(1000); // 쉬고

	while(1) { // 데이터 받는 부분.. 오는거 계속 출력한다. 주구장창.. ^^
		if((result = read(fd, &buf, 1)) > 0) {
			printf("%02x ", buf);
			fflush(stdout); // 뿌려주3.
		}
		
	}

	close(fd); // 끝내기 닫고
}
