/**
 * SPDX-FileCopyrightText: 2024 Infineon Technologies AG
 * SPDX-License-Identifier: MIT
 *
 * \author Infineon Technologies AG
 *
 * \file pal_gpio.c
 *
 * \brief   This file implements the platform abstraction layer APIs for GPIO.
 *
 * \ingroup  grPAL
 *
 * @{
 */

#include "pal/pal_os_memory.h"

#include <zephyr/kernel.h>

void* pal_os_malloc(uint32_t blk_sz) {
    void* ret;

    ret = k_malloc(blk_sz);

    return (ret);
}

void* pal_os_calloc(uint32_t num_of_blk, uint32_t blk_sz) {
    void* ret;

    ret = k_calloc(num_of_blk, blk_sz);

    return (ret);
}

void pal_os_free(void* block) {
    k_free(block);
}

void pal_os_memcpy(void* dst, void const* src, uint32_t sz) {
    // flawfinder: ignore
    memcpy(dst, src, sz);
}

void pal_os_memset(void* buf, uint32_t val, uint32_t sz) {
    memset(buf, (int32_t)val, sz);
}
