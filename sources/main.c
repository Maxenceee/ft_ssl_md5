/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 21:15:18 by mgama             #+#    #+#             */
/*   Updated: 2025/10/27 17:19:09 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "files.h"
#include "md5.h"

static void
print_hash(const uint8_t *hash, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		(void)printf("%02x", hash[i]);
	}
	(void)printf("\n");
}

static int
read_and_hash(const char *filename)
{
	int fd = 0;
	if (filename != NULL)
	{
		fd = open(filename, O_RDONLY);
		if (fd < 0)
		{
			perror("Error opening file");
			return (1);
		}	
	}

	size_t size = 0;
	uint8_t *input = ft_read_file(fd, NULL, &size);
	if (!input)
	{
		perror("Error reading file");
		if (fd > 0)
			close(fd);
		return (1);
	}
	
	uint8_t output[HASH_LENGTH];

	md5hash(input, size, output);

	printf("MD5(%s)= ", filename ? filename : "stdin");
	print_hash(output, HASH_LENGTH);
	return (0);
}

int
main(int argc, char **argv)
{
	int return_code = 0;

	if (argc == 1)
	{
		return read_and_hash(NULL);
	}

	for (size_t i = 1; i < argc; i++)
	{
		if (read_and_hash(argv[i]))
			return_code = 1;
	}

	return (return_code);
}
