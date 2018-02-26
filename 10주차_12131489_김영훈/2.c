#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void main(){

	unsigned char FND_DATA[10] = {0x3f, 0x06, 0x5b, 0x4f,
	0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f};

	DDRC = 0xff;
	DDRG = 0x0f;


while(1){
	for(int i = 0; i <= 6000; i++){

		PORTC = FND_DATA[i%10];
		PORTG = 0x01;
		_delay_ms(2.5);

		
		PORTC = FND_DATA[(i/10) % 10];
		PORTG = 0x02;
		_delay_ms(2.5);

		
		PORTC = FND_DATA[(i/100) % 10] | 0x80;
		PORTG = 0x04;
		_delay_ms(2.5);

		
		PORTC = FND_DATA[i/1000];
		PORTG = 0x08;
		_delay_ms(2.5);
		
		}
	}
}
