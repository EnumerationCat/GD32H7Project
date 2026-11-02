#include "gd32h7xx.h"

void sdk_init()
{

}

void sdk_deinit()
{

}

// Newlib hooks.
void hardware_init_hook() {}

void software_init_hook()
{
    sdk_init();

    // OS.
}
