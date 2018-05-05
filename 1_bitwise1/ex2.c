#include <stdio.h>

void main() {
	int T = 4;
	unsigned char LEDs = 00000001;

	while (T--) {
		for (int i = 0; i < 7; i++) {
			printf("%d,", LEDs);
			LEDs = LEDs << 1;
		}
		for (int i = 0; i < 7; i++) {
			printf("%d,", LEDs);
			LEDs = LEDs >> 1;;
		}
	}
	printf("%d\n", LEDs);
}