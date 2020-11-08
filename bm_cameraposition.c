

#include <stdio.h>
#include <signal.h>
#include <stdlib.h> // for atoi
#include <unistd.h>
#include <rc/i2c.h>
#include <curses.h>
#include <math.h>

#define I2CBUS 1
#define MCP 0x27
#define PORTA 0x12
#define PORTB 0x13
#define DELAY 5			/* twise time in ms for one step								*/
#define SWEEP 200
#define OFFSET_YAW 35		/* Offset for Cameraposition in Yaw-Axis					*/
#define OFFSET_PITCH 10		/* Offset for Cameraposition in Pitch-Axis					*/

#define POSITION_MAX 100
#define POSITION_MIN -100


void zero_yaw(int *number_steps);	/* Prototype of Function							*/
void zero_pitch(int *number_steps);
int step_yaw(int number_steps, int *all_steps, int max_pos_steps, int max_neg_steps);
int step_pitch(int number_steps, int *all_steps, int max_pos_steps, int max_neg_steps);
int kbhit(void);

static int running = 0;


int kbhit(void){    /* Function "keybordhit" gives a 1 back     */ 
                    /* when a key is pressed            */
                    /* otherwise a 0.               */          
    int ch = getch();

    if(ch != ERR) {
        ungetch(ch);
        return 1;
    } else{
        return 0;
    }
}


void zero_yaw(int *number_steps){
	/* First running in anticlovkwise direction and checking the endswitch STEP1_END1	*/
	/* Then running SWEEP/2 +/- offset steps for middle position in clockwise direction.*/

	int i;
	uint8_t flag_end1;
	uint8_t data;
	int move = 1;

	rc_i2c_read_byte(I2CBUS, PORTA, &flag_end1); /* Read STEP1_END1 for the first time. */
	flag_end1 = flag_end1 & 0x01;	/* mask the unnessecary bits						*/ 

	while (move == 1){	/* First movement to max. anticlockwise position				*/
		rc_i2c_read_byte(I2CBUS, PORTA, &data);	/* Read Port A and store content in data */
		data = data & 0x01;
		if(data ^ flag_end1){	/* Endposition of STEP1_END1 reached.						*/
			move = 0;		/* Stopp movement											*/
			usleep(500 * DELAY);
		}else{
			usleep(500 * DELAY);
			rc_i2c_write_byte(I2CBUS, PORTB, 0x01);	/* DIR = 0 , STEP = 1 of Steppermotor 0 */
			usleep(500 * DELAY);
			rc_i2c_write_byte(I2CBUS, PORTB, 0x00);	/* DIR = 0 , STEP = 0 of Steppermotor 0 */
			usleep(500 * DELAY);
		}
	}

						/* Second movement in the middle between STEP1_END1 and STEP1_END2.	*/
	for(i = 0; i < (SWEEP/2 + OFFSET_YAW); i++){
		rc_i2c_write_byte(I2CBUS, PORTB, 0x03);	/* DIR = 1 , STEP = 1 of Steppermotor 0 */
		usleep(500 * DELAY);
		rc_i2c_write_byte(I2CBUS, PORTB, 0x02);	/* DIR = 1 , STEP = 0 of Steppermotor 0 */
		usleep(500 * DELAY);
	}

	*number_steps = SWEEP;
}


