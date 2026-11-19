#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f4xx.h"
#include "delay.h"

typedef enum{
	Angle_0 = 0,
	Angle_45,
	Angle_90,
	Angle_135,
	Angle_180,	
}ServoAngle;

extern void Servo_Init();
extern void Servo_Demo();

#endif