#define F_CPU 16000000UL // CPU Ŭ�� �� = 16 Mhz
#define F_SCK 40000UL // SCK Ŭ�� �� = 40 Khz
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define ATS75_ADDR 0x98 // 0b10011000, 7��Ʈ�� 1��Ʈ left shift
#define ATS75_CONFIG_REG 1
#define ATS75_TEMP_REG 0
void init_twi_port();
void write_twi_1byte_nopreset(char reg, char data);
int read_twi_2byte_nopreset(char reg);
void display_FND(int value);
volatile int mode = 0;

ISR(INT4_vect)
{
	mode = 0;
}


ISR(INT5_vect)
{
	mode = 1;
}

int main() {

	DDRE = 0xcf;
	EICRB = 0x0a;
	EIMSK = 0x30;
	SREG |= 1 << 7;

	int temperature;
	init_twi_port(); // TWI �� ��Ʈ �ʱ�ȭ
	write_twi_1byte_nopreset(ATS75_CONFIG_REG, 0x00); // 9��Ʈ, Normal
	_delay_ms(100); // ���� ����Ŭ�� ���Ͽ� ��� ��ٸ�
	while (1) { // �µ��� �о� FND ���÷���
		temperature = read_twi_2byte_nopreset(ATS75_TEMP_REG);
		display_FND(temperature);
	}
}
void init_twi_port() {
	DDRC = 0xff;
	DDRG = 0xff; // FND ��� ����
	PORTD = 3; // For Internal pull-up for SCL & SCK
	SFIOR &= ~(1 << PUD); // PUD = 0 : Pull Up Disable
	TWBR = (F_CPU / F_SCK - 16) / 2; // ���� ����, bit trans rate ����
	TWSR = TWSR & 0xfc; // Prescaler �� = 00 (1��)
}
void write_twi_1byte_nopreset(char reg, char data) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);// START ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x08); // START ���� �˻�, ���� ��� ���� �˻�
	TWDR = ATS75_ADDR | 0; // SLA+W �غ�, W=0
	TWCR = (1 << TWINT) | (1 << TWEN); // SLA+W ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18);
	TWDR = reg; // aTS75 Reg �� �غ�
	TWCR = (1 << TWINT) | (1 << TWEN); // aTS75 Reg �� ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x28);
	TWDR = data; // DATA �غ�
	TWCR = (1 << TWINT) | (1 << TWEN); // DATA ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x28);
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // STOP ����
}
int read_twi_2byte_nopreset(char reg) {
	char high_byte, low_byte;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);// START ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x08); // START ���� �˻�, ���� ACK �� ���� �˻�
	TWDR = ATS75_ADDR | 0; // SLA+W �غ�, W=0
	TWCR = (1 << TWINT) | (1 << TWEN); // SLA+W ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18);
	TWDR = reg; // aTS75 Reg �� �غ�
	TWCR = (1 << TWINT) | (1 << TWEN); // aTS75 Reg �� ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x28);
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);// RESTART ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x10); // RESTART ���� �˻�, ���� ACK, NO_ACK ���� �˻�
	TWDR = ATS75_ADDR | 1; // SLA+R �غ�, R=1
	TWCR = (1 << TWINT) | (1 << TWEN); // SLA+R ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x40);
	TWCR = (1 << TWINT) | (1 << TWEN | 1 << TWEA);// 1st DATA �غ�
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x50);
	high_byte = TWDR; // 1st DATA ����
	TWCR = (1 << TWINT) | (1 << TWEN);// 2nd DATA �غ�
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x58);
	low_byte = TWDR; // 2nd DATA ����
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // STOP ����
	return((high_byte << 8) | low_byte); // ���� DATA ����
}

void display_FND(int value) {
	unsigned char digit[12] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x40, 0x00 };
	unsigned char fnd_sel[4] = { 0x01, 0x02, 0x04, 0x08 };
	char value_int, value_deci, num[4];
	int i;
	if ((value & 0x8000) != 0x8000) // Sign ��Ʈ üũ
		num[3] = 11;	// ����
	else {
		num[3] = 10;	// ����
		value = (~value) - 1; // 2��s Compliment
	}

	value_int = (char)((value & 0x7f00) >> 8);
	value_deci = (char)(value & 0x00ff);

	//ȭ��
	if (mode)
		value_int = value_int * 1.8 + 32;

	num[2] = (value_int / 10) % 10;
	num[1] = value_int % 10;
	num[0] = ((value_deci & 0x80) == 0x80) * 5;


	for (i = 0; i < 4; i++) {
		PORTC = digit[num[i]];
		PORTG = fnd_sel[i];
		if (i == 1)
			PORTC |= 0x80;
		_delay_ms(2);
	}
}
