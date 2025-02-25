/*
 * Copyright 2019-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <stdio.h>
#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include "prov/provider_ctx.h"
#include "prov/implementations.h"
#include "prov/names.h"
#include "prov/providercommon.h"

/*
 * Forward declarations to ensure that interface functions are correctly
 * defined.
 */
static OSSL_FUNC_provider_gettable_params_fn legacy_gettable_params;
static OSSL_FUNC_provider_get_params_fn legacy_get_params;
static OSSL_FUNC_provider_query_operation_fn legacy_query;

#define ALG(NAMES, FUNC) { NAMES, "provider=legacy", FUNC }

#ifdef STATIC_LEGACY
OSSL_provider_init_fn ossl_legacy_provider_init;
# define OSSL_provider_init ossl_legacy_provider_init
#endif

/* Parameters we provide to the core */
static const OSSL_PARAM legacy_param_types[] = {
    OSSL_PARAM_DEFN(OSSL_PROV_PARAM_NAME, OSSL_PARAM_UTF8_PTR, NULL, 0),
    OSSL_PARAM_DEFN(OSSL_PROV_PARAM_VERSION, OSSL_PARAM_UTF8_PTR, NULL, 0),
    OSSL_PARAM_DEFN(OSSL_PROV_PARAM_BUILDINFO, OSSL_PARAM_UTF8_PTR, NULL, 0),
    OSSL_PARAM_DEFN(OSSL_PROV_PARAM_STATUS, OSSL_PARAM_INTEGER, NULL, 0),
    OSSL_PARAM_END
};

static const OSSL_PARAM *legacy_gettable_params(void *provctx)
{
    return legacy_param_types;
}

static int legacy_get_params(void *provctx, OSSL_PARAM params[])
{
    OSSL_PARAM *p;

    p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_NAME);
    if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, "OpenSSL Legacy Provider"))
        return 0;
    p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_VERSION);
    if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, OPENSSL_VERSION_STR))
        return 0;
    p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_BUILDINFO);
    if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, OPENSSL_FULL_VERSION_STR))
        return 0;
    p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_STATUS);
    if (p != NULL && !OSSL_PARAM_set_int(p, ossl_prov_is_running()))
        return 0;
    return 1;
}

static const OSSL_ALGORITHM legacy_digests[] = {
#ifndef OPENSSL_NO_MD4
    ALG(PROV_NAMES_MD4, ossl_md4_functions),
#endif
#ifndef OPENSSL_NO_MDC2
    ALG(PROV_NAMES_MDC2, ossl_mdc2_functions),
#endif /* OPENSSL_NO_MDC2 */
#ifndef OPENSSL_NO_WHIRLPOOL
    ALG(PROV_NAMES_WHIRLPOOL, ossl_wp_functions),
#endif /* OPENSSL_NO_WHIRLPOOL */
#ifndef OPENSSL_NO_RMD160
    ALG(PROV_NAMES_RIPEMD_160, ossl_ripemd160_functions),
#endif /* OPENSSL_NO_RMD160 */
    { NULL, NULL, NULL }
};

