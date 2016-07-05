#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termio.h>
#include <string.h>
 
int main(void)
{
    int fd;
    fd=open("/dev/ttyUSB0", O_RDWR | O_NOCTTY );  // 컨트롤 c 로 취소안되게 하기 | O_NOCTTY
    assert(fd != -1);
    
    struct termios newtio;
    // newtio <-- serial port setting.
    memset(&newtio, 0, sizeof(struct termios));
    newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;//속도
    newtio.c_iflag    = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG);
    
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
 
    //
    const char *str = "serial program. sending text from linux to window \r\n";
    write(fd, str, strlen(str)+1);

    close(fd);
    return 0;
}
