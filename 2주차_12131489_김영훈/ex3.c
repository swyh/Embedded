#include <stdio.h>

void main() {
	unsigned char LEDs;

	int input;
	scanf("%d", &input);
	
	int num = 0;

	while (input) {
		int temp = input % 2;
		if (temp)num++;
		input /= 2;
	}
	printf("1의 개수 : %d\n", num);

	int temp = 128, result = 0;
	while (num--) {
		result += temp;
		temp /= 2;
	}
	printf("Shift시의 값은? : %d\n", result);
}