#include "z80.hpp"
#include<iostream>


int main()
{
    Z80::Z80 cpu;

    cpu.load("test.asm");
    while(true)
    {
        cpu.step();
    }

    return 0;
}
