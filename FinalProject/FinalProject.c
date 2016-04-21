#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define F_CPU   16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "open_interface.h"
#include "lcd.h"
#include "util.h"

#define BAUD 57600

// clock is 2 MHz = 0.5 us / cycle#define PULSE_PERIOD 43860

int SERVO_CAL_MAX = 4500;
int SERVO_CAL_MIN = 970;

#define LINE_THRESHOLD 800
enum {CLEAR, BUMPER_LEFT, BUMPER_RIGHT, LINE_LEFT, LINE_FRONTLEFT, LINE_FRONTRIGHT, LINE_RIGHT, CLIFF_LEFT, CLIFF_FRONTLEFT, CLIFF_FRONTRIGHT, CLIFF_RIGHT} current_emergency;

volatile unsigned long rising = 0;
volatile unsigned long falling = 0;
volatile unsigned long overflows = 0;volatile enum {RISING, FALLING, DONE} state;


const char turn_speed = 70;
const char drive_speed = 100;
oi_t* sensor_data;

char transmit_buffer[60] = "";

ISR (TIMER1_CAPT_vect)
{
	cli();
	if (state == RISING)
	{
		rising = ICR1;
		state = FALLING;
		TCCR1B &= ~_BV(ICES1);
	}
	else if (state == FALLING)
	{
		falling = ICR1;
		TCCR1B |= _BV(ICES1);
		state = DONE;
	}
	sei();
}

ISR (TIMER1_OVF_vect)
{
	cli();
	overflows++;
	sei();
}

// /*
// USART FUNCTIONS
// */

void uart_init(void)
{
	cli();
	UBRR0H = 0;
	UBRR0L = 34;

	UCSR0A = (1 << U2X1);
	UCSR0B = (1 << RXEN) | (1 << TXEN);
	UCSR0C = (1 << USBS) | (1 << UCSZ0) | (1 << UCSZ1); // | (1 << UPM01); parity bits
	sei();
}

void uart_transmit(char data)
{
	cli();
	while (!(UCSR0A & (1 << UDRE)));
	UDR0 = (uint8_t) data;
	sei();
}

void uart_transmit_string(char* string)
{
	cli();
	while(*string != '\0')
	{
		uart_transmit(*string++);
	}
	sei();
}

unsigned char uart_receive(void)
{
	cli();
	while (!(UCSR0A & (1 << RXC0)));
	return UDR0;
	sei();
}

void uart_receive_string(char* destination)
{
	*destination = uart_receive();
	
	while (*destination != '\n') {
		destination++;
		*destination = uart_receive();
	}
	destination++;
	*destination = '\0';
}

//MOVEMENT FUNCTIONS

int emergency() {
	
	if (sensor_data->bumper_left) { return BUMPER_LEFT; }
	else if (sensor_data->bumper_right) { return BUMPER_RIGHT; }
	
	else if (sensor_data->cliff_left_signal > LINE_THRESHOLD) { return LINE_LEFT; }
	else if (sensor_data->cliff_frontleft_signal > LINE_THRESHOLD) { return LINE_FRONTLEFT; }
	else if (sensor_data->cliff_frontright_signal > LINE_THRESHOLD) { return LINE_FRONTRIGHT; }
	else if (sensor_data->cliff_right_signal > LINE_THRESHOLD) { return LINE_RIGHT; }
	
	else if (sensor_data->cliff_left) { return CLIFF_LEFT; }
	else if (sensor_data->cliff_frontleft) { return CLIFF_FRONTLEFT; }
	else if (sensor_data->cliff_frontright) { return CLIFF_FRONTRIGHT; }
	else if (sensor_data->cliff_right) { return CLIFF_RIGHT; }
	
	return CLEAR;
}

void move(int distance){
	current_emergency = CLEAR;
	int distance_moved = 0;
	int speed = ((distance > 0) * 2 - 1) * drive_speed;
	oi_set_wheels(speed, speed);
	while (abs(distance_moved) < abs(distance)) {
		oi_update(sensor_data);
		distance_moved += sensor_data->distance;
		current_emergency = emergency();
		if(current_emergency && distance > 0) { break; }
	}
	oi_set_wheels(0, 0);
	sprintf(transmit_buffer, "moved:%d\n", distance_moved);
	uart_transmit_string(transmit_buffer);
	if (current_emergency) {
		sprintf(transmit_buffer, "emergency:%d\n", current_emergency);
		uart_transmit_string(transmit_buffer);
	}
}

