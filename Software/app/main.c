#include <system.h>
#include <sys/alt_stdio.h>
#include "opencores_i2c.h"


#define ADXL345_address 0x1D


int data;


int main(int argc, char *argv[])
{
    I2C_init(OPENCORES_I2C_0_BASE,ALT_CPU_CPU_FREQ,100000);

    data = I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,0);

    if ( data == 0){
        alt_printf("ADXL345 found at the address : 0x%x\n",ADXL345_address);
    }
    else {
        alt_printf("ADXL345 address problem\n");
    }

    while(1){

	}

    return 0;
}
