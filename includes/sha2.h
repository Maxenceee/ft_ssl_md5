/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sha2.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 13:05:28 by mgama             #+#    #+#             */
/*   Updated: 2025/11/16 16:16:53 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHA2_H
#define SHA2_H

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define WORD_LENGTH 4 // 32 bits
#define SHA256_HASH_LENGTH 32 // 256 bits
#define CHUNK_LENGTH 64 // 512 bits

typedef uint32_t	word_t; // 32 bits

/**
 * @brief Construct a new hash object from the given input using the MD5 algorithm.
 * 
 * @param input The input data to hash.
 * @param input_length The length of the input data in bytes.
 * @param output An optional output buffer to store the resulting hash (32 bytes).
 * 
 * @return The function returns a non-zero value on failure.
 */
int sha256hash(const uint8_t *input, size_t input_length, uint8_t output[SHA256_HASH_LENGTH]);

#endif /* SHA2_H */