void rotate(int angle){
	int  angle_rotated = 0;
	if (angle > 0) {
		oi_set_wheels(turn_speed, -turn_speed);
	}
	else {
		oi_set_wheels(-turn_speed, turn_speed);
	}
	while (abs(angle_rotated) < abs(angle)) {
		oi_update(sensor_data);
		angle_rotated += sensor_data->angle;
	}
	oi_set_wheels(0, 0);
	
	sprintf(transmit_buffer, "rotated:%d\n", angle_rotated);
	uart_transmit_string(transmit_buffer);
}



//******************************************************************************** SWEEP FUNCTIONS (SERVO/IR/SONAR) *********************************************************************************
// /*
// SERVO FUNCTIONS
// */

void servo_init(void)
{
	OCR3A = PULSE_PERIOD-1;
	// OCR3B = SERVO_MIDDLE-1;
	TCCR3A |= _BV(COM3B1) | _BV(WGM31) | _BV(WGM30); // set COM and WGM (bits 3 and 2)
	TCCR3B |= _BV(CS31) | _BV(WGM33) | _BV(WGM32); // set WGM (bits 1 and 0) and CS
	DDRE |= _BV(PE4);
}

void move_servo_to_degrees(int degrees)
{
	int us = (int)(((long)((degrees - (-90))) * (long)(SERVO_CAL_MAX - SERVO_CAL_MIN)) / (long)(90 - (-90)) + SERVO_CAL_MIN);
	static int prev_us;
	OCR3B = us; // set pulse width
	int delta = us - prev_us;
	if (delta < 0) { delta = -delta; }
	if (delta < 10) { delta = 10; }
	wait_ms(delta);
	prev_us = us;
	
}

// /*
// IR SENSOR (ADC) FUNCTIONS
// */

