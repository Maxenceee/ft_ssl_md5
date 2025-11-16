/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sha512.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 13:05:07 by mgama             #+#    #+#             */
/*   Updated: 2025/11/16 17:29:46 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "sha2.h"

#define WORD_LENGTH 8 // 64 bits
#define CHUNK_LENGTH 128 // 1024 bits

typedef uint64_t	word_t; // 64 bits

/**
 * Precomputed constants for SHA-512 algorithm
 * These constants are derived from the fractional parts of the square roots of the first 80 prime numbers
 * and are used in the SHA-512 hash computation.
 * Which was computed as: floor(2^64 * frac(sqrt(prime(i)))) for i = 0 to 79
 * 
 * @see https://www.rfc-editor.org/rfc/rfc4634 Section 5.1
 */
static const word_t precomputed_fractional_part_of_prime_sqrt[] = {
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538, 
	0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe, 
	0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 
	0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab, 
	0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725, 
	0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 
	0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b, 
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218, 
	0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 
	0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 
	0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec, 
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c, 
	0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6, 
	0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 
	0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
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
	return word_right_rotate(x, 1) ^ word_right_rotate(x, 8) ^ (x >> 7);
}

static inline word_t
ssigma1(word_t x)
{
	return word_right_rotate(x, 19) ^ word_right_rotate(x, 61) ^ (x >> 6);
}
	
static inline word_t
bsigma0(word_t x)
{
	return word_right_rotate(x, 28) ^ word_right_rotate(x, 34) ^ word_right_rotate(x, 39);
}

static inline word_t
bsigma1(word_t x)
{
	return word_right_rotate(x, 14) ^ word_right_rotate(x, 18) ^ word_right_rotate(x, 41);
}

static uint8_t*
pad_input(const uint8_t *input, size_t input_length, size_t *new_length)
{
	/**
	 * The padding process for SHA-2 involves the following steps:
	 * 1. Append a single '1' bit to the end of the message.
	 * 2. Append '0' bits until the length of the message (in bits) is congruent to 896 modulo 1024.
	 * 3. Append the original length of the message (before padding) as a 128-bit little-endian integer.
	 * 
	 * This ensures that the total length of the padded message is a multiple of 1024 bits (128 bytes) and
	 * uniquely represents the original message.
	 */

	size_t pad_len = input_length + 1;
	while (pad_len % 128 != 112)
		pad_len++;

	/* total length after padding + 16 bytes for length */
	size_t total_length = pad_len + 16;
	uint8_t *padded_input = malloc(total_length);
	if (!padded_input)
	{
		*new_length = 0;
		return (NULL);
	}

	memcpy(padded_input, input, input_length);
	padded_input[input_length] = 0x80;
	if (pad_len > input_length + 1)
		memset(padded_input + input_length + 1, 0, pad_len - input_length - 1);

	/* write the message length in bits as a 128-bit big-endian integer */
	__uint128_t bit_len = (__uint128_t)input_length * 8;
	for (int i = 0; i < 16; i++)
	{
		padded_input[pad_len + 15 - i] = (uint8_t)(bit_len & 0xFF);
		bit_len >>= 8;
	}

	*new_length = total_length;
	return (padded_input);
}

static inline word_t
lecpy64(word_t w)
{
	return ((w & 0x00000000000000FFULL) << 56) |
           ((w & 0x000000000000FF00ULL) << 40) |
           ((w & 0x0000000000FF0000ULL) << 24) |
           ((w & 0x00000000FF000000ULL) << 8)  |
           ((w & 0x000000FF00000000ULL) >> 8)  |
           ((w & 0x0000FF0000000000ULL) >> 24) |
           ((w & 0x00FF000000000000ULL) >> 40) |
           ((w & 0xFF00000000000000ULL) >> 56);
}

int
sha512hash(const uint8_t *input, size_t input_length, uint8_t output[SHA512_HASH_LENGTH])
{
	if (NULL == input || 0 == input_length || NULL == output)
	{
		return (0);
	}

	/**
	 * Initial hash values for SHA-512, as RFC 6234 specifies
	 */
	word_t H0 = 0x6a09e667f3bcc908;
	word_t H1 = 0xbb67ae8584caa73b;
	word_t H2 = 0x3c6ef372fe94f82b;
	word_t H3 = 0xa54ff53a5f1d36f1;
	word_t H4 = 0x510e527fade682d1;
	word_t H5 = 0x9b05688c2b3e6c1f;
	word_t H6 = 0x1f83d9abfb41bd6b;
	word_t H7 = 0x5be0cd19137e2179;

	size_t padded_input_length;
	uint8_t *padded_input = pad_input(input, input_length, &padded_input_length);
	if (!padded_input)
		return (1);

	/**
	 * The SHA-512 algorithm processes the input in successive 1024-bit (128-byte) chunks.
	 * For each chunk, it performs a series of operations that update the state variables.
	 */
	for (size_t chunk = 0; chunk < padded_input_length; chunk += CHUNK_LENGTH)
	{
		uint8_t *chunk_data = padded_input + chunk;

		/**
		 * The chunk is divided into sixty-four 32-bit words W[0..79] in big-endian format.
		 */
		word_t w[80];
		/**
		 * Fill the first 16 words W[0..15] with the chunk data
		 */
		for (size_t j = 0; j < 16; j++)
		{
			w[j] = lecpy64(*(word_t *)(chunk_data + j*8));
		}
		/**
		 * Fill the remaining words W[16..79] using the formula 
		 * W[t] = σ1(W[t-2]) + W[t-7] + σ0(W[t-15]) + W[t-16]
		 * where σ0 and σ1 are defined as:
		 * σ0(x) = ROTR^1(x) ⊕ ROTR^8(x) ⊕ SHR^7(x)
		 * σ1(x) = ROTR^19(x) ⊕ ROTR^61(x) ⊕ SHR^6(x)
		 */
		for (size_t j = 16; j < 80; j++)
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
		 * The main loop of the SHA-512 algorithm, consisting of 64 operations.
		 * This is a fixed sequence of operations that mix the input data with the state variables.
		 */
		for (size_t i = 0; i < 80; i++)
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
		lecpy64(H0),
		lecpy64(H1),
		lecpy64(H2),
		lecpy64(H3),
		lecpy64(H4),
		lecpy64(H5),
		lecpy64(H6),
		lecpy64(H7),
	};

	memcpy(output, final_hash, SHA512_HASH_LENGTH);

	return (0);
}
