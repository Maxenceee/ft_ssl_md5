# ft_ssl_md5

My **ft_ssl** project for the 42 school cursus.

## Getting started

You need to compile the project with `make`.

### Usage

```
Usage: ft_ssl <command> [-pqr] [-s string] [files ...]

Commands:
  md5       Compute MD5 hash
  sha256    Compute SHA-256 hash
Options:
  -p        Echo STDIN to STDOUT and append the checksum to STDOUT
  -q        Quiet mode
  -r        Reverse the format of the output
  -s        Print the sum of the given string
```

## Overview

This project implements a simplified version of the `md5` and `sha256` hashing algorithms. It supports various command-line options to customize the output format and input sources.

A cryptographic hash function is a mathematical algorithm that transforms an arbitrary block of data into a fixed-size bit string, known as a hash value or digest. The primary purpose of a hash function is to ensure data integrity by producing a unique representation of the input data. Even a small change in the input will produce a significantly different hash value.

The `md5` algorithm produces a 128-bit hash value, typically represented as a 32-character hexadecimal number. It is defined in RFC 1321. Unfortunately, MD5 is no longer considered secure for cryptographic purposes due to vulnerabilities that allow for collision attacks.

The `sha256` algorithm produces a 256-bit hash value, typically represented as a 64-character hexadecimal number. It is part of the SHA-2 family of cryptographic hash functions and is widely used for data integrity verification and digital signatures.
