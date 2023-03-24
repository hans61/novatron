

/* Testing

*/


#include <stdlib.h>
#include <string.h>
#include <gigatron/sys.h>
#include <gigatron/libc.h>
#include <stdarg.h>


/* -------------------- QUICK PRINTF CODE ---------------- */

#define FGBG 0x3f20

typedef struct {
	char *addr;
	char x;
	char y;
} screenpos_t;

void clear_lines(int l1, int l2)
{
	int i;
	for (i=l1; i<l2; i++) {
		char *row = (char*)(videoTable[i+i]<<8);
		memset(row, FGBG & 0xff, 160);
	}
}

void clear_screen(screenpos_t *pos)
{
	int i;
	for (i=0; i<120; i++) {
		videoTable[i+i] = 8 + i;
		videoTable[i+i+1] = 0;
	}
	clear_lines(0,120);
	pos->x = pos->y = 0;
	pos->addr = (char*)(videoTable[0]<<8);
}

void scroll(void)
{
	char pages[8];
	int i;
	for (i=0; i<8; i++)
		pages[i] = videoTable[i+i];
	for (i=0; i<112; i++)
		videoTable[i+i] = videoTable[i+i+16];
	for (i=112; i<120; i++)
		videoTable[i+i] = pages[i-112];
}

void newline(screenpos_t *pos)
{
	pos->x = 0;
	pos->y += 1;
	if (pos->y >  14) {
		scroll();
		clear_lines(112,120);
		pos->y = 14;
	}
	pos->addr = (char*)(videoTable[16*pos->y]<<8);
}

void print_char(screenpos_t *pos, int ch)
{
	unsigned int fntp;
	char *addr;
	int i;
	if (ch < 32) {
		if (ch == '\n') 
			newline(pos);
		return;
	} else if (ch < 82) {
		fntp = font32up + 5 * (ch - 32);
	} else if (ch < 132) {
		fntp = font82up + 5 * (ch - 82);
	} else {
		return;
	}
	addr = pos->addr;
	for (i=0; i<5; i++) {
		SYS_VDrawBits(FGBG, SYS_Lup(fntp), addr);
		addr += 1;
		fntp += 1;
	}
	pos->x += 1;
	pos->addr = addr + 1;
	if (pos->x > 24)
		newline(pos);
}

screenpos_t pos;

void print_unsigned(unsigned int n, int radix)
{
	static char digit[] = "0123456789abcdef";
	char buffer[8];
	char *s = buffer;
	do {
		*s++ = digit[n % radix];
		n = n / radix;
	} while (n);
	while (s > buffer)
		print_char(&pos, *--s);
}

void print_int(int n, int radix)
{
	if (n < 0) {
		print_char(&pos, '-');
		n = -n;
	}
	print_unsigned(n, radix);
}

int myprintf(const char *fmt, ...)
{
	char c;
	va_list ap;
	va_start(ap, fmt);
	while (c = *fmt++) {
		if (c != '%') {
			print_char(&pos, c);
			continue;
		}
		if (c = *fmt++) {
			if (c == 'd')
				print_int(va_arg(ap, int), 10);
			else if (c == 'u')
				print_unsigned(va_arg(ap, unsigned), 10);
			else if (c == 'x')
				print_unsigned(va_arg(ap, unsigned), 16);
			else
				print_char(&pos, c);
		}
	}
	va_end(ap);
	return 0;
}


/* -------------------- THIS IS THE TEST ---------------- */

int main()
{

	clear_screen(&pos);
	myprintf("Hello World\n");
	
}
