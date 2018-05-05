#include <avr/io.h>
#define CDS_VALUE 871
#define F_CPU 16000000UL
#include <util/delay.h>

void init_adc();
unsigned short read_adc();
void show_adc(unsigned short value);
void show_fnd(unsigned short value);
unsigned char digit[10] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66,
	0x6d, 0x7c, 0x07, 0x7f, 0x67
};
unsigned char fnd_sel[4] =
{
	0x01, 0x02, 0x04, 0x08
};


int main() {
	unsigned short value;
	DDRA = 0xff;
	DDRC = 0xff;	//FND
	DDRG = 0x0f;	//FND ����

	init_adc();

	while (1) {
		value = read_adc();
		show_fnd(value);
		show_adc(value);

	}
}

void init_adc() {
	ADMUX = 0x20;	/* REFS(1:0) = "00": AREF(+5V) �������� ���
					* ADLAR = '0': ����Ʈ ������ ����(5�� ��Ʈ), 1 : ���� ����
					* MUX(4:0) = "00000": ADC0 ���, �ܱ� �Է�
					*/
	ADCSRA = 0X87;	/* ADEN = '1': ADC�� Enable
					* ADFR = '0': single conversion ���
					* ADPS(2:0) = "111": ���������Ϸ� 128����
					*/
}

unsigned short read_adc() {
	unsigned char adc_low, adc_high;
	unsigned short value;
	ADCSRA |= 0x40;	/* ADSC = '1': ADC start conversion */
	while ((ADCSRA & 0x10) != 0x10);	/* ADC ��ȯ �Ϸ� �˻� */
	adc_low = ADCL;	/* ��ȯ�� low�� high�� �о���� */
	adc_high = ADCH;
	value = (adc_high << 2) | (adc_low >> 6);	/* 16��Ʈ ������ ���� */

	return value;
}

int min = 780, max = 960;

unsigned char dis_fnd[9] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };

void show_adc(unsigned short value) {
	int unit = (max - min) / 8;

	int i = 0;
	int std = max;

	while (i < 8) {
		if (value < std) {
			std -= unit;
		}
		else
			break;
		i++;
	}
	PORTA = dis_fnd[i];
}

void show_fnd(unsigned short value)
{
	int i, fnd[4];

	fnd[3] = (value / 1000) % 10;
	fnd[2] = (value / 100) % 10;
	fnd[1] = (value / 10) % 10;
	fnd[0] = value % 10;
	for (i = 0; i < 4; i++)
	{
		PORTC = digit[fnd[i]];
		PORTG = fnd_sel[i];
		_delay_ms(2);
	}
}
