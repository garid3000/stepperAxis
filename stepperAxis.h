#ifndef GARIDS_STEPPER_AXIS
#define GARIDS_STEPPER_AXIS


#ifndef AXIS_SERIAL
#define AXIS_SERIAL Serial1
#endif


#ifndef LWcheckThres
#define LWcheckThres 4
#endif

#ifndef DELIMETERS
#define DELIMETERS " ,\r\n\t"
#endif


#define AT_SW0_POSITION 10
#define AT_SW1_POSITION 11
#define AT_NOR_POSITION 0
#define USR_TERMINATION 20



#define rotStepsforULN 2048
#include "Arduino.h"
#include <Stepper.h>


class StepperAxis: public Stepper{
private:
	//hardware connections
	uint8_t pin1_or_dir;
	uint8_t pin2_or_stp;
	uint8_t pin3_or_ena;
	uint8_t pin4_or_slp;
	uint8_t sw0, sw1;

	//hard-soft TYPE related	// Default
	int8_t directionType;		// 1 normal			   , -1 means reverse
	uint8_t enableType;			// 1 means high-working, 0 means low-working
	uint8_t sleepType;			// 1 means high-working, 0 means low-working
	uint8_t uln_or_driver;		// 1 means driver      , 0 means uln driver

								// Default	|	Value	
	uint8_t sw0Type;			// (1)		|	1 means the sensor outputs HIGH when sensed
	uint8_t sw1Type;			// (1)		|	0 means LOW when sensed

								// Default	|	Value
	uint8_t overrideSW0;		// (1)		|	1 means won't go below When sw0 sensed
	uint8_t overrideSW1;		// (1)		|	0: WARNING: it overrides & goes when sw0 sensed

	int udelay;
	//software related
	long int curStep;
	long int maxStep;
public:
    StepperAxis(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

	void init(int8_t=1, uint8_t=1, uint8_t=1, uint8_t=1, uint8_t=1, uint8_t=1, int=1000);
	void    set_overrideSW0(uint8_t);
	void    set_overrideSW1(uint8_t);
	uint8_t get_overrideSW0();
	uint8_t get_overrideSW1();

	inline void disable();
	inline void enable();
	inline uint8_t checkLim0();
	inline uint8_t checkLim1();
	inline uint8_t checkLim();

	long int get_curStep();
	long int get_maxStep();
	void     set_curStep(long int);
	void     set_maxStep(long int);


	inline uint8_t adv1step(int8_t);

	uint8_t gotoLim0();
	uint8_t gotoLim1();

	uint8_t advStep(long int);
	uint8_t adv_then_limit0(long int);
	uint8_t goto_then_limit0(long int);
	uint8_t gotoStep(long int);
	uint8_t gotoStep_sudo(long int);
    void    handler();
    void    print(char *);


};


#endif
