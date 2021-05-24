/* Author: Jeffrey Wang jwang619@ucr.edu
 * Lab Section: 22
 * Assignment: Lab # 10  Exercise # 4
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 *
 *  Demo Link:
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;

	OCR1A = 125;

	TIMSK1 = 0x02;

	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;
	if(frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }

		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }

		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

unsigned char GetKeypadKey() {
	PORTC = 0xEF;
	asm("nop");
	if(GetBit(PINC, 0) == 0) { return('1'); }
	if(GetBit(PINC, 1) == 0) { return('4'); }
	if(GetBit(PINC, 2) == 0) { return('7'); }
	if(GetBit(PINC, 3) == 0) { return('*'); }

	PORTC = 0xDF;
	asm("nop");
	if(GetBit(PINC, 0) == 0) { return('2'); }
	if(GetBit(PINC, 1) == 0) { return('5'); }
        if(GetBit(PINC, 2) == 0) { return('8'); }
        if(GetBit(PINC, 3) == 0) { return('0'); }

	PORTC = 0xBF;
	asm("nop");
	if(GetBit(PINC, 0) == 0) { return('3'); }
        if(GetBit(PINC, 1) == 0) { return('6'); }
        if(GetBit(PINC, 2) == 0) { return('9'); }
        if(GetBit(PINC, 3) == 0) { return('#'); }

	PORTC = 0x7F;
        asm("nop");
        if(GetBit(PINC, 0) == 0) { return('A'); }
        if(GetBit(PINC, 1) == 0) { return('B'); }
        if(GetBit(PINC, 2) == 0) { return('C'); }
        if(GetBit(PINC, 3) == 0) { return('D'); }


	return('\0');
}

typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct) (int);
} task;

unsigned char i = 0x00;
unsigned char code[5] = {'#','1','2','3','4'};
enum SMStates {SMStart, Init, Wait, Check, Reset, Unlock} ;

int SMFunc(int state) {
	unsigned char x;
	unsigned char outputB = 0x00;

	outputB = PORTB;
	x = GetKeypadKey();

	switch(state) {
                case SMStart:
                        state = Init;
                       
                        break;
                case Init:
                        state = Wait;
                       
                        break;
                case Wait:
			if(x == '\0') {
				state = Wait;
			}
                        
			else if(x == code[i]) {
                                state = Check;
                        }
                        else if(x != code[i]) {
                                state = Reset;
                        }
                        break;
                case Check:
                        if(i == 0x04) {
                                state = Unlock;
                        }
                        else if (x == '\0') {
                                state = Wait;
				++i;
                        }
                        break;
                case Reset:
                        if(x == '\0') {
                                state = Wait;
                        }
                        i = 0x00;
                   
                        break;
                case Unlock:
                        if(x == '\0') {
                                state = Wait;
                        }
                        i = 0x00;
                        
                        break;
                
                default:
                        state = SMStart;
                        break;

        }

        switch(state) {
                case SMStart:
                        break;
                case Init:
                        outputB = 0x00;
                        break;
                case Wait:
			
                        break;
                case Check:
			
                        break;
                case Reset:
			
                        break;
                case Unlock:
                        outputB = 0x01;
                        break;
		
                default:
                        break;
        }

	PORTB = outputB;
	return state;
}
		
enum SMStates2 {SMStart2, Wait2, Lock};

int SMFunc2(int state) {
	unsigned char inputB = 0x00;
	unsigned char outputB = 0x00;
	inputB = ~PINB & 0x80;
        outputB = PORTB & 0x7F;	
	
	switch(state) {
		case SMStart2:
			state = Wait2;
			break;
		case Wait2:
			state = ((inputB & 0x80) == 0x80)? Lock: Wait2; break;
		case Lock:
			state = ((inputB & 0x80) != 0x80)? Wait2: Lock; break;
		default: state = SMStart2; break;
	}

	switch(state) {
		case SMStart2: break;
		case Wait2: break;
		case Lock:
			outputB = 0x00;
			break;
		default: break;
	}

	PORTB = outputB;
	return state;

	
}

//-----------------
enum SMStates3 {SMStart3, Init3, WaitPlay, WaitNext, PlayNote, WaitTime} ;
enum SMStates4 {SMStart4, Init2, WaitOn, IsOn, Check2} ;

unsigned char iArray = 0x00;
unsigned char iTimer = 0x00;
double notes[6] = {392.00, 329.63, 293.66, 349.23, 523.25, 440.00};
int playTimes[6] = {1, 1, 3, 1, 3, 4};
int waitTimes[6] = {1, 1, 1, 1, 1, 1};
unsigned char IsFinished = 0x00;

int SMFunc3(int state) {
	unsigned char inputA = 0x00;
//        unsigned char outputB = 0x00;

 //       outputB = PORTB;
        inputA = ~PINA;
	switch(state) {
		case SMStart3:
			state = Init3;
			break;
		case Init3:
			iArray = 0x00;
			iTimer = 0x00;
			state = WaitPlay;
			break;
		case WaitPlay:
			if((inputA & 0x80) == 0x80) {
                                state = PlayNote;
                        }
			break;
		case WaitNext:
			if(IsFinished == 0x01) {
				IsFinished = 0x00;
				state = WaitPlay;
			}
			break;
		case PlayNote:
			if(iArray == 0x06) {
				state = WaitNext;
				iArray = 0x00;
                       		iTimer = 0x00;
				break;
			}
			if(!(iTimer < playTimes[iArray])) {
				iTimer = 0;
				state = WaitTime;;
			}
			else {
				++iTimer;
			}
			break;
		case WaitTime:		
                        if(!(iTimer < waitTimes[iArray])) {
                                iTimer = 0;
                                state = PlayNote;
                                ++iArray;
                        }
                        else {
                                ++iTimer;
                        }
			break;
		default:
			state = SMStart3;
			break;
	}
	switch(state) {
                case SMStart3:
                        break;
		case Init:
			break;
                case WaitPlay:
                        break;
		case WaitNext:
			break;
		case PlayNote:
			set_PWM(notes[iArray]);
			break;
		case WaitTime:
                        set_PWM(0);
                        break;
                default:
                        break;
        }
	return state;
}


int SMFunc4(int state) {
        unsigned char inputA = 0x00;
//        unsigned char outputB = 0x00;

 //       outputB = PORTB;
        inputA = ~PINA;
        switch(state) {
                case SMStart4:
                        state = Init2;
                        break;
                case Init2:
                        state = WaitOn;
                        break;
                case WaitOn:
                        if((inputA & 0x80) == 0x80) {
                                state = IsOn;
                        }
                        break;
                case IsOn:
                        if((inputA & 0x80) != 0x80) {
                                state = Check2;
                        }
                        break;
		case Check2:
			if(iArray == 0x00) {
                                state = WaitOn;
				IsFinished = 0x01;
                        }
			break;
		default:
			state = SMStart4;
			break;
	}
	switch(state) {
                case SMStart4:
                        break;
                case Init2:
                        break;
                case WaitOn:
                        break;
                case IsOn:
                        break;
		case Check2:
			break;
		default:
			break;
	}
	return state;
}
//-----------------
//
enum editStates {CodeStart, WaitComb, WaitBuffer, InputComb, CombBuffer} ;

int editFunc(int state) {
	unsigned char inputB = ~PINB;
//	unsigned char outputB = 0x00;
//	       outputB = PORTB;
	unsigned char x = 0x00;
		x = GetKeypadKey();
	unsigned char j = 0x01;

	switch(state) {
		case CodeStart: 
			state = WaitComb; 
			break;

		case WaitComb: //state = (((inputB & 0x80) == 0x80) && (x == '*'))? WaitBuffer: WaitComb; break;
				if(((inputB & 0x80) == 0x80) && (x == '*')) {
					state = WaitBuffer;
				}
				break;
		case WaitBuffer: //state = (((inputB & 0x80) != 0x80) && (x != '*'))? InputComb: WaitBuffer; break;
				if (((inputB & 0x80) != 0x80) && (x != '*')) {
					state = InputComb;
				}
				break;
		
		case InputComb:
				if(j >= 5) {
					j = 0x01;
					state = WaitComb;
				}
				else if (x != '\0') {
					state = CombBuffer;
				}
				break;
		case CombBuffer:
				if(x == '\0') {
					++j;
					state = InputComb;
				}
				break;
		default:
			state = CodeStart;
			break;
	}
	switch(state) {
		case CodeStart: break;
//			outputB = 0x0F;
		case WaitComb: break;
//			outputB = 0x01;
		case WaitBuffer: break;
//			outputB = 0x02;
		case InputComb: break;
//			outputB = 0x03;
		case CombBuffer:
			code[j] = x;
//			outputB = 0x04;
			break;
		default:
			break;
	}
//	PORTB = outputB;
	return state;
}



unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
	DDRB = 0x7F; PORTB = 0x80;
	DDRC = 0xF0; PORTC = 0x0F;
	
	static task task1, task2, task3, task4, task5;
	task *tasks[] = {&task1, &task2, &task3, &task4, &task5};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;

	task1.state = start;
	task1.period = 1;
	task1.elapsedTime = task1.period;
	task1.TickFct = &SMFunc;

	task2.state = start;
        task2.period = 1;
        task2.elapsedTime = task2.period;
        task2.TickFct = &SMFunc2;

	task3.state = start;
        task3.period = 200;
        task3.elapsedTime = task3.period;
        task3.TickFct = &SMFunc3;

        task4.state = start;
        task4.period = 1;
        task4.elapsedTime = task4.period;
        task4.TickFct = &SMFunc4;

	task5.state = start;
        task5.period = 1;
        task5.elapsedTime = task5.period;
        task5.TickFct = &editFunc;


	unsigned short i2;

	unsigned long GCD = tasks[0]->period;
	for(i2 = 1; i2 < numTasks; i2++) {
		GCD = findGCD(GCD, tasks[i2]->period);
	}

	TimerSet(GCD);
	TimerOn();
	PWM_on();

    /* Insert your solution below */
    while (1) {
    	for(i2 = 0; i2 < numTasks; i2++) {
		if (tasks[i2]->elapsedTime == tasks[i2]->period ) {
			tasks[i2]->state = tasks[i2]->TickFct(tasks[i2]->state);
			tasks[i2]->elapsedTime = 0;
		}
		tasks[i2]->elapsedTime += GCD;
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 0;
}
