#include "includes.h"
#define F_CPU	16000000UL	// CPU frequency = 16 Mhz
#include <avr/io.h>	
#include <util/delay.h>
#include <avr/interrupt.h>
#define ON 		1
#define OFF		0

#define DAY		2
#define HOUR	1
#define MIN		0

#define MAX		1
#define MIN		0

#define GET		2
#define SET		1
#define CLS		0
//--------mode ----------------
#define MENU_SIZE 6
#define GET_SIZE 1	//������
#define SET_SIZE 4	//������
#define CLS_SIZE 6	//...

#define MENU	  10
#define ALARM	  11

#define GET_CLOCK 0
#define GET_LIGET 1

#define SET_CLOCK 2
#define SET_TIMER 3
#define SET_LIGHT 4
//#define SET_TMP	  5
#define CLS_TIMER 5
#define CLS_LIGET 6

#define  TASK_STK_SIZE  OS_TASK_DEF_STK_SIZE            

OS_STK	TimeTaskStk[TASK_STK_SIZE];
OS_STK	LedTaskStk[TASK_STK_SIZE];
OS_STK	FndTaskStk[TASK_STK_SIZE];
OS_STK	AdcTaskStk[TASK_STK_SIZE];
OS_STK	SetTaskStk[TASK_STK_SIZE];
OS_STK	AlarmTaskStk[TASK_STK_SIZE];

//--------------EVENT--------------------------------
OS_FLAG_GRP		*set_time_wait;

//----------------value------------------------------
volatile int data[8] = { 17,43,66,77,97,114,129,137 };	// buzzer �������ļ�
volatile int menu_select = GET_CLOCK;	// menu_select�� ���� mode ������ ������ �� ����
volatile int mode = SET_CLOCK; //�ð� �������� ����
volatile int set_time = HOUR;
volatile int set_stime = DAY;
volatile int status = GET; // ���� or non ����

volatile unsigned short m_sec = 0;	// ���� �ð� (�ð�/��)
volatile unsigned short m_hour = 0;	// ���� �ð� (�ð�/��)
volatile unsigned short m_min = 0;	// ���� �ð� (�ð�/��)
volatile unsigned short m_day = 0;	// ���� ��¥

volatile unsigned short s_hour = 0;	// ���� �ð� (�ð�/��)
volatile unsigned short s_min = 0;	// ���� �ð� (�ð�/��)
volatile unsigned short s_day = 0;	// ���� ��¥

volatile unsigned short light = 0;
volatile unsigned short max_light = 0;
volatile unsigned short min_light = 0;
volatile int max_min = 0;

volatile int alarm_timer = OFF;
volatile int alarm_light = OFF;
volatile int alarm_tmp = OFF;


//--------------TASK--------------------------------
void TimeTask(void *data);
void LedTask(void *data);
void FndTask(void *data);
void AdcTask(void *data);
void SdcTask(void *data);
void AlarmTask(void *data);
//--------------function-----------------------------
void init_switch(void);
void init_adc(void);
void init_buzzer(void);
unsigned short read_adc(void);


unsigned char digit[10] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66,
	0x6d, 0x7c, 0x07, 0x7f, 0x67
};
unsigned char fnd_sel[4] =
{
	0x01, 0x02, 0x04, 0x08
};

//------------------------------------------------
int main(void)
{
	OSInit();

	OS_ENTER_CRITICAL();
	TCCR0 = 0x03;
	TIMSK = _BV(TOIE0);
	TCNT0 = 256 - (CPU_CLOCK_HZ / OS_TICKS_PER_SEC / 1024);
	OS_EXIT_CRITICAL();

	INT8U err;

	set_time_wait = OSFlagCreate(0x00, &err);	// set_time_wait flag ����

	init_switch();
	//init_buzzer();

	OSTaskCreate(TimeTask, (void *)0, (void *)&TimeTaskStk[TASK_STK_SIZE - 1], 1);
	OSTaskCreate(FndTask, (void *)0, (void *)&FndTaskStk[TASK_STK_SIZE - 1], 2);

	OSTaskCreate(AlarmTask, (void *)0, (void *)&AlarmTaskStk[TASK_STK_SIZE - 1], 3);
	OSTaskCreate(AdcTask, (void *)0, (void *)&AdcTaskStk[TASK_STK_SIZE - 1], 4);
	OSTaskCreate(LedTask, (void *)0, (void *)&LedTaskStk[TASK_STK_SIZE - 1], 5);


	OSStart();

	return 0;
}
//-----------------init--------------------
void init_adc() {	// �������� ����ϱ� ���� A/D ��ȯ�� �ʱ�ȭ
	ADMUX = 0x20;	/* REFS(1:0) = "00": AREF(+5V) �������� ���
					* ADLAR = '0': ����Ʈ ������ ����(5�� ��Ʈ), 1 : ���� ����
					* MUX(4:0) = "00000": ADC0 ���, �ܱ� �Է�
					*/
	ADCSRA = 0X87;	/* ADEN = '1': ADC�� Enable
					* ADFR = '0': single conversion ���
					* ADPS(2:0) = "111": ���������Ϸ� 128����
					*/
}

