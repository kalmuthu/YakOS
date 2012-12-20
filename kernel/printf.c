#include <stdarg.h>
#include <string.h>
#include "kernel/printf.h"

#define LONGFLAG     0x00000001
#define LONGLONGFLAG 0x00000002
#define HALFFLAG     0x00000004
#define HALFHALFFLAG 0x00000008
#define SIZETFLAG    0x00000010
#define ALTFLAG      0x00000020
#define CAPSFLAG     0x00000040
#define SHOWSIGNFLAG 0x00000080
#define SIGNEDFLAG   0x00000100
#define LEFTFORMATFLAG 0x00000200
#define LEADZEROFLAG 0x00000400

typedef long          ssize_t;
typedef long long     off_t;

extern void __console_init(void);

void console_init(void)
{
  __console_init();
}

void puts(const char *s)
{
  __puts(s);
}

size_t
strlen(char const *s)
{
	size_t i;

	i= 0;
	while(s[i]) {
		i+= 1;
	}

	return i;
}


static char *longlong_to_string(char *buf, unsigned long long n, int len, unsigned int flag)
{
	int pos = len;
	int negative = 0;
	int digit;

	if((flag & SIGNEDFLAG) && (long long)n < 0) {
		negative = 1;
		n = -n;
	}

	buf[--pos] = 0;
	
	/* only do the math if the number is >= 10 */
	while(n >= 10) {
	  digit = (unsigned int)n % 10;
	  n = (unsigned int)n / 10;

		buf[--pos] = digit + '0';
	}
	buf[--pos] = n + '0';
	
	if(negative)
		buf[--pos] = '-';
	else if((flag & SHOWSIGNFLAG))
		buf[--pos] = '+';

	return &buf[pos];
}

