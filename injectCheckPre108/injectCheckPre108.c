//
//  KQueueScanContinuePatch.c
//  KQueueScanContinuePatch
//
//  Created by Wowfunhappy with assistance from krackers.
//

#include "injectCheckPre108.h"
#include "helperFn.c"

static void injectInstructions() {
        // Add a bunch of nops so there is enough dead space in your func
#ifdef __LP64__
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        //IOLog("injectCheckPre108::%s: Success\n", __func__);
#ifdef DEBUG
        __builtin_trap();
#endif
        __asm__(".intel_syntax \t\n"
                "test       r12, r12"); //new
        __asm__(".intel_syntax \t\n"
                "mov        cl, byte ptr [rax+r15]"); //original
        __asm__("je0:");
        __asm__(".intel_syntax \t\n"
                "je         [je0 - 0x8000536a12]"); //original
        __asm__(".intel_syntax\t\n"
                "cmp      dword ptr [rbp-0x44], 0x0"); // new
        __asm__("je1:");
        __asm__(".intel_syntax \t\n"
                "je         [je1 - 0x8000536a12]"); //new, but simply a jump after check for the fp variable.
        
        //jump back if these checks are both false.
        __asm__("jmp0:");
        __asm__(".intel_syntax noprefix\t\n"
                "jmp        [0xffffff80005369f9]");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
#else
        // Add a bunch of nops so there is enough dead space in your func
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        
        __asm__(".intel_syntax \t\n"
                "test       ebx, ebx"); //new
        __asm__(".intel_syntax \t\n"
                "mov        cl, byte ptr ds:[eax+esi]"); //original
        __asm__("je0:");
        __asm__(".intel_syntax \t\n"
                "je         [je1relativeaddress]"); //original
        __asm__(".intel_syntax \t\n"
                "cmp        dword ptr [ebp-0x20], 0x0"); // new
        __asm__("je1:");
        __asm__(".intel_syntax \t\n"
                "je         [je1relativeaddress]"); //new, but simply a jump after check for the fp variable.
        
        //jump back if these checks are both false.
        __asm__("jmp0:");
        __asm__(".intel_syntax noprefix \t\n"
                "jmp        [jmp0relativeaddress]");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
#endif
        
}

