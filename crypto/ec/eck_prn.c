/* $OpenBSD: eck_prn.c,v 1.30 2023/11/21 22:05:33 tb Exp $ */
/*
 * Written by Nils Larsch for the OpenSSL project.
 */
/* ====================================================================
 * Copyright (c) 1998-2005 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* ====================================================================
 * Copyright 2002 Sun Microsystems, Inc. ALL RIGHTS RESERVED.
 * Portions originally developed by SUN MICROSYSTEMS, INC., and
 * contributed to the OpenSSL project.
 */

#include <stdio.h>
#include <string.h>

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include "ec_local.h"

int
ECPKParameters_print_fp(FILE *fp, const EC_GROUP *x, int off)
{
	BIO *b;
	int ret;

	if ((b = BIO_new(BIO_s_file())) == NULL) {
		ECerror(ERR_R_BUF_LIB);
		return (0);
	}
	BIO_set_fp(b, fp, BIO_NOCLOSE);
	ret = ECPKParameters_print(b, x, off);
	BIO_free(b);
	return (ret);
}
LCRYPTO_ALIAS(ECPKParameters_print_fp);

int
EC_KEY_print_fp(FILE *fp, const EC_KEY *x, int off)
{
	BIO *b;
	int ret;

	if ((b = BIO_new(BIO_s_file())) == NULL) {
		ECerror(ERR_R_BIO_LIB);
		return (0);
	}
	BIO_set_fp(b, fp, BIO_NOCLOSE);
	ret = EC_KEY_print(b, x, off);
	BIO_free(b);
	return (ret);
}
LCRYPTO_ALIAS(EC_KEY_print_fp);

int
ECParameters_print_fp(FILE *fp, const EC_KEY *x)
{
	BIO *b;
	int ret;

	if ((b = BIO_new(BIO_s_file())) == NULL) {
		ECerror(ERR_R_BIO_LIB);
		return (0);
	}
	BIO_set_fp(b, fp, BIO_NOCLOSE);
	ret = ECParameters_print(b, x);
	BIO_free(b);
	return (ret);
}
LCRYPTO_ALIAS(ECParameters_print_fp);

int
EC_KEY_print(BIO *bp, const EC_KEY *x, int off)
{
	EVP_PKEY *pk;
	int ret = 0;

	if ((pk = EVP_PKEY_new()) == NULL)
		goto err;

	if (!EVP_PKEY_set1_EC_KEY(pk, (EC_KEY *) x))
		goto err;

	ret = EVP_PKEY_print_private(bp, pk, off, NULL);
 err:
	EVP_PKEY_free(pk);
	return ret;
}
LCRYPTO_ALIAS(EC_KEY_print);

int
ECParameters_print(BIO *bp, const EC_KEY *x)
{
	EVP_PKEY *pk;
	int ret = 0;

	if ((pk = EVP_PKEY_new()) == NULL)
		goto err;

	if (!EVP_PKEY_set1_EC_KEY(pk, (EC_KEY *) x))
		goto err;

	ret = EVP_PKEY_print_params(bp, pk, 4, NULL);
 err:
	EVP_PKEY_free(pk);
	return ret;
}
LCRYPTO_ALIAS(ECParameters_print);

static int
ecpk_print_asn1_parameters(BIO *bp, const EC_GROUP *group, int off)
{
	const char *nist_name;
	int nid;
	int ret = 0;

	if (!BIO_indent(bp, off, 128)) {
		ECerror(ERR_R_BIO_LIB);
		goto err;
	}

	if ((nid = EC_GROUP_get_curve_name(group)) == NID_undef) {
		ECerror(ERR_R_INTERNAL_ERROR);
		goto err;
	}

	if (BIO_printf(bp, "ASN1 OID: %s\n", OBJ_nid2sn(nid)) <= 0) {
		ECerror(ERR_R_BIO_LIB);
		goto err;
	}

	if ((nist_name = EC_curve_nid2nist(nid)) != NULL) {
		if (!BIO_indent(bp, off, 128)) {
			ECerror(ERR_R_BIO_LIB);
			goto err;
		}
		if (BIO_printf(bp, "NIST CURVE: %s\n", nist_name) <= 0) {
			ECerror(ERR_R_BIO_LIB);
			goto err;
		}
	}

	ret = 1;
 err:

	return ret;
}

