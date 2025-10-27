/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   files.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 17:17:28 by mgama             #+#    #+#             */
/*   Updated: 2025/10/27 17:19:01 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILES_H
#define FILES_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

/**
 * @brief Reads the entire content of a file descriptor into a dynamically allocated buffer.
 * @returns A pointer to the buffer containing the file content, or NULL on failure.
 */
uint8_t	*ft_read_file(int fd, unsigned char *file, size_t *rsize);

#endif /* FILES_H */