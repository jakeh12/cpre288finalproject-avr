/*
 * movement.c
 *
 * Created: 1/29/2016 12:34:27 PM
 *  Author: pbonnie
 */ 
#include "open_interface.h"
#include "movement.h"
#include "lcd.h"
#include "util.h"
#include <math.h>


typedef struct config_struct{
	float driveScalar;
	float turnScalar;//close to 1; bigger scalar = smaller turn
}config;

int BOT_NUM = 2;
config BOT_CONFIG;
config bot_configs[12];

void initConfigs(){
	bot_configs[2].driveScalar = 1.05;
	bot_configs[2].turnScalar = 1.00;//1.0054;//speed:scalar;200:1.13;100:
	BOT_CONFIG = bot_configs[BOT_NUM];
}

void main() {
	lcd_init();
	oi_t *sensor_data = oi_alloc();
	oi_init(sensor_data);
	initConfigs();
	int speed = 180;
	//lprintf("%d",(int)(100*BOT_CONFIG.turnScalar));
	
	//driveStraight(100,200,sensor_data);
	
	for(int i = 0;i<1;i++){
		driveTurn(180,speed,sensor_data);
		wait_ms(200);
	}
	oi_free(sensor_data);
}

driveStraight(int distance, int speed, oi_t* sensor_data){
		oi_init(sensor_data);
		int sum = 0;
		oi_set_wheels(speed, speed); // move forward
		while (fabs(sum*BOT_CONFIG.driveScalar) < abs(distance*10)) {
			oi_update(sensor_data);
			sum += sensor_data->distance;
		}
		oi_set_wheels(0, 0); // stop
		wait_ms(100);
}

driveTurn(int angle, int speed, oi_t* sensor_data){
		oi_init(sensor_data);
		float sum = 0;
		if(angle>0){
			oi_set_wheels(-speed, speed); // right
		}else{			
			oi_set_wheels(speed, -speed); // left
		}
		while (fabs(sum*BOT_CONFIG.turnScalar) < abs(angle)) {
			oi_update(sensor_data);
			sum -= sensor_data->angle;
		}
		oi_set_wheels(0, 0); // stop
		oi_update(sensor_data);
		sum -= sensor_data->angle;
		lprintf("%d",sum);
		wait_ms(100);
}

int driveTillBump(int distance, int speed, oi_t* sensor_data){
	oi_init(sensor_data);
	int sum = 0;
	oi_set_wheels(speed, speed); // move forward
	while (sensor_data->bumper_right != 1 && sensor_data->bumper_left != 1 && sum*1.05 < distance) {
		oi_update(sensor_data);
		sum += sensor_data->distance;
	}
	oi_set_wheels(0, 0); // stop
	wait_ms(100);
	return sum*1.05;
}



//void init() {
	////oi_t *sensor_data = oi_alloc();
	////oi_init(sensor_data);
	//
	//UBRR1L = 16; // UBRR = (FOSC/16/BAUD-1);
	//UCSR1B = (1 << RXEN) | (1 << TXEN);
	//UCSR1C = (3 << UCSZ10);
	//
	//oi_byte_tx(128);
	//
	//oi_byte_tx(129); // baud code for 28800
	//oi_byte_tx(8); // baud code for 28800
	//wait_ms(100);
	//
	//UBRR1L = 33; // UBRR = (FOSC/16/BAUD-1);
	//
	//oi_byte_tx(132);
	//
	//oi_set_leds(1, 1, 7, 255);
//}

//void turn180_ccw() {
	//
			//oi_byte_tx(152);
			//oi_byte_tx(60);
			//
			///*
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(200);
			//oi_byte_tx(0);
			//oi_byte_tx(1);
			//
			//
			//oi_byte_tx(157);
			//oi_byte_tx(0);
			//oi_byte_tx(172);
			//
			//
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//
			//oi_byte_tx(155);
			//oi_byte_tx(20);
			//
			//oi_byte_tx(153);
			//*/
			//
			//for (int i = 0; i < 4; i++)
			//{
				//oi_byte_tx(137);//move motors
				//oi_byte_tx(0);//speed
				//oi_byte_tx(65);//speed
				//oi_byte_tx(0);
				//oi_byte_tx(1);
				//
				//
				//oi_byte_tx(157);
				//oi_byte_tx(0);
				//oi_byte_tx(180-4); // actual angle
				//
				//
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//// wait .1 second units
				//oi_byte_tx(155);
				//oi_byte_tx(20);
			//}
			//
			//oi_byte_tx(153);
			//
			//
			//
