/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 21:15:18 by mgama             #+#    #+#             */
/*   Updated: 2025/10/27 13:49:38 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "pcolors.h"

const uint32_t precomputed_integer_part_of_sines[] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

const uint8_t precomputed_round_shift[] = {
	7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
	5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
	4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
	6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,
};

#define WORD_LENGTH 4 // 32 bits
#define BLOCK_LENGTH 16 // 128 bits
#define CHUNK_LENGTH 64 // 512 bits

typedef uint8_t	word_t[WORD_LENGTH]; // 32 bits
typedef uint8_t	block_t[BLOCK_LENGTH]; // 128 bits

typedef struct state_s
{
	word_t	A;
	word_t	B;
	word_t	C;
	word_t	D;
} state_t;

void
print_hex(const word_t word, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		printf("%02x ", word[i]);
	}
	printf("\n");
}

void
print_hash(const uint8_t *hash)
{
	for (size_t i = 0; i < BLOCK_LENGTH; i++)
	{
		printf("%02x", hash[i]);
	}
	printf("\n");
}

void
print_bin(const word_t word, size_t length)
{
	for (size_t i = 0; i < length; i++)
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
	for (size_t i = 0; i < WORD_LENGTH; i++)
		dst[i] |= src[i];

	return (dst);
}

uint8_t*
word_and(word_t dst, const word_t src)
{
	for (size_t i = 0; i < WORD_LENGTH; i++)
		dst[i] &= src[i];

	return (dst);
}

uint8_t*
word_xor(word_t dst, const word_t src)
{
	for (size_t i = 0; i < WORD_LENGTH; i++)
		dst[i] ^= src[i];

	return (dst);
}

uint8_t*
word_not(word_t dst)
{
	for (size_t i = 0; i < WORD_LENGTH; i++)
		dst[i] = ~dst[i];

	return (dst);
}

uint8_t*
word_left_rotate(word_t dst, uint8_t bits)
{
	unsigned char *byte, *last;
	unsigned char firstBit;
	byte = dst;
	last = byte + bits-1;
	firstBit = byte[0] >> (CHAR_BIT-1);
	while (byte < last) {
		byte[0] = (byte[0] << 1) | (byte[1] >> (CHAR_BIT-1));
		++byte;
	}
	(*last) = (*last) << 1 | firstBit;

	return (dst);
}

uint8_t*
word_right_rotate(word_t dst, uint8_t bits)
{
	for (size_t i = 0; i < WORD_LENGTH; i++)
	{
		dst[i] = (dst[i] << bits) | (dst[i] >> ((8 * WORD_LENGTH) - bits));
	}

	return (dst);
}

uint8_t*
word_add(word_t dst, const word_t src)
{
	for (size_t i = 0; i < WORD_LENGTH; i++)
	{
		uint16_t sum = dst[i] + src[i];
		dst[i] = (uint8_t)(sum & 0xFF);
	}

	return (dst);
}

struct data_s
{
	uint8_t *data;
	size_t length;
};

struct data_s
pad_input(const uint8_t *input, size_t input_length)
{
	size_t total_length = input_length + input_length % BLOCK_LENGTH;
	// printf("original length: %zu total length: %zu\n", input_length, total_length);

	uint8_t *padded_input = (uint8_t *)calloc(total_length, sizeof(uint8_t));
	if (!padded_input)
		return (struct data_s){NULL, 0};
	memcpy(padded_input, input, input_length);

	memset(padded_input + input_length, 0b10000000, 1); // Append the '1' bit

	size_t remainder = input_length;
	// printf("remainder: %zu\n", remainder);
	memcpy(padded_input + input_length + 1, &remainder, 8); // Append original length at the end

	// print_bin(input, input_length);
	// printf("\n");
	// print_bin(padded_input, total_length);
	// printf("\n");

	return (struct data_s){
		.data = padded_input,
		.length = total_length
	};
}

// state_t
// get_next_state(const uint8_t *input_block)
// {
// 	static size_t offset = 0;

// 	state_t state = {
// 		.A = {input_block[offset + 0], input_block[offset + 1], input_block[offset + 2], input_block[offset + 3]},
// 		.B = {input_block[offset + 4], input_block[offset + 5], input_block[offset + 6], input_block[offset + 7]},
// 		.C = {input_block[offset + 8], input_block[offset + 9], input_block[offset + 10], input_block[offset + 11]},
// 		.D = {input_block[offset + 12], input_block[offset + 13], input_block[offset + 14], input_block[offset + 15]},
// 	};

// 	offset += BLOCK_LENGTH;

// 	return (state);
// }

void
test_words(void)
{
	word_t word_1;
	word_t word_2;

	// printf("-- OR:\n");

	// bcopy("Helo", word_1, WORD_LENGTH);
	// bcopy("Miss", word_2, WORD_LENGTH);

	// print_bin(word_1, WORD_LENGTH);
	// print_bin(word_2, WORD_LENGTH);

	// print_bin(word_or(word_1, word_2), WORD_LENGTH);

	// printf("\n");

	// printf("-- AND:\n");

	// bcopy("Helo", word_1, WORD_LENGTH);
	// bcopy("Miss", word_2, WORD_LENGTH);

	// print_bin(word_1, WORD_LENGTH);
	// print_bin(word_2, WORD_LENGTH);

	// print_bin(word_and(word_1, word_2), WORD_LENGTH);

	// printf("\n");

	// printf("-- XOR:\n");

	// bcopy("Helo", word_1, WORD_LENGTH);
	// bcopy("Miss", word_2, WORD_LENGTH);

	// print_bin(word_1, WORD_LENGTH);
	// print_bin(word_2, WORD_LENGTH);

	// print_bin(word_xor(word_1, word_2), WORD_LENGTH);

	// printf("\n");

	// printf("-- NOT:\n");

	// bcopy("Helo", word_1, WORD_LENGTH);
	// bcopy("Miss", word_2, WORD_LENGTH);

	// print_bin(word_1, WORD_LENGTH);
	// print_bin(word_not(word_1), WORD_LENGTH);
	
	// print_bin(word_2, WORD_LENGTH);
	// print_bin(word_not(word_2), WORD_LENGTH);

	// printf("\n");

	printf("-- RLEFT(2):\n");

	bcopy("Helo", word_1, WORD_LENGTH);
	bcopy("Miss", word_2, WORD_LENGTH);

	print_bin(word_1, WORD_LENGTH);
	print_bin(word_left_rotate(word_1, 2), WORD_LENGTH);

	printf("\n");

	print_bin(word_2, WORD_LENGTH);
	print_bin(word_left_rotate(word_2, 2), WORD_LENGTH);

	printf("\n");

	printf("-- RLEFT(7):\n");

	bcopy("Helo", word_1, WORD_LENGTH);
	bcopy("Miss", word_2, WORD_LENGTH);

	print_bin(word_1, WORD_LENGTH);
	print_bin(word_left_rotate(word_1, 7), WORD_LENGTH);

	printf("\n");

	print_bin(word_2, WORD_LENGTH);
	print_bin(word_left_rotate(word_2, 7), WORD_LENGTH);

	printf("\n");

	printf("-- RLEFT(12):\n");

	bcopy("Helo", word_1, WORD_LENGTH);
	bcopy("Miss", word_2, WORD_LENGTH);

	print_bin(word_1, WORD_LENGTH);
	print_bin(word_left_rotate(word_1, 12), WORD_LENGTH);

	printf("\n");

	print_bin(word_2, WORD_LENGTH);
	print_bin(word_left_rotate(word_2, 12), WORD_LENGTH);

	printf("\n");

	printf("-- RLEFT(21):\n");

	bcopy("Helo", word_1, WORD_LENGTH);
	bcopy("Miss", word_2, WORD_LENGTH);

	print_bin(word_1, WORD_LENGTH);
	print_bin(word_left_rotate(word_1, 21), WORD_LENGTH);

	printf("\n");

	print_bin(word_2, WORD_LENGTH);
	print_bin(word_left_rotate(word_2, 21), WORD_LENGTH);

	printf("\n");

	// printf("-- RRIGHT:\n");

	// bcopy("Helo", word_1, WORD_LENGTH);
	// bcopy("Miss", word_2, WORD_LENGTH);

	// print_bin(word_1, WORD_LENGTH);
	// print_bin(word_right_rotate(word_1, 2), WORD_LENGTH);
	// print_bin(word_right_rotate(word_1, 2), WORD_LENGTH);

	// printf("\n");

	// print_bin(word_2, WORD_LENGTH);
	// print_bin(word_right_rotate(word_2, 2), WORD_LENGTH);
	// print_bin(word_right_rotate(word_2, 2), WORD_LENGTH);

	// printf("\n");

	// printf("-- ADD:\n");

	// bcopy("Helo", word_1, WORD_LENGTH);
	// bcopy("Miss", word_2, WORD_LENGTH);

	// print_bin(word_1, WORD_LENGTH);
	// print_bin(word_2, WORD_LENGTH);
	// print_bin(word_add(word_1, word_2), WORD_LENGTH);

	// printf("\n");
}

int
main(void)
{
	test_words();

	const uint8_t input_1[] = "Lorem ipsum dolor sit amet consectetur adipisicing elit.";
	// const uint8_t input_2[] = "Explicabo velit obcaecati soluta. Ea velit ipsa eius, explicabo pariatur ratione voluptatem nulla exercitationem.";
	// const uint8_t input_3[] = "Repudiandae esse ea saepe dolores porro earum ut?";

	// Initial state values
	// state_t current_state = {
	// 	.A = {0x67, 0x45, 0x23, 0x01},
	// 	.B = {0xef, 0xcd, 0xab, 0x89},
	// 	.C = {0x98, 0xba, 0xdc, 0xfe},
	// 	.D = {0x10, 0x32, 0x54, 0x76},
	// };

	// struct data_s padded_input_1 = pad_input(input_1, strlen((const char*)input_1));

	// for (size_t chunk = 0; chunk < padded_input_1.length; chunk += CHUNK_LENGTH)
	// {
	// 	uint8_t *chunk_data = padded_input_1.data + chunk;

	// 	for (size_t i = 0; i < CHUNK_LENGTH; i += 1)
	// 	{
	// 		printf("\nProcessing block starting at byte %zu (r=%zu):\n", i, (i >> 4));
	// 		uint8_t *input_block;
	// 		word_t F;

	// 		switch ((i >> 4))
	// 		{
	// 		case 0: {
	// 			printf("Using input block at index %zu\n", i);
	// 			uint8_t *tf = word_or(
	// 				word_and(
	// 					current_state.B,
	// 					current_state.C
	// 				),
	// 				word_and(
	// 					word_not(current_state.B),
	// 					current_state.D
	// 				)
	// 			);
	// 			memcpy(F, tf, WORD_LENGTH);
	// 			input_block = chunk_data + i;
	// 			break;
	// 		}
	// 		case 1: {
	// 			printf("Using input block at index %zu\n", (5 * i + 1) % 16);
	// 			uint8_t *tf = word_or(
	// 				word_and(
	// 					current_state.D,
	// 					current_state.B
	// 				),
	// 				word_and(
	// 					word_not(current_state.D),
	// 					current_state.C
	// 				)
	// 			);
	// 			input_block = chunk_data + ((5 * i) + 1) % 16;
	// 			break;
	// 		}
	// 		case 2: {
	// 			printf("Using input block at index %zu\n", (3 * i + 5) % 16);
	// 			uint8_t *tf = word_xor(
	// 				current_state.B,
	// 				word_xor(
	// 					current_state.C,
	// 					current_state.D
	// 				)
	// 			);
	// 			memcpy(F, tf, WORD_LENGTH);
	// 			input_block = chunk_data + ((3 * i) + 5) % 16;
	// 			break;
	// 		}
	// 		case 3: {
	// 			printf("Using input block at index %zu\n", (7 * i) % 16);
	// 			uint8_t *tf = word_xor(
	// 				current_state.C,
	// 				word_or(
	// 					current_state.B,
	// 					word_not(current_state.D)
	// 				)
	// 			);
	// 			memcpy(F, tf, WORD_LENGTH);
	// 			input_block = chunk_data + (7 * i) % 16;
	// 			break;
	// 		}
	// 		}

	// 		printf("F="); print_bin(F, WORD_LENGTH);
	// 		word_add(F, current_state.A);
	// 		printf("F + A="); print_bin(F, WORD_LENGTH);
	// 		word_add(F, input_block);
	// 		printf("F + A + M[g]="); print_bin(F, WORD_LENGTH);
	// 		word_add(F, (uint8_t *)&precomputed_integer_part_of_sines[i]);
	// 		printf("F + A + M[g] + K[i]="); print_bin(F, WORD_LENGTH);
	// 		word_left_rotate(F, precomputed_round_shift[i]);
	// 		printf("LEFTROT(F + A + M[g] + K[i], s[i](%i))=", precomputed_round_shift[i]); print_bin(F, WORD_LENGTH);
	// 		word_add(F, current_state.B);
	// 		printf("F + B="); print_bin(F, WORD_LENGTH);

	// 		// printf("A="); print_bin(current_state.A, WORD_LENGTH);
	// 		// printf("B="); print_bin(current_state.B, WORD_LENGTH);
	// 		// printf("C="); print_bin(current_state.C, WORD_LENGTH);
	// 		// printf("D="); print_bin(current_state.D, WORD_LENGTH);

	// 		state_t next_state;

	// 		memcpy(next_state.A, current_state.D, WORD_LENGTH);
	// 		memcpy(next_state.B, F, WORD_LENGTH);
	// 		memcpy(next_state.C, current_state.B, WORD_LENGTH);
	// 		memcpy(next_state.D, current_state.C, WORD_LENGTH);

	// 		// Update current state for next block
	// 		memcpy(&current_state, &next_state, sizeof(state_t));
	// 	}
	// }

	// uint8_t final_hash[BLOCK_LENGTH];

	// memcpy(final_hash + 0, current_state.A, WORD_LENGTH);
	// memcpy(final_hash + 4, current_state.B, WORD_LENGTH);
	// memcpy(final_hash + 8, current_state.C, WORD_LENGTH);
	// memcpy(final_hash + 12, current_state.D, WORD_LENGTH);

	// print_hash(final_hash);

	return (0);
}
