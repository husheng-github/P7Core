/* R_KEYGEN.C - key-pair generation for RSAREF
 */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
     Inc., created 1991. All rights reserved.
 */
//#include <linux/slab.h>


#include "global.h"
#include "rsaref.h"
#include "r_random.h"
#include "nn.h"
#include "prime.h"

static int RSAFilter PROTO_LIST((NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int));
static int RelativelyPrime PROTO_LIST((NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int));

/* Generates an RSA key pair with a given length and public exponent.
 */
int R_GeneratePEMKeys (publicKey, privateKey, protoKey, randomStruct)
R_RSA_PUBLIC_KEY *publicKey;                          /* new RSA public key */
R_RSA_PRIVATE_KEY *privateKey;                       /* new RSA private key */
R_RSA_PROTO_KEY *protoKey;                             /* RSA prototype key */
R_RANDOM_STRUCT *randomStruct;                          /* random structure */
{
    int status;
    unsigned int nDigits, pBits, pDigits, qBits;
	/*
	NN_DIGIT d[MAX_NN_DIGITS], dP[MAX_NN_DIGITS], dQ[MAX_NN_DIGITS],
    e[MAX_NN_DIGITS], n[MAX_NN_DIGITS], p[MAX_NN_DIGITS], phiN[MAX_NN_DIGITS],
    pMinus1[MAX_NN_DIGITS], q[MAX_NN_DIGITS], qInv[MAX_NN_DIGITS],
    qMinus1[MAX_NN_DIGITS], t[MAX_NN_DIGITS], u[MAX_NN_DIGITS],
    v[MAX_NN_DIGITS];
    */
    NN_DIGIT *d,*dP,*dQ,*e,*n,*p,*phiN,*pMinus1,*q,*qInv,*qMinus1,*t,*u,*v;

	
    if ((protoKey->bits < MIN_RSA_MODULUS_BITS) ||
            (protoKey->bits > MAX_RSA_MODULUS_BITS))
        return (RE_MODULUS_LEN);
	
    nDigits = (protoKey->bits + NN_DIGIT_BITS - 1) / NN_DIGIT_BITS;
    pDigits = (nDigits + 1) / 2;
    pBits = (protoKey->bits + 1) / 2;
    qBits = protoKey->bits - pBits;
	


	d = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(d == NULL)
    {
    	return -1;
    }
	dP = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(dP == NULL)
    {
        pcicorecallfunc.mem_free(d);
    	return -1;
    }
	dQ = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(dQ == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
    	return -1;
    }
	e = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(e == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
    	return -1;
    }
	n = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(n == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
    	return -1;
    }
	p = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(p == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
    	return -1;
    }
	phiN = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(phiN == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
		pcicorecallfunc.mem_free(p);
    	return -1;
    }
	pMinus1 = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(pMinus1 == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
		pcicorecallfunc.mem_free(p);
		pcicorecallfunc.mem_free(phiN);
    	return -1;
    }
	q = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(q == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
		pcicorecallfunc.mem_free(p);
		pcicorecallfunc.mem_free(phiN);
		pcicorecallfunc.mem_free(pMinus1);
    	return -1;
    }
	qInv = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(qInv == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
		pcicorecallfunc.mem_free(p);
		pcicorecallfunc.mem_free(phiN);
		pcicorecallfunc.mem_free(pMinus1);
		pcicorecallfunc.mem_free(q);
    	return -1;
    }
	qMinus1 = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(qMinus1 == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
		pcicorecallfunc.mem_free(p);
		pcicorecallfunc.mem_free(phiN);
		pcicorecallfunc.mem_free(pMinus1);
		pcicorecallfunc.mem_free(q);
		pcicorecallfunc.mem_free(qInv);
    	return -1;
    }
	t = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(t == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
		pcicorecallfunc.mem_free(p);
		pcicorecallfunc.mem_free(phiN);
		pcicorecallfunc.mem_free(pMinus1);
		pcicorecallfunc.mem_free(q);
		pcicorecallfunc.mem_free(qInv);
		pcicorecallfunc.mem_free(qMinus1);
    	return -1;
    }
	u = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(u == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
		pcicorecallfunc.mem_free(p);
		pcicorecallfunc.mem_free(phiN);
		pcicorecallfunc.mem_free(pMinus1);
		pcicorecallfunc.mem_free(q);
		pcicorecallfunc.mem_free(qInv);
		pcicorecallfunc.mem_free(qMinus1);
		pcicorecallfunc.mem_free(t);
    	return -1;
    }
	v = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    if(v == NULL)
    {
        pcicorecallfunc.mem_free(d);
		pcicorecallfunc.mem_free(dP);
		pcicorecallfunc.mem_free(dQ);
		pcicorecallfunc.mem_free(e);
		pcicorecallfunc.mem_free(n);
		pcicorecallfunc.mem_free(p);
		pcicorecallfunc.mem_free(phiN);
		pcicorecallfunc.mem_free(pMinus1);
		pcicorecallfunc.mem_free(q);
		pcicorecallfunc.mem_free(qInv);
		pcicorecallfunc.mem_free(qMinus1);
		pcicorecallfunc.mem_free(t);
		pcicorecallfunc.mem_free(u);
    	return -1;
    }

	

    /* NOTE: for 65537, this assumes NN_DIGIT is at least 17 bits. */
    NN_ASSIGN_DIGIT
    (e, protoKey->useFermat4 ? (NN_DIGIT)65537 : (NN_DIGIT)3, nDigits);

    /* Generate prime p between 3*2^(pBits-2) and 2^pBits-1, searching
         in steps of 2, until one satisfies gcd (p-1, e) = 1.
     */
    NN_Assign2Exp (t, pBits - 1, pDigits);
    NN_Assign2Exp (u, pBits - 2, pDigits);
    NN_Add (t, t, u, pDigits);
    NN_ASSIGN_DIGIT (v, 1, pDigits);
    NN_Sub (v, t, v, pDigits);
    NN_Add (u, u, v, pDigits);
    NN_ASSIGN_DIGIT (v, 2, pDigits);
	//DEBUG_INFO();
    do {
        status = GeneratePrime (p, t, u, v, pDigits, randomStruct);
        if (0 != status)
        {
            pcicorecallfunc.mem_free(d);
			pcicorecallfunc.mem_free(dP);
			pcicorecallfunc.mem_free(dQ);
			pcicorecallfunc.mem_free(e);
			pcicorecallfunc.mem_free(n);
			pcicorecallfunc.mem_free(p);
			pcicorecallfunc.mem_free(phiN);
			pcicorecallfunc.mem_free(pMinus1);
			pcicorecallfunc.mem_free(q);
			pcicorecallfunc.mem_free(qInv);
			pcicorecallfunc.mem_free(qMinus1);
			pcicorecallfunc.mem_free(t);
			pcicorecallfunc.mem_free(u);
			pcicorecallfunc.mem_free(v);
            return (status);
        }
    }
    while (! RSAFilter (p, pDigits, e, 1));
	//DEBUG_INFO();

    /* Generate prime q between 3*2^(qBits-2) and 2^qBits-1, searching
         in steps of 2, until one satisfies gcd (q-1, e) = 1.
     */
    NN_Assign2Exp (t, qBits - 1, pDigits);
    NN_Assign2Exp (u, qBits - 2, pDigits);
    NN_Add (t, t, u, pDigits);
    NN_ASSIGN_DIGIT (v, 1, pDigits);
    NN_Sub (v, t, v, pDigits);
    NN_Add (u, u, v, pDigits);
    NN_ASSIGN_DIGIT (v, 2, pDigits);
	//DEBUG_INFO();
    do {
        status = GeneratePrime (q, t, u, v, pDigits, randomStruct);
        if (0 != status)
        {
            pcicorecallfunc.mem_free(d);
			pcicorecallfunc.mem_free(dP);
			pcicorecallfunc.mem_free(dQ);
			pcicorecallfunc.mem_free(e);
			pcicorecallfunc.mem_free(n);
			pcicorecallfunc.mem_free(p);
			pcicorecallfunc.mem_free(phiN);
			pcicorecallfunc.mem_free(pMinus1);
			pcicorecallfunc.mem_free(q);
			pcicorecallfunc.mem_free(qInv);
			pcicorecallfunc.mem_free(qMinus1);
			pcicorecallfunc.mem_free(t);
			pcicorecallfunc.mem_free(u);
			pcicorecallfunc.mem_free(v);
            return (status);
        }
    }
    while (! RSAFilter (q, pDigits, e, 1));
	//DEBUG_INFO();

    /* Sort so that p > q. (p = q case is extremely unlikely.)
     */
    if (NN_Cmp (p, q, pDigits) < 0) {
        NN_Assign (t, p, pDigits);
        NN_Assign (p, q, pDigits);
        NN_Assign (q, t, pDigits);
    }
	//DEBUG_INFO();

    /* Compute n = pq, qInv = q^{-1} mod p, d = e^{-1} mod (p-1)(q-1),
       dP = d mod p-1, dQ = d mod q-1.
     */
    NN_Mult (n, p, q, pDigits);
    NN_ModInv (qInv, q, p, pDigits);

    NN_ASSIGN_DIGIT (t, 1, pDigits);
    NN_Sub (pMinus1, p, t, pDigits);
    NN_Sub (qMinus1, q, t, pDigits);
    NN_Mult (phiN, pMinus1, qMinus1, pDigits);

    NN_ModInv (d, e, phiN, nDigits);
    NN_Mod (dP, d, nDigits, pMinus1, pDigits);
    NN_Mod (dQ, d, nDigits, qMinus1, pDigits);

	//DEBUG_INFO();
    publicKey->bits = privateKey->bits = protoKey->bits;
    NN_Encode (publicKey->modulus, MAX_RSA_MODULUS_LEN, n, nDigits);
    NN_Encode (publicKey->exponent, MAX_RSA_MODULUS_LEN, e, 1);
    R_memcpy
    ((POINTER)privateKey->modulus, (POINTER)publicKey->modulus,
     MAX_RSA_MODULUS_LEN);
    R_memcpy
    ((POINTER)privateKey->publicExponent, (POINTER)publicKey->exponent,
     MAX_RSA_MODULUS_LEN);
    NN_Encode (privateKey->exponent, MAX_RSA_MODULUS_LEN, d, nDigits);
    NN_Encode (privateKey->prime[0], MAX_RSA_PRIME_LEN, p, pDigits);
    NN_Encode (privateKey->prime[1], MAX_RSA_PRIME_LEN, q, pDigits);
    NN_Encode (privateKey->primeExponent[0], MAX_RSA_PRIME_LEN, dP, pDigits);
    NN_Encode (privateKey->primeExponent[1], MAX_RSA_PRIME_LEN, dQ, pDigits);
    NN_Encode (privateKey->coefficient, MAX_RSA_PRIME_LEN, qInv, pDigits);
	//DEBUG_INFO();

    /* Zeroize sensitive information.
     */
     /*
    R_memset ((POINTER)d, 0, sizeof (d));
    R_memset ((POINTER)dP, 0, sizeof (dP));
    R_memset ((POINTER)dQ, 0, sizeof (dQ));
    R_memset ((POINTER)p, 0, sizeof (p));
    R_memset ((POINTER)phiN, 0, sizeof (phiN));
    R_memset ((POINTER)pMinus1, 0, sizeof (pMinus1));
    R_memset ((POINTER)q, 0, sizeof (q));
    R_memset ((POINTER)qInv, 0, sizeof (qInv));
    R_memset ((POINTER)qMinus1, 0, sizeof (qMinus1));
    R_memset ((POINTER)t, 0, sizeof (t));
    */
    R_memset ((POINTER)d, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)dP, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)dQ, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)p, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)phiN, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)pMinus1, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)q, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)qInv, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)qMinus1, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
    R_memset ((POINTER)t, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
	
	//DEBUG_INFO();

	pcicorecallfunc.mem_free(d);
	pcicorecallfunc.mem_free(dP);
	pcicorecallfunc.mem_free(dQ);
	pcicorecallfunc.mem_free(e);
	pcicorecallfunc.mem_free(n);
	pcicorecallfunc.mem_free(p);
	pcicorecallfunc.mem_free(phiN);
	pcicorecallfunc.mem_free(pMinus1);
	pcicorecallfunc.mem_free(q);
	pcicorecallfunc.mem_free(qInv);
	pcicorecallfunc.mem_free(qMinus1);
	pcicorecallfunc.mem_free(t);
	pcicorecallfunc.mem_free(u);
	pcicorecallfunc.mem_free(v);
    return (0);
}