static char *longlong_to_hexstring(char *buf, unsigned long long u, int len, unsigned int flag)
{
	int pos = len;
	static const char hextable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	static const char hextable_caps[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	const char *table;

	if((flag & CAPSFLAG))
		table = hextable_caps;
	else
		table = hextable;

	buf[--pos] = 0;
	do {
		unsigned int digit = u % 16;
		u /= 16;
	
		buf[--pos] = table[digit];
	} while(u != 0);

	return &buf[pos];
}

int vsnprintf(char *str, size_t len, const char *fmt, va_list ap)
{
	char c;
	unsigned char uc;
	const char *s;
	unsigned long long n;
	void *ptr;
	int flags;
	unsigned int format_num;
	size_t chars_written = 0;
	char num_buffer[32];

#define OUTPUT_CHAR(c) do { (*str++ = c); chars_written++; if (chars_written + 1 == len) goto done; } while(0)
#define OUTPUT_CHAR_NOLENCHECK(c) do { (*str++ = c); chars_written++; } while(0)

	for(;;) {	
		/* handle regular chars that aren't format related */
		while((c = *fmt++) != 0) {
			if(c == '%')
				break; /* we saw a '%', break and start parsing format */
			OUTPUT_CHAR(c);
		}

		/* make sure we haven't just hit the end of the string */
		if(c == 0)
			break;

		/* reset the format state */
		flags = 0;
		format_num = 0;

next_format:
		/* grab the next format character */
		c = *fmt++;
		if(c == 0)
			break;
					
		switch(c) {
			case '0'...'9':
				if (c == '0' && format_num == 0)
					flags |= LEADZEROFLAG;
				format_num *= 10;
				format_num += c - '0';
				goto next_format;
			case '.':
				/* XXX for now eat numeric formatting */
				goto next_format;
			case '%':
				OUTPUT_CHAR('%');
				break;
			case 'c':
				uc = va_arg(ap, unsigned int);
				OUTPUT_CHAR(uc);
				break;
			case 's':
				s = va_arg(ap, const char *);
				if(s == 0)
					s = "<null>";
				goto _output_string;
			case '-':
				flags |= LEFTFORMATFLAG;
				goto next_format;
			case '+':
				flags |= SHOWSIGNFLAG;
				goto next_format;
			case '#':
				flags |= ALTFLAG;
				goto next_format;
			case 'l':
				if(flags & LONGFLAG)
					flags |= LONGLONGFLAG;
				flags |= LONGFLAG;
				goto next_format;
			case 'h':
				if(flags & HALFFLAG)
					flags |= HALFHALFFLAG;
				flags |= HALFFLAG;
				goto next_format;
		    case 'z':
				flags |= SIZETFLAG;
				goto next_format;
			case 'D':
				flags |= LONGFLAG;
				/* fallthrough */
			case 'i':
			case 'd':
				n = (flags & LONGLONGFLAG) ? va_arg(ap, long long) :
					(flags & LONGFLAG) ? va_arg(ap, long) : 
					(flags & HALFHALFFLAG) ? (signed char)va_arg(ap, int) :
					(flags & HALFFLAG) ? (short)va_arg(ap, int) :
					(flags & SIZETFLAG) ? va_arg(ap, ssize_t) :
					va_arg(ap, int);
				flags |= SIGNEDFLAG;
				s = longlong_to_string(num_buffer, n, sizeof(num_buffer), flags);
				goto _output_string;
			case 'U':
				flags |= LONGFLAG;
				/* fallthrough */
			case 'u':
				n = (flags & LONGLONGFLAG) ? va_arg(ap, unsigned long long) :
					(flags & LONGFLAG) ? va_arg(ap, unsigned long) : 
					(flags & HALFHALFFLAG) ? (unsigned char)va_arg(ap, unsigned int) :
					(flags & HALFFLAG) ? (unsigned short)va_arg(ap, unsigned int) :
					(flags & SIZETFLAG) ? va_arg(ap, size_t) :
					va_arg(ap, unsigned int);
				s = longlong_to_string(num_buffer, n, sizeof(num_buffer), flags);
				goto _output_string;
			case 'p':
				flags |= LONGFLAG | ALTFLAG;
				goto hex;
			case 'X':
				flags |= CAPSFLAG;
				/* fallthrough */
hex:
			case 'x':
				n = (flags & LONGLONGFLAG) ? va_arg(ap, unsigned long long) :
				    (flags & LONGFLAG) ? va_arg(ap, unsigned long) : 
					(flags & HALFHALFFLAG) ? (unsigned char)va_arg(ap, unsigned int) :
					(flags & HALFFLAG) ? (unsigned short)va_arg(ap, unsigned int) :
					(flags & SIZETFLAG) ? va_arg(ap, size_t) :
					va_arg(ap, unsigned int);
				s = longlong_to_hexstring(num_buffer, n, sizeof(num_buffer), flags);
				if(flags & ALTFLAG) {
					OUTPUT_CHAR('0');
					OUTPUT_CHAR((flags & CAPSFLAG) ? 'X': 'x');
				}
				goto _output_string;
			case 'n':
				ptr = va_arg(ap, void *);
				if(flags & LONGLONGFLAG)
					*(long long *)ptr = chars_written;
				else if(flags & LONGFLAG)
					*(long *)ptr = chars_written;
				else if(flags & HALFHALFFLAG)
					*(signed char *)ptr = chars_written;
				else if(flags & HALFFLAG)
					*(short *)ptr = chars_written;
				else if(flags & SIZETFLAG)
					*(size_t *)ptr = chars_written;
				else 
					*(int *)ptr = chars_written;
				break;
			default:
				OUTPUT_CHAR('%');
				OUTPUT_CHAR(c);
				break;
		}

		/* move on to the next field */
		continue;

		/* shared output code */
_output_string:
		if (flags & LEFTFORMATFLAG) {
			/* left justify the text */
			unsigned int count = 0;
			while(*s != 0) {
				OUTPUT_CHAR(*s++);
				count++;
			}

			/* pad to the right (if necessary) */
			for (; format_num > count; format_num--)
				OUTPUT_CHAR(' ');
		} else {
			/* right justify the text (digits) */
			size_t string_len = strlen(s);
			char outchar = (flags & LEADZEROFLAG) ? '0' : ' ';
			for (; format_num > string_len; format_num--)
				OUTPUT_CHAR(outchar);

			/* output the string */
			while(*s != 0)
				OUTPUT_CHAR(*s++);
		}
		continue;
	}

done:
	/* null terminate */
	OUTPUT_CHAR_NOLENCHECK('\0');
	chars_written--; /* don't count the null */

#undef OUTPUT_CHAR
#undef OUTPUT_CHAR_NOLENCHECK

	return chars_written;
}

int _dvprintf(const char *fmt, va_list ap)
{
	char buf[256];
	int err;

	err = vsnprintf(buf, sizeof(buf), fmt, ap);

	puts(buf);

	return err;
}

int printf(const char *fmt, ...)
{
	int err;

	va_list ap;
	va_start(ap, fmt);
	err = _dvprintf(fmt, ap);
	va_end(ap);

	return err;
}