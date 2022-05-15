void kernelstart();

//this will be loaded first at boot
void _start(void)
{
	kernelstart();
}

#include "test.h"
#include "cpubasics.h"
#include "stdlib.h"

void kernelstart()
{
	screen::clear();
	screen::print("hewwo ");
	cpubasics::cpuinit();
	for(int i = 12; i < 5000; i+=12) {
		memcpy((char*)RAMSCREEN + i, (char*)RAMSCREEN, 12);
	}
    while(1) {

    }
}