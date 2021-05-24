/* Author: Jeffrey Wang jwang619@ucr.edu
 * Lab Section: 22
 * Assignment: Lab # 10  Exercise # 2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 *
 *  Demo Link: https://drive.google.com/file/d/1v8RB9NWwUNVUyqu112REDBeo1SIL800M/view?usp=sharing
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
unsigned char code[6] = {'#','1','2','3','4','5'};
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
                        if(i == 0x05) {
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
	DDRB = 0x7F; PORTB = 0x80;
	DDRC = 0xF0; PORTC = 0x0F;
	
	static task task1, task2;
	task *tasks[] = {&task1, &task2};
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


	unsigned short i2;

	unsigned long GCD = tasks[0]->period;
	for(i2 = 1; i2 < numTasks; i2++) {
		GCD = findGCD(GCD, tasks[i2]->period);
	}

	TimerSet(GCD);
	TimerOn();

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
