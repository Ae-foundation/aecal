/*
 * Copyright (c) 2026, Ae-foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * software name includes “ae”.
 *
 * THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"

static inline int
number(char *str)
{
	char *s = str;
	uint8_t c;
	int n = 0;

	while ((c = *s++)) {
		if (!isdigit(c))
			return 0;
		n = n * 10 + c - '0';
	}

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
	i = n + 1;
	while (i-- && *(--s) == ' ')
		;
	n = i;
	if (n <= 0)
		return;

	for (i = 0; i < n; i++) {
#if CURRENT_DAY_FLAG == 1
		/* highlighting bit 0x80 */
		if (str[i] & 0x80) {
			printf("\x1b" ANSI_HIGHLIGHT "%c\x1b[0m",
			    (str[i] & 0x3f));
			continue;
		}
#endif

#if DAYOFF_FLAG == 1
		/* highlighting bit 0x40 */
		if (str[i] & 0x40) {
			printf("\x1b" ANSI_HIGHLIGHT_DO "%c\x1b[0m",
			    (str[i] & 0x3f));
			continue;
		}
#endif

		putchar(str[i] & 0x3f);
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

static inline void
easterg(int year, bool orthodox, struct tm *date)
{
	int a, b, c, dval, e, f, g, h, i, k, l, n, day, month;

	if (!date)
		return;
	if (year < 1583)
		orthodox = 1;

	if (orthodox) {
		a = year % 19;
		b = year % 4;
		c = year % 7;
		dval = (19 * a + 15) % 30;
		e = (2 * b + 4 * c + 6 * dval + 6) % 7;
		day = 22 + dval + e;
		day += year / 100 - year / 400 - 2;

		if (day > 61) {
			month = 5;
			day -= 61;
		} else if (day > 31) {
			month = 4;
			day -= 31;
		} else
			month = 3;
	} else {
		a = year % 19;
		b = year / 100;
		c = year % 100;
		dval = b / 4;
		e = b % 4;
		f = (b + 8) / 25;
		g = (b - f + 1) / 3;
		h = (19 * a + b - dval - g + 15) % 30;
		i = c / 4;
		k = c % 4;
		l = (32 + 2 * e + 2 * i - h - k) % 7;
		n = (a + 11 * h + 22 * l) / 451;
		month = (h + l - 7 * n + 114) / 31;
		day = ((h + l - 7 * n + 114) % 31) + 1;
	}

	date->tm_year = year - 1900;
	date->tm_mon = month - 1;
	date->tm_mday = day;
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
		int wb = 0, cb = 0;

		if (i == 3 && mon[m] == 19) {
			i += 11;
			mon[m] += 11;
		}

		/* 0x80 bit for highlighting current day */
		if (i == cur)
			cb = 0x80;

		/* 0x40 bit for highlighting day off */
		if ((mflg && d >= 5) || (!mflg && (d == 0 || d == 6)))
			wb = 0x40;

		/* master processing */
		if (i > 9)
			*s = i / 10 + '0' | cb | wb;
		*++s = i % 10 + '0' | cb | wb;
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
#if OLD_DAY_NAMES == 1
		" M Tu  W Th  F  S  S"
#else
		"Mo Tu We Th Fr Sa Su"
#endif
#else
#if OLD_DAY_NAMES == 1
		" S  M Tu  W Th  F  S"
#else
		"Su Mo Tu We Th Fr Sa"
#endif
#endif
	};

	time_t t = time(NULL);
	struct tm *tm1 = localtime(&t);

	if (c > 1) {
		if ((!strcmp(av[1], "help") || !strcmp(av[1], "-h") ||
			!strcmp(av[1], "h"))) {
			fprintf(stderr, "Usage: %s\n", av[0]);
			fprintf(stderr, "       %s year [mouth]\n", av[0]);
			fprintf(stderr, "       %s -e [year]\n", av[0]);
			fprintf(stderr, "       %s -y\n", av[0]);
			fprintf(stderr, "       %s -h\n", av[0]);
			return 0;
		} else if ((!strcmp(av[1], "year") || !strcmp(av[1], "-y") ||
			       !strcmp(av[1], "y"))) {
			y = tm1->tm_year + 1900;
			goto xlong;
		} else if ((!strcmp(av[1], "easter") || !strcmp(av[1], "-e") ||
			       !strcmp(av[1], "e"))) {
			struct tm tm = { 0 };

			if (c == 3) {
				y = number(av[2]);
				if (y < 1 || y > 9999)
					goto badarg;
			} else
				y = tm1->tm_year + 1900;

			i = 1;
		eastern:
			easterg(y, i, &tm);
			strftime(str, sizeof(str), "%B %e %Y", &tm);
			printf("%s %s\n", str, (i) ? "Catholic" : "Orthodox");

			if (i--)
				goto eastern;

			return 0;
		}
	} else {
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
	puts(dayw);

#define CURRENTDAY(m, y, tm)                                        \
	(((m) == (tm)->tm_mon + 1 && (y) == (tm)->tm_year + 1900) ? \
		(tm)->tm_mday :                                     \
		0)

	cal(m, y, str, 24, CURRENTDAY(m, y, tm1), MONDAY_FLAG);

	for (i = 0; i < 6 * 24; i += 24)
		pstr(str + i, 24);

	putchar('\n');
	return 0;

	/*
	 * print out complete year
	 */
xlong:
	printf("\t\t\t\t%u\n\n", y);

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

		putchar('\n');
	}

	return 0;

badarg:
	puts("Bad argument");
	return 1;
}
