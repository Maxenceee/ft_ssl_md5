/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sha2.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 13:05:28 by mgama             #+#    #+#             */
/*   Updated: 2025/11/18 16:15:02 by mgama            ###   ########.fr       */
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

#define SHA224_HASH_LENGTH 28 // 224 bits length
#define SHA256_HASH_LENGTH 32 // 256 bits length
#define SHA384_HASH_LENGTH 48 // 384 bits length
#define SHA512_HASH_LENGTH 64 // 512 bits length

/**
 * @brief Construct a new hash object from the given input using the SHA-224 algorithm.
 * 
 * @param input The input data to hash.
 * @param input_length The length of the input data in bytes.
 * @param output An optional output buffer to store the resulting hash (28 bytes).
 * 
 * @return The function returns a non-zero value on failure.
 */
int sha224hash(const uint8_t *input, size_t input_length, uint8_t output[SHA224_HASH_LENGTH]);

/**
 * @brief Construct a new hash object from the given input using the SHA-256 algorithm.
 * 
 * @param input The input data to hash.
 * @param input_length The length of the input data in bytes.
 * @param output An optional output buffer to store the resulting hash (32 bytes).
 * 
 * @return The function returns a non-zero value on failure.
 */
int sha256hash(const uint8_t *input, size_t input_length, uint8_t output[SHA256_HASH_LENGTH]);

/**
 * @brief Construct a new hash object from the given input using the SHA-384 algorithm.
 * 
 * @param input The input data to hash.
 * @param input_length The length of the input data in bytes.
 * @param output An optional output buffer to store the resulting hash (48 bytes).
 * 
 * @return The function returns a non-zero value on failure.
 */
int sha384hash(const uint8_t *input, size_t input_length, uint8_t output[SHA384_HASH_LENGTH]);

/**
 * @brief Construct a new hash object from the given input using the SHA-512 algorithm.
 * 
 * @param input The input data to hash.
 * @param input_length The length of the input data in bytes.
 * @param output An optional output buffer to store the resulting hash (64 bytes).
 * 
 * @return The function returns a non-zero value on failure.
 */
int sha512hash(const uint8_t *input, size_t input_length, uint8_t output[SHA512_HASH_LENGTH]);

#endif /* SHA2_H */
