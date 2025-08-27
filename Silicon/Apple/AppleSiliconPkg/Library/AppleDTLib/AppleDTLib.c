/*
 * pongoOS - https://checkra.in
 *
 * Copyright (C) 2019-2023 checkra1n team
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

#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/AppleDTLib.h>


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>



EFI_STATUS EFIAPI AppleDTLibInitialize(VOID) {
    return EFI_SUCCESS;
}

static void get_cells(uint64_t *dst, const uint32_t **src, int cells)
{
    *dst = 0;
    for (int i = 0; i < cells; i++)
        *dst |= ((uint64_t) * ((*src)++)) << (32 * i);
}

int dt_check(void *mem, size_t size, size_t *offp)
{
    if(size < sizeof(dt_node_t)) return -1;
    dt_node_t *node = mem;
    size_t off = sizeof(dt_node_t);
    for(size_t i = 0, max = node->nprop; i < max; ++i)
    {
        if(size < off + sizeof(dt_prop_t)) return -1;
        dt_prop_t *prop = (dt_prop_t*)((uintptr_t)mem + off);
        size_t l = prop->len & 0xffffff;
        off += sizeof(dt_prop_t) + ((l + 0x3) & ~0x3);
        if(size < off) return -1;
    }
    for(size_t i = 0, max = node->nchld; i < max; ++i)
    {
        size_t add = 0;
        int r = dt_check((void*)((uintptr_t)mem + off), size - off, &add);
        if(r != 0) return r;
        off += add;
    }
    if(offp) *offp = off;
    return 0;
}

int dt_parse(dt_node_t *node, int depth, size_t *offp, int (*cb_node)(void*, dt_node_t*, int), void *cbn_arg, int (*cb_prop)(void*, dt_node_t*, int, const char*, void*, size_t), void *cbp_arg)
{
    if(cb_node)
    {
        int r = cb_node(cbn_arg, node, depth);
        if(r != 0) return r;
    }
    if(depth >= 0 || cb_prop)
    {
        size_t off = sizeof(dt_node_t);
        for(size_t i = 0, max = node->nprop; i < max; ++i)
        {
            dt_prop_t *prop = (dt_prop_t*)((uintptr_t)node + off);
            size_t l = prop->len & 0xffffff;
            off += sizeof(dt_prop_t) + ((l + 0x3) & ~0x3);
            if(cb_prop)
            {
                int r = cb_prop(cbp_arg, node, depth, prop->key, prop->val, l);
                if(r != 0) return r;
            }
        }
        if(depth >= 0)
        {
            for(size_t i = 0, max = node->nchld; i < max; ++i)
            {
                size_t add = 0;
                int r = dt_parse((dt_node_t*)((uintptr_t)node + off), depth + 1, &add, cb_node, cbn_arg, cb_prop, cbp_arg);
                if(r != 0) return r;
                off += add;
            }
            if(offp) *offp = off;
        }
    }
    return 0;
}

typedef struct
{
    const char *name;
    dt_node_t *node;
    int matchdepth;
} dt_find_cb_t;

static int dt_find_cb(void *a, dt_node_t *node, int depth, const char *key, void *val, size_t len)
{
    dt_find_cb_t *arg = a;
    if(AsciiStrCmp(key, "name") != 0)
    {
        return 0;
    }
    const char *name = arg->name;
    if(name[0] == '/') // Absolute path
    {
        // Don't require "/device-tree" prefix for everything.
        if(depth == 0)
        {
            return 0;
        }
        // If we're in the subtree of a node we didn't match against, then ignore everything.
        if(depth > arg->matchdepth)
        {
            return 0;
        }
        // If this condition is ever true, then we traversed back out of an entry
        // that we matched against, without finding a matching child node.
        if(depth < arg->matchdepth)
        {
            return -1;
        }
        ++name;
        const char *end = AsciiStrStr(name, "/");
        if(end) // Handle non-leaf segment
        {
            size_t size = end - name;
            if(AsciiStrnCmp(name, val, size) == 0 && size + 1 == len && ((const char*)val)[size] == '\0')
            {
                arg->name = end;
                arg->matchdepth = depth + 1;
            }
            return 0;
        }
        // Leaf segment can fall through
    }
    // Simple name
    if(AsciiStrnCmp(name, val, len) == 0 && AsciiStrLen(name) + 1 == len)
    {
        arg->node = node;
        return 1;
    }
    return 0;
}

dt_node_t* dt_find(dt_node_t *node, const char *name)
{
    dt_find_cb_t arg = { name, NULL, 1 };
    dt_parse(node, 0, NULL, NULL, NULL, &dt_find_cb, &arg);
    return arg.node;
}

typedef struct
{
    const char *key;
    void *val;
    size_t len;
} dt_prop_cb_t;

static int dt_prop_cb(void *a, dt_node_t *node, int depth, const char *key, void *val, size_t len)
{
    dt_prop_cb_t *arg = a;
    if(AsciiStrnCmp(arg->key, key, DT_KEY_LEN) == 0)
    {
        arg->val = val;
        arg->len = len;
        return 1;
    }
    return 0;
}

void* dt_prop(dt_node_t *node, const char *key, size_t *lenp)
{
    dt_prop_cb_t arg = { key, NULL, 0 };
    dt_parse(node, -1, NULL, NULL, NULL, &dt_prop_cb, &arg);
    if(arg.val && lenp) *lenp = arg.len;
    return arg.val;
}


typedef struct
{
    const char *name;
    const char *prop;
    size_t size;
} dt_arg_t;


// ========== Legacy/Compat ==========

int dt_check_32(void *mem, uint32_t size, uint32_t *offp) __asm__("_dt_check$32");
int dt_parse_32(dt_node_t *node, int depth, uint32_t *offp, int (*cb_node)(void*, dt_node_t*, int), void *cbn_arg, int (*cb_prop)(void*, dt_node_t*, int, const char*, void*, uint32_t), void *cbp_arg) __asm__("_dt_parse$32");
void* dt_prop_32(dt_node_t *node, const char *key, uint32_t *lenp) __asm__("_dt_prop$32");

int dt_check_32(void *mem, uint32_t size, uint32_t *offp)
{
    size_t off = 0;
    int r = dt_check(mem, size, &off);
    if(offp) *offp = (uint32_t)off;
    return r;
}

typedef struct
{
    int (*cb)(void *a, dt_node_t *node, int depth, const char *key, void *val, uint32_t len);
    void *arg;
} dt_parse_32_cbp_t;

static int dt_parse_32_cbp(void *a, dt_node_t *node, int depth, const char *key, void *val, size_t len)
{
    dt_parse_32_cbp_t *args = a;
    return args->cb(args->arg, node, depth, key, val, (uint32_t)len);
}

int dt_parse_32(dt_node_t *node, int depth, uint32_t *offp, int (*cb_node)(void*, dt_node_t*, int), void *cbn_arg, int (*cb_prop)(void*, dt_node_t*, int, const char*, void*, uint32_t), void *cbp_arg)
{
    dt_parse_32_cbp_t cbp_arg_32 =
    {
        .cb  = cb_prop,
        .arg = cbp_arg,
    };
    int (*cb_prop_32)(void*, dt_node_t*, int, const char*, void*, size_t) = dt_parse_32_cbp;
    cbp_arg = &cbp_arg_32;
    if(!cb_prop)
    {
        cb_prop_32 = NULL;
        cbp_arg = NULL;
    }
    size_t off = 0;
    int r = dt_parse(node, depth, &off, cb_node, cbn_arg, cb_prop_32, cbp_arg);
    if(offp) *offp = (uint32_t)off;
    return r;
}

void* dt_prop_32(dt_node_t *node, const char *key, uint32_t *lenp)
{
    size_t len = 0;
    void *val = dt_prop(node, key, &len);
    if(lenp) *lenp = (uint32_t)len;
    return val;
}



dt_node_t* dt_node(dt_node_t *node, const char *name)
{
    dt_node_t *dev = dt_find(node, name);
    if(!dev)
    {
        DEBUG((DEBUG_INFO, "Missing DeviceTree node: %a\n", name));
    }
    return dev;
}

typedef struct
{
    dt_node_t *path[8];
    dt_node_t *target;
    dt_node_t *parent;
} dt_node_parent_cb_t;

static int dt_node_parent_cb(void *a, dt_node_t *node, int depth)
{
    dt_node_parent_cb_t *arg = a;
    if(node == arg->target)
    {
        if(depth < 1)
        {
            DEBUG((DEBUG_INFO, "DeviceTree parent depth underflow: %d\n", depth));
        }
        arg->parent = arg->path[depth - 1];
        return 1;
    }
    if(depth < 0 || depth >= 8)
    {
        DEBUG((DEBUG_INFO, "DeviceTree parent depth out of bunds: %d\n", depth));
    }
    arg->path[depth] = node;
    return 0;
}

dt_node_t* dt_node_parent(dt_node_t *node)
{
    // Parsing the tree again just to find the parent node is really ugly and inefficient, but for now we're stuck with this.
    // Ideally we'd parse the DeviceTree entirely into heap memory, so we can:
    // a) traverse it faster and in either direction
    // b) overwrite/relocate the original DeviceTree in memory (e.g. for loading a bigger kernel)
    // The problem is that we currently allow clients to directly modify DeviceTree velues and we will need that
    // in one form or another regardless, so we'd have to think about how to design such a writeback.
    dt_node_parent_cb_t arg = { .target = node };
    dt_parse((dt_node_t*)FixedPcdGet64(PcdAdtPointer), 0, NULL, &dt_node_parent_cb, &arg, NULL, NULL);
    return arg.parent;
}

dt_node_t* dt_get(const char *name)
{
    return dt_node((dt_node_t*)FixedPcdGet64(PcdAdtPointer), name);
}

void* dt_node_prop(dt_node_t *node, const char *prop, size_t *size)
{
    void *val = dt_prop(node, prop, size);
    if(!val)
    {
        DEBUG((DEBUG_INFO, "Missing DeviceTree prop: %a\n", prop));
    }
    return val;
}

void* dt_get_prop(const char *device, const char *prop, size_t *size)
{
    return dt_node_prop(dt_get(device), prop, size);
}

uint32_t dt_node_u32(dt_node_t *node, const char *prop, uint32_t idx)
{
    size_t len = 0;
    uint32_t *val = dt_node_prop(node, prop, &len);
    if(len < (idx + 1) * sizeof(*val))
    {
        DEBUG((DEBUG_INFO, "DeviceTree u32 out of bounds: %a[%u]\n", prop, idx));
        ASSERT(FALSE);
    }
    return val[idx];
}

uint32_t dt_get_u32(const char *device, const char *prop, uint32_t idx)
{
    return dt_node_u32(dt_get(device), prop, idx);
}

uint64_t dt_node_u64(dt_node_t *node, const char *prop, uint32_t idx)
{
    size_t len = 0;
    uint64_t *val = dt_node_prop(node, prop, &len);
    if(len < (idx + 1) * sizeof(*val))
    {
        DEBUG((DEBUG_INFO, "DeviceTree u64 out of bounds: %a[%u]\n", prop, idx));
        ASSERT(FALSE);
    }
    return val[idx];
}

uint64_t dt_get_u64(const char *device, const char *prop, uint32_t idx)
{
    return dt_node_u64(dt_get(device), prop, idx);
}

void* dt_get_prop_32(const char *device, const char *prop, uint32_t *size) __asm__("_dt_get_prop$32");
void* dt_get_prop_32(const char *device, const char *prop, uint32_t *size)
{
    size_t len = 0;
    void *val = dt_get_prop(device, prop, &len);
    if(size) *size = (uint32_t)len;
    return val;
}

//borrowed from m1n1
int dt_node_reg(dt_node_t *node, uint32_t idx, uint64_t *paddr, uint64_t *psize)
{
    dt_node_t *parent = dt_node_parent(node);
    dt_node_t *cur = node;

    uint32_t a_cells = dt_node_u32(parent, "#address-cells", 0);
    uint32_t s_cells = dt_node_u32(parent, "#size-cells", 0);

    if (a_cells < 1 || a_cells > 2 || s_cells > 2)
    {
        DEBUG((DEBUG_ERROR, "bad n-cells\n"));
        return 1;
    }

    size_t reg_len = 0;
    const uint32_t *reg = dt_node_prop(node, "reg", &reg_len);

    if (!reg || !reg_len)
    {
        DEBUG((DEBUG_ERROR, "reg not found or empty\n"));
        return 1;
    }

    if (reg_len < (idx + 1) * (a_cells + s_cells) * 4)
    {
        DEBUG((DEBUG_ERROR, "bad reg property length %d\n", reg_len));
        return 1;
    }

    reg += idx * (a_cells + s_cells);

    uint64_t addr, size = 0;
    get_cells(&addr, &reg, a_cells);
    get_cells(&size, &reg, s_cells);

    while (parent)
    {
        cur = parent;
        parent = dt_node_parent(cur);

        size_t ranges_len;
        const uint32_t *ranges = dt_node_prop(cur, "ranges", &ranges_len);
        if (!ranges)
            break;

        uint32_t pa_cells = dt_node_u32(parent, "#address-cells", 0);

        if (pa_cells < 1 || pa_cells > 2 || s_cells > 2)
        {
            DEBUG((DEBUG_ERROR, "bad reg property length %d\n", reg_len));
            return 1;
        }

        int range_cnt = ranges_len / (4 * (pa_cells + a_cells + s_cells));

        while (range_cnt--)
        {
            uint64_t c_addr, p_addr, c_size;
            get_cells(&c_addr, &ranges, a_cells);
            get_cells(&p_addr, &ranges, pa_cells);
            get_cells(&c_size, &ranges, s_cells);

            //dprintf(" ranges %lx %lx %lx\n", c_addr, p_addr, c_size);

            if (addr >= c_addr && (addr + size) <= (c_addr + c_size)) {
                //dprintf(" translate %lx", addr);
                addr = addr - c_addr + p_addr;
                //dprintf(" -> %lx\n", addr);
                break;
            }
        }

        s_cells = dt_node_u32(parent, "#size-cells", 0);

        a_cells = pa_cells;
    }


    if (paddr)
        *paddr = addr;
    if (psize)
        *psize = size;

    return 0;
}