static void computeRelativeAddresses() {
        //this isn't right, but just trying to get a feel of what in eed to do.
        //will need greater care selecting registers
        int64_t absAddr = 0;
#ifdef DEBUG
        asm ("movabs $je0, %0 \n\t"
             : "=r" (absAddr)
             );
        IOLog("injectCheckPre108::%s: je0 absolute: %llx\n", __func__, absAddr);
        absAddr = 0;
#endif
        
        asm ("movabs $je0, %0 \n\t"
             "sub %1, %0 \n\t"
             : "=r" (absAddr)
             : "r" (0xffffff8000536a12)
             );
        je0_rel = (int32_t) absAddr;
#ifdef DEBUG
        IOLog("injectCheckPre108::%s: je0 absolute: %llx, 32bit: %x\n", __func__, absAddr, je0_rel);
#endif
        absAddr = 0;
        asm ("movabs $je1, %0 \n\t"
             "sub %1, %0 \n\t"
             : "=r" (absAddr)
             : "r" (0xffffff8000536a12)
             );
        je1_rel = (int32_t) absAddr;
#ifdef DEBUG
        IOLog("injectCheckPre108::%s: je1 absolute: %llx, 32bit: %x\n", __func__, absAddr, je1_rel);
#endif
        
        absAddr = 0;
        asm ("movabs $jmp0, %0 \n\t"
             "sub %1, %0 \n\t"
             : "=r" (absAddr)
             : "r" (0xffffff80005369f9)
             );
        jmp0_rel = (int32_t) absAddr;
#ifdef DEBUG
        IOLog("injectCheckPre108::%s: jmp0 absolute: %llx, 32bit: %x\n", __func__, absAddr, jmp0_rel);
#endif
        
}
kern_return_t injectCheckPre108_start(kmod_info_t * ki, void *d)
{
        IOLog("injectCheckPre108::%s: START\n", __func__);
        kernel_base = get_kernel_base();
        char search_bytes[sizeof(possible_search_bytes[0])];
        
        
        for (int i = 0; i < LENGTH(possible_kqueue_scan_continue_panic_start_locations); i++) {
                kqueue_scan_continue_panic_start_location = kernel_base + possible_kqueue_scan_continue_panic_start_locations[i];
                kqueue_scan_continue_panic_end_location = kernel_base + possible_kqueue_scan_continue_panic_end_locations[i];
                memcpy(search_bytes, possible_search_bytes[i], sizeof(search_bytes));
                //memcpy(replacement_bytes, possible_replacement_bytes[i], sizeof(replacement_bytes));
                
                kscpb = (uint8_t*) kqueue_scan_continue_panic_start_location;
                originAddress = kqueue_scan_continue_panic_start_location;
                if (memcmp(kscpb, search_bytes, sizeof(search_bytes)) == 0) {
                        break;
                }
                if (i == LENGTH(possible_kqueue_scan_continue_panic_start_locations) - 1) {
                        IOLog("injectCheckPre108::%s: Memory region not found. You are probably using an unsupported kernel, or your kernel has already been patched.\n", __func__);
                        return KERN_FAILURE;
                }
        }
        
        IOLog("injectCheckPre108::%s: Pre-Patch: Bytes at kqueue_scan_continue panic location: ", __func__);
        for (int k=0; k < 39; k ++)
                IOLog(" %02x", kscpb[k]);
        IOLog(" %02x\n", kscpb[39]);
        
        unsigned long extra_space_to_fill = kqueue_scan_continue_panic_end_location - kqueue_scan_continue_panic_start_location - sizeof(replacement_bytes);
        
        if (kqueue_scan_continue_panic_start_location + sizeof(replacement_bytes) + extra_space_to_fill != kqueue_scan_continue_panic_end_location) {
                IOLog("injectCheckPre108::%s: kqueue_scan_continue_panic_start_location + sizeof(replacement_bytes) + extra_space_to_fill != kqueue_scan_continue_panic_end_location\n", __func__);
                return KERN_FAILURE;
        }
        
        interrupt_status = ml_get_interrupts_enabled();
        write_protection_status = write_protection_is_enabled();
        
        if(disableInterruptsAndProtection(interrupt_status, write_protection_status) == KERN_FAILURE)
                return KERN_FAILURE;
        
        long long funcAddr = (long long) &injectInstructions;
        
        uint8_t *matchOpCodeBytes = (uint8_t*) &injectInstructions;
        long long byteCount = 0;
        
#ifdef DEBUG
        /*The DEBUG macro inserts an invalid opcode trap
         * to verify we have jumped to the right function
         */
        while (true) {
                
                IOLog("injectCheckPre108::%s:: %llx %02x\n", __func__, funcAddr + byteCount, *matchOpCodeBytes);
                if (*matchOpCodeBytes == 0x0F && matchOpCodeBytes[1] == 0x0B) // 2 nops in a row, we're probably after the prologue
                        break;
                matchOpCodeBytes += 1;
                byteCount += 1;
        }
        IOLog("injectCheckPre108::%s: Trap at %llx\n", __func__, (long long) funcAddr + byteCount);
#endif

#ifdef DEBUG
        // this is to print the raw opcodes (probably way way past) of the trampoline
        // it allows you to verify the opcodes at the memory address are going to
        // agree with the label addresses from the computeRelativeAddresses section
        for (int k=0; k<128;k++) {
                IOLog("injectCheckPre108::%s:: %llx %02x\n", __func__, funcAddr + byteCount, matchOpCodeBytes[k]);
                byteCount += 1;
        }
#endif
        //the point of this function is to calculate the relative address from the memory locations given by the labels je0, je1, jmp0, which the compiler will insert through the clever stackexchange post found by @krackers
        //at this point we should be confident the relative addresses are correct, but i could be wrong.
        computeRelativeAddresses();
        
        //commence memory rewriting
        IOLog("injectCheckPre108::%s: Jumping to Dummy function\n", __func__);
        matchOpCodeBytes = (uint8_t*)  &injectInstructions;
        byteCount = 0;
        while (true) {
#ifdef DEBUG
                IOLog("injectCheckPre108::%s:: %llx %02x\n", __func__, funcAddr + byteCount, *matchOpCodeBytes);
#endif
                if (*matchOpCodeBytes == (unsigned char) 0x90 && matchOpCodeBytes[1] == (unsigned char) 0x90) // 2 nops in a row, we're probably after the prologue
                        break;
                matchOpCodeBytes += 1;
                byteCount += 1;
        }
        
        IOLog("injectCheckPre108::%s: funcAddr: %llx, real start %llx\n", __func__, funcAddr, funcAddr + byteCount);
        IOLog("injectCheckPre108::%s: current %llx\n", __func__, originAddress);
        uint32_t pcDelta = (funcAddr + byteCount) - originAddress;
        IOLog("injectCheckPre108::%s: Offset is %x, full %llx\n", __func__, pcDelta, (funcAddr + byteCount) - originAddress);
        
        
        
        /* presumably have to subtract 5 bytes to save/offset something in the counter,
         * since https://defuse.ca/online-x86-assembler.htm decodes to an address that
         * seems to add 5 bytes to the address
         */
        pcDelta -= 5;
        //save original bytes first
        memcpy(&original_bytes[0], (void *)kqueue_scan_continue_panic_start_location, sizeof(original_bytes));
        
        //create new jmp asm instruction.
        memset(&replacement_bytes[0], 0x90, sizeof(replacement_bytes));
        memset(&replacement_bytes[0], 0xE9, 1);
        memcpy(&replacement_bytes[1], &pcDelta, sizeof(pcDelta));
        
        /*
         * effectively wiping out the 3 lines with this jump (since replacement_bytes is 10 bytes):
         *
         *  ffffff80005369ef         mov        cl, byte [ds:rax+r15]
         *  ffffff80005369f3         cmp        dword [ss:rbp+var_44], 0x0
         *  ffffff80005369f7         je         0xffffff8000536a12
         */
        // memcpy((void *)kqueue_scan_continue_panic_start_location, replacement_bytes, sizeof(replacement_bytes));
        
        //conclude memory rewriting
        enableInterruptsAndProtection(interrupt_status, write_protection_status);
        
        
        IOLog("injectCheckPre108::%s: Post-Patch: Bytes at kqueue_scan_continue panic location: ", __func__);
        for (int k=0; k < 39; k ++)
                IOLog(" %02x", kscpb[k]);
        IOLog(" %02x\n", kscpb[39]);
        
        return KERN_SUCCESS;
}

kern_return_t injectCheckPre108_stop(kmod_info_t *ki, void *d)
{
        //should write back the old shit.
        disableInterruptsAndProtection(interrupt_status, write_protection_status);
        memcpy((void *)kqueue_scan_continue_panic_start_location, &original_bytes, sizeof(original_bytes));
        enableInterruptsAndProtection(interrupt_status, write_protection_status);
        //        IOLog("injectCheckPre108::%s: STOP\n", __func__);
        //        IOLog("injectCheckPre108::%s: UNLOAD: Bytes at kqueue_scan_continue panic location: ", __func__);
        //        for (int k=0; k < 39; k ++)
        //                IOLog(" %02x", kscpb[k]);
        //        IOLog(" %02x\n", kscpb[39]);
        
        //__asm__("jmp         0xffffff8000536a12");
        return KERN_SUCCESS;
}