static const OSSL_ALGORITHM legacy_ciphers[] = {
#ifndef OPENSSL_NO_BF
    ALG(PROV_NAMES_BF_ECB, ossl_blowfish128ecb_functions),
    ALG(PROV_NAMES_BF_CBC, ossl_blowfish128cbc_functions),
    ALG(PROV_NAMES_BF_OFB, ossl_blowfish64ofb64_functions),
    ALG(PROV_NAMES_BF_CFB, ossl_blowfish64cfb64_functions),
#endif /* OPENSSL_NO_BF */
#ifndef OPENSSL_NO_IDEA
    ALG(PROV_NAMES_IDEA_ECB, ossl_idea128ecb_functions),
    ALG(PROV_NAMES_IDEA_CBC, ossl_idea128cbc_functions),
    ALG(PROV_NAMES_IDEA_OFB, ossl_idea128ofb64_functions),
    ALG(PROV_NAMES_IDEA_CFB, ossl_idea128cfb64_functions),
#endif /* OPENSSL_NO_IDEA */
#ifndef OPENSSL_NO_RC2
    ALG(PROV_NAMES_RC2_ECB, ossl_rc2128ecb_functions),
    ALG(PROV_NAMES_RC2_CBC, ossl_rc2128cbc_functions),
    ALG(PROV_NAMES_RC2_40_CBC, ossl_rc240cbc_functions),
    ALG(PROV_NAMES_RC2_64_CBC, ossl_rc264cbc_functions),
    ALG(PROV_NAMES_RC2_CFB, ossl_rc2128cfb128_functions),
    ALG(PROV_NAMES_RC2_OFB, ossl_rc2128ofb128_functions),
#endif /* OPENSSL_NO_RC2 */
#ifndef OPENSSL_NO_RC4
    ALG(PROV_NAMES_RC4, ossl_rc4128_functions),
    ALG(PROV_NAMES_RC4_40, ossl_rc440_functions),
# ifndef OPENSSL_NO_MD5
    ALG(PROV_NAMES_RC4_HMAC_MD5, ossl_rc4_hmac_ossl_md5_functions),
# endif /* OPENSSL_NO_MD5 */
#endif /* OPENSSL_NO_RC4 */
#ifndef OPENSSL_NO_RC5
    ALG(PROV_NAMES_RC5_ECB, ossl_rc5128ecb_functions),
    ALG(PROV_NAMES_RC5_CBC, ossl_rc5128cbc_functions),
    ALG(PROV_NAMES_RC5_OFB, ossl_rc5128ofb64_functions),
    ALG(PROV_NAMES_RC5_CFB, ossl_rc5128cfb64_functions),
#endif /* OPENSSL_NO_RC5 */
#ifndef OPENSSL_NO_DES
    ALG(PROV_NAMES_DESX_CBC, ossl_tdes_desx_cbc_functions),
    ALG(PROV_NAMES_DES_ECB, ossl_des_ecb_functions),
    ALG(PROV_NAMES_DES_CBC, ossl_des_cbc_functions),
    ALG(PROV_NAMES_DES_OFB, ossl_des_ofb64_functions),
    ALG(PROV_NAMES_DES_CFB, ossl_des_cfb64_functions),
    ALG(PROV_NAMES_DES_CFB1, ossl_des_cfb1_functions),
    ALG(PROV_NAMES_DES_CFB8, ossl_des_cfb8_functions),
#endif /* OPENSSL_NO_DES */
    { NULL, NULL, NULL }
};

static const OSSL_ALGORITHM legacy_kdfs[] = {
    ALG(PROV_NAMES_PBKDF1, ossl_kdf_pbkdf1_functions),
    { NULL, NULL, NULL }
};

static const OSSL_ALGORITHM *legacy_query(void *provctx, int operation_id,
                                          int *no_cache)
{
    *no_cache = 0;
    switch (operation_id) {
    case OSSL_OP_DIGEST:
        return legacy_digests;
    case OSSL_OP_CIPHER:
        return legacy_ciphers;
    case OSSL_OP_KDF:
        return legacy_kdfs;
    }
    return NULL;
}

static void legacy_teardown(void *provctx)
{
    OSSL_LIB_CTX_free(PROV_LIBCTX_OF(provctx));
    ossl_prov_ctx_free(provctx);
}

/* Functions we provide to the core */
static const OSSL_DISPATCH legacy_dispatch_table[] = {
    { OSSL_FUNC_PROVIDER_TEARDOWN, (void (*)(void))legacy_teardown },
    { OSSL_FUNC_PROVIDER_GETTABLE_PARAMS, (void (*)(void))legacy_gettable_params },
    { OSSL_FUNC_PROVIDER_GET_PARAMS, (void (*)(void))legacy_get_params },
    { OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)(void))legacy_query },
    { 0, NULL }
};

int OSSL_provider_init(const OSSL_CORE_HANDLE *handle,
                       const OSSL_DISPATCH *in,
                       const OSSL_DISPATCH **out,
                       void **provctx)
{
    OSSL_LIB_CTX *libctx = NULL;

    if ((*provctx = ossl_prov_ctx_new()) == NULL
        || (libctx = OSSL_LIB_CTX_new_child(handle, in)) == NULL) {
        OSSL_LIB_CTX_free(libctx);
        legacy_teardown(*provctx);
        *provctx = NULL;
        return 0;
    }
    ossl_prov_ctx_set0_libctx(*provctx, libctx);
    ossl_prov_ctx_set0_handle(*provctx, handle);

    *out = legacy_dispatch_table;

    return 1;
}
