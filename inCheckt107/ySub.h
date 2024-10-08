//
//  KQueueScanContinuePatch.c
//  KQueueScanContinuePatch
//
//  Created by Wowfunhappy with assistance from krackers.
//
#ifndef inCheckt107_h
#define inCheckt107_h
#include <i386/proc_reg.h>
#include <mach-o/loader.h>
#include <IOKit/IOLib.h>

#if __LP64__
#define NUM_SUPPORTED_KERNELS 1
#else
#define NUM_SUPPORTED_KERNELS 1
#endif

#define LENGTH(arr) sizeof(arr) / sizeof(*arr)

// Copied from github.com/leiless/ksymresolver/blob/master/ksymresolver/ksymresolver.h
#define KERN_TEXT_BASE ((vm_offset_t) 0xffffff8000200000ULL)

#define        CR0_WP 0x00010000

static vm_offset_t possible_kqueue_scan_continue_panic_start_locations[NUM_SUPPORTED_KERNELS] = {
#if __LP64__
        0xffffff80005369ef, //10.7.5 11G63, xnu-1699.32.7
#else
        0x005566fb,         //10.7.5 11G63, xnu-1699.32.7 (32 bit)
#endif
};

static char possible_search_bytes[NUM_SUPPORTED_KERNELS][4] = {
#if __LP64__
        //10.7.5 11G63, xnu-1699.32.7
        {
                0x42, 0x8A, 0x0C, 0x38, //mov   cl, byte [ds:rax+r15]
        },
#else
        //10.7.5 11G63, xnu-1699.32.7 (32 bit)
        {
                0x89, 0x4c, 0x24, 0x04, //mov   cl, byte [ds:eax+esi]
        },
#endif
};

//static char possible_replacement_bytes[NUM_SUPPORTED_KERNELS][4] = {
//        {0x00, 0x00, 0x00, 0x00},                        // jmp [abs] (gotta figure out address of deadspace
//};
//


static long long originAddress  = 0;
static boolean_t interrupt_status = 0;
static boolean_t write_protection_status = 0;

static char replacement_bytes[10];
static char original_bytes[10];

#endif /* inCheckt107_h */
