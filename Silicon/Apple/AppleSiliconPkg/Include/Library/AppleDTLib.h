/*
 * pongoOS - https://checkra.in
 *
 * Copyright (C) 2019-2024 checkra1n team
 *
 * This file is part of pongoOS.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#ifndef APPLEDTLIB_H
#define APPLEDTLIB_H

#include <stddef.h>
#include <stdint.h>


#define CMDLINE_LENGTH_RV1 256
#define CMDLINE_LENGTH_RV2 608
#define CMDLINE_LENGTH_RV3 1024

struct boot_video {
    UINT64 base;
    UINT64 display;
    UINT64 stride;
    UINT64 width;
    UINT64 height;
    UINT64 depth;
};

struct boot_args {
    UINT16 revision;
    UINT16 version;
    UINT64 virt_base;
    UINT64 phys_base;
    UINT64 mem_size;
    UINT64 top_of_kernel_data;
    struct boot_video video;
    UINT32 machine_type;
    VOID *devtree;
    UINT32 devtree_size;
    union {
        struct {
            char cmdline[CMDLINE_LENGTH_RV1];
            UINT64 boot_flags;
            UINT64 mem_size_actual;
        } rv1;
        struct {
            char cmdline[CMDLINE_LENGTH_RV2];
            UINT64 boot_flags;
            UINT64 mem_size_actual;
        } rv2;
        struct {
            char cmdline[CMDLINE_LENGTH_RV3];
            UINT64 boot_flags;
            UINT64 mem_size_actual;
        } rv3;
    };
};



#define DT_KEY_LEN 0x20

typedef struct
{
    uint32_t nprop;
    uint32_t nchld;
    char prop[];
} dt_node_t;

typedef struct
{
    char key[DT_KEY_LEN];
    uint32_t len;
    char val[];
} dt_prop_t;

int dt_check(void *mem, size_t size, size_t *offp) __asm__("_dt_check$64");
int dt_parse(dt_node_t *node, int depth, size_t *offp, int (*cb_node)(void*, dt_node_t*, int), void *cbn_arg, int (*cb_prop)(void*, dt_node_t*, int, const char*, void*, size_t), void *cbp_arg) __asm__("_dt_parse$64");
dt_node_t* dt_find(dt_node_t *node, const char *name);
void* dt_prop(dt_node_t *node, const char *key, size_t *lenp) __asm__("_dt_prop$64");


struct memmap
{
    uint64_t addr;
    uint64_t size;
};

dt_node_t* dt_node(dt_node_t *node, const char *name);
dt_node_t* dt_node_parent(dt_node_t *node);
dt_node_t* dt_get(const char *name);
void* dt_node_prop(dt_node_t *node, const char *prop, size_t *size);
void* dt_get_prop(const char *device, const char *prop, size_t *size) __asm__("_dt_get_prop$64");
uint32_t dt_node_u32(dt_node_t *node, const char *prop, uint32_t idx);
uint32_t dt_get_u32(const char *device, const char *prop, uint32_t idx);
uint64_t dt_node_u64(dt_node_t *node, const char *prop, uint32_t idx);
uint64_t dt_get_u64(const char *device, const char *prop, uint32_t idx);
int dt_node_reg(dt_node_t *node, uint32_t idx, uint64_t *paddr, uint64_t *psize);

#endif /* APPLEDTLIB_H */
