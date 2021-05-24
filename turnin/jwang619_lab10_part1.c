/* Author: Jeffrey Wang jwang619@ucr.edu
 * Lab Section: 22
 * Assignment: Lab # 10  Exercise # 1
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

enum SMStates { SMStart, Wait, LightOn} ;

int SMFunc(int state) {
	unsigned char x;
	unsigned char outputB = 0x00;

	outputB = PORTB;
	x = GetKeypadKey();

	switch (state) {
		case SMStart:
			state = Wait;
			break;
		case Wait:
			state = (x == '1' || x == '2' || x == '3' || x == '4' || x == '5' || x == '6' || x == '7' || x == '8' || x == '9' || x == '0' || x == 'A' || x == 'B' || x == 'C' || x == 'D' || x == '*' || x == '#')? LightOn: Wait; break;
		case LightOn:
                        state = (x != '1' && x != '2' && x != '3' && x != '4' && x != '5' && x != '6' && x != '7' && x != '8' && x != '9' && x != '0' && x != 'A' && x != 'B' && x != 'C' && x != 'D' && x != '*' && x != '#')? Wait: LightOn; break;
		default:
			state = SMStart; break;
	}
	switch(state) {
		case SMStart: break;
		case Wait:
			outputB = 0x00; 
			break;
		case LightOn:
			outputB = 0x80;
			break;
		default:
			break;
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
	unsigned char x;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	
	static task task1;
	task *tasks[] = {&task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;

	task1.state = start;
	task1.period = 1;
	task1.elapsedTime = task1.period;
	task1.TickFct = &SMFunc;

	unsigned short i;

	unsigned long GCD = tasks[0]->period;
	for(i = 1; i < numTasks; i++) {
		GCD = findGCD(GCD, tasks[i]->period);
	}

	TimerSet(GCD);
	TimerOn();

    /* Insert your solution below */
    while (1) {
    	for(i = 0; i < numTasks; i++) {
		if (tasks[i]->elapsedTime == tasks[i]->period ) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += GCD;
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 0;
}
