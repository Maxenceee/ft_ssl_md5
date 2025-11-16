/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sha224.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 13:05:07 by mgama             #+#    #+#             */
/*   Updated: 2025/11/16 17:18:29 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "sha2.h"

#define WORD_LENGTH 4 // 32 bits
#define CHUNK_LENGTH 64 // 512 bits

typedef uint32_t	word_t; // 32 bits

/**
 * Precomputed constants for SHA-224 algorithm
 * These constants are derived from the fractional parts of the square roots of the first 64 prime numbers
 * and are used in the SHA-224 hash computation.
 * Which was computed as: floor(2^32 * frac(sqrt(prime(i)))) for i = 0 to 63
 * 
 * @see https://www.rfc-editor.org/rfc/rfc4634 Section 5.1
 */
static const word_t precomputed_fractional_part_of_prime_sqrt[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static inline word_t
word_right_rotate(word_t x, uint8_t bits)
{
	return (x >> bits) | (x << (sizeof(x) * 8 - bits));
}

static inline word_t
choice(word_t x, word_t y, word_t z)
{
	return (x & y) ^ (~x & z);
}

static inline word_t
majority(word_t x, word_t y, word_t z)
{
	return (x & y) ^ (x & z) ^ (y & z);
}

static inline word_t
ssigma0(word_t x)
{
	return word_right_rotate(x, 7) ^ word_right_rotate(x, 18) ^ (x >> 3);
}

static inline word_t
ssigma1(word_t x)
{
	return word_right_rotate(x, 17) ^ word_right_rotate(x, 19) ^ (x >> 10);
}
	
static inline word_t
bsigma0(word_t x)
{
	return word_right_rotate(x, 2) ^ word_right_rotate(x, 13) ^ word_right_rotate(x, 22);
}

static inline word_t
bsigma1(word_t x)
{
	return word_right_rotate(x, 6) ^ word_right_rotate(x, 11) ^ word_right_rotate(x, 25);
}

static uint8_t*
pad_input(const uint8_t *input, size_t input_length, size_t *new_length)
{
	/**
	 * The padding process for SHA-2 involves the following steps:
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
	{
		*new_length = 0;
		return (NULL);
	}

	memcpy(padded_input, input, input_length);
	padded_input[input_length] = 0x80; // add one bit '1'

	// Fill with '0' bits up to the required length
	memset(padded_input + input_length + 1, 0, pad_len - input_length - 1);

	// Add the original length in bits as a 64-bit little-endian integer at the end
	uint64_t bit_len = (uint64_t)input_length * 8;
	for (int i = 0; i < 8; i++)
	{
		padded_input[pad_len + 7 - i] = (uint8_t)(bit_len >> (i * 8));
	}

	*new_length = total_length;
	return (padded_input);
}

static inline word_t
lecpy32(word_t w)
{
	return ((w & 0x000000FF) << 24) |
			((w & 0x0000FF00) << 8) |
			((w & 0x00FF0000) >> 8) |
			((w & 0xFF000000) >> 24);
}

int
sha224hash(const uint8_t *input, size_t input_length, uint8_t output[SHA224_HASH_LENGTH])
{
	if (NULL == input || 0 == input_length || NULL == output)
	{
		return (0);
	}

	/**
	 * Initial hash values for SHA-224, as RFC 6234 specifies
	 */
	word_t H0 = 0xc1059ed8;
	word_t H1 = 0x367cd507;
	word_t H2 = 0x3070dd17;
	word_t H3 = 0xf70e5939;
	word_t H4 = 0xffc00b31;
	word_t H5 = 0x68581511;
	word_t H6 = 0x64f98fa7;
	word_t H7 = 0xbefa4fa4;

	size_t padded_input_length;
	uint8_t *padded_input = pad_input(input, input_length, &padded_input_length);
	if (!padded_input)
		return (1);

	/**
	 * The SHA-224 algorithm processes the input in successive 512-bit (64-byte) chunks.
	 * For each chunk, it performs a series of operations that update the state variables.
	 */
	for (size_t chunk = 0; chunk < padded_input_length; chunk += CHUNK_LENGTH)
	{
		uint8_t *chunk_data = padded_input + chunk;

		/**
		 * The chunk is divided into sixty-four 32-bit words W[0..63] in big-endian format.
		 */
		word_t w[64];
		/**
		 * Fill the first 16 words W[0..15] with the chunk data
		 */
		for (size_t j = 0; j < 16; j++)
		{
			w[j] = lecpy32(*(word_t *)(chunk_data + j*4));
		}
		/**
		 * Fill the remaining words W[16..63] using the formula 
		 * W[t] = σ1(W[t-2]) + W[t-7] + σ0(W[t-15]) + W[t-16]
		 * where σ0 and σ1 are defined as:
		 * σ0(x) = ROTR^7(x) ⊕ ROTR^18(x) ⊕ SHR^3(x)
		 * σ1(x) = ROTR^17(x) ⊕ ROTR^19(x) ⊕ SHR^10(x)
		 */
		for (size_t j = 16; j < 64; j++)
		{
			w[j] = ssigma1(w[j-2]) + w[j-7] + ssigma0(w[j-15]) + w[j-16];
		}

		/**
		 * For each chunk, initialize hash value for this chunk.
		 */
		word_t a = H0;
		word_t b = H1;
		word_t c = H2;
		word_t d = H3;
		word_t e = H4;
		word_t f = H5;
		word_t g = H6;
		word_t h = H7;

		/**
		 * The main loop of the SHA-224 algorithm, consisting of 64 operations.
		 * This is a fixed sequence of operations that mix the input data with the state variables.
		 */
		for (size_t i = 0; i < 64; i++)
		{
			word_t T1 = h + bsigma1(e) + choice(e, f, g) + precomputed_fractional_part_of_prime_sqrt[i] + w[i];
			word_t T2 = bsigma0(a) + majority(a, b, c);

			h = g;
			g = f;
			f = e;
			e = d + T1;
			d = c;
			c = b;
			b = a;
			a = T1 + T2;
		}

		// After processing the chunk, add the chunk's hash to the result so far

		H0 += a;
		H1 += b;
		H2 += c;
		H3 += d;
		H4 += e;
		H5 += f;
		H6 += g;
		H7 += h;
	}

	free(padded_input);

	word_t final_hash[] =
	{
		lecpy32(H0),
		lecpy32(H1),
		lecpy32(H2),
		lecpy32(H3),
		lecpy32(H4),
		lecpy32(H5),
		lecpy32(H6),
		lecpy32(H7),
	};

	memcpy(output, final_hash, SHA224_HASH_LENGTH);

	return (0);
}
