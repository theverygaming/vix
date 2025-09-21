#pragma once

namespace arch::startup {
    /*
     * This function is called after the PMM has finished initializing(but hasn't been used yet!)
     */
    void stage2_startup();
    /*
     * This function is called after memory allocators finish initializing(but have not been used yet!) but constructors have not been called yet
     * as of September 2022 on x86 it is used to allocate some memory that is used for a ramfs before any memory allocators have been used
     */
    void stage3_startup();
    /*
     * This function does everything needed to start up applications. It initializes device drivers, enables multitasking etc.
     * When it is called all memory allocators must have finished initializing and constructors must have been called
     */
    void stage4_startup();
    /*
     * This function is called from inside the first kernel thread started
     */
    void kthread0();
}