//}
//
//void turn90_ccw() {
	//
			//oi_byte_tx(152);
			//oi_byte_tx(60);
			//
			///*
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(200);
			//oi_byte_tx(0);
			//oi_byte_tx(1);
			//
			//
			//oi_byte_tx(157);
			//oi_byte_tx(0);
			//oi_byte_tx(172);
			//
			//
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//
			//oi_byte_tx(155);
			//oi_byte_tx(20);
			//
			//oi_byte_tx(153);
			//*/
			//
			//for (int i = 0; i < 4; i++)
			//{
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(65);
				//oi_byte_tx(0);
				//oi_byte_tx(1);
				//
				//
				//oi_byte_tx(157);
				//oi_byte_tx(0);
				//oi_byte_tx(90-2); // actual angle
				//
				//
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//// wait .1 second units
				//oi_byte_tx(155);
				//oi_byte_tx(20);
			//}
			//
			//oi_byte_tx(153);
			//
			//
			//
//}
//
//void turn90_cw() {
	//
			//oi_byte_tx(152);
			//oi_byte_tx(60);
			//
			///*
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(200);
			//oi_byte_tx(0);
			//oi_byte_tx(1);
			//
			//
			//oi_byte_tx(157);
			//oi_byte_tx(0);
			//oi_byte_tx(172);
			//
			//
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//
			//oi_byte_tx(155);
			//oi_byte_tx(20);
			//
			//oi_byte_tx(153);
			//*/
			//
			//for (int i = 0; i < 4; i++)
			//{
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(65);
				//oi_byte_tx(0xFF);
				//oi_byte_tx(0xFF);
				//
				//
				//oi_byte_tx(157);
				//oi_byte_tx(0xFF);
				//oi_byte_tx(0xA6+2); // actual angle
				//
				//
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//// wait .1 second units
				//oi_byte_tx(155);
				//oi_byte_tx(20);
			//}
			//
			//oi_byte_tx(153);
			//
			//
			//
//}
//
//
//void turn22_5_ccw() {
	//
			//oi_byte_tx(152);
			//oi_byte_tx(60);
			//
			///*
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(200);
			//oi_byte_tx(0);
			//oi_byte_tx(1);
			//
			//
			//oi_byte_tx(157);
			//oi_byte_tx(0);
			//oi_byte_tx(172);
			//
			//
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//
			//oi_byte_tx(155);
			//oi_byte_tx(20);
			//
			//oi_byte_tx(153);
			//*/
			//
			//for (int i = 0; i < 4; i++)
			//{
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(65);
				//oi_byte_tx(0);
				//oi_byte_tx(1);
				//
				//
				//oi_byte_tx(157);
				//oi_byte_tx(0);
				//oi_byte_tx(23-2); // actual angle
				//
				//
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//// wait .1 second units
				//oi_byte_tx(155);
				//oi_byte_tx(20);
			//}
			//
			//oi_byte_tx(153);
			//
			//
			//
//}
//
//void turn45_ccw() {
	//
			//oi_byte_tx(152);
			//oi_byte_tx(60);
			//
			///*
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(200);
			//oi_byte_tx(0);
			//oi_byte_tx(1);
			//
			//
			//oi_byte_tx(157);
			//oi_byte_tx(0);
			//oi_byte_tx(172);
			//
			//
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//
			//oi_byte_tx(155);
			//oi_byte_tx(20);
			//
			//oi_byte_tx(153);
			//*/
			//
			//for (int i = 0; i < 4; i++)
			//{
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(65);
				//oi_byte_tx(0);
				//oi_byte_tx(1);
				//
				//
				//oi_byte_tx(157);
				//oi_byte_tx(0);
				//oi_byte_tx(45-2); // actual angle
				//
				//
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//// wait .1 second units
				//oi_byte_tx(155);
				//oi_byte_tx(20);
			//}
			//
			//oi_byte_tx(153);
			//
			//
			//
//}
//
//void turn45_cw() {
	//
			//oi_byte_tx(152);
			//oi_byte_tx(60);
			//
			///*
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(200);
			//oi_byte_tx(0);
			//oi_byte_tx(1);
			//
			//
			//oi_byte_tx(157);
			//oi_byte_tx(0);
			//oi_byte_tx(172);
			//
			//
			//oi_byte_tx(137);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//oi_byte_tx(0);
			//
			//oi_byte_tx(155);
			//oi_byte_tx(20);
			//
			//oi_byte_tx(153);
			//*/
			//
			//for (int i = 0; i < 4; i++)
			//{
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(65);
				//oi_byte_tx(0xFF);
				//oi_byte_tx(0xFF);
				//
				//
				//oi_byte_tx(157);
				//oi_byte_tx(0xFF);
				//oi_byte_tx(0xD3 + 2); // actual angle
				//
				//
				//oi_byte_tx(137);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//oi_byte_tx(0);
				//// wait .1 second units
				//oi_byte_tx(155);
				//oi_byte_tx(20);
			//}
			//
			//oi_byte_tx(153);
			//
			//
			//
//}

//int main() {
	//
	//init();
	//
	//turn90_cw();
	//wait_ms(20000);
	//turn90_ccw();
	//
	//
	//
	//
//
	//return 0;
//}