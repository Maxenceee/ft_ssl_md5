/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 21:15:18 by mgama             #+#    #+#             */
/*   Updated: 2025/11/18 16:14:03 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "files.h"
#include "md.h"
#include "sha2.h"

#define F_ECHO		0x0100
#define F_QUIET		0x0200
#define F_REVERSE	0x0400
#define F_STRING	0x0800

struct command_t
{
	const char	*name;
	int			(*handler)(const uint8_t *input, size_t input_length, uint8_t *output);
	size_t		hash_length;
};

static const struct command_t commands[] = {
	{"md5", md5hash, MD5_HASH_LENGTH},
	{"sha224", sha224hash, SHA224_HASH_LENGTH},
	{"sha256", sha256hash, SHA256_HASH_LENGTH},
	{"sha384", sha384hash, SHA384_HASH_LENGTH},
	{"sha512", sha512hash, SHA512_HASH_LENGTH},
	{0}
};

static void
usage(void)
{
	(void)fprintf(stderr, "Usage: ft_ssl <command> [-pqr] [-s string] [files ...]\n");
	(void)fprintf(stderr, "\n");
	(void)fprintf(stderr, "Commands:\n");
	for (size_t i = 0; commands[i].name != NULL; i++)
	{
		(void)fprintf(stderr, "  %-8s  Compute %s hash\n", commands[i].name, commands[i].name);
	}
	(void)fprintf(stderr, "Options:\n");
	(void)fprintf(stderr, "  -p        Echo STDIN to STDOUT and append the checksum to STDOUT\n");
	(void)fprintf(stderr, "  -q        Quiet mode\n");
	(void)fprintf(stderr, "  -r        Reverse the format of the output\n");
	(void)fprintf(stderr, "  -s        Print the sum of the given string\n");
	exit(64);
}

inline static char
ft_toupper(unsigned char c)
{
	if (c >= 'a' && c <= 'z')
		return (c - 32);
	return (c);
}

static void
print_hash_name(const char* name, const char* channel)
{
	size_t i = 0;
	while (name[i])
	{
		printf("%c", ft_toupper(name[i++]));
	}
	printf("(%s)= ", channel);
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
read_and_hash(const struct command_t *command, const char *filename, int cflags)
{
	size_t size = 0;
	uint8_t *input;

	if (cflags & F_STRING)
	{
		input = (uint8_t *)strdup(filename);
		if (!input)
		{
			ft_perror(filename);
			return (1);
		}
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

	if (filename && !(cflags & F_STRING) && !(cflags & F_ECHO) && !(cflags & F_QUIET) && !(cflags & F_REVERSE))
	{
		print_hash_name(command->name, filename);
	}
#ifndef __APPLE__
	else
	{
		print_hash_name(command->name, "stdin");
	}
#endif /* __APPLE__ */

	/**
	 * The following dynamic stack allocation (VLA) is safe because
	 * the hash lengths are small and known at compile time.
	 */
	uint8_t output[command->hash_length];

	if (command->handler(input, size, output))
	{
		free(input);
		ft_perror("Hashing failed");
		return (1);
	}
	free(input);
	print_hash(output, command->hash_length);

	if (filename && cflags & F_REVERSE && !(cflags & F_STRING) && !(cflags & F_ECHO) && !(cflags & F_QUIET))
	{
		printf(" %s", filename);
	}
	(void)printf("\n");

	return (0);
}

static int
validate_command(const int argc, char **argv, struct command_t *handler)
{
	if (argc < 2)
		return (1);

	for (size_t i = 0; commands[i].name != NULL; i++)
	{
		if (strcmp(argv[1], commands[i].name) == 0)
		{
			*handler = commands[i];
			return (0);
		}
	}

	return (1);
}

int
main(int argc, char **argv)
{
	char c;
	char *target = NULL;
	int cflags = 0;
	struct command_t command = {0};

	if (validate_command(argc, argv, &command))
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
			usage();
		}
	}

	int return_code = 0;

	if (argc - optind == 0 || (cflags & F_ECHO) || (cflags & F_STRING))
	{
		return (read_and_hash(&command, target, cflags));
	}

	for (int i = optind; i < argc; i++)
	{
		if (read_and_hash(&command, argv[i], cflags))
			return_code = 1;
	}

	return (return_code);
}
