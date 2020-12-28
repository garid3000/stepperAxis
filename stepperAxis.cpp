#include "Arduino.h"
#include "stepperAxis.h"
#include "Stepper.h"


StepperAxis::StepperAxis(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4, 
							uint8_t s0, uint8_t s1 = 255): Stepper(rotStepsforULN, in1, in2, in3, in4)
{
	pin1_or_dir = in1;
	pin2_or_stp = in2;
	pin3_or_ena = in3;
	pin4_or_slp = in4;
	sw0 = s0;
	sw1 = s1*(s1!=255) + s0*(s1==255);
} 

void StepperAxis::init(	int8_t dirType    = 1,
						uint8_t enType     = 1,
						uint8_t slpType    = 1, 
						uint8_t driverType = 1, 
						uint8_t s0type     = 1, 
						uint8_t s1type     = 1,
						int    ud = 1000){

	directionType = dirType;		
	enableType    = enType;			
	sleepType     = slpType;			
	uln_or_driver = driverType;
	sw0Type       = s0type;
	sw1Type       = s1type;
	udelay        = ud;
	//-------------------------//
	if (uln_or_driver == 0){ //this means it's using uln stepper
		pinMode(pin1_or_dir, OUTPUT); 
		pinMode(pin2_or_stp, OUTPUT); 
		pinMode(pin3_or_ena, OUTPUT); 
		pinMode(pin4_or_slp, OUTPUT); 
		setSpeed(10);
	}
}

inline void StepperAxis::disable(){
	if (uln_or_driver == 0){ //this means it's using uln stepper
		digitalWrite(pin1_or_dir, !enableType);
		digitalWrite(pin2_or_stp, !enableType);
		digitalWrite(pin3_or_ena, !enableType);
		digitalWrite(pin4_or_slp, !enableType);
	}
	else{
		digitalWrite(pin3_or_ena, !enableType);
		digitalWrite(pin4_or_slp, !sleepType);
	}
}

inline void StepperAxis::enable(){
	if (uln_or_driver == 0){ //this means it's using uln stepper
		//nothing;
	}
	else{
		digitalWrite(pin3_or_ena, enableType);
		digitalWrite(pin4_or_slp, sleepType);
	}
}

inline uint8_t StepperAxis::checkLim0(){
	uint8_t tmp = 0;// delay(1);
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	tmp = tmp + (~((uint8_t)digitalRead(sw0)^sw0Type))&0x01;
	return tmp >= LWcheckThres;	
}

inline uint8_t StepperAxis::checkLim1(){
	if (sw0 == sw1){
		return curStep>=maxStep;
	}
	else{
		uint8_t tmp = 0;// delay(1);
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		tmp = tmp + (~((uint8_t)digitalRead(sw1)^sw1Type))&0x01;
		return tmp >= LWcheckThres;	
	}
}



inline uint8_t StepperAxis::checkLim(){
	return checkLim0() | checkLim1();	
}


long int StepperAxis::get_curStep(){
	return curStep;
}
void     StepperAxis::set_curStep(long int new_curStep){
	curStep = new_curStep;
}
void     StepperAxis::set_maxStep(long int new_maxStep){
	maxStep = new_maxStep;
}


inline uint8_t StepperAxis::adv1step(int8_t dir){
	if (checkLim0()) return AT_SW0_POSITION; // check whether at 0 position
	if (checkLim1()) return AT_SW1_POSITION; // check whether at 1 position
	

	enable();				 //enabling steppers
	if (uln_or_driver == 0){ //this means it's using uln stepper
		step(dir * directionType);
	}
	else{ 					 //this means it's using driver
		digitalWrite(pin1_or_dir, (dir * directionType) > 0);

		digitalWrite(pin2_or_stp, HIGH);
		delayMicroseconds(udelay);
		digitalWrite(pin2_or_stp, LOW);
		delayMicroseconds(udelay);  
	}
	disable();				 //disabling the steppers
	return AT_NOR_POSITION;
}

uint8_t StepperAxis::gotoLim0(){
	uint8_t tmp;
	while(true){
		tmp = adv1step(-1);
		if (tmp == AT_SW0_POSITION){
			break;
		}
	}
	delay(400);
	for (uint8_t i = 0; i < 250; i++){
		tmp = adv1step(1);
	}
	while(true){
		tmp = adv1step(-1);
		if (tmp == AT_SW0_POSITION){
			break;
		}
	}
}


uint8_t StepperAxis::gotoLim1(){
	uint8_t tmp;
	while(true){
		tmp = adv1step(1);
		if (tmp == AT_SW1_POSITION){
			break;
		}
	}
	delay(400);
	for (uint8_t i = 0; i < 250; i++){
		tmp = adv1step(-1);
	}
	while(true){
		tmp = adv1step(1);
		if (tmp == AT_SW1_POSITION){
			break;
		}
	}
}


uint8_t StepperAxis::advStep(long int x){
	;
}
uint8_t StepperAxis::gotoStep(long int x){
	;
}