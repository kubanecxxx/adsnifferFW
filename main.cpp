#include "ch.hpp"
#include "hal.h"
#include "usb_user.h"
#include "sampler.h"

void* operator new(size_t sz)
{
	return chCoreAlloc(sz);
}

void operator delete(void* m)
{
	(void) m;
}

int main(void)
{
	chibios_rt::System::Init();
	halInit();

	sampler sampl;
	palSetGroupMode(GPIOD,0b1111,12,PAL_MODE_OUTPUT_PUSHPULL);

	chThdSetPriority(LOWPRIO);
	usb_user_thread(NULL);


	return 1;
}


