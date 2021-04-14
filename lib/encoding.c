

/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* ====================================================================
 * Copyright (c) 1995-1999 The Apache Group.  All rights reserved.
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
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache"
 *    nor may "Apache" appear in their names without prior written
 *    permission of the Apache Group.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
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
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on public domain software written at the National Center for
 * Supercomputing Applications, University of Illinois, Urbana-Champaign.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 *
 */

/* Base64 encoder/decoder. Originally Apache file ap_base64.c
 */

#include "encoding.h"

#include <string.h>
#include <stdint.h>
#include <string.h>

/* aaaack but it's fast and const should make it shared text page. */
static const unsigned char pr2six[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

int base64decode_len(const char *bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register int nprbytes;

    bufin = (const unsigned char *)bufcoded;
    while (pr2six[*(bufin++)] <= 63)
        ;

    nprbytes = (bufin - (const unsigned char *)bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    return nbytesdecoded + 1;
}

int base64decode(char *bufplain, const char *bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register unsigned char *bufout;
    register int nprbytes;

    bufin = (const unsigned char *)bufcoded;
    while (pr2six[*(bufin++)] <= 63)
        ;
    nprbytes = (bufin - (const unsigned char *)bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *)bufplain;
    bufin = (const unsigned char *)bufcoded;

    while (nprbytes > 4)
    {
        *(bufout++) =
            (unsigned char)(pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char)(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char)(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1)
    {
        *(bufout++) =
            (unsigned char)(pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2)
    {
        *(bufout++) =
            (unsigned char)(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3)
    {
        *(bufout++) =
            (unsigned char)(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}

static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64encode_len(int len)
{
    return ((len + 2) / 3 * 4) + 1;
}

int base64encode(char *encoded, const char *string, int len)
{
    int i;
    char *p;

    p = encoded;
    for (i = 0; i < len - 2; i += 3)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        *p++ = basis_64[((string[i] & 0x3) << 4) |
                        ((int)(string[i + 1] & 0xF0) >> 4)];
        *p++ = basis_64[((string[i + 1] & 0xF) << 2) |
                        ((int)(string[i + 2] & 0xC0) >> 6)];
        *p++ = basis_64[string[i + 2] & 0x3F];
    }
    if (i < len)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        if (i == (len - 1))
        {
            *p++ = basis_64[((string[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else
        {
            *p++ = basis_64[((string[i] & 0x3) << 4) |
                            ((int)(string[i + 1] & 0xF0) >> 4)];
            *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }

    *p++ = '\0';
    return p - encoded;
}

/*******************************************************************************************
 *                                  HMAC SHA1 implementation
 * 
 * 
 * 
 ********************************************************************************************
*/


#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#define blk0L(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00)     \
                  | (rol(block->l[i], 8) & 0x00FF00FF))

#define blk0B(i) (block->l[i])

#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^        \
                                       block->l[(i + 8) & 15] ^         \
                                       block->l[(i + 2) & 15] ^         \
                                       block->l[i & 15], 1))

#define R0_L(v, w, x, y, z, i)                                          \
    z += ((w & (x ^ y)) ^ y) + blk0L(i) + 0x5A827999 + rol(v, 5);       \
    w = rol(w, 30);
#define R0_B(v, w, x, y, z, i)                                          \
    z += ((w & (x ^ y)) ^ y) + blk0B(i) + 0x5A827999 + rol(v, 5);       \
    w = rol(w, 30);
#define R1(v, w, x, y, z, i)                                            \
    z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5);         \
    w = rol(w, 30);
#define R2(v, w, x, y, z, i)                                            \
    z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5);                 \
    w = rol(w, 30);
#define R3(v, w, x, y, z, i)                                            \
    z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5);   \
    w = rol(w, 30);
#define R4(v, w, x, y, z, i)                                            \
    z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5);                 \
    w = rol(w, 30);

#define R0A_L(i) R0_L(a, b, c, d, e, i)
#define R0B_L(i) R0_L(b, c, d, e, a, i)
#define R0C_L(i) R0_L(c, d, e, a, b, i)
#define R0D_L(i) R0_L(d, e, a, b, c, i)
#define R0E_L(i) R0_L(e, a, b, c, d, i)

#define R0A_B(i) R0_B(a, b, c, d, e, i)
#define R0B_B(i) R0_B(b, c, d, e, a, i)
#define R0C_B(i) R0_B(c, d, e, a, b, i)
#define R0D_B(i) R0_B(d, e, a, b, c, i)
#define R0E_B(i) R0_B(e, a, b, c, d, i)

#define R1A(i) R1(a, b, c, d, e, i)
#define R1B(i) R1(b, c, d, e, a, i)
#define R1C(i) R1(c, d, e, a, b, i)
#define R1D(i) R1(d, e, a, b, c, i)
#define R1E(i) R1(e, a, b, c, d, i)

#define R2A(i) R2(a, b, c, d, e, i)
#define R2B(i) R2(b, c, d, e, a, i)
#define R2C(i) R2(c, d, e, a, b, i)
#define R2D(i) R2(d, e, a, b, c, i)
#define R2E(i) R2(e, a, b, c, d, i)

#define R3A(i) R3(a, b, c, d, e, i)
#define R3B(i) R3(b, c, d, e, a, i)
#define R3C(i) R3(c, d, e, a, b, i)
#define R3D(i) R3(d, e, a, b, c, i)
#define R3E(i) R3(e, a, b, c, d, i)

#define R4A(i) R4(a, b, c, d, e, i)
#define R4B(i) R4(b, c, d, e, a, i)
#define R4C(i) R4(c, d, e, a, b, i)
#define R4D(i) R4(d, e, a, b, c, i)
#define R4E(i) R4(e, a, b, c, d, i)

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1Context;

static void SHA1_transform(uint32_t state[5], const unsigned char buffer[64]) {
    uint32_t a, b, c, d, e;

    typedef union {
        unsigned char c[64];
        uint32_t l[16];
    } u;

    unsigned char w[64];
    u *block = (u *) w;

    memcpy(block, buffer, 64);

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    static uint32_t endianness_indicator = 0x1;
    if (((unsigned char *) &endianness_indicator)[0]) {
        R0A_L( 0);
        R0E_L( 1);
        R0D_L( 2);
        R0C_L( 3);
        R0B_L( 4);
        R0A_L( 5);
        R0E_L( 6);
        R0D_L( 7);
        R0C_L( 8);
        R0B_L( 9);
        R0A_L(10);
        R0E_L(11);
        R0D_L(12);
        R0C_L(13);
        R0B_L(14);
        R0A_L(15);
    } else {
        R0A_B( 0);
        R0E_B( 1);
        R0D_B( 2);
        R0C_B( 3);
        R0B_B( 4);
        R0A_B( 5);
        R0E_B( 6);
        R0D_B( 7);
        R0C_B( 8);
        R0B_B( 9);
        R0A_B(10);
        R0E_B(11);
        R0D_B(12);
        R0C_B(13);
        R0B_B(14);
        R0A_B(15);
    }
    R1E(16);
    R1D(17);
    R1C(18);
    R1B(19);
    R2A(20);
    R2E(21);
    R2D(22);
    R2C(23);
    R2B(24);
    R2A(25);
    R2E(26);
    R2D(27);
    R2C(28);
    R2B(29);
    R2A(30);
    R2E(31);
    R2D(32);
    R2C(33);
    R2B(34);
    R2A(35);
    R2E(36);
    R2D(37);
    R2C(38);
    R2B(39);
    R3A(40);
    R3E(41);
    R3D(42);
    R3C(43);
    R3B(44);
    R3A(45);
    R3E(46);
    R3D(47);
    R3C(48);
    R3B(49);
    R3A(50);
    R3E(51);
    R3D(52);
    R3C(53);
    R3B(54);
    R3A(55);
    R3E(56);
    R3D(57);
    R3C(58);
    R3B(59);
    R4A(60);
    R4E(61);
    R4D(62);
    R4C(63);
    R4B(64);
    R4A(65);
    R4E(66);
    R4D(67);
    R4C(68);
    R4B(69);
    R4A(70);
    R4E(71);
    R4D(72);
    R4C(73);
    R4B(74);
    R4A(75);
    R4E(76);
    R4D(77);
    R4C(78);
    R4B(79);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

static void SHA1_init(SHA1Context *context) {
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

static void SHA1_update(SHA1Context *context, const unsigned char *data,
                        unsigned int len) {
    uint32_t i, j;

    j = (context->count[0] >> 3) & 63;

    if ((context->count[0] += len << 3) < (len << 3)) {
        context->count[1]++;
    }

    context->count[1] += (len >> 29);

    if ((j + len) > 63) {
        memcpy(&(context->buffer[j]), data, (i = 64 - j));
        SHA1_transform(context->state, context->buffer);
        for ( ; (i + 63) < len; i += 64) {
            SHA1_transform(context->state, &(data[i]));
        }
        j = 0;
    } else {
        i = 0;
    }

    memcpy(&(context->buffer[j]), &(data[i]), len - i);
}

static void SHA1_final(unsigned char digest[20], SHA1Context *context) {
    uint32_t i;
    unsigned char finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)
                        ((context->count[(i >= 4 ? 0 : 1)] >>
                          ((3 - (i & 3)) * 8)) & 255);
    }

    SHA1_update(context, (unsigned char *) "\200", 1);

    while ((context->count[0] & 504) != 448) {
        SHA1_update(context, (unsigned char *) "\0", 1);
    }

    SHA1_update(context, finalcount, 8);

    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)
                    ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }

    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(&finalcount, 0, 8);

    SHA1_transform(context->state, context->buffer);
}

// HMAC-SHA-1:
//
// K - is key padded with zeros to 512 bits
// m - is message
// OPAD - 0x5c5c5c...
// IPAD - 0x363636...
//
// HMAC(K,m) = SHA1((K ^ OPAD) . SHA1((K ^ IPAD) . m))
void HMAC_SHA1(unsigned char hmac[HMAC_LEN], const unsigned char *key, int key_len,
               const unsigned char *message, int message_len) {
    unsigned char kopad[64], kipad[64];
    int i;

    // sha1 输出一个长度为 20 bytes 的摘要 
    unsigned char digest[20];
    SHA1Context context;
    char sha1_key[key_len+1];
    strcpy(sha1_key, key);
    if (key_len > 64) {
        SHA1_init(&context);
        SHA1_update(&context, sha1_key, key_len);
        SHA1_final(digest, &context);
        strcpy(sha1_key, digest);
        key_len = 20;
    }

    for (i = 0; i < key_len; i++) {
        kopad[i] = sha1_key[i] ^ 0x5c;
        kipad[i] = sha1_key[i] ^ 0x36;
    }

    for ( ; i < 64; i++) {
        kopad[i] = 0 ^ 0x5c;
        kipad[i] = 0 ^ 0x36;
    }

    SHA1_init(&context);
    SHA1_update(&context, kipad, 64);
    SHA1_update(&context, message, message_len);
    SHA1_final(digest, &context);

    SHA1_init(&context);
    SHA1_update(&context, kopad, 64);
    SHA1_update(&context, digest, 20);
    SHA1_final(hmac, &context);
}

void 
HMAC2HEX(unsigned char hmac[HMAC_LEN], char* out){
    int i;
    for(i=0; i<HMAC_LEN; i++){
        out[i*2]   = "0123456789abcdef"[hmac[i] >> 4];
		out[i*2+1] = "0123456789abcdef"[hmac[i] & 0x0F];
    }
}
void SHA1(unsigned char hmac[HMAC_LEN],
          const unsigned char *message,
          int message_len)
{
    SHA1Context context;
    SHA1_init(&context);
    SHA1_update(&context, message, message_len);
    SHA1_final(hmac, &context);
}

#define rot(x,k) (((x) << (k)) | ((x) >> (32 - (k))))

uint64_t hash(const unsigned char *k, int length) {
    uint32_t a, b, c;

    a = b = c = 0xdeadbeef + ((uint32_t) length);

    static uint32_t endianness_indicator = 0x1;
    if (((unsigned char *) &endianness_indicator)[0]) {
        while (length > 12) {
            a += k[0];
            a += ((uint32_t) k[1]) << 8;
            a += ((uint32_t) k[2]) << 16;
            a += ((uint32_t) k[3]) << 24;
            b += k[4];
            b += ((uint32_t) k[5]) << 8;
            b += ((uint32_t) k[6]) << 16;
            b += ((uint32_t) k[7]) << 24;
            c += k[8];
            c += ((uint32_t) k[9]) << 8;
            c += ((uint32_t) k[10]) << 16;
            c += ((uint32_t) k[11]) << 24;
            a -= c;
            a ^= rot(c, 4);
            c += b;
            b -= a;
            b ^= rot(a, 6);
            a += c;
            c -= b;
            c ^= rot(b, 8);
            b += a;
            a -= c;
            a ^= rot(c, 16);
            c += b;
            b -= a;
            b ^= rot(a, 19);
            a += c;
            c -= b;
            c ^= rot(b, 4);
            b += a;
            length -= 12;
            k += 12;
        }

        switch(length) {
        case 12:
            c += ((uint32_t) k[11]) << 24;
        case 11:
            c += ((uint32_t) k[10]) << 16;
        case 10:
            c += ((uint32_t) k[9]) << 8;
        case 9 :
            c += k[8];
        case 8 :
            b += ((uint32_t) k[7]) << 24;
        case 7 :
            b += ((uint32_t) k[6]) << 16;
        case 6 :
            b += ((uint32_t) k[5]) << 8;
        case 5 :
            b += k[4];
        case 4 :
            a += ((uint32_t) k[3]) << 24;
        case 3 :
            a += ((uint32_t) k[2]) << 16;
        case 2 :
            a += ((uint32_t) k[1]) << 8;
        case 1 :
            a += k[0];
            break;
        case 0 :
            goto end;
        }
    } else {
        while (length > 12) {
            a += ((uint32_t) k[0]) << 24;
            a += ((uint32_t) k[1]) << 16;
            a += ((uint32_t) k[2]) << 8;
            a += ((uint32_t) k[3]);
            b += ((uint32_t) k[4]) << 24;
            b += ((uint32_t) k[5]) << 16;
            b += ((uint32_t) k[6]) << 8;
            b += ((uint32_t) k[7]);
            c += ((uint32_t) k[8]) << 24;
            c += ((uint32_t) k[9]) << 16;
            c += ((uint32_t) k[10]) << 8;
            c += ((uint32_t) k[11]);
            a -= c;
            a ^= rot(c, 4);
            c += b;
            b -= a;
            b ^= rot(a, 6);
            a += c;
            c -= b;
            c ^= rot(b, 8);
            b += a;
            a -= c;
            a ^= rot(c, 16);
            c += b;
            b -= a;
            b ^= rot(a, 19);
            a += c;
            c -= b;
            c ^= rot(b, 4);
            b += a;
            length -= 12;
            k += 12;
        }

        switch(length) {
        case 12:
            c += k[11];
        case 11:
            c += ((uint32_t) k[10]) << 8;
        case 10:
            c += ((uint32_t) k[9]) << 16;
        case 9 :
            c += ((uint32_t) k[8]) << 24;
        case 8 :
            b += k[7];
        case 7 :
            b += ((uint32_t) k[6]) << 8;
        case 6 :
            b += ((uint32_t) k[5]) << 16;
        case 5 :
            b += ((uint32_t) k[4]) << 24;
        case 4 :
            a += k[3];
        case 3 :
            a += ((uint32_t) k[2]) << 8;
        case 2 :
            a += ((uint32_t) k[1]) << 16;
        case 1 :
            a += ((uint32_t) k[0]) << 24;
            break;
        case 0 :
            goto end;
        }
    }

    c ^= b;
    c -= rot(b, 14);
    a ^= c;
    a -= rot(c, 11);
    b ^= a;
    b -= rot(a, 25);
    c ^= b;
    c -= rot(b, 16);
    a ^= c;
    a -= rot(c, 4);
    b ^= a;
    b -= rot(a, 14);
    c ^= b;
    c -= rot(b, 24);

end:
    return ((((uint64_t) c) << 32) | b);
}

int is_blank(char c) {
    return ((c == ' ') || (c == '\t'));
}




//************************************************URL Query escape************************************
int should_escape(int c){
	switch(c){
	case '$':
    case '&':
    case '+':
    case ',':
    case ':':
    case ';':
    case '=':
    case '?':
    case '@': 
        return 1;
	}

	return 0;
}

void
query_escape(char *out, const char* query, unsigned int query_len){
    if(query_len == 0){
        query_len = strlen(query);
    }
	int spaceCount = 0;
    int hexCount = 0;
    char *ret_s = out;
    int i;
	for(i = 0; i < query_len; i++){
		int c = query[i];
		if(should_escape(c)){
			if(c == ' '){
				spaceCount++;
			} else {
				hexCount++;
			}
		}
	}
	if(spaceCount == 0 && hexCount == 0){
        memcpy(ret_s, query, query_len);
		return;
	}

	if(hexCount == 0){
        memcpy(ret_s, query, query_len);
		for(i = 0; i < query_len; i++){
			if(query[i] == ' '){
				ret_s[i] = '+';
			}
		}
		return;
	}

    int j = 0;
    for(i = 0; i < query_len; i++){
        int c = query[i];
        if(c== ' '){
            ret_s[i] = '+';
            j++;
        }else if(should_escape(c)){
			ret_s[j] = '%';
			ret_s[j+1] = "0123456789ABCDEF"[c>>4];
			ret_s[j+2] = "0123456789ABCDEF"[c&15];
			j += 3;
        }else{
			ret_s[j] = query[i];
			j++;
        }
	}
}
