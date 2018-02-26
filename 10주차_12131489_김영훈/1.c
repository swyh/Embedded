#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void main(){

	DDRA = 0xff;

	int light = 1;
	int direct = 0;

	while(1){

		PORTA = light;

			if(direct == 0)
				light <<= 1;
			else
				light >>= 1;
				
			if(light == 0x80)
				direct = 1;
			else if(light == 1)
				direct = 0;
			
			_delay_ms(200);

	}
}
