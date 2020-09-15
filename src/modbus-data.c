/*
 * Copyright © 2010-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <stdlib.h>

#ifndef _MSC_VER
#  include <stdint.h>
#else
#  include "stdint.h"
#endif

#include <string.h>

#include <rtthread.h>
#include <rtdevice.h>

#include "modbus.h"


static inline uint16_t bswap_16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

static inline uint32_t bswap_32(uint32_t x)
{
    return (bswap_16(x & 0xffff) << 16) | (bswap_16(x >> 16));
}

#define modbus_ntohl(x)     (x)
#define modbus_htonl(x)     (x)
/* Sets many bits from a single byte value (all 8 bits of the byte value are
   set) */
void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value)
{
    int i;

    for (i=0; i < 8; i++) {
        dest[idx+i] = (value & (1 << i)) ? 1 : 0;
    }
}

/* Sets many bits from a table of bytes (only the bits between idx and
   idx + nb_bits are set) */
void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits,
                                const uint8_t *tab_byte)
{
    unsigned int i;
    int shift = 0;

    for (i = idx; i < idx + nb_bits; i++) {
        dest[i] = tab_byte[(i - idx) / 8] & (1 << shift) ? 1 : 0;
        /* gcc doesn't like: shift = (++shift) % 8; */
        shift++;
        shift %= 8;
    }
}

/* Gets the byte value from many bits.
   To obtain a full byte, set nb_bits to 8. */
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx,
                                  unsigned int nb_bits)
{
    unsigned int i;
    uint8_t value = 0;

    if (nb_bits > 8) {
        /* Assert is ignored if NDEBUG is set */
        RT_ASSERT(nb_bits < 8);
        nb_bits = 8;
    }

    for (i=0; i < nb_bits; i++) {
        value |= (src[idx+i] << i);
    }

    return value;
}

/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float modbus_get_float_abcd(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = modbus_ntohl(((uint32_t)src[0] << 16) + src[1]);
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a long from 4 bytes (Modbus) without any conversion (ABCD) */
long modbus_get_long_abcd(const uint16_t *src)
{
    long l;
    uint32_t i;

    i = modbus_ntohl(((uint32_t)src[0] << 16) + src[1]);
    memcpy(&l, &i, sizeof(long));

    return l;
}

/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float modbus_get_float_dcba(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = modbus_ntohl(bswap_32((((uint32_t)src[0]) << 16) + src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a long from 4 bytes (Modbus) in inversed format (DCBA) */
long modbus_get_long_dcba(const uint16_t *src)
{
    long l;
    uint32_t i;

    i = modbus_ntohl(bswap_32((((uint32_t)src[0]) << 16) + src[1]));
    memcpy(&l, &i, sizeof(long));

    return l;
}

/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float modbus_get_float_badc(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = modbus_ntohl((uint32_t)(bswap_16(src[0]) << 16) + bswap_16(src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a long from 4 bytes (Modbus) with swapped bytes (BADC) */
long modbus_get_long_badc(const uint16_t *src)
{
    long l;
    uint32_t i;

    i = modbus_ntohl((uint32_t)(bswap_16(src[0]) << 16) + bswap_16(src[1]));
    memcpy(&l, &i, sizeof(long));

    return l;
}

/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float modbus_get_float_cdab(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = modbus_ntohl((((uint32_t)src[1]) << 16) + src[0]);
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a long from 4 bytes (Modbus) with swapped words (CDAB) */
long modbus_get_long_cdab(const uint16_t *src)
{
    long l;
    uint32_t i;

    i = modbus_ntohl((((uint32_t)src[1]) << 16) + src[0]);
    memcpy(&l, &i, sizeof(long));

    return l;
}

/* DEPRECATED - Get a float from 4 bytes in sort of Modbus format */
float modbus_get_float(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = (((uint32_t)src[1]) << 16) + src[0];
    memcpy(&f, &i, sizeof(float));

    return f;
}


/* Set a float to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_float_abcd(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a long to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_long_abcd(long l, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &l, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a float to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_float_dcba(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = bswap_32(modbus_htonl(i));
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a long to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_long_dcba(long l, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &l, sizeof(uint32_t));
    i = bswap_32(modbus_htonl(i));
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a float to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_float_badc(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)bswap_16(i >> 16);
    dest[1] = (uint16_t)bswap_16(i & 0xFFFF);
}

/* Set a long to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_long_badc(long l, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &l, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)bswap_16(i >> 16);
    dest[1] = (uint16_t)bswap_16(i & 0xFFFF);
}

/* Set a float to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_float_cdab(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}

/* Set a long to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_long_cdab(long l, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &l, sizeof(uint32_t));
    i = modbus_htonl(i);
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}

/* DEPRECATED - Set a float to 4 bytes in a sort of Modbus format! */
void modbus_set_float(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}
