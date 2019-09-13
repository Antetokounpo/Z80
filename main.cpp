#include "z80.hpp"
#include<iostream>


int main()
{
    Z80::Z80 cpu;

    cpu.load("2+8.8xp");
    while(true)
    {
        cpu.step();
    }

    return 0;
}