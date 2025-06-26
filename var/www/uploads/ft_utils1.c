/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils1.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbourlot <hbourlot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/19 13:20:56 by hbourlot          #+#    #+#             */
/*   Updated: 2024/05/10 13:23:15 by hbourlot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_format(const char *fmt, va_list *ap)
{
	int	i;

	i = 0;
	if (fmt[0] == 'c')
		i += ft_putchar(va_arg(*ap, int));
	else if (fmt[0] == 's')
		i += ft_putstr(va_arg(*ap, char *));
	else if (fmt[0] == 'p')
		i += handle_flags((long)va_arg(*ap, unsigned long), 'p',
				"0123456789abcdef");
	else if (fmt[0] == 'd' || fmt[0] == 'i')
		i += handle_flags(va_arg(*ap, int), 'i', "0123456789");
	else if (fmt[0] == 'u')
		i += handle_flags(va_arg(*ap, unsigned int), 'u', "0123456789");
	else if (fmt[0] == 'X')
		i += handle_flags(va_arg(*ap, unsigned int), 'X', "0123456789ABCDEF");
	else if (fmt[0] == 'x')
		i += handle_flags(va_arg(*ap, unsigned int), 'x', "0123456789abcdef");
	else if (fmt[0] == '%')
		i += ft_putchar('%');
	return (i);
}

int	ft_printf(const char *fmt, ...)
{
	va_list	ap;
	int		i;
	int		result;

	if (fmt == NULL)
		return (-1);
	va_start(ap, fmt);
	result = 0;
	i = 0;
	while (*fmt)
	{
		if (*fmt == '%')
			result += ft_format(++fmt, &ap);
		else
		{
			ft_putchar(*fmt);
			i++;
		}
		fmt++;
	}
	va_end(ap);
	return (result + i);
}
// #include "ft_printf.h"
// #include <limits.h>
// #include <stdio.h>

// #define FAIL "***************** TEST FAILED **********************\n\n"

// int	main(void)
// {
// 	int a;
// 	int b;

// 	// x tests
// 	printf("x tests\n");
// 	printf(" printf out\n%d printf length\n", a = printf("%x", 0));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x", 0));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x", -1));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x", -1));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x", 1));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x", 1));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x", 10));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x", 10));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x", 99));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x", 99));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x", -101));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x", -101));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x", INT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x",
// 			INT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x", INT_MIN));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x",
// 			INT_MIN));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x", UINT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x",
// 			UINT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%x %x %x %x", INT_MAX,
// 			INT_MIN, 0, -42));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%x %x %x %x",
// 			INT_MAX, INT_MIN, 0, -42));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	// X tests
// 	printf("X tests\n");
// 	printf(" printf out\n%d printf length\n", a = printf("%X", 0));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X", 0));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X", -1));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X", -1));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X", 1));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X", 1));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X", 10));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X", 10));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X", 99));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X", 99));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X", -101));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X", -101));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X", INT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X",
// 			INT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X", INT_MIN));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X",
// 			INT_MIN));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X", UINT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X",
// 			UINT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%X %X %X %X", INT_MAX,
// 			INT_MIN, 0, -42));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%X %X %X %X",
// 			INT_MAX, INT_MIN, 0, -42));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	// c tests
// 	printf("c tests\n");
// 	printf(" printf out\n%d printf length\n", a = printf("%c", '0'));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%c", '0'));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %c", '0'));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %c", '0'));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%c ", '0'));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%c ", '0'));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %c ", '0'));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %c ", '0'));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %c", '0' - 256));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %c", '0'
// 			- 256));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%c ", '0' + 256));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%c ", '0'
// 			+ 256));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %c %c %c ", '1', '2',
// 			'3'));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %c %c %c ",
// 			'1', '2', '3'));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf("%c ", '0' + 256));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%c ", '0'
// 			+ 256));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %c %c %c ", '1', '2',
// 			'3'));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %c %c %c ",
// 			'1', '2', '3'));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	// d & i tests
// 	printf("d & i tests\n");
// 	printf(" printf out\n%d printf length\n", a = printf(" %d ", -99));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %d ", -99));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %d ", 100));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %d ", 100));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %d ", INT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %d ",
// 			INT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %d ", INT_MIN));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %d ",
// 			INT_MIN));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %d ", UINT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %d ",
// 			UINT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	// p tests
// 	printf("p tests\n");
// 	printf(" printf out\n%d printf length\n", a = printf(" %p ", (void *)-1));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %p ",
// 			(void *)-1));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %p ", (void *)1));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %p ",
// 			(void *)1));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %p ", (void *)15));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %p ",
// 			(void *)15));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %p ", (void *)0));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %p ",
// 			(void *)0));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf("p tests\n");
// 	printf(" printf out\n%d printf length\n", a = printf(" %p ", NULL));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %p ",
// 			NULL));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %p %p ",
// 			(void *)LONG_MIN, (void *)LONG_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %p %p ",
// 			(void *)LONG_MIN, (void *)LONG_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %p %p ",
// 			(void *)INT_MIN, (void *)INT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %p %p ",
// 			(void *)INT_MIN, (void *)INT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %p %p ",
// 			(void *)ULONG_MAX, (void *)-ULONG_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %p %p ",
// 			(void *)ULONG_MAX, (void *)-ULONG_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	// s tests

// 	// char *s = "this is a longer test string to test printf";

// 	printf("s tests\n");
// 	printf(" printf out\n%d printf length\n", a = printf("%s", ""));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf("%s", ""));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %s %s ", "", "-"));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %s %s ", "",
// 			"-"));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	// u tests
// 	printf("u tests\n");
// 	printf(" printf out\n%d printf length\n", a = printf(" %u ", -100));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %u ",
// 			-100));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %u ", INT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %u ",
// 			INT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %u ", INT_MIN));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %u ",
// 			INT_MIN));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// 	printf(" printf out\n%d printf length\n", a = printf(" %u ", UINT_MAX));
// 	printf(" ft_printf out\n%d ft_printf length\n", b = ft_printf(" %u ",
// 			UINT_MAX));
// 	a == b ? printf("TEST PASSED\n\n") : printf(FAIL);

// }
