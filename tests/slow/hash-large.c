/*
 * Copyright (C) 2016 Nikos Mavrogiannopoulos
 *
 * This file is part of GnuTLS.
 *
 * GnuTLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GnuTLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>
#include <limits.h>
#include "utils.h"

#define MIN(x,y) ((x)<(y))?(x):(y)

/* Test hashing on large buffers */

void doit(void)
{
	unsigned char digest[32];
	int err;
	char *buf, *p;
	ssize_t size, left, size2;
	gnutls_hash_hd_t td;

	if (sizeof(size) <= 4)
		exit(77);

	global_init();

	size = (ssize_t)UINT_MAX + (ssize_t)64*1024;
	buf = calloc(1, size);
	if (buf == NULL)
		exit(77);

	if (size < (ssize_t)UINT_MAX)
		exit(77);


	err =
	    gnutls_hash_fast(GNUTLS_DIG_SHA256, buf, size,
			     digest);
	if (err < 0)
		fail("gnutls_hash_fast(SHA256) failed: %d\n", err);
	else {
#define SHA256_HASH "\x80\x92\xd9\xbe\x54\xa0\xe9\xd7\x7c\xb8\xe4\x2d\xd3\x7c\x19\xfe\x4e\x68\x84\x33\x71\xef\x1c\x81\xd6\x44\x36\x52\x06\xd8\x4b\x8a"
		if (memcmp(digest, SHA256_HASH, 32) == 0) {
			if (debug)
				success("gnutls_hash_fast(SHA256) %lu OK\n", (unsigned long)size);
		} else {
			hexprint(digest, 32);
			fail("gnutls_hash_fast(SHA256) failure\n");
		}
	}

	err = gnutls_hash_init(&td, GNUTLS_DIG_SHA256);
	if (err < 0) {
		fail("failed in %d\n", __LINE__);
	}

	size2 = size;
	p = buf;
	while(size2 > 0) {
		left = MIN(64*1024, size2);
		gnutls_hash(td, p, left);
		size2 -= left;
		p += left;
	}

	gnutls_hash_output(td, digest);
	gnutls_hash_deinit(td, NULL);
	if (memcmp(digest, SHA256_HASH, 32) == 0) {
		if (debug)
			success("gnutls_hash_fast(SHA256) %lu OK\n", (unsigned long)size);
	} else {
		hexprint(digest, 32);
		fail("gnutls_hash(SHA256) failure\n");
	}

	/* SHA1 */

	err =
	    gnutls_hash_fast(GNUTLS_MAC_SHA1, buf, size,
			     digest);
	if (err < 0)
		fail("gnutls_hash_fast(SHA1) failed: %d\n", err);
	else {
#define SHA1_HASH "\x75\xd2\x67\x3f\xec\x73\xe4\x57\xb8\x40\xb3\xb5\xf1\xc7\xa8\x1a\x2d\x11\x7e\xd9"
		if (memcmp(digest, SHA1_HASH, 20) == 0) {
			if (debug)
				success("gnutls_hash_fast(SHA1) OK\n");
		} else {
			hexprint(digest, 20);
			fail("gnutls_hash_fast(SHA1) failure\n");
		}
	}

	err =
	    gnutls_hmac_fast(GNUTLS_MAC_SHA1, "keykeykey", 9, buf, size,
			     digest);
	if (err < 0)
		fail("gnutls_hmac_fast(SHA1) failed: %d\n", err);
	else {
#define SHA1_MAC "\xe2\xe9\x84\x48\x53\xe3\x0b\xfe\x45\x04\xf6\x6b\x5b\x6d\x4d\x2c\xa3\x0f\xcf\x23"
		if (memcmp(digest, SHA1_MAC, 20) == 0) {
			if (debug)
				success("gnutls_hmac_fast(SHA1) OK\n");
		} else {
			hexprint(digest, 20);
			fail("gnutls_hmac_fast(SHA1) failure\n");
		}
	}

	free(buf);
	gnutls_global_deinit();
}