static int
ecpk_print_explicit_parameters(BIO *bp, const EC_GROUP *group, int off)
{
	BN_CTX *ctx = NULL;
	const BIGNUM *order;
	BIGNUM *p, *a, *b, *cofactor;
	BIGNUM *gen = NULL;
	const EC_POINT *generator;
	const char *conversion_form;
	const unsigned char *seed;
	size_t seed_len;
	point_conversion_form_t form;
	int nid;
	int ret = 0;

	if ((ctx = BN_CTX_new()) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}

	BN_CTX_start(ctx);

	if ((p = BN_CTX_get(ctx)) == NULL)
		goto err;
	if ((a = BN_CTX_get(ctx)) == NULL)
		goto err;
	if ((b = BN_CTX_get(ctx)) == NULL)
		goto err;
	if ((cofactor = BN_CTX_get(ctx)) == NULL)
		goto err;
	if ((gen = BN_CTX_get(ctx)) == NULL)
		goto err;

	if (!EC_GROUP_get_curve(group, p, a, b, ctx)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if ((order = EC_GROUP_get0_order(group)) == NULL) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if (!EC_GROUP_get_cofactor(group, cofactor, NULL)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}

	if ((generator = EC_GROUP_get0_generator(group)) == NULL) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	form = EC_GROUP_get_point_conversion_form(group);
	if (EC_POINT_point2bn(group, generator, form, gen, ctx) == NULL) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}

	if (!BIO_indent(bp, off, 128))
		goto err;

	nid = EC_METHOD_get_field_type(EC_GROUP_method_of(group));
	if (BIO_printf(bp, "Field Type: %s\n", OBJ_nid2sn(nid)) <= 0)
		goto err;

	if (!bn_printf(bp, p, off, "Prime:"))
		goto err;
	if (!bn_printf(bp, a, off, "A:   "))
		goto err;
	if (!bn_printf(bp, b, off, "B:   "))
		goto err;

	if (form == POINT_CONVERSION_COMPRESSED)
		conversion_form = "compressed";
	else if (form == POINT_CONVERSION_UNCOMPRESSED)
		conversion_form = "uncompressed";
	else if (form == POINT_CONVERSION_HYBRID)
		conversion_form = "hybrid";
	else
		conversion_form = "unknown";
	if (!bn_printf(bp, gen, off, "Generator (%s):", conversion_form))
		goto err;

	if (!bn_printf(bp, order, off, "Order: "))
		goto err;
	if (!bn_printf(bp, cofactor, off, "Cofactor: "))
		goto err;
	if ((seed = EC_GROUP_get0_seed(group)) != NULL) {
		size_t i;

		seed_len = EC_GROUP_get_seed_len(group);

		/* XXX - ecx_buf_print() has a CBS version of this - dedup. */
		if (!BIO_indent(bp, off, 128))
			goto err;
		if (BIO_printf(bp, "Seed:") <= 0)
			goto err;

		for (i = 0; i < seed_len; i++) {
			const char *sep = ":";

			if (i % 15 == 0) {
				if (BIO_printf(bp, "\n") <= 0)
					goto err;
				if (!BIO_indent(bp, off + 4, 128))
					goto err;
			}

			if (i + 1 == seed_len)
				sep = "";
			if (BIO_printf(bp, "%02x%s", seed[i], sep) <= 0)
				goto err;
		}

		if (BIO_printf(bp, "\n") <= 0)
			goto err;
	}

	ret = 1;
 err:
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);

	return ret;
}

int
ECPKParameters_print(BIO *bp, const EC_GROUP *group, int off)
{
	if (group == NULL) {
		ECerror(ERR_R_PASSED_NULL_PARAMETER);
		return 0;
	}

	if (EC_GROUP_get_asn1_flag(group))
		return ecpk_print_asn1_parameters(bp, group, off);

	return ecpk_print_explicit_parameters(bp, group, off);
}
LCRYPTO_ALIAS(ECPKParameters_print);
