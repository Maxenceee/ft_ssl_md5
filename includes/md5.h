/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   md5.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 16:52:33 by mgama             #+#    #+#             */
/*   Updated: 2025/10/28 13:03:16 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MD5_H
#define MD5_H

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define WORD_LENGTH 4 // 32 bits
#define HASH_LENGTH 16 // 128 bits
#define CHUNK_LENGTH 64 // 512 bits

typedef uint32_t	word_t; // 32 bits
typedef uint32_t	block_t[WORD_LENGTH]; // 128 bits

struct data_s
{
	uint8_t *data;
	size_t length;
};

/**
 * @brief Construct a new hash object from the given input using the MD5 algorithm.
 * 
 * @param input The input data to hash.
 * @param input_length The length of the input data in bytes.
 * @param output An optional output buffer to store the resulting hash (16 bytes).
 * 
 * @return The function returns a non-zero value on failure.
 */
int md5hash(const uint8_t *input, size_t input_length, uint8_t output[HASH_LENGTH]);

#endif /* MD5_H */
