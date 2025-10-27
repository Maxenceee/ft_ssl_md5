/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   md5.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 16:43:17 by mgama             #+#    #+#             */
/*   Updated: 2025/10/27 17:35:58 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "md5.h"

/**
 * Precomputed constants for MD5 algorithm
 * These constants are derived from the sine function and are used in the MD5 hash computation.
 * Which was computed as: floor(2^32 * abs(sin(i + 1))) for i = 0 to 63
 * 
 * @see https://www.rfc-editor.org/rfc/rfc1321 Section 3.4
 */
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

/**
 * Predefined left rotation amounts for each round in the MD5 algorithm.
 * These values determine how many bits to rotate during each operation in the MD5 hash computation.
 * 
 * @see https://www.rfc-editor.org/rfc/rfc1321 Section 3.4
 */
const uint8_t precomputed_round_shift[] = {
	7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
	5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
	4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
	6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,
};

static inline word_t
word_left_rotate(word_t x, uint8_t bits)
{
    return (x << bits) | (x >> (32 - bits));
}

static struct data_s
pad_input(const uint8_t *input, size_t input_length)
{
	/**
	 * The padding process for MD5 involves the following steps:
	 * 1. Append a single '1' bit to the end of the message.
	 * 2. Append '0' bits until the length of the message (in bits) is congruent to 448 modulo 512.
	 * 3. Append the original length of the message (before padding) as a 64-bit little-endian integer.
	 * 
	 * This ensures that the total length of the padded message is a multiple of 512 bits (64 bytes) and
	 * uniquely represents the original message.
	 */

	// Calculate the difference to reach 448 bits mod 512
	size_t pad_len = input_length + 1;
	while (pad_len % 64 != 56)
		pad_len++;

	// The total length after padding and adding the length (8 bytes)
	size_t total_length = pad_len + 8;
	uint8_t *padded_input = malloc(total_length);
	if (!padded_input)
		return (struct data_s){
			.data = NULL,
			.length = 0
		};

	memcpy(padded_input, input, input_length);
	padded_input[input_length] = 0x80; // add one bit '1'

	// Fill with '0' bits up to the required length
	memset(padded_input + input_length + 1, 0, pad_len - input_length - 1);

	// Add the original length in bits as a 64-bit little-endian integer at the end
	uint64_t bit_len = (uint64_t)input_length * 8;
	memcpy(padded_input + pad_len, &bit_len, 8);

	return (struct data_s){
		.data = padded_input,
		.length = total_length
	};
}

int
md5hash(const uint8_t *input, size_t input_length, uint8_t output[HASH_LENGTH])
{
	if (NULL == output)
	{
		return (0);
	}

	/**
	 * Initialize MD5 state variables, as per RFC 1321 section 3.3.
	 */
	uint32_t A0 = 0x67452301;
	uint32_t B0 = 0xefcdab89;
	uint32_t C0 = 0x98badcfe;
	uint32_t D0 = 0x10325476;

	struct data_s padded_input = pad_input(input, input_length);
	if (!padded_input.data)
		return (1);

	/**
	 * The MD5 algorithm processes the input in successive 512-bit (64-byte) chunks.
	 * For each chunk, it performs a series of operations that update the state variables.
	 */
	for (size_t chunk = 0; chunk < padded_input.length; chunk += CHUNK_LENGTH)
	{
		uint8_t *chunk_data = padded_input.data + chunk;

		/**
		 * The chunk is divided into sixteen 32-bit words M[0..15] in little-endian format.
		 */
		uint32_t M[16];
		for (size_t j = 0; j < 16; j++)
		{
			M[j] = *(uint32_t*)(chunk_data + j*4);
		}

		/**
		 * For each chunk, initialize hash value for this chunk.
		 */
		word_t a = A0;
		word_t b = B0;
		word_t c = C0;
		word_t d = D0;

		/**
		 * Main loop of the MD5 algorithm, consisting of 64 operations.
		 * This is a fixed sequence of operations that mix the input data with the state variables.
		 */
		for (size_t i = 0; i < 64; i++)
		{
			uint8_t g;
			uint32_t F;

			/**
			 * The processing is divided into four rounds of 16 operations each,
			 * with different functions and index calculations for each round.
			 */
			switch ((i >> 4))
			{
			case 0:
				F = (b & c) | (~b & d);
				g = i;
				break;
			case 1:
				F = (d & b) | (~d & c);
				g = (5 * i + 1) % 16;
				break;
			case 2:
				F = b ^ c ^ d;
				g = (3 * i + 5) % 16;
				break;
			case 3:
				F = c ^ (b | ~d);
				g = (7 * i) % 16;
				break;
			}

			// Finally combine all the operations and update the state variables

			F = a + F + precomputed_integer_part_of_sines[i] + M[g];

			uint32_t rotated = word_left_rotate(F, precomputed_round_shift[i]);
			uint32_t new_b = b + rotated;

			a = d;
			d = c;
			c = b;
			b = new_b;
		}

		// After processing the chunk, add the chunk's hash to the result so far
		
		A0 += a;
		B0 += b;
		C0 += c;
		D0 += d;
	}

	free(padded_input.data);

	memcpy(output +  0, &A0, 4);
	memcpy(output +  4, &B0, 4);
	memcpy(output +  8, &C0, 4);
	memcpy(output + 12, &D0, 4);

	return (0);
}
