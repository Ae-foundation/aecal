#include <sys/types.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"

static inline int
number(char *str)
{
	char *s = str;
	int n = 0;
	u_char c;

	for (; (c = *s++); n = n * 10 + c - '0')
		if (!isdigit(c))
			return 0;

	return n;
}

static void
pstr(char *str, int n)
{
	char *s = str;
	int i = n;

	while (i--)
		if (*s++ == 0)
			*(s - 1) = ' ';

	for (i = n + 1; i-- && *--s == ' ';)
		;

	n = i;
	for (i = 0; i < n; i++) {
		/* highlighting bit 0x80 */
		if (str[i] & 0x80)
#if CURRENT_DAY_FLAG == 1
			printf("\x1b" ANSI_HIGHLIGHT "%c\x1b[0m",
			    (str[i] & 0x7f));
#else
			printf("%c", (str[i] & 0x7f));
#endif
		else
			putchar(str[i]);
	}
	putchar('\n');
}

/*
 *		J A N 1
 *
 * return day of the week of jan 1 of
 * given year
 */
static inline int
jan1(int yr)
{
	/*
	 * normal gregorian calendar
	 * one extra day per four years
	 */
	int d = 4 + yr + (yr + 3) / 4;

	/*
	 * julian calendar
	 * regular gregorian
	 * less three days per 400
	 */
	if (yr > 1800) {
		d -= (yr - 1701) / 100;
		d += (yr - 1601) / 400;
	}

	/*
	 * great calendar changeover instant
	 */
	if (yr > 1752)
		d += 3;

	return (d % 7);
}

static void
cal(int m, int y, char *p, int w, int cur, bool mflg)
{
	char mon[] = {
		0,
		31,
		29,
		31,
		30,
		31,
		30,
		31,
		31,
		30,
		31,
		30,
		31,
	};
	int d = jan1(y), i;
	char *s = p;

	switch ((jan1(y + 1) + 7 - d) % 7) {
	case 1: /* non-leap year */
		mon[2] = 28;
		break;

	default: /* 1752 */
		mon[9] = 19;
		break;

	case 2:; /* leap year */
	}

	for (i = 1; i < m; i++)
		d += mon[i];

	d %= 7;
	if (mflg)
		d = (d + 6) % 7;
	s += 3 * d;

	for (i = 1; i <= mon[m]; i++) {
		if (i == 3 && mon[m] == 19) {
			i += 11;
			mon[m] += 11;
		}

		if (i > 9)
			*s = i / 10 + '0';

		/* 0x80 bit for highlighting */
		if (i == cur) {
			if (i < 9)
				*s = ' ';
			else
				*s |= 0x80;
		}

		*++s = i % 10 + '0';

		if (i == cur) {
			*s |= 0x80;
			*++s = ' ';
			s++;
		} else
			s += 2;

		if (++d == 7) {
			d = 0;
			s = p + w;
			p = s;
		}
	}
}

int
main(int c, char **av)
{
	char *smon[] = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December",
	};
	char str[432] = { 0 };
	int y, i, j, m;

	char dayw[64] = {
#if MONDAY_FLAG == 1
	" M Tu  W Th  F  S  S"
#else
	" S  M Tu  W Th  F  S"
#endif
	};

	time_t t = time(NULL);
	struct tm *tm1 = localtime(&t);

	if (c == 2) {
		if ((!strcmp(av[1], "help") || !strcmp(av[1], "-h") ||
			!strcmp(av[1], "h"))) {
			fprintf(stderr, "Usage: %s year [mouth]\n", av[0]);
			return 0;
		}
		if ((!strcmp(av[1], "year") || !strcmp(av[1], "-y") ||
			!strcmp(av[1], "y"))) {
			y = tm1->tm_year + 1900;
			goto xlong;
		}
	}

	if (c < 2) {
		m = tm1->tm_mon + 1;
		y = tm1->tm_year + 1900;

		goto xshort;
	}

	y = number(av[1]);
	if (y < 1 || y > 9999)
		goto badarg;

	if (c == 2)
		goto xlong;

	m = number(av[2]);
	if (m < 1 || m > 12)
		goto badarg;

	/*
	 * print out just month
	 */
xshort:
	printf("    %s %u\n", smon[m - 1], y);
	printf("%s\n", dayw);

#define CURRENTDAY(m, y, tm)                                        \
	(((m) == (tm)->tm_mon + 1 && (y) == (tm)->tm_year + 1900) ? \
		(tm)->tm_mday :                                     \
		0)

	cal(m, y, str, 24, CURRENTDAY(m, y, tm1), MONDAY_FLAG);

	for (i = 0; i < 6 * 24; i += 24)
		pstr(str + i, 24);

	return 0;

	/*
	 * print out complete year
	 */
xlong:
	printf("\t\t\t\t%u\n", y);
	putchar('\n');

	for (i = 0; i < 12; i += 3) {
		memset(str, 0, 432);
		printf("	 %.3s", smon[i]);
		printf("\t\t\t%.3s", smon[i + 1]);
		printf("		       %.3s\n", smon[i + 2]);
		printf("%s   %s   %s\n", dayw, dayw, dayw);

		cal(i + 1, y, str, 72, CURRENTDAY(i + 1, y, tm1), MONDAY_FLAG);
		cal(i + 2, y, str + 23, 72, CURRENTDAY(i + 2, y, tm1),
		    MONDAY_FLAG);
		cal(i + 3, y, str + 46, 72, CURRENTDAY(i + 3, y, tm1),
		    MONDAY_FLAG);

		for (j = 0; j < 432; j += 72)
			pstr(str + j, 72);
	}

	return 0;

badarg:
	puts("Bad argument");
	return 1;
}