void zero_pitch(int *number_steps){
	/* First running in up direction and checking the endswitch STEP2_END1			*/
	/* Then running SWEEP/2 +/- offset steps for middle position in down direction.	*/

	int i;
	uint8_t flag_end1;
	uint8_t data;
	int move = 1;

	rc_i2c_read_byte(I2CBUS, PORTA, &flag_end1); /* Read STEP1_END1 for the first time. */
	flag_end1 = flag_end1 & 0x04;	/* mask the unnessecary bits						*/ 

	while (move == 1){	/* First movement to max. anticlockwise position				*/
		rc_i2c_read_byte(I2CBUS, PORTA, &data);	/* Read Port A and store content in data */
		data = data & 0x04;
		if(data ^ flag_end1){	/* Endposition of STEP2_END1 reached.						*/
			move = 0;		/* Stopp movement											*/
			usleep(500 * DELAY);
		}else{
			usleep(500 * DELAY);
			rc_i2c_write_byte(I2CBUS, PORTB, 0x0C);	/* DIR = 1 , STEP = 1 of Steppermotor 2 */
			usleep(500 * DELAY);
			rc_i2c_write_byte(I2CBUS, PORTB, 0x08);	/* DIR = 1 , STEP = 0 of Steppermotor 2 */
			usleep(500 * DELAY);
		}
	}

						/* Second movement in the middle between STEP2_END1 and STEP2_END2.	*/
	for(i = 0; i < (SWEEP/2 + OFFSET_PITCH); i++){
		rc_i2c_write_byte(I2CBUS, PORTB, 0x04);	/* DIR = 0 , STEP = 1 of Steppermotor 2 */
		usleep(500 * DELAY);
		rc_i2c_write_byte(I2CBUS, PORTB, 0x00);	/* DIR = 0 , STEP = 0 of Steppermotor 2 */
		usleep(500 * DELAY);
	}

	*number_steps = SWEEP;
}


int step_yaw(int number_steps, int *all_steps, int max_pos_steps, int max_neg_steps){
	/* Run the steppermotor for the yaw axis in case that number_steps plus *all_steps	*/
	/* in a range given by max_neg_steps and max_pos_steps. If not function gives a -1 	*/
	/* back. Positive values in number_steps move the yaw-axis anticlockwise, negativ	*/
	/* values clockwise. The updated value in *all_steps are given back.				*/
	int i;
	int count;

	if(*all_steps + number_steps > max_pos_steps){
		return -1;
	}
	if(*all_steps - number_steps < max_neg_steps){
		return -1;
	}
	*all_steps = *all_steps + number_steps;	/* Updated value in *all_steps				*/
	if (number_steps > 0){
    	for(i = 0; i < number_steps; i++){
        	rc_i2c_write_byte(I2CBUS, PORTB, 0x03); /* DIR = 1 , STEP = 1 of Steppermotor 1 */
        	usleep(500 * DELAY);
        	rc_i2c_write_byte(I2CBUS, PORTB, 0x02); /* DIR = 1 , STEP = 0 of Steppermotor 1 */
        	usleep(500 * DELAY);
		}
    }else{
		count = -number_steps;
    	for(i = 0; i < count; i++){
        	rc_i2c_write_byte(I2CBUS, PORTB, 0x01); /* DIR = 0 , STEP = 1 of Steppermotor 1 */
        	usleep(500 * DELAY);
        	rc_i2c_write_byte(I2CBUS, PORTB, 0x00); /* DIR = 0 , STEP = 0 of Steppermotor 1 */
        	usleep(500 * DELAY);
		}
    }

	return 0;

}

int step_pitch(int number_steps, int *all_steps, int max_pos_steps, int max_neg_steps){
	/* Run the steppermotor for the pitch axis in case that number_steps plus *all_steps	*/
	/* in a range given by max_neg_steps and max_pos_steps. If not function gives a -1 	*/
	/* back. Positive values in number_steps move the yaw-axis anticlockwise, negativ	*/
	/* values clockwise. The updated value in *all_steps are given back.				*/
	int i;
	int count;

	if(*all_steps + number_steps > max_pos_steps){
		return -1;
	}
	if(*all_steps - number_steps < max_neg_steps){
		return -1;
	}
	*all_steps = *all_steps + number_steps;	/* Updated value in *all_steps				*/
	if (number_steps > 0){
    	for(i = 0; i < number_steps; i++){
        	rc_i2c_write_byte(I2CBUS, PORTB, 0x0C); /* DIR = 1 , STEP = 1 of Steppermotor 2 */
        	usleep(500 * DELAY);
        	rc_i2c_write_byte(I2CBUS, PORTB, 0x08); /* DIR = 1 , STEP = 0 of Steppermotor 2 */
        	usleep(500 * DELAY);
		}
    }else{
		count = -number_steps;
    	for(i = 0; i < count; i++){
        	rc_i2c_write_byte(I2CBUS, PORTB, 0x04); /* DIR = 0 , STEP = 1 of Steppermotor 2 */
        	usleep(500 * DELAY);
        	rc_i2c_write_byte(I2CBUS, PORTB, 0x00); /* DIR = 0 , STEP = 0 of Steppermotor 2 */
        	usleep(500 * DELAY);
		}
    }

	return 0;

}


