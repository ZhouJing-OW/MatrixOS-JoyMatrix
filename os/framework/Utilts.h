
#define bitRead(input, bit) (((input) >> (bit)) & 0b00000001)

#define middleOfThree(a, b, c) (((b <= a && a <= c) || (c <= a && a <= b)) ? a : (((a <= b && b <= c) || (c <= b && b <= a)) ? b : c))

#define minOfThree(a, b, c) ((a < b && a < c) ? a : (b < a && b < c) ? b : c)

#define sys_random() (esp_random())