void init_buzzer() {	// ���� �ʱ�ȭ
						//����
	DDRB = 0x10;	// ��ƮB�� 4����Ʈ ��»���
	PORTB = 0x10;	// On ���� ����

	TCCR2 = 0x03;	//32����(32�ֱ⸶�� ī���� ����)
	TIMSK |= 0x40;
	TCNT2 = data[2]; // ī������ ��
	sei();
}

void init_switch() {
	DDRE = 0xcf;	// 11001111	4,5����Ʈ �Է»��·�
	EICRB = 0x0a;	// 4,5�� ���ͷ�Ʈ 1010 �ϰ�����
	EIMSK = 0x30;	// 4,5�� �ܺ� ���ͷ�Ʈ Ȱ��ȭ
	SREG |= 1 << 7;	// ���ͷ�Ʈ Ȱ��ȭ
}

//-------------function--------------------
unsigned short read_adc() {	// ������ ��ġ �ҷ���
	unsigned char adc_low, adc_high;
	unsigned short value;
	ADCSRA |= 0x40;	/* ADSC = '1': ADC start conversion */
	while ((ADCSRA & 0x10) != 0x10);	/* ADC ��ȯ �Ϸ� �˻� */
	adc_low = ADCL;	/* ��ȯ�� low�� high�� �о���� */
	adc_high = ADCH;
	value = (adc_high << 2) | (adc_low >> 6);	/* 16��Ʈ ������ ���� */

	return value;
}

//----------interrupt-------------------------

int state = ON, cnt = 3;

