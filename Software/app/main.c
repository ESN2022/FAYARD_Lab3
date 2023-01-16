#include <system.h>
#include <sys/alt_stdio.h>
#include <unistd.h>
#include "opencores_i2c.h"
#include <alt_types.h>
#include <stdio.h>
#include <altera_avalon_pio_regs.h>
#include <math.h>
#include <sys/alt_irq.h>
#include <alt_types.h>
#include <altera_avalon_timer_regs.h>
#include <altera_avalon_timer.h>


//Registers
#define ADXL345_address 0x1D
#define ADXL345_OFSX  	0x1E
#define ADXL345_OFSY  	0x1F
#define ADXL345_OFSZ  	0x20
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
int i2c_data = 0;

//	Acceleration
unsigned int DATAX0 = 0, DATAX1 = 0, x_unsigned = 0;
unsigned int DATAY0 = 0, DATAY1 = 0, y_unsigned = 0;
unsigned int DATAZ0 = 0, DATAZ1 = 0, z_unsigned = 0;
int x_signed = 0, y_signed = 0, z_signed = 0;
int X_g = 0, Y_g = 0, Z_g = 0;
enum axis { X_axis, Y_axis, Z_axis};
float mg_LSB = 3.9;						//Typical for the default range : + or - 2g
int comp_2 = 0;

//	7 segments
int c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0;
int sev_seg = 0;

//	Push button IRQ
int print_sev_seg = 0;

//	Timer IRQ
int timer_irq_flag = 0;

//	Offsets
int X_offset = 3;
int Y_offset = 6;
int Z_offset = -1;


//Functions
int comp2(unsigned int value){
	if(value & 0x8000){                         //If the MSB is 1 (negative number)
		comp_2 = -(((~value) & 0xFFFF) + 1);    //2-complement
	}
	else {
		comp_2 = value;
	}

	return comp_2;
}

void write_byte(int reg, int data){
	I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,0);      //Start bit + slave address + write bit
	I2C_write(OPENCORES_I2C_0_BASE,reg,0);       			//Register to write in
	I2C_write(OPENCORES_I2C_0_BASE,data,1);       			//Write data + stop bit
}

unsigned int read_byte(int reg){
	I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,0);      //Start bit + slave address + write
	I2C_write(OPENCORES_I2C_0_BASE,reg,0);       			//Register to read on
	I2C_start(OPENCORES_I2C_0_BASE,ADXL345_address,1);      //Start + slave address + read bit
	i2c_data =  I2C_read(OPENCORES_I2C_0_BASE,1);           //Collect last data (stop bit)

	return i2c_data;
}

void read_axis(enum axis a){
	int register0 = 0, register1 = 0;
	int DATA0 = 0, DATA1 = 0;

	//Assign the register to read, based on the chosen axis
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
		alt_printf("Problem read_axis\n");
	}

	//Read the LSB and MSB register value
	DATA0 = read_byte(register0);
	DATA1 = read_byte(register1);

	//Assign the read data on the corresponding values
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
		alt_printf("Problem read_axis\n");
	}
}

void sev_seg_print(int value){
	if (value<0){
		c5 = 10;                                        //Corresponds to "-" in the VHDL (for negative numbers)
		c4 = -value /10000;
		c3 = (-value / 1000) % 10;
		c2 = (-value /100) % 10;
		c1 = (-value/10) % 10;
		c0 = -value % 10;
	}
	else{
		c5 = 11;                                        //Corresponds to nothing on in the VHDL (for positive numbers)
		c4 = value /10000;
		c3 = (value / 1000) % 10;
		c2 = (value /100) % 10;
		c1 = (value/10) % 10;
		c0 = value % 10;
	}

	//Number to send to the 7-segment PIO
	sev_seg = (c5 << 20) + (c4 << 16) +(c3 << 12) + (c2 << 8) + (c1 <<4) + c0;

	//Write the number on the 7 segment
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE,sev_seg);
}

