// #define min(a,b) ((a)<(b)?(a):(b))
// #define max(a,b) ((a)>(b)?(a):(b))


#define bitRead(input, bit) (((input) >> (bit)) & 0b00000001)
#define offset(structure, member) ((uint32_t)&(((structure *)0)->member)) //32-bit system

