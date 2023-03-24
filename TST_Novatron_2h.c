

/* Testing 


*/


#include <stdlib.h>
#include <string.h>
#include <gigatron/sys.h>
#include <gigatron/libc.h>
#include <stdarg.h>


/* -------------------- QUICK PRINTF CODE ---------------- */

typedef struct {
	char *addr;
	char x;
	char y;
} screenpos_t;

unsigned int fgbg = 0x3f24;

void clear_lines(int l1, int l2)
{
	int i;
	char f;
	f = fgbg & 7;
	f = f | (f<<3);
	for (i=l1; i<l2; i++) {
		char *row = (char*)(videoTable[i+i]<<8);
		memset(row, f, 160);
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

void setPixel(int x, int y, char color)
{
	char *adr;
	char c, d;
	adr = (char*)((videoTable[2*y]<<8) + (x>>1));
	c = color & 7;
	d = *adr;
	if((x & 1) == 0 ){
		d = (d & 0xc7) | (c<<3);
	}else{
		d = (d & 0xf8) | c;
	}
	*adr = d;
}

int myprintf(const char *fmt, ...);

void print_smal_char(screenpos_t *pos, int ch)
{
	unsigned int fntp;
	char *addr;
	int i,j;
	char fg, bg, cc;
	fg = (fgbg>>8) & 7;
	bg = fgbg & 7;
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
	for (i=0; i<5; i++) {
		cc = SYS_Lup(fntp);
		for (j=0; j<8; j++){
			if((cc & 0x80) == 0){
				setPixel(pos->x*6+i, pos->y*8+j, bg);
			}else{
				setPixel(pos->x*6+i, pos->y*8+j, fg);
			}
			cc = cc<<1;
		}
		fntp += 1;
	}
	pos->x += 1;
	if (pos->x > 48)
		newline(pos);
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
		SYS_VDrawBits(fgbg, SYS_Lup(fntp), addr);
		addr += 1;
		fntp += 1;
	}
	pos->x += 2;
	pos->addr = addr + 1;
	if (pos->x > 48)
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

int myprintsmalf(const char *fmt, ...)
{
	char c;
	va_list ap;
	va_start(ap, fmt);
	while (c = *fmt++) {
		if (c != '%') {
			print_smal_char(&pos, c);
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
	int i,j;
	clear_screen(&pos);

	myprintsmalf("Hello World\n");
	myprintf("Hello World\n");
	myprintf("0123456789012\n");
	fgbg = 0x0004;                          // black/blue
	myprintsmalf(" Hello World\n");
	fgbg = 0x0104;                          // red/blue
	myprintsmalf("  Hello World\n");
	fgbg = 0x0204;                          // green/blue
	myprintsmalf("   Hello World\n");
	fgbg = 0x0304;
	myprintsmalf("    Hello World\n");
	fgbg = 0x0504;
	myprintsmalf("     Hello World\n");
	fgbg = 0x0604;
	myprintsmalf("      Hello World\n");
	fgbg = 0x0704;                          // white/blue
	myprintsmalf("       Hello World\n");
	
	for(i=0; i<120; i++){
		setPixel(i + 157, i, 0);
		setPixel(i + 163, i, 1);
		setPixel(i + 169, i, 2);
		setPixel(i + 175, i, 3);
		setPixel(i + 181, i, 4);
		setPixel(i + 187, i, 5);
		setPixel(i + 193, i, 6);
		setPixel(i + 199, i, 7);
	}
	
	return 0;
}
