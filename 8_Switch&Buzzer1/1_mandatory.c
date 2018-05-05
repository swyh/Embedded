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
	// ����ġ
	DDRE = 0xcf;	// 11001111	4,5����Ʈ �Է»��·�
	EICRB = 0x0a;	// 4,5�� ���ͷ�Ʈ 1010 �ϰ�����
	EIMSK = 0x30;	// 4,5�� �ܺ� ���ͷ�Ʈ Ȱ��ȭ
	SREG |= 1 << 7;	// ���ͷ�Ʈ Ȱ��ȭ

	//����
	DDRB = 0x10;	// ��ƮB�� 4����Ʈ ��»���
	PORTB = 0x10;	// On ���� ����

	TCCR0 = 0x03;	//32����(32�ֱ⸶�� ī���� ����)
	TIMSK = 0x01;	//overflow ��� ���
	//
	TCNT0 = data[0]; // ī������ ��
	sei();
	while (1);
}
