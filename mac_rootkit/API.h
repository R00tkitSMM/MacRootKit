/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef __KERNEL__

#include <stdio.h>
#include <unistd.h>

#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <machine/limits.h>

enum kIOKernelRootKitOperation
{
	kIOKernelRootKitHookKernelFunction,
	kIOKernelRootKitAddBreakpoint,
	kIOKernelRootKitKernelCall,
	kIOKernelRootKitGetKaslrSlide,
	kIOKernelRootKitGetKernelBase,
	kIOKernelRootKitGetKernelSymbol,
	kIOKernelRootKitGetKextSymbol,
	kIOKernelRootKitKernelRead,
	kIOKernelRootKitKernelReadUnsafe,
	kIOKernelRootKitKernelWrite,
	kIOKernelRootKitKernelWriteUnsafe,
	kIOKernelRootKitKernelVmAllocate,
	kIOKernelRootKitKernelVmDeallocate,
	kIOKernelRootKitKernelVmProtect,
	kIOKernelRootKitKernelVmRemap,
	kIOKernelRootKitKalloc,
	kIOKernelRootKitPhysicalRead,
	kIOKernelRootKitPhysicalWrite,
	kIOKernelRootKitKernelVirtualToPhysical,
	kIOKernelRootKitTaskForPid,
	kIOKernelRootKitGetTaskForPid,
	kIOKernelRootKitGetProcForPid,
	kIOKernelRootKitGetTaskByName,
	kIOKernelRootKitGetProcByName,
	kIOKernelRootKitMachVmRead,
	kIOKernelRootKitMachVmWrite,
	kIOKernelRootKitMachVmAllocate,
	kIOKernelRootKitMachVmDeallocate,
	kIOKernelRootKitMachVmProtect,
	kIOKernelRootKitMachVmCall,
	kIOKernelRootKitVirtualToPhysical,
	kIOKernelRootKitMmap,
	kIOKernelRootKitMachMsgSend,
	kIOKernelRootKitCopyIn,
	kIOKernelRootKitCopyOut,
	kIOKernelRootKitCreateSharedMemory,
	kIOKernelRootKitMapSharedMemory,
};

#define xStringify(a) Stringify(a)
#define Stringify(a) #a

#define xConcat(a, b) Concat(a, b)
#define Concat(a, b) a ## b

/**
 *  Prefix name with your plugin name (to ease symbolication and avoid conflicts)
 */
#define ADDPR(a) xConcat(xConcat(PRODUCT_NAME, _), a)

extern void* kern_os_malloc(size_t size);
extern void* kern_os_calloc(size_t num, size_t size);
extern void* kern_os_realloc(void *addr, size_t nsize);
extern void kern_os_free(void *addr);

extern void* memmem(const void *h0, size_t k, const void *n0, size_t l);
extern void* memchr(const void *src, int c, size_t n);
extern void  qsort(void *a, size_t n, size_t es, int (*cmp)(const void*, const void*));
