/**
 * SPDX-FileCopyrightText: 2021-2024 Infineon Technologies AG
 * SPDX-License-Identifier: MIT
 *
 * \author Infineon Technologies AG
 *
 * \file pal_os_memory.h
 *
 * \brief   This file provides the prototype declarations of PAL OS MEMORY.
 *
 * \ingroup  grPAL
 *
 * @{
 */

#ifndef _PAL_OS_MEMORY_H_
#define _PAL_OS_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pal/pal.h"

/**
 * \brief Allocates a block of memory specified by the block size and return the pointer to it.
 *
 * <br>
 *
 * \details
 * - Allocates a block of memory specified by the block size and return the pointer to it
 *
 * \pre
 * - None
 *
 * \note
 * - None
 *
 * \param[in] blk_sz Size of the block
 *
 * \retval  Block Pointer  Memory allocation is successful
 * \retval  NULL           Memory allocation is not successful
 */
LIBRARY_EXPORTS void* pal_os_malloc(uint32_t blk_sz);

/**
 * @brief Allocates a block of memory specified by the block size and return the pointer to it.
 *
 * <br>
 *
 * \details
 * - Allocates a block of memory specified by the block size and return the pointer to it
 *
 * \pre
 * - None
 *
 * \note
 * - None
 *
 * \param[in] num_of_blk Number of block to allocate
 * \param[in] blk_sz     Size of the block
 *
 * \retval  Block Pointer  Memory allocation is successful
 * \retval  NULL           Memory allocation is not successful
 */
LIBRARY_EXPORTS void* pal_os_calloc(uint32_t num_of_blk, uint32_t blk_sz);

/**
 * @brief Frees the block of memory specified by the block pointer.
 *
 * <br>
 *
 * \details
 * - Frees the block of memory specified by the block pointer
 *
 * \pre
 * - None
 *
 * \note
 * - None
 *
 * \param[in] block Pointer to memory block to be freed
 *
 */
LIBRARY_EXPORTS void pal_os_free(void* block);

/**
 * @brief Copies data from source to destination.
 *
 * <br>
 *
 * \details
 * - Copies data from source to destination
 *
 * \pre
 * - None
 *
 * \note
 * - None
 *
 * \param[in] dst Destination to copy the data to
 * \param[in] src Source to copy the data from
 * \param[in] sz  Size of the data to copy
 *
 */
LIBRARY_EXPORTS void pal_os_memcpy(void* dst, void const* src, uint32_t sz);

/**
 * @brief Sets the buffer with the given value.
 *
 * <br>
 *
 * \details
 * - Sets the buffer with the given value
 *
 * \pre
 * - None
 *
 * \note
 * - None
 *
 * \param[in] buf Pointer to buffer
 * \param[in] val Value to be set in the buffer
 * \param[in] sz  Size of the buffer
 *
 */
LIBRARY_EXPORTS void pal_os_memset(void* buf, uint32_t val, uint32_t sz);

#ifdef __cplusplus
}
#endif

#endif /* _PAL_OS_MEMORY_H_ */

/**
 * @}
 */
