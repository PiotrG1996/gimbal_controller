/**
 * @file bm_test_endswitch.c
 *
 * Test the i2c connection with device file /dev/i2c-1 and the i2c client addr 24 (hex) of the
 * BeagleBone Blue. Readoperation form the endswitch end1 via Portexpander  MCP 
 * 2020.01.07 Bruemmer
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for atoi
#include <unistd.h>
#include <rc/i2c.h>
#include <rc/led.h>
#include <rc/start_stop.h>


#define I2CBUS 1
#define MCP 0x25
#define PORTA 0x12
#define PORTB 0x13

static int running = 1;


// interrupt handler to catch ctrl-c
static void __signal_handler(__attribute__ ((unused)) int dummy)
{
	running=0;
	return;
}


int main(){

	signal(SIGINT, __signal_handler);
	int res;
	uint8_t data;

	// start with LEDs off
	if(rc_led_set(RC_LED_GREEN, 0)==-1){
		fprintf(stderr, "ERROR in rc_blink, failed to set RC_LED_GREEN\n");
		return -1;
	}

	if((res = rc_i2c_init(I2CBUS, MCP)) != 0){
		printf("Can not open I2C-Dev-File\n");
		return -1;
	}

	/* Preparation of the Portexpander MCP23017 */

	rc_i2c_write_byte(I2CBUS, 0x00, 0xff);	/* all pins of Port A are inputs */
	rc_i2c_write_byte(I2CBUS, 0x01, 0x00);	/* all pins of Port B are outputs */


	while(running){
		rc_i2c_read_byte(I2CBUS, PORTA, &data);	/* Read Port A and store content in data */
		if(data && 0x01){
			rc_led_set(RC_LED_GREEN,1);
		}
		else{
			rc_led_set(RC_LED_GREEN,0);
		}

	}

	rc_led_set(RC_LED_GREEN, 0);
	rc_led_cleanup();
	return 0;
}
