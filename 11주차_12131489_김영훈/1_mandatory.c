#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#define ON 		1
#define OFF		0
#define DO_data	43

volatile int data[8] = { 17,43,66,77,97,114,129,137 };
volatile int cnt = 0;
volatile int state = OFF;
volatile int sound = OFF;

ISR(TIMER0_OVF_vect)
{
	if (sound == ON) {
		TCNT0 = data[cnt];

		if (state == ON)
		{
			PORTB = 0x00;
			state = OFF;
		}
		else
		{
			PORTB = 0x10;
			state = ON;
		}
	}
}

ISR(INT4_vect)
{
	cnt = (cnt + 1) % 8;
	_delay_ms(100);
}


ISR(INT5_vect)
{
	if (sound == ON)
	{
		sound = OFF;
	}
	else
	{
		sound = ON;
	}
	_delay_ms(100);
}


int main()
{
	// 스위치
	DDRE = 0xcf;	// 11001111	4,5번비트 입력상태로
	EICRB = 0x0a;	// 4,5번 인터럽트 1010 하강엣지
	EIMSK = 0x30;	// 4,5번 외부 인터럽트 활성화
	SREG |= 1 << 7;	// 인터럽트 활성화

	//버저
	DDRB = 0x10;	// 포트B의 4번비트 출력상태
	PORTB = 0x10;	// On 상태 유지

	TCCR0 = 0x03;	//32분주(32주기마다 카운터 증가)
	TIMSK = 0x01;	//overflow 방식 사용
	//
	TCNT0 = data[0]; // 카운터의 값
	sei();
	while (1);
}