void axis_calc(enum axis a, unsigned int value0, unsigned int value1){
	int data_unsigned = 0, data_signed = 0, data_mg = 0;

	data_unsigned = (value1 << 8) | value0;

	data_signed = comp2(data_unsigned);

	data_mg = round( mg_LSB * data_signed );			//Value in milli g

	//Assign the calculated data on the corresponding values
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
		alt_printf("Problem axis_calc\n");
	}
}

void UART_print(enum axis a){
	if (a == X_axis){
		//printf("x_unsigned : %d\t\t", x_unsigned);
		//printf("x_signed : %d\t\t", x_signed);
		printf("X_g  : %d\n", X_g);
	}
	else if (a == Y_axis){
		//printf("y_unsigned : %d\t\t", y_unsigned);
		//printf("y_signed : %d\t\t", y_signed);
		printf("Y_g  : %d\n", Y_g);
	}
	else if (a == Z_axis){
		//printf("z_unsigned : %d\t\t", z_unsigned);
		//printf("z_signed : %d\t\t", z_signed);
		printf("Z_g  : %d\n", Z_g);
	}
	else {
		alt_printf("Problem UART_print\n");
	}
}

static void push_b_IRQ (void * context, alt_u32 id)
{
	//Choose the axis to display with the push button 1 (0 = X ; 1 = Y; 2 = Z)
	if (print_sev_seg < 2){
		print_sev_seg = print_sev_seg + 1;
	}
	else{
		print_sev_seg = 0;
	}

	//Clear interruption
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_1_BASE,0x01);
}

void push_b_IRQ_init(){
    //Set the input that trigger interrupts (slide buttons)
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PIO_1_BASE, 0x01);

	//Reset the edge capture register
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_1_BASE,0x01);

	//Register the ISR to the corresponding interrupt
	alt_irq_register (PIO_1_IRQ, NULL, (void*) push_b_IRQ);
}

static void timer_IRQ (void * context, alt_u32 id)
{
    //Timer irq flag raised
    timer_irq_flag = 1;

	//Clear the interrupt
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0x00);
}

void timer_IRQ_init(){
	//Clear the interrupt
	IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0x00);

	//Settle parameters : continous count, IRQ request + Start
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE , ALTERA_AVALON_TIMER_CONTROL_CONT_MSK | ALTERA_AVALON_TIMER_CONTROL_START_MSK | ALTERA_AVALON_TIMER_CONTROL_ITO_MSK);

	//Register the ISR to the corresponding interrupt
	alt_irq_register (TIMER_0_IRQ , NULL, (void*) timer_IRQ);
}

void set_offset(enum axis a, int value){
	int reg_offset;

	value = value & 0xFFFF;

	//Assign the register to set the offset, based on the chosen axis
	if (a == X_axis){
		reg_offset = ADXL345_OFSX;
	}
	else if (a == Y_axis){
		reg_offset = ADXL345_OFSY;
	}
	else if (a == Z_axis){
		reg_offset = ADXL345_OFSZ;
	}
	else {
		alt_printf("Problem set_offset\n");
	}
    write_byte(reg_offset,value);
}

void print_offset(){
	alt_printf("X offset: %x\t;",read_byte(ADXL345_OFSX));
    alt_printf("\tY offset: %x\t",read_byte(ADXL345_OFSY));
    alt_printf("\tZ offset: %x\n",read_byte(ADXL345_OFSZ));
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

    //Push button interruption init
	push_b_IRQ_init();

	//Timer interruption init
	timer_IRQ_init();

	//Axis offsets
	print_offset();
    set_offset(X_axis, X_offset);
    set_offset(Y_axis, Y_offset);
    set_offset(Z_axis, Z_offset);
    print_offset();


    while(1){
		if (timer_irq_flag == 1) {
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
			if (print_sev_seg == 0){
				sev_seg_print(X_g);
			}
			else if (print_sev_seg == 1){
				sev_seg_print(Y_g);
			}
			else {
				sev_seg_print(Z_g);
			}

            //Clear the timer interrupt flag
			timer_irq_flag = 0;
		}
	}

    return 0;
}
