#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#define ON 		1
#define OFF		0
unsigned char digit[10] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66,
	0x6d, 0x7c, 0x07, 0x7f, 0x67
};
unsigned char fnd_sel[4] =
{
	0x01, 0x02, 0x04, 0x08
};
volatile int count = 0;
volatile int state = OFF;	// button state
volatile int s_state = OFF;	// sound state
volatile int background = OFF;
volatile int data[8] = { 17,43,66,77,97,114,129,137 };
volatile int type = 0;
volatile int sound_time = 0;
ISR(TIMER0_OVF_vect)
{
	if (sound_time < 500) {
		if (s_state == ON)
		{
			PORTB = 0x00;
			s_state = OFF;
		}
		else
		{
			PORTB = 0x10;
			s_state = ON;
		}
		TCNT0 = data[type];
		sound_time++;
	}
}

void display_fnd(int count)
{
	int i, fnd[4];

	fnd[3] = (count / 1000) % 10;
	fnd[2] = (count / 100) % 10;
	fnd[1] = (count / 10) % 10;
	fnd[0] = count % 10;
	for (i = 0; i < 4; i++)
	{
		PORTC = digit[fnd[i]];
		if (i == 2)
			PORTC |= 0x80;
		PORTG = fnd_sel[i];
		_delay_ms(2.5);
	}
}

void sound(int sound_type) {
	sound_time = 0;
	type = sound_type;
}

ISR(INT4_vect)
{
	if (state == ON) {
		state = OFF;
		sound(0);
	}
	else {
		state = ON;
		sound(1);
	}

	_delay_ms(10);
}

ISR(INT5_vect)
{
	if (state == ON && background == OFF) {
		background = ON;
		sound(2);
	}
	else if (state == ON && background == ON) {
		background = OFF;
		sound(3);
	}
	else {
		count = 0;
		sound(4);
	}
	_delay_ms(10);
}


int main()
{
	DDRC = 0xff;	// LED 데이터 신호 (입력상태로)
	DDRG = 0x0f;	// LED 선택 신호

	//스위치
	DDRE = 0xcf;
	EICRB = 0x0a;
	EIMSK = 0x30;
	SREG |= 1 << 7;
	//

	//버저
	DDRB = 0x10;
	PORTB = 0x10;
	TCCR0 = 0x03;	//32분주
	TIMSK = 0x01;	//overflow
					//


	int display_cnt = count;

	while (1) {
		if (state == ON)
			count++;
		if (background == OFF)
			display_cnt = count;
		if (count > 6000)
			count = 0;
		display_fnd(display_cnt);
	}
}
