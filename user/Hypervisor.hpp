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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <Types.h>

#include <Hypervisor/Hypervisor.h>

#include "Fuzzer.hpp"

#include "virt.h"

namespace Virtualization {
#define BOOT_LINE_LENGTH 256

#define FRAMEBUFFER_WIDTH 640
#define FRAMEBUFFER_HEIGHT 960
#define FRAMEBUFFER_DEPTH_BITS 32
#define FRAMEBUFFER_STRIDE_BYTES (FRAMEBUFFER_WIDTH * FRAMEBUFFER_DEPTH_BITS / 8)
#define FRAMEBUFFER_SIZE_BYTES (FRAMEBUFFER_STRIDE_BYTES * FRAMEBUFFER_HEIGHT * 3)

    /*
     * Video information..
     */

    struct Boot_Video {
        unsigned long v_baseAddr; /* Base address of video memory */
        unsigned long v_display;  /* Display Code (if Applicable */
        unsigned long v_rowBytes; /* Number of bytes per pixel row */
        unsigned long v_width;    /* Width */
        unsigned long v_height;   /* Height */
        unsigned long v_depth;    /* Pixel Depth and other parameters */
    };

#define kBootVideoDepthMask (0xFF)
#define kBootVideoDepthDepthShift (0)
#define kBootVideoDepthRotateShift (8)
#define kBootVideoDepthScaleShift (16)
#define kBootVideoDepthBootRotateShift (24)

#define kBootFlagsDarkBoot (1 << 0)

    typedef struct Boot_Video Boot_Video;

/* Boot argument structure - passed into Mach kernel at boot time.
 */
#define kBootArgsRevision 1
#define kBootArgsRevision2 2 /* added boot_args->bootFlags */
#define kBootArgsVersion1 1
#define kBootArgsVersion2 2

    typedef struct boot_args {
        UInt16 Revision;                    /* Revision of boot_args structure */
        UInt16 Version;                     /* Version of boot_args structure */
        UInt64 virtBase;                    /* Virtual base of memory */
        UInt64 physBase;                    /* Physical base of memory */
        UInt64 memSize;                     /* Size of memory */
        UInt64 topOfKernelData;             /* Highest physical address used in kernel data area */
        Boot_Video Video;                   /* Video Information */
        UInt32 machineType;                 /* Machine Type */
        void* deviceTreeP;                  /* Base of flattened device tree */
        UInt32 deviceTreeLength;            /* Length of flattened tree */
        char CommandLine[BOOT_LINE_LENGTH]; /* Passed in command line */
        UInt64 bootFlags;                   /* Additional flags specified by the bootloader */
        UInt64 memSizeActual;               /* Actual size of memory */
    } boot_args;

    /* Valid Syndrome Register EC field values */
    enum arm_exception_class {
        EC_UNCATEGORIZED = 0x00,
        EC_WFX_TRAP = 0x01,
        EC_CP15RTTRAP = 0x03,
        EC_CP15RRTTRAP = 0x04,
        EC_CP14RTTRAP = 0x05,
        EC_CP14DTTRAP = 0x06,
        EC_ADVSIMDFPACCESSTRAP = 0x07,
        EC_FPIDTRAP = 0x08,
        EC_PACTRAP = 0x09,
        EC_BXJTRAP = 0x0a,
        EC_CP14RRTTRAP = 0x0c,
        EC_BTITRAP = 0x0d,
        EC_ILLEGALSTATE = 0x0e,
        EC_AA32_SVC = 0x11,
        EC_AA32_HVC = 0x12,
        EC_AA32_SMC = 0x13,
        EC_AA64_SVC = 0x15,
        EC_AA64_HVC = 0x16,
        EC_AA64_SMC = 0x17,
        EC_SYSTEMREGISTERTRAP = 0x18,
        EC_SVEACCESSTRAP = 0x19,
        EC_SMETRAP = 0x1d,
        EC_INSNABORT = 0x20,
        EC_INSNABORT_SAME_EL = 0x21,
        EC_PCALIGNMENT = 0x22,
        EC_DATAABORT = 0x24,
        EC_DATAABORT_SAME_EL = 0x25,
        EC_SPALIGNMENT = 0x26,
        EC_AA32_FPTRAP = 0x28,
        EC_AA64_FPTRAP = 0x2c,
        EC_SERROR = 0x2f,
        EC_BREAKPOINT = 0x30,
        EC_BREAKPOINT_SAME_EL = 0x31,
        EC_SOFTWARESTEP = 0x32,
        EC_SOFTWARESTEP_SAME_EL = 0x33,
        EC_WATCHPOINT = 0x34,
        EC_WATCHPOINT_SAME_EL = 0x35,
        EC_AA32_BKPT = 0x38,
        EC_VECTORCATCH = 0x3a,
        EC_AA64_BKPT = 0x3c,
        EC_AA64_APPLE = 0x3f,
    };

    // ARM64 reset tramp
    // I don't know why the CPU resets at EL0, so this is a trampoline
    // that takes you to EL1.

    // UPDATE: See CPSR below

    const UInt8 sArm64ResetVector[] = {
        0x01, 0x00, 0x00, 0xD4, // SVC #0
        0x00, 0x00, 0x20, 0xD4, // BRK #0
    };

    const UInt8 sArm64ResetTrampoline[] = {
        0x00, 0x00, 0xB0, 0xD2, // MOV X0, #0x80000000
        0x00, 0x00, 0x1F, 0xD6, // BR  X0
        0x00, 0x00, 0x20, 0xD4, // BRK #0
    };

#define HYP_ASSERT_SUCCESS(ret) assert((hv_return_t)(ret) == HV_SUCCESS)

    static constexpr UInt64 gAdrResetTrampoline = 0xF0000000;
    static constexpr UInt64 gResetTrampolineMemorySize = 0x10000;

    static constexpr UInt64 gMainMemory = 0x80000000;
    static constexpr UInt64 gMainMemSize = 0x40000000;

    class Hypervisor {
    public:
        explicit Hypervisor(Fuzzer::Harness* harness, xnu::Mach::VmAddress virtualBase, UInt64 base,
                            Size size, xnu::Mach::VmAddress entryPoint);

        Fuzzer::Harness* getHarness() {
            return harness;
        }

        void* getPhysicalMainMemory() {
            return mainMemory;
        }

        hv_vcpu_t getVirtualCpu() {
            return vcpu;
        }

        hv_vcpu_exit_t* getVirtualCpuExit() {
            return vcpu_exit;
        }

        UInt64 getBase() {
            return base;
        }

        Size getSize() {
            return size;
        }

        xnu::Mach::VmAddress getEntryPoint() {
            return entryPoint;
        }

        void synchronizeCpuState();

        void flushCpuState();

        int sysregRead(UInt32 reg, UInt32 rt);
        int sysregWrite(UInt32 reg, UInt64 val);

        int prepareSystemMemory();

        void prepareBootArgs(const char* deviceTreePath);

        void configure();

        void start();

        void destroy();

    private:
        Fuzzer::Harness* harness;

        HvfArm64State state;

        struct boot_args boot_args;

        UInt64 base;

        xnu::Mach::VmAddress virtualBase;

        Size size;

        xnu::Mach::VmAddress entryPoint;

        hv_vcpu_t vcpu;

        hv_vcpu_exit_t* vcpu_exit;

        void* resetTrampolineMemory;
        void* mainMemory;

        UInt64 bootArgsOffset;
        UInt64 deviceTreeOffset;

        UInt64 framebufferOffset;
    };
} // namespace Virtualization
