#include <system.h>
#include <sys/alt_stdio.h>
#include <unistd.h>
#include "opencores_i2c.h"


#define ADXL345_address 0x1D
#define ADXL345_DATAX0  0x32
#define ADXL345_DATAX1  0x33
#define ADXL345_DATAY0  0x34
#define ADXL345_DATAY1  0x35
#define ADXL345_DATAZ0  0x36
#define ADXL345_DATAZ1  0x37


int chip_adress = -1, data;
int DATAX0, DATAX1, x_value;
int DATAY0, DATAY1, y_value;
int DATAZ0, DATAZ1, z_value;


int main(int argc, char *argv[])
{
    I2C_init(OPENCORES_I2C_0_BASE,ALT_CPU_CPU_FREQ,100000);

    chip_adress = I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,0);

    if ( chip_adress == 0){
        alt_printf("ADXL345 found at the address : 0x%x\n", ADXL345_address);
    }
    else if ( chip_adress == 1){
        alt_printf("ADXL345 not found\n", ADXL345_address);
    }
    else {
        alt_printf("Communication pb\n");
    }

    while(1){
        I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,0);      //Start bit + slave address + write

        I2C_write(OPENCORES_I2C_0_BASE,ADXL345_DATAX0,0);       //Register
        I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,1);      //Start + slave address + read
        DATAX0 =  I2C_read(OPENCORES_I2C_0_BASE,0);             //Collect data

        I2C_write(OPENCORES_I2C_0_BASE,ADXL345_DATAX1,0);       //Register
        I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,1);      //Start + slave address + read
        DATAX1 =  I2C_read(OPENCORES_I2C_0_BASE,1);             //Collect last data

        x_value = (DATAX1 << 8) | DATAX0;
        alt_printf("X axis : 0x%x\n", x_value);

        usleep(1000000);
	}

    return 0;
}
