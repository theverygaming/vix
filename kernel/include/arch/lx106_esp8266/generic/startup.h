#pragma once

namespace arch::generic::startup {
    /*
     * this function is called after memory allocators finish intializing(but have not been used yet!) but constructors have not been called yet
     * as of September 2022 on x86 it is used to allocate some memory that is used for a ramfs before any memory allocators have been used
     */
    void stage2_startup();
    /*
     * this function does everything needed to start up applications. It initializes device drivers, enables multitasking etc.
     * When it is called all memory allocators must have finished initializing and constructors must have been called
     */
    void stage3_startup();
    /*
     * this function is called after the init process has been started up
     */
    void after_init();
}
