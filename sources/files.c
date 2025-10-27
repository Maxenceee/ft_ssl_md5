/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   files.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 17:16:57 by mgama             #+#    #+#             */
/*   Updated: 2025/10/27 17:18:02 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "files.h"

static uint8_t*
ft_memjoin(unsigned char *s1, unsigned char *s2, size_t size, size_t length)
{
	unsigned char	*str;

	if (!s1)
		s1 = (unsigned char *)malloc(sizeof(unsigned char));
	if (!s1)
		return (NULL);
	if (!s2)
		return (free(s1), NULL);
	str = malloc(sizeof(unsigned char) * (size + length));
	if (!str)
		return (free(s1), NULL);
	memcpy(str, s1, size);
	memcpy(str + size, s2, length);
	free(s1);
	return (str);
}

uint8_t*
ft_read_file(int fd, unsigned char *file, size_t *rsize)
{
	unsigned char	*buff;
	int				read_bytes;
	size_t			size;

	size = 0;
	buff = malloc((BUFFER_SIZE + 1) * sizeof(unsigned char));
	if (!buff)
		return (NULL);
	read_bytes = 1;
	while (read_bytes != 0)
	{
		read_bytes = read(fd, buff, BUFFER_SIZE);
		if (read_bytes == -1)
			return (free(buff), NULL);
		buff[read_bytes] = '\0';
		file = ft_memjoin(file, buff, size, read_bytes);
		if (!file)
			return (free(buff), NULL);
		size += read_bytes;
	}
	*rsize = size;
	free(buff);
	return (file);
}
