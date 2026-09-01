/**************************** Crypto yield hook *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st Sep 2026
******************************************************************************/

#ifndef _CRYPTO_YIELD_H_
#define _CRYPTO_YIELD_H_

typedef void (*crypto_yield_fn)(void);

/**
 * Install the cooperative yield the elliptic curve inner loops call, so a long
 * key operation can let the rest of the device run. Pass nullptr to remove it.
 */
void crypto_set_yield_hook(crypto_yield_fn fn);

/**
 * Call the installed yield if there is one, and do nothing when there is not.
 */
void crypto_yield(void);

#endif
