#include <system.h>
#include <sys/alt_stdio.h>
#include <unistd.h>
#include "opencores_i2c.h"
#include <alt_types.h>
#include <stdio.h>
#include <altera_avalon_pio_regs.h>
#include <math.h>


//Registers
#define ADXL345_address 0x1D
#define ADXL345_DATAX0  0x32
#define ADXL345_DATAX1  0x33
#define ADXL345_DATAY0  0x34
#define ADXL345_DATAY1  0x35
#define ADXL345_DATAZ0  0x36
#define ADXL345_DATAZ1  0x37


//Variables
//	ADXL345
int chip_adress = -1;

//	I2C
int i2c_data;

//	Acceleration
unsigned int DATAX0, DATAX1, x_unsigned;
unsigned int DATAY0, DATAY1, y_unsigned;
unsigned int DATAZ0, DATAZ1, z_unsigned; 
int x_signed, y_signed, z_signed;
int X_g, Y_g, Z_g;
enum axis { X_axis, Y_axis, Z_axis};
float mg_LSB = 4.3;						//For the default range : + or - 2g			

//	7 segments
int c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0;
int sev_seg = 0;


//Functions
int comp2(unsigned int value){
	if(value & 0x8000){
		c2 = -(((~value)& 0xFFFF) + 1);
	}
	else {
		c2 = value;
	}
	
	return c2;
}

unsigned int read_byte(int reg){
	I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,0);      //Start bit + slave address + write
	I2C_write(OPENCORES_I2C_0_BASE,reg,0);       			//Register
	I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,1);      //Start + slave address + read
	i2c_data =  I2C_read(OPENCORES_I2C_0_BASE,1);             	//Collect last data
	
	return i2c_data;
}

void read_axis(enum axis a){
	int register0, register1;
	int DATA0=0, DATA1=0;
	if (a == X_axis){
		register0 = ADXL345_DATAX0;
		register1 = ADXL345_DATAX1;
	}
	else if (a == Y_axis){
		register0 = ADXL345_DATAY0;
		register1 = ADXL345_DATAY1;
	}
	else if (a == Z_axis){
		register0 = ADXL345_DATAZ0;
		register1 = ADXL345_DATAZ1;
	}
	else {
		alt_printf("Problem\n");
	}
	
	DATA0 = read_byte(register0);
	DATA1 = read_byte(register1);

	if (a == X_axis){
		DATAX0 = DATA0;
		DATAX1 = DATA1;
	}
	else if (a == Y_axis){
		DATAY0 = DATA0 ;
		DATAY1 = DATA1;
	}
	else if (a == Z_axis){
		DATAZ0 = DATA0 ;
		DATAZ1 = DATA1;
	}
	else {
		alt_printf("Problem\n");
	}
}

void sev_seg_print(int value){
	if (value<0){
		c5 = 10;
		c4 = -value /10000;
		c3 = (-value / 1000) % 10;
		c2 = (-value /100) % 10;
		c1 = (-value/10) % 10;
		c0 = -value % 10;
	}
	else{
		c5 = 11;
		c4 = value /10000;
		c3 = (value / 1000) % 10;
		c2 = (value /100) % 10;
		c1 = (value/10) % 10;
		c0 = value % 10;
	}

	sev_seg = (c5 << 20) + (c4 << 16) +(c3 << 12) + (c2 << 8) + (c1 <<4) + c0;
	
	//Write the number on the 7 segment
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE,sev_seg);
}

void axis_calc(enum axis a, unsigned int value0, unsigned int value1){
	int data_unsigned = 0, data_signed = 0, data_mg = 0;
	
	data_unsigned = (value1 << 8) | value0;
	
	data_signed = comp2(data_unsigned);
	
	data_mg = round( mg_LSB * data_signed );								//Value in milli g
	
	if (a == X_axis){
		x_unsigned = data_unsigned;
		x_signed = data_signed;
		X_g = data_mg;
	}
	else if (a == Y_axis){
		y_unsigned = data_unsigned;
		y_signed = data_signed;
		Y_g = data_mg;
	}
	else if (a == Z_axis){
		z_unsigned = data_unsigned;
		z_signed = data_signed;
		Z_g = data_mg;
	}
	else {
		alt_printf("Problem\n");
	}
}

void UART_print(enum axis a){
	if (a == X_axis){
		printf("x_unsigned : %d\t\t", x_unsigned);
		printf("x_signed : %d\t\t", x_signed);
		printf("X_g  : %d\n", X_g);
	}
	else if (a == Y_axis){
		printf("y_unsigned : %d\t\t", y_unsigned);
		printf("y_signed : %d\t\t", y_signed);
		printf("Y_g  : %d\n", Y_g);
	}
	else if (a == Z_axis){
		printf("z_unsigned : %d\t\t", z_unsigned);
		printf("z_signed : %d\t\t", z_signed);
		printf("Z_g  : %d\n", Z_g);
	}
	else {
		alt_printf("Problem\n");
	}
}


//Main
int main(int argc, char *argv[])
{
	//I2C initialisation
    I2C_init(OPENCORES_I2C_0_BASE,ALT_CPU_CPU_FREQ,100000);

	//Search ADXL345
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
		//Read the ADXL345 value
        read_axis(X_axis);   
        read_axis(Y_axis); 
        read_axis(Z_axis);        
        
        //Calculate the 2 complement and value in m/s²
        axis_calc(X_axis, DATAX0, DATAX1);
        axis_calc(Y_axis, DATAY0, DATAY1);
        axis_calc(Z_axis, DATAZ0, DATAZ1);
		
		//Print the values via UART
		UART_print(X_axis);
		UART_print(Y_axis);
		UART_print(Z_axis);
		printf("\n");
		
		//Print on the 7 segments
		sev_seg_print(X_g);

		//Delay
        usleep(250000);
	}

    return 0;
}