/* Returns nonzero iff GCD (a-1, b) = 1.

   Lengths: a[aDigits], b[bDigits].
   Assumes aDigits < MAX_NN_DIGITS, bDigits < MAX_NN_DIGITS.
 */
static int RSAFilter (a, aDigits, b, bDigits)
NN_DIGIT *a, *b;
unsigned int aDigits, bDigits;
{
    int status;
    NN_DIGIT *aMinus1,*t;//[MAX_NN_DIGITS], t[MAX_NN_DIGITS];

	aMinus1 = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
	if(aMinus1 == NULL)
	{
		return -1;
	}
	t = (NN_DIGIT *)pcicorecallfunc.mem_malloc(sizeof(NN_DIGIT)*MAX_NN_DIGITS);
	if(t == NULL)
	{
		return -1;
	}

    NN_ASSIGN_DIGIT (t, 1, aDigits);
    NN_Sub (aMinus1, a, t, aDigits);

    status = RelativelyPrime (aMinus1, aDigits, b, bDigits);

    /* Zeroize sensitive information.
     */
    //R_memset ((POINTER)aMinus1, 0, sizeof (aMinus1));
    R_memset ((POINTER)aMinus1, 0, sizeof(NN_DIGIT)*MAX_NN_DIGITS);
	
	pcicorecallfunc.mem_free(aMinus1);
	pcicorecallfunc.mem_free(t);

    return (status);
}

/* Returns nonzero iff a and b are relatively prime.

   Lengths: a[aDigits], b[bDigits].
   Assumes aDigits >= bDigits, aDigits < MAX_NN_DIGITS.
 */
static int RelativelyPrime (a, aDigits, b, bDigits)
NN_DIGIT *a, *b;
unsigned int aDigits, bDigits;
{
    int status;
    NN_DIGIT t[MAX_NN_DIGITS], u[MAX_NN_DIGITS];

    NN_AssignZero (t, aDigits);
    NN_Assign (t, b, bDigits);
    NN_Gcd (t, a, t, aDigits);
    NN_ASSIGN_DIGIT (u, 1, aDigits);

    status = NN_EQUAL (t, u, aDigits);

    /* Zeroize sensitive information.
     */
    R_memset ((POINTER)t, 0, sizeof (t));

    return (status);
}

