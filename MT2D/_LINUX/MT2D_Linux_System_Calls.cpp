#if defined(linux) &&  !defined(__ANDROID__)
#include "../MT2D_Terminal_Define.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

void MT2D_Linux_System_Delay(int time_millisecond) {
	usleep(time_millisecond);
}


void MT2D_Linux_System_Beep(int frequency, int time_milisecond) {
	/* start beep */
	ioctl(2, KIOCSOUND, 1193180 / frequency);//http://www.hardware.com.br/comunidade/beep-linux/730836/
											 /* sleep for the duration time  */
	usleep(time_milisecond * 1000);
	/* stop beep */
	ioctl(2, KIOCSOUND, 0);

}
#endif