#include <stdio.h>

int LED_function(int input) {
	return (1 << input);
}

void main() {
	unsigned char LEDs;

	int input;
	scanf("%d", &input);

	LEDs = LED_function(input);

	printf("%d\n", LEDs);
}