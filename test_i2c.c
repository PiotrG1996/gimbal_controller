/**
 * @file bm_test_i2c.c
 *
 * Test the i2c connection with device file /dev/i2c-1 and the i2c clinent addre 24 (hex) of the
 * BeagleBone Blue.
 * 2020.01.03 Bruemmer
 */


#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for atoi
#include <unistd.h>
#include <rc/i2c.h>


#define I2CBUS 1
#define MCP 0x25
#define PORTA 0x12
#define PORTB 0x13


static int running = 0;


// interrupt handler to catch ctrl-c
static void __signal_handler(__attribute__ ((unused)) int dummy)
{
	running=0;
	return;
}

int main(int argc, char **argv)
{
	int res = 0;
	int delay;
	int steps;
	int dir;
	int i;

	if(argc != 4){
		printf("Please call the programm with 'bm_test_i2c d ssss \n");
		printf("d -> direction 0 = clockwise 1 = anticlockwise \n");
		printf("ssss -> number od steps \n");
		printf("wwww -> delay in ms after every step \n");
		return -1;
	}
	// set signal handler so the loop can exit cleanly
	signal(SIGINT, __signal_handler);
	running = 1;

	sscanf(argv[1], "%d", &dir);
	if(dir == 0){
		dir = 0;
	}
	else{
		dir = 1;
	}					/* limitation of dir -> only 0 or 1	*/

	sscanf(argv[2], "%d", &steps);
	if(steps < 0){
		printf("Please give a positive number of steps!\n");
		return -1;
	}

	sscanf(argv[3], "%d", &delay);
	if(delay < 0){
		printf("Please give a positive delay!\n");
		return -1;
	}

	if((res = rc_i2c_init(I2CBUS, MCP)) != 0){
		printf("Can not open I2C-Dev-File\n");
		return -1;
	}

	/* Preparation of the Portexpander MCP23017 */

	rc_i2c_write_byte(I2CBUS, 0x00, 0xff);	/* all pins of Port A are inputs */
	rc_i2c_write_byte(I2CBUS, 0x01, 0x00);	/* all pins of Port B are outputs */


	for(i = 0; i < steps; i++){
		if(dir == 0){
			rc_i2c_write_byte(I2CBUS, PORTB, 0x01);	/* DIR = 0 , STEP = 1 of Steppermotor 0 */
			usleep(500 * delay);
			rc_i2c_write_byte(I2CBUS, PORTB, 0x00);	/* DIR = 0 , STEP = 0 of Steppermotor 0 */
			usleep(500 * delay);
		}

		if(dir == 1){
			rc_i2c_write_byte(I2CBUS, PORTB, 0x03);	/* DIR = 1 , STEP = 1 of Steppermotor 0 */
			usleep(500 * delay);
			rc_i2c_write_byte(I2CBUS, PORTB, 0x02);	/* DIR = 1 , STEP = 0 of Steppermotor 0 */
			usleep(500 * delay);
		}
		 
	}

	rc_i2c_close(I2CBUS);
	return 0;

}