void ADC_init()
{
	ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(REFS1) | _BV(MUX1);
	ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

unsigned int ADC_read()
{
	cli();
	ADCSRA |= _BV(ADSC); // start conversion
	while(ADCSRA & _BV(ADSC));	// wait for the conversion to finish
	return ADC; // return result
	sei();
}

unsigned int ADC_getDistance()
{
	return (unsigned int)(46074*pow(ADC_read(),-1.224));
}

// /*
// PING SENSOR
// */

void ping_init()
{
	cli();
	TCCR1B |= _BV(ICNC1) | _BV(ICES1) |_BV(CS11) | _BV(CS10);
	TIMSK |= _BV(TICIE1) | _BV(TOIE1);
	sei();
}

void ping_chirp()
{
	cli();
	DDRD |= _BV(PIND4);  //Set pin 4 port D as output
	PORTD |= _BV(PIND4); // set port D pin 4 high
	_delay_us(5);
	PORTD &= ~_BV(PIND4); // set port D pin 4 low
	DDRD &= ~_BV(PIND4);  //Set pin 4 port D as input
	TIFR |= _BV(ICF1) | _BV(TOV1);
	sei();
}

unsigned long ping_read()
{
	cli();
	state = RISING;
	ping_chirp();
	overflows = 0;
	while (state != DONE);
	unsigned long delta = 0;
	delta = overflows * 65535 + falling - rising;
	return delta;
}

int ping_getDistance()
{
	return (int)(ping_read() / 15);
}

void scan()
{
	wait_ms(1000);
	int IR_SAMPLES = 4;
	int US_SAMPLES = 1;
	
	typedef struct
	{
		int angle;
		int distance;
	} EdgeMeasurement;
	
	typedef struct
	{
		EdgeMeasurement leftEdge;
		EdgeMeasurement rightEdge;
		int linearWidth;
	} ObjectMeasurement;
	
	ObjectMeasurement objectArray[10];
	int objectsFound = 0;
	ObjectMeasurement foundObject;

	int ping_distance = 0;
	int long ir_distance = 0;
	
	int single_ping_distance = 0;
	int single_ir_distance = 0;
	
	int ir_measurements[181];
	int ping_measurements[181];

	// scanning -90 to 90 deg
	for (int i = 90; i >= -90; i--)
	{
		ping_distance = 0;
		
		// get distance from sensors
		for (int j = 0; j < US_SAMPLES; j++)
		{
			single_ping_distance = ping_getDistance();
			if (single_ping_distance > 80) { single_ping_distance = 80; }
			ping_distance += single_ping_distance;
		}
		ping_distance /= US_SAMPLES;
		
		ir_distance = 0;
		for (int j = 0; j < IR_SAMPLES; j++)
		{
			single_ir_distance = ADC_getDistance();
			if (single_ir_distance > 80) { single_ir_distance = 80; }
			ir_distance += single_ir_distance;
		}
		ir_distance /= IR_SAMPLES;
		
		// put measured data in an array
		ir_measurements[90+i] = ir_distance;
		ping_measurements[90+i] = ping_distance;
		
		// move servo
		move_servo_to_degrees(i);
	}
	
	enum { CLEAR, WORKING } search_state;
	search_state = CLEAR;
	
	for (int i = 90; i >= -90; i--)
	{
		if (search_state == CLEAR)
		{
			if ((ir_measurements[i+90] < 80) && (ping_measurements[i+90] < 80))
			{
				// left edge detected and next point is not 80 again, definitely an object, save it
				if (ir_measurements[i+90+1] < 80)
				{
					foundObject.leftEdge.angle = i;
					foundObject.leftEdge.distance = ping_measurements[i+90];
					search_state = WORKING;
				}
			}
		}
		if (search_state == WORKING)
		{
			if ((ir_measurements[i+90] >= 80) && (ping_measurements[i+90] < 80))
			{
				// right edge detected, save it
				foundObject.rightEdge.angle = i+1;
				foundObject.rightEdge.distance = ping_measurements[i+90+1];
				foundObject.linearWidth = (int) round(sqrt(pow(foundObject.leftEdge.distance, 2) + pow(foundObject.rightEdge.distance, 2) - 2.0 * foundObject.leftEdge.distance*foundObject.rightEdge.distance * cos(abs(foundObject.rightEdge.angle-foundObject.leftEdge.angle) * M_PI / 180.0)));
				objectArray[objectsFound++] = foundObject;
				
				foundObject.leftEdge.angle = 0;
				foundObject.leftEdge.distance = 0;
				foundObject.rightEdge.angle = 0;
				foundObject.rightEdge.distance = 0;
				foundObject.linearWidth = 0;
				
				search_state = CLEAR;
			}
		}
	}
	
	sprintf(transmit_buffer, "scan:%d\n", objectsFound);
	uart_transmit_string(transmit_buffer);
	
	for (int i = 0; i < objectsFound; i++)
	{
		sprintf(transmit_buffer, "object:%d,%d,%d,%d,%d\n", objectArray[i].leftEdge.angle, objectArray[i].leftEdge.distance, objectArray[i].rightEdge.angle, objectArray[i].rightEdge.distance, objectArray[i].linearWidth);
		uart_transmit_string(transmit_buffer);
	}
	
}


int main(void)
{
	init_push_buttons();
	shaft_encoder_init();
		
	char delimiter[2] = ":";
	lcd_init();
	ADC_init();
	ping_init();
	servo_init();
	uart_init();
	sensor_data = oi_alloc();
	oi_init(sensor_data);
	
	
	
	//////////// CALIBRATION START
	move_servo_to_degrees(90);
	while(read_push_buttons() != 1) {
		lprintf("CALIB +90 (CCW)\n%d, ", OCR3B);
		OCR3B += ((signed char) read_shaft_encoder()) * 10;
	}
	
	SERVO_CAL_MAX = OCR3B;
	
	move_servo_to_degrees(-90);
	while(read_push_buttons() != 1) {
		lprintf("CALIB +90 (CCW)\n%d, ", OCR3B);
		OCR3B += ((signed char) read_shaft_encoder()) * 10;
	}
	
	SERVO_CAL_MIN = OCR3B;
	
	//////////// CALIBRATION END
	
	
	lprintf("%s","running");
	
	current_emergency = CLEAR;
	
	char input_buffer[60] = "";
	int input_value = 0;
	
	char* input_command;
	
	while (1)
	{
		// wait for a command
		uart_receive_string(input_buffer);
		
		// split input string (colon)
		input_command = strtok(input_buffer, delimiter);
		input_value = atoi(strtok(NULL, delimiter));
		
		// process command
		if (strcmp(input_command, "move") == 0) {
			move(input_value);
			lprintf("move: %d", input_value);
		}
		else if (strcmp(input_command, "rotate") == 0) {
			rotate(input_value);
			lprintf("rotate: %d", input_value);
		}
		else if (strcmp(input_command, "scan") == 0) {
			scan();
			lprintf("scan");
		}
		else if (strcmp(input_command, "play") == 0) {
			// play(input_value);
			lprintf("play: %d", input_value);
		}
	}
}
