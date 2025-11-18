#!/usr/bin/env bash

ALGOS=(md5 sha224 sha256 sha384 sha512)
BIN="${BIN:-./ft_ssl}"

if [ $# -lt 1 ]; then
    printf 'Usage: %s <file1> [file2 ...]\n' "$(basename "$0")" >&2
    exit 2
fi

if [ ! -x "$BIN" ]; then
    printf 'Error: executable %s not found or not executable\n' "$BIN" >&2
    exit 2
fi

# All remaining parameters are files to test
FILES=("$@")

script="$(basename "$0")"
binname="$(basename "$BIN")"

tmp_openssl="$(mktemp)"
tmp_ftssl="$(mktemp)"
trap 'rm -f "$tmp_openssl" "$tmp_ftssl"' EXIT

fail=0

extract_hash() {
    local text="$1"
    local hash
    hash="$(printf '%s\n' "$text" | grep -oE '[0-9a-fA-F]{32,128}' | head -n1 | tr '[:upper:]' '[:lower:]')"
    if [ -n "$hash" ]; then
        printf '%s' "$hash"
    else
        printf '%s' "$text"
    fi
}

for file in "${FILES[@]}"; do
    if [ ! -f "$file" ]; then
        printf 'Error: file %s not found\n' "$file" >&2
        fail=1
        continue
    fi

    fname="$(basename "$file")"
    [ "$fname" = "$script" ] && continue
    [ "$fname" = "$binname" ] && continue

    printf 'File: %s\n' "$fname"

    for alg in "${ALGOS[@]}"; do
        printf '  %-7s: ' "$alg"

        if ! openssl_out="$(openssl dgst -"$alg" "$file" 2>&1)"; then
            printf 'openssl failed\n'
            continue
        fi
        printf '%s' "$openssl_out" >"$tmp_openssl"

        if ! ftssl_out="$("$BIN" "$alg" "$file" 2>&1)"; then
            printf 'ft_ssl failed\n\n'
            continue
        fi
        printf '%s' "$ftssl_out" >"$tmp_ftssl"

        op_hash="$(extract_hash "$openssl_out")"
        ft_hash="$(extract_hash "$ftssl_out")"

        if [ "$op_hash" != "$ft_hash" ]; then
            printf 'DIFF\n'
            printf '    --- openssl\n'
            sed 's/^/    /' "$tmp_openssl"
            printf '    --- ft_ssl\n'
            sed 's/^/    /' "$tmp_ftssl"
            printf '    --- unified diff\n'
            diff -u "$tmp_openssl" "$tmp_ftssl" | sed 's/^/    /' || true
            fail=1
        else
            printf 'OK\n'
        fi
    done

    printf '\n'
done

exit $fail