// interrupt handler to catch ctrl-c
static void __signal_handler(__attribute__ ((unused)) int dummy)
{
	running=0;
	return;
}

int main()
{

	int res;
	int number_steps;
	int c;
	int row, col;
	int yaw, pitch;
	int delta_pos;

	// set signal handler so the loop can exit cleanly
	signal(SIGINT, __signal_handler);
	running = 1;

	if((res = rc_i2c_init(I2CBUS, MCP)) != 0){
		printf("Can not open I2C-Dev-File\n");
		return -1;
	}


	/* Preparation of the Portexpander MCP23017 */

	rc_i2c_write_byte(I2CBUS, 0x00, 0xff);	/* all pins of Port A are inputs */
	rc_i2c_write_byte(I2CBUS, 0x01, 0x00);	/* all pins of Port B are outputs */

 	zero_yaw(&number_steps);
//	printf("Referencing yaw done \n");
 	zero_pitch(&number_steps);
 	zero_pitch(&number_steps);
//	printf("Referencing pitch done \n");


	initscr();
	keypad (stdscr, TRUE);
	noecho();
	cbreak();
	nodelay(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	
	c = 0;
	getmaxyx(stdscr, row, col);

	mvprintw (2, (col/2)-20, "CONTROL-PANEL FOR A CAMERAPOSITION-CONTROLLER");
	mvprintw (4, (col/2)-6, "CHANGE POSITION WITH");
	mvprintw (8, col/2, " ");
	addch(ACS_UARROW);
	mvprintw (9, (col/2)-2, " ");
	addch(ACS_LARROW);
	mvprintw (9, (col/2)+2, " ");
	addch(ACS_RARROW);
	mvprintw (10, col/2, " ");
	addch(ACS_DARROW);
	mvprintw (12, (col/2)-5, "Stopp with 'q'");
	refresh();
	yaw = 0;
	pitch = 0;
	delta_pos = 1;			/* Number of steps		*/


	while (c != 'q' && running == 1) {
		if (!kbhit()){
			mvprintw (row-4, (col/2)-9, "Setpoint Yaw --> %*d\n\n", 3, yaw );
			mvprintw (row-3, (col/2)-9, "Setpoint Pitch --> %*d\n\n", 3, pitch);
			refresh();

		}
		else{
			c = getch();
			switch(c) {
				case KEY_RIGHT:
					if ((yaw + delta_pos) < POSITION_MAX){
 						step_yaw(delta_pos, &yaw, POSITION_MAX, POSITION_MIN);
					}
				break;
				case KEY_LEFT:
					if ((yaw - delta_pos) > POSITION_MIN){
						step_yaw(-delta_pos, &yaw, POSITION_MAX, POSITION_MIN);
					}
				break;
				case KEY_UP:
					if ((pitch + delta_pos) < POSITION_MAX){
						step_pitch(delta_pos, &pitch, POSITION_MAX, POSITION_MIN);
					}
				break;
				case KEY_DOWN:
					if ((pitch  - delta_pos) > POSITION_MIN){
						step_pitch(-delta_pos, &pitch, POSITION_MAX, POSITION_MIN);
					}
				break;

			}
		}
	}
	endwin();

	rc_i2c_close(I2CBUS);
	return 0;

}


