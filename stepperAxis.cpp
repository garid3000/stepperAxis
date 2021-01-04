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
	overrideSW0   = 1;
	overrideSW1   = 1;
	//-------------------------//
	if (uln_or_driver == 0){ //this means it's using uln stepper
		pinMode(pin1_or_dir, OUTPUT); 
		pinMode(pin2_or_stp, OUTPUT); 
		pinMode(pin3_or_ena, OUTPUT); 
		pinMode(pin4_or_slp, OUTPUT); 
		setSpeed(10);
	}
}
void StepperAxis::set_overrideSW0(uint8_t x){
	overrideSW0 = x;
}
uint8_t StepperAxis::get_overrideSW0(){
	return overrideSW0;
}
void StepperAxis::set_overrideSW1(uint8_t x){
	overrideSW1 = x;
}
uint8_t StepperAxis::get_overrideSW1(){
	return overrideSW1;
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
	uint8_t tmp = 0;
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	tmp = tmp + (digitalRead(sw0) == sw0Type);
	return tmp >= LWcheckThres;	
}

inline uint8_t StepperAxis::checkLim1(){
	if (sw0 == sw1){
		return curStep>=maxStep;
	}
	else{
		uint8_t tmp = 0;// delay(1);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		tmp = tmp + (digitalRead(sw1) == sw1Type);
		return tmp >= LWcheckThres;	
	}
}



inline uint8_t StepperAxis::checkLim(){
	return checkLim0() | checkLim1();	
}


long int StepperAxis::get_curStep(){
	return curStep;
}
long int StepperAxis::get_maxStep(){
	return maxStep;
}
void     StepperAxis::set_curStep(long int new_curStep){
	curStep = new_curStep;
}
void     StepperAxis::set_maxStep(long int new_maxStep){
	maxStep = new_maxStep;
}


inline uint8_t StepperAxis::adv1step(int8_t dir){
	uint8_t tmp_return = AT_NOR_POSITION;
	if (checkLim0()) tmp_return = AT_SW0_POSITION; // check whether at 0 position
	if (checkLim1()) tmp_return = AT_SW1_POSITION; // check whether at 1 position
	
	while (AXIS_SERIAL.available()){
		char tmp = AXIS_SERIAL.read();
		if (tmp == ';') return USR_TERMINATION;
	}
	if (uln_or_driver == 0){ //this means it's using uln stepper
		step(dir * directionType);
	}
	else{ 					 //this means it's using driver
		digitalWrite(pin1_or_dir, (dir * directionType) > 0);

		digitalWrite(pin2_or_stp, HIGH);
		delayMicroseconds(udelay);
		digitalWrite(pin2_or_stp, LOW);
		delayMicroseconds(udelay); 
		AXIS_SERIAL.print("1"); 
	}
	return tmp_return;
}

uint8_t StepperAxis::gotoLim0(){
	uint8_t tmp;
	if (overrideSW0 == 0){	//if it's overrided curStep may be at below 0
							//So we need to 1st make it into Positive curStep 
							//before we proceed to goto limit0 position
		gotoStep(1000);		
							//if 1000 isn't enought, YOU ARE ON YOUR OWN !
	}


	while(true){
		tmp = adv1step(-1);
		if (tmp == AT_SW0_POSITION){
			break;
		} else if (tmp == USR_TERMINATION){
			return USR_TERMINATION;
		}
	}
	delay(400);
	for (int i = 0; i < 400; i++){
		tmp = adv1step(1);
		if (tmp == USR_TERMINATION){
			return USR_TERMINATION;
		}
	}
	while(true){
		tmp = adv1step(-1);
		if (tmp == AT_SW0_POSITION){
			break;
		} else if (tmp == USR_TERMINATION){
			return USR_TERMINATION;
		}
	}
	curStep = 0;
	return AT_SW0_POSITION;
}


uint8_t StepperAxis::gotoLim1(){
	uint8_t tmp;
	while(true){
		tmp = adv1step(1);
		if (tmp == AT_SW1_POSITION){
			break;
		} else if (tmp == USR_TERMINATION){
			return USR_TERMINATION;
		}
	}
	delay(400);
	for (uint8_t i = 0; i < 250; i++){
		tmp = adv1step(-1);
		if (tmp == USR_TERMINATION){
			return USR_TERMINATION;
		}
	}
	while(true){
		tmp = adv1step(1);
		if (tmp == AT_SW1_POSITION){
			break;
		} else if (tmp == USR_TERMINATION){
			return USR_TERMINATION;
		}
	}
	curStep = maxStep;
	return AT_SW1_POSITION;
}


uint8_t StepperAxis::advStep(long int x){
	int8_t relativeDir = 1*(x>0) - 1*(x<0);
	uint8_t tmp;

	x = abs(x);
	for(long int i = 0; i < x; i++){
		tmp = adv1step(relativeDir);

		if(tmp == AT_SW0_POSITION){
			if(relativeDir < 0 && overrideSW0){
				curStep = 0; 
				return AT_SW0_POSITION;}
		}
		if(tmp == AT_SW1_POSITION){
			if(relativeDir > 0 && overrideSW1){
				curStep = maxStep; 
				return AT_SW1_POSITION;
			}
		}
		if (tmp == USR_TERMINATION) {
			curStep = curStep + x * (i); 
			return USR_TERMINATION;
		}
	}
	curStep = curStep + x * relativeDir;

	return AT_NOR_POSITION;
}
uint8_t StepperAxis::gotoStep(long int x){
	if (x <= 0) 			{return gotoLim0();}
	else if (x >= maxStep)	{return gotoLim1();}
	else{
		return advStep(x - curStep);
	}
}

void StepperAxis::print(char * str){
	AXIS_SERIAL.print(str);
}

void StepperAxis::handler() {
	char* argument = strtok(NULL, DELIMETERS);
	AXIS_SERIAL.println(argument);

	if (argument == NULL) {
		AXIS_SERIAL.println("No argument found");
		return;
	}
	else if (!strcmp(argument, "goto"  ))      {enable(); argument = strtok(NULL, DELIMETERS);long int tmp = atol(argument); gotoStep(tmp); disable();}
	else if (!strcmp(argument, "adv"   ))      {enable(); argument = strtok(NULL, DELIMETERS);long int tmp = atol(argument); advStep(tmp) ; disable();}
	else if (!strcmp(argument, "limit0"))      {enable(); gotoLim0(); disable();}
	else if (!strcmp(argument, "limit1"))      {enable(); gotoLim1(); disable();}
	else if (!strcmp(argument, "check0" ))     {AXIS_SERIAL.println(checkLim0());}
	else if (!strcmp(argument, "check1" ))     {AXIS_SERIAL.println(checkLim1());}
	else if (!strcmp(argument, "curStep"))     {AXIS_SERIAL.println(get_curStep());}
	else if (!strcmp(argument, "maxStep"))     {AXIS_SERIAL.println(get_maxStep());}
	else if (!strcmp(argument, "set_curStep")) {argument = strtok(NULL, DELIMETERS);long int tmp = atol(argument); set_curStep(tmp);}
	else if (!strcmp(argument, "set_maxStep")) {argument = strtok(NULL, DELIMETERS);long int tmp = atol(argument); set_maxStep(tmp);}

					 //disabling the steppers

	AXIS_SERIAL.println("done");
}