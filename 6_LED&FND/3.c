#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>


void main(){

	unsigned char FND_DATA[10] = {0x3f, 0x06, 0x5b, 0x4f,
	0x66, 0x6d, 0x7d, 0x27};

	unsigned char fnd_sel[4] = {0x01, 0x02, 0x04, 0x08};

	int random;
	int light = 1;

	DDRA = 0xff;
	DDRC = 0xff;
	DDRG = 0x0f;

	while(1){
		random = rand()%8;
		PORTC = FND_DATA[random];


		PORTA = light << random;
		for(int k = 1; k <= 50; k++){
			for(int i = 1; i <= 0x08; i <<= 1){
				PORTG = i;
				_delay_ms(2);
			}
		}
	}

}
