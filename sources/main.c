/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 21:15:18 by mgama             #+#    #+#             */
/*   Updated: 2025/10/26 22:17:44 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pcolors.h"

typedef uint8_t	word_t[16];

void
print_word_hex(const word_t word)
{
	for (size_t i = 0; i < 16; i++)
	{
		printf("%02x ", word[i]);
	}
	printf("\n");
}

void
print_word_bin(const word_t word)
{
	for (size_t i = 0; i < 16; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			uint8_t bit = (word[i] >> (7 - j)) & 1;
			printf("%s%d%s", bit ? GREEN : RED, bit, RESET);
		}
		printf(" ");
	}
	printf("\n");
}

uint8_t*
word_or(word_t dst, const word_t src)
{
	for (size_t i = 0; i < 16; i++)
		dst[i] |= src[i];

	return (dst);
}

uint8_t*
word_and(word_t dst, const word_t src)
{
	for (size_t i = 0; i < 16; i++)
		dst[i] &= src[i];

	return (dst);
}

uint8_t*
word_xor(word_t dst, const word_t src)
{
	for (size_t i = 0; i < 16; i++)
		dst[i] ^= src[i];

	return (dst);
}

uint8_t*
word_not(word_t dst)
{
	for (size_t i = 0; i < 16; i++)
		dst[i] = ~dst[i];

	return (dst);
}

uint8_t*
word_left_rotate(word_t dst, uint8_t bits)
{
	for (size_t i = 0; i < 16; i++)
		dst[i] = (dst[i] << bits) | (dst[i] >> (8 - bits));

	return (dst);
}

uint8_t*
word_right_rotate(word_t dst, uint8_t bits)
{
	for (size_t i = 0; i < 16; i++)
		dst[i] = (dst[i] >> bits) | (dst[i] << (8 - bits));

	return (dst);
}

uint8_t*
word_add(word_t dst, const word_t src)
{
	for (size_t i = 0; i < 16; i++)
	{
		uint16_t sum = dst[i] + src[i];
		dst[i] = (uint8_t)(sum & 0xFF);
	}

	return (dst);
}

int
main(void)
{
	word_t word_1;
	word_t word_2;

	printf("-- OR:\n");

	bcopy("Hello, World!!!", word_1, 16);
	bcopy("I miss you!!!", word_2, 16);

	print_word_bin(word_1);
	print_word_bin(word_2);

	print_word_bin(word_or(word_1, word_2));

	printf("\n");

	printf("-- AND:\n");

	bcopy("Hello, World!!!", word_1, 16);
	bcopy("I miss you!!!", word_2, 16);

	print_word_bin(word_1);
	print_word_bin(word_2);

	print_word_bin(word_and(word_1, word_2));

	printf("\n");

	printf("-- XOR:\n");

	bcopy("Hello, World!!!", word_1, 16);
	bcopy("I miss you!!!", word_2, 16);

	print_word_bin(word_1);
	print_word_bin(word_2);

	print_word_bin(word_xor(word_1, word_2));

	printf("\n");

	printf("-- NOT:\n");

	bcopy("Hello, World!!!", word_1, 16);
	bcopy("I miss you!!!", word_2, 16);

	print_word_bin(word_1);
	print_word_bin(word_not(word_1));
	
	print_word_bin(word_2);
	print_word_bin(word_not(word_2));

	printf("\n");

	printf("-- RLEFT:\n");

	bcopy("Hello, World!!!", word_1, 16);
	bcopy("I miss you!!!", word_2, 16);

	print_word_bin(word_1);
	print_word_bin(word_left_rotate(word_1, 2));

	printf("\n");

	print_word_bin(word_2);
	print_word_bin(word_left_rotate(word_2, 2));

	printf("\n");

	printf("-- RRIGHT:\n");

	bcopy("Hello, World!!!", word_1, 16);
	bcopy("I miss you!!!", word_2, 16);

	print_word_bin(word_1);
	print_word_bin(word_right_rotate(word_1, 2));
	print_word_bin(word_right_rotate(word_1, 2));

	printf("\n");

	print_word_bin(word_2);
	print_word_bin(word_right_rotate(word_2, 2));
	print_word_bin(word_right_rotate(word_2, 2));

	printf("\n");

	printf("-- ADD:\n");

	bcopy("Hello, World!!!", word_1, 16);
	bcopy("I miss you!!!", word_2, 16);

	print_word_bin(word_1);
	print_word_bin(word_2);
	print_word_bin(word_add(word_1, word_2));

	printf("\n");

	return (0);
}