ISR(TIMER2_OVF_vect)
{
	if (mode == ALARM) {
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
} // buzzer

ISR(INT4_vect)
{     
	switch (mode)
	{
	case SET_CLOCK:
		if (set_time == HOUR) {	// �ð� ����
			m_hour = (m_hour + 1) % 24;
		}
		else if (set_time == MIN) {	// �� ����
			m_min = (m_min + 1) % 60;
		}
		break;

	case SET_TIMER:
		if (set_stime == DAY) {	// �ֱ� ����
			s_day = (s_day + 1) % 30;
		}
		if (set_stime == HOUR) {	// �ð� ����
			s_hour = (s_hour + 1) % 24;
		}
		else if (set_stime == MIN) {	// �� ����
			s_min = (s_min + 1) % 60;
		}
		break;
	case GET_LIGET:
		mode = MENU;
		break;
	case SET_LIGHT:
		if (max_min == MAX)
			max_light = (max_light + 1) % 11;
		else if (max_min == MIN)
			min_light = (min_light + 1) % 11;
		break;
	case MENU:
		menu_select++;

		if (MENU_SIZE < menu_select)
			menu_select = 0;

		if (menu_select <= GET_SIZE)
			status = GET;
		else if (menu_select <= SET_SIZE)
			status = SET;
		else if (menu_select <= CLS_SIZE)
			status = CLS;
		break;

	case ALARM:
		mode = MENU;
		alarm_timer = OFF;
		alarm_light = OFF;
		break;
	}
	_delay_ms(120);
}

ISR(INT5_vect)	//Ȯ�� ��ư
{
	INT8U err;

	switch (mode)
	{
	case SET_CLOCK:
		if (set_time == DAY)
			set_time = HOUR;
		else if (set_time == HOUR) 	// �ð� ����
			set_time = MIN;
		else if (set_time == MIN) {	// �� ����
			set_time = HOUR;	//set_time �ʱ�ȭ
			mode = MENU;
			OSFlagPost(set_time_wait, 0x01, OS_FLAG_SET, &err);
		}
		break;
	case SET_TIMER:
		if (set_stime == DAY)
			set_stime = HOUR;
		else if (set_stime == HOUR) 	// �ð� ����
			set_stime = MIN;
		else if (set_stime == MIN) {	// �� ����I
			set_stime = DAY;	//set_time �ʱ�ȭ
			mode = MENU;
			alarm_timer = ON;
		}
		break;

	case GET_LIGET:
		mode = MENU;
		break;

	case GET_CLOCK:
		mode = MENU;
		break;

	case SET_LIGHT:
		if (max_min == MIN)
			max_min = MAX;
		else {
			mode = MENU;
			max_min = MIN;
			alarm_light = ON;
		}
		break;
	case CLS_LIGET:
		alarm_light = OFF;
		break;
	case CLS_TIMER:
		alarm_timer = OFF;
		break;
	case MENU:
		mode = menu_select;
		//menu_select = GET_CLOCK;
		break;

	case ALARM:
		mode = MENU;
		break;
	}

	_delay_ms(120);
}

//-------------Task---------------------

void TimeTask(void *pdata) {	// ���� �ð� ����
	INT8U err;
	for (;;) {
		if (mode == SET_CLOCK) {	// �ð� ������ �� �ɶ����� ����Ѵ�.
			OSFlagPend(set_time_wait, 0x01, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);
		}
		else {
			if (m_sec < 60) {
				m_sec++;
			}
			else { // m_sec >= 60
				if (m_min < 60)
					m_min++;
				else {	//m_min >= 60
					if (m_hour < 24)
						m_hour++;
					else {
						m_day++;
						m_hour = 0;
					}
					m_min = 0;
				}
				m_sec = 0;
			}
		}
		OSTimeDly(1000);
	}
}

void FndTask(void *pdata)
{
	int i, fnd[4], digit_mood;

	DDRC = 0xff;	//FND
	DDRG = 0x0f;	//FND ����
					//unsigned short value = 0;
	for (;;) {
		if (mode == MENU) {
			digit_mood = OFF;
			if (status == GET) {	//GET
				fnd[3] = 0x3d;
				fnd[2] = 0x79;
				fnd[1] = 0x31;
			}
			else if (status == SET) {	//SET�� ǥ��
				fnd[3] = 0x6d;
				fnd[2] = 0x79;
				fnd[1] = 0x31;
			}
			else if (status == CLS) {	//CLS�� ǥ��
				fnd[3] = 0x39;
				fnd[2] = 0x38;
				fnd[1] = 0x6d;
			}
			fnd[0] = digit[menu_select % 10];
		}
		else {
			digit_mood = ON;
			if (mode == GET_CLOCK || mode == SET_CLOCK) {	// �ð� ���� / ���� �ð� Ȯ��
				fnd[3] = (m_hour / 10) % 10;
				fnd[2] = m_hour % 10;
				fnd[1] = (m_min / 10) % 10;
				fnd[0] = m_min % 10;
			}
			else if (mode == SET_TIMER) {
				if (set_stime == DAY) {
					digit_mood = OFF;
					fnd[3] = 0x5e;// DAY ǥ��
					fnd[2] = 0x6e;//
					fnd[1] = digit[(s_day / 10) % 10];
					fnd[0] = digit[s_day % 10];
				}
				else {
					fnd[3] = (s_hour / 10) % 10;
					fnd[2] = s_hour % 10;
					fnd[1] = (s_min / 10) % 10;
					fnd[0] = s_min % 10;
				}
			}
			else if (mode == SET_LIGHT) {
				fnd[3] = (min_light / 10) % 10;
				fnd[2] = min_light % 10;
				fnd[1] = (max_light / 10) % 10;
				fnd[0] = max_light % 10;
			}
			else if (mode == ALARM) {
				digit_mood = OFF;
				fnd[3] = 0x77;// �˶�ǥ��
				fnd[2] = 0x38;//
				fnd[1] = 0x77;//
				fnd[0] = 0x37;//
			}
			else if (mode == GET_LIGET) {
				digit_mood = OFF;
				fnd[3] = 0x38;// ���Ǽ���ǥ��
				fnd[2] = 0x31;
				fnd[1] = digit[(light / 10) % 10];
				fnd[0] = digit[light % 10];
			}
		}

		for (i = 0; i < 4; i++)
		{
			if (digit_mood == OFF)
				PORTC = fnd[i];
			else {
				PORTC = digit[fnd[i]];
				if(i == 2) PORTC |= 0x80;
			}

			PORTG = fnd_sel[i];
			OSTimeDly(2.5);
			//_delay_ms(2);
		}
		//OSTimeDly(2.5);
	}
}

void LedTask(void *pdata)
{
		pdata = pdata;
	
		DDRA = 0xff;
		for (;;) {
			// LED display
			PORTA = 1 << menu_select;

		OSTimeDly(500);		// context switch�� �Ͼ
	}
}

void AdcTask(void *pdata)
{
	init_adc();
	for (;;)
	{
		light = (read_adc()/100)%10;	// �������� ���� value�� �ҷ��鿩��. 0~10
		OSTimeDly(500);
	}

}

void AlarmTask(void * padata) {
	for (;;) {
		
		if (alarm_timer == ON ) {
			if (s_day == m_day && s_hour == m_hour && s_min == m_min && m_sec == 0) {
				mode = ALARM;
				m_day = 0;
			}
		}
		else if (alarm_light == ON) {
			if (min_light > light || max_light < light) {
				mode = ALARM;
			}
		}

			OSTimeDly(1000);
	}
}