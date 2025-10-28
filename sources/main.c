/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 21:15:18 by mgama             #+#    #+#             */
/*   Updated: 2025/10/28 12:54:20 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "files.h"
#include "md5.h"

#define F_ECHO		0x01
#define F_QUIET		0x02
#define F_REVERSE	0x04
#define F_STRING	0x08

static void
usage(void)
{
	(void)fprintf(stderr, "Usage: ft_ssl <command> [-pqr] [-s string] [files ...]\n");
	(void)fprintf(stderr, "\n");
	(void)fprintf(stderr, "Commands:\n");
	(void)fprintf(stderr, "  md5       Compute MD5 hash\n");
	(void)fprintf(stderr, "  sha256    Compute SHA-256 hash\n");
	(void)fprintf(stderr, "Options:\n");
	(void)fprintf(stderr, "  -p        Echo STDIN to STDOUT and append the checksum to STDOUT\n");
	(void)fprintf(stderr, "  -q        Quiet mode\n");
	(void)fprintf(stderr, "  -r        Reverse the format of the output\n");
	(void)fprintf(stderr, "  -s        Print the sum of the given string\n");
	exit(64);
}

static void
print_hash(const uint8_t *hash, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		(void)printf("%02x", hash[i]);
	}
}

static int
read_and_hash(const char *filename, int cflags)
{
	size_t size = 0;
	uint8_t *input;

	if (cflags & F_STRING)
	{
		input = (uint8_t *)filename;
		size = strlen(filename);
	}
	else
	{
		int fd = 0;
		if (filename != NULL)
		{
			fd = open(filename, O_RDONLY);
			if (fd < 0)
			{
				ft_perror(filename);
				return (1);
			}	
		}

		input = ft_read_file(fd, NULL, &size);
		if (!input)
		{
			ft_perror(filename);
			if (fd > 0)
				(void)close(fd);
			return (1);
		}

		if (fd > 0)
			(void)close(fd);
	}

	if (cflags & F_ECHO && !(cflags & F_STRING))
		(void)printf("%s", input);
	
	uint8_t output[HASH_LENGTH];

	md5hash(input, size, output);

	if (filename && !(cflags & F_STRING) && !(cflags & F_ECHO) && !(cflags & F_QUIET) && !(cflags & F_REVERSE))
		printf("MD5 (%s) = ", filename);

	print_hash(output, HASH_LENGTH);

	if (filename && cflags & F_REVERSE && !(cflags & F_STRING) && !(cflags & F_ECHO) && !(cflags & F_QUIET))
	{
		printf(" %s", filename);
	}
	(void)printf("\n");
	return (0);
}

static char*
extract_command(const int argc, char **argv)
{
	if (argc < 2)
		return (NULL);

	char *command = argv[1];
	if (strcmp(command, "md5") == 0 || strcmp(command, "md5sum") == 0)
		return (command);
	if (strcmp(command, "sha256") == 0)
		return (command);

	return (NULL);
}

int
main(int argc, char **argv)
{
	char c;
	char *command;
	char *target = NULL;
	int cflags = 0;

	if ((command = extract_command(argc, argv)) == NULL)
		usage();

	optind = 2; // Skip the command argument
	while ((c = getopt(argc, argv, "pqrs:")) !=  -1)
	{
		switch (c)
		{
		case 'p':
			cflags |= F_ECHO;
			break;
		case 'q':
			cflags |= F_QUIET;
			break;
		case 'r':
			cflags |= F_REVERSE;
			break;
		case 's':
			cflags |= F_STRING;
			target = optarg;
			break;
		default:
			(void)fprintf(stderr, FT_PREFIX"invalid option -- %c\n", c);
			usage();
		}
	}

	int return_code = 0;

	if (argc - optind == 0 || (cflags & F_ECHO) || (cflags & F_STRING))
	{
		return (read_and_hash(target, cflags));
	}

	for (size_t i = optind; i < argc; i++)
	{
		if (read_and_hash(argv[i], cflags))
			return_code = 1;
	}

	return (return_code);
}
