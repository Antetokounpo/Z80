#include<cstdint>
#include<cstring>
#include<iostream>
#include<fstream>
#include<thread>
#include<chrono>

#include "z80.hpp"

#define OUT(DST, SRC) ports[DST] = SRC
#define IN(DST, SRC) DST = ports[SRC]

namespace Z80
{
    Z80::Z80()
    {
        cpu_frequency = 4.8 * 1000000;
        refresh_rate = 60;
    }

    bool Z80::load(const char* filename)
    {
        std::streampos size;
        char* buffer;

        std::ifstream file(filename, std::ios::binary|std::ios::ate);
        if(file.is_open()){
            size = file.tellg();
            buffer = new char[size];
            rom_size = size;

            file.seekg(0, std::ios::beg);
            file.read(buffer, size);
            file.close();

            rom = new uint8_t[size];
            memcpy(rom, buffer, size);

            delete[] buffer;
            return true;
        }else{
            printf("No such file: %s\n", filename);
            return false;
        }
    }

    void Z80::execute(uint8_t opcode)
    {
        #ifdef DEBUG
	        std::cout << std::hex << "PC: " << (uint)pc << std::endl;
            std::cout << std::hex << "opcode: " << (uint)opcode << std::endl;
        #endif

        uint8_t* registers[] = {B, C, D, E, H, L, &(memory[HL.p]), A};

        uint8_t low_nibble = opcode & 0xF;

        switch (opcode)
        {
            case 0x00: /* nop */
                cycles += 4;
                pc++; break;
            case 0x01: /* ld bc, ** */
                cycles += 10;
                ld(BC.p, get_operand(2));
                pc += 3; break;
            case 0x02: /* ld (bc), a */
                cycles += 7;
                ld(get_memory(BC.p), *A);
                pc++; break;
            case 0x03: /* inc bc */
                cycles += 6;
                inc(BC.p);
                pc++; break;
            case 0x04: /* inc b */
                cycles += 4;
                inc(*B);
                pc++; break;
            case 0x05: /* dec b */
                cycles += 4;
                dec(*B);
                pc++; break;
            case 0x06: /* ld b, * */
                cycles += 7;
                ld(*B, get_operand(1));
                pc += 2; break;
            case 0x07: /* rlca */
                cycles += 4;
                rlca();
                pc++; break;
            case 0x08: /* ex af, af' */
                cycles += 4;
                std::swap(AF.p, AF_.p);
                pc++; break;
            case 0x09: /* add hl, bc */
                cycles += 11;
                add(HL.p, BC.p);
                pc++; break;
            case 0x0A: /* ld a, (bc) */
                cycles += 7;
                ld(*A, memory[BC.p]);
                pc++; break;
            case 0x0B: /* dec bc */
                cycles += 6;
                dec(BC.p);
                pc++; break;
            case 0x0C: /* inc c */
                cycles += 4;
                inc(*C);
                pc++; break;
            case 0x0D: /* dec c */
                cycles += 4;
                dec(*C);
                pc++; break;
            case 0x0E: /* ld c, * */
                cycles += 7;
                ld(*C, get_operand(1));
                pc += 2; break;
            case 0x0F: /* rrca */
                cycles += 4;
                rrca();
                pc++; break;
            
            case 0x10: /* djnz */
                djnz(static_cast<int8_t>(get_operand(1))); /* Cycle incrementation inside of function */
                break;
            case 0x11: /* ld de, ** */
                cycles += 10;
                ld(DE.p, get_operand(2));
                pc += 3; break;
            case 0x12: /* ld (de), a */
                cycles += 7;
                ld(memory[DE.p], *A);
                pc++; break;
            case 0x13: /* inc de */
                cycles += 6;
                inc(DE.p);
                pc++; break;
            case 0x14: /* inc d */
                cycles += 4;
                inc(*D);
                pc++; break;
            case 0x15: /* dec d */
                cycles += 4;
                dec(*D);
                pc++; break;
            case 0x16: /* ld d, * */
                cycles += 7;
                ld(*D, get_operand(1));
                pc += 2; break;
            case 0x17: /* rla */
                cycles += 4;
                rla();
                pc++; break;
            case 0x18: /* jr * */
                cycles += 12;
                pc += static_cast<int8_t>(get_operand(1))+2; break;
            case 0x19: /* add hl, de */
                cycles += 11;
                add(HL.p, DE.p);
                pc++; break;
            case 0x1A: /* ld a, (de) */
                cycles += 7;
                ld(*A, memory[DE.p]);
                pc++; break;
            case 0x1B: /* dec de */
                cycles += 6;
                dec(DE.p);
                pc++; break;
            case 0x1C: /* inc e */
                cycles += 4;
                inc(*E);
                pc++; break;
            case 0x1D: /* dec e */
                cycles += 4;
                dec(*E);
                pc++; break;
            case 0x1E: /* ld e, * */
                cycles += 7;
                ld(*E, get_operand(1));
                pc += 2; break;
            case 0x1F: /* rra */
                cycles += 4;
                rra();
                pc++; break;
            
            case 0x20: /* jr nz, * */
                if(!get_flag(6)) {cycles += 12; pc += static_cast<int8_t>(get_operand(1));}
                else cycles += 7;
                pc += 2; break;
            case 0x21: /* ld hl, ** */
                cycles += 10;
                ld(HL.p, get_operand(2));
                pc += 3; break;
            case 0x22: /* ld (**), hl */
                cycles += 16;
                ld(memory[get_operand(2)], HL.r[1]);
                ld(memory[get_operand(2)+1], HL.r[0]);
                pc += 3; break;
            case 0x23: /* inc hl */
                cycles += 6;
                inc(HL.p);
                pc++; break;
            case 0x24: /* inc h */
                cycles += 4;
                inc(HL.r[0]);
                pc++; break;
            case 0x25: /* dec h */
                cycles += 4;
                dec(HL.r[0]);
                pc++; break;
            case 0x26: /* ld h, * */
                cycles += 7;
                ld(*H, get_operand(1));
                pc += 2; break;
            case 0x27: /* daa */
                cycles += 4;
                daa();
                break;
            case 0x28: /* jr z, * */
                if(get_flag(6)) {cycles += 12; pc += static_cast<int8_t>(get_operand(1));}
                else cycles += 7;
                pc += 2; break;
            case 0x29: /* add hl, hl */
                cycles += 11;
                add(HL.p, HL.p);
                pc++; break;
            case 0x2A: /* ld hl, (**) */
                cycles += 16;
                ld(HL.r[1], memory[get_operand(1)]);
                ld(HL.r[0], memory[get_operand(1) + 1]);
                pc += 3; break;
            case 0x2B: /* dec hl */
                cycles += 6;
                dec(HL.p);
                pc++; break;
            case 0x2C: /* inc l */
                cycles += 4;
                inc(*L);
                pc++; break;
            case 0x2D: /* dec l */
                cycles += 4;
                dec(*L);
                pc++; break;
            case 0x2E: /* ld l, * */
                cycles += 7;
                ld(*L, get_operand(1));
                pc += 2; break;
            case 0x2F: /* cpl */
                cycles += 4;
                cpl();
                pc++; break;
            
            case 0x30: /* jr nc, * */
                if(!get_flag(0)) {cycles += 12; pc += static_cast<int8_t>(get_operand(1));} 
                else cycles += 7;
                pc += 2; break;
            case 0x31: /* ld sp, ** */
                cycles += 10;
                ld(sp, get_operand(2));
                pc += 3; break;
            case 0x32: /* ld (**), a */
                cycles += 13;
                ld(memory[get_operand(2)], *A);
                pc += 3; break;
            case 0x33:
                cycles += 6;
                inc(sp);
                pc++; break;
            case 0x34:
                cycles += 11;
                inc(memory[HL.p]);
                pc++; break;
            case 0x35:
                cycles += 11;
                dec(memory[HL.p]);
                pc++; break;
            case 0x36:
                cycles += 10;
                ld(memory[HL.p], get_operand(1));
                pc += 2; break;
            case 0x37: /* scf */
                cycles += 4;
                set_CF(true);
                pc++; break;
            case 0x38: /* jr c, * */
                if(get_flag(0)) {cycles += 12; pc += static_cast<int8_t>(get_operand(1));}
                else cycles += 7;
                pc += 2; break;
            case 0x39:
                cycles += 11;
                add(HL.p, sp);
                pc++; break;
            case 0x3A:
                cycles += 13;
                ld(*A, memory[get_operand(2)]);
                pc += 3; break;
            case 0x3B:
                cycles += 6;
                dec(sp);
                pc++; break;
            case 0x3C:
                cycles += 4;
                inc(*A);
                pc++; break;
            case 0x3D:
                cycles += 4;
                dec(*A);
                pc++; break;
            case 0x3E:
                cycles += 7;
                ld(*A, get_operand(1));
                pc += 2; break;
            case 0x3F: /* ccf */
                cycles += 4;
                set_CF(!(*F & 0x1));
                pc++; break;
            
            case 0x46:
                cycles += 3;
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x47:
                cycles += 4;
                ld(*B, *(registers[low_nibble]));
                pc++; break;
            case 0x4E:
                cycles += 3; 
            case 0x48:
            case 0x49:
            case 0x4A:
            case 0x4B:
            case 0x4C:
            case 0x4D:
            case 0x4F:
                cycles += 4;
                ld(*C, *(registers[low_nibble - 0x8]));
                pc++; break;
            
            case 0x56:
                cycles += 3;
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x57:
                cycles += 4;
                ld(*D, *(registers[low_nibble]));
                pc++; break;
            case 0x5E:
                cycles += 3; 
            case 0x58:
            case 0x59:
            case 0x5A:
            case 0x5B:
            case 0x5C:
            case 0x5D:
            case 0x5F:
                cycles += 4;
                ld(*E, *(registers[low_nibble - 0x8]));
                pc++; break;

            case 0x66:
                cycles += 3;
            case 0x60:
            case 0x61:
            case 0x62:
            case 0x63:
            case 0x64:
            case 0x65:
            case 0x67:
                cycles += 4;
                ld(*H, *(registers[low_nibble]));
                pc++; break;
            case 0x6E:
                cycles += 3;
            case 0x68:
            case 0x69:
            case 0x6A:
            case 0x6B:
            case 0x6C:
            case 0x6D:
            case 0x6F:
                cycles += 4;
                ld(*L, *(registers[low_nibble - 0x8]));
                pc++; break;
                
            case 0x70:
            case 0x71:
            case 0x72:
            case 0x73:
            case 0x74:
            case 0x75:
            case 0x77:
                cycles += 7;
                ld(memory[HL.p], *(registers[low_nibble]));
                pc++; break;
            case 0x76: /* halt */
                cycles += 4;
                pins[17] = true;
                break;
            case 0x7E:
                cycles += 3;
            case 0x78:
            case 0x79:
            case 0x7A:
            case 0x7B:
            case 0x7C:
            case 0x7D:
            case 0x7F:
                cycles += 4;
                ld(*A, *(registers[low_nibble - 0x8]));
                pc++; break;

            case 0x86:
                cycles += 3;
            case 0x80:
            case 0x81:
            case 0x82:
            case 0x83:
            case 0x84:
            case 0x85:
            case 0x87:
                cycles += 4;
                add(*A, *(registers[low_nibble]));
                pc++; break;
            case 0x8E:
                cycles += 3;
            case 0x88:
            case 0x89:
            case 0x8A:
            case 0x8B:
            case 0x8C:
            case 0x8D:
            case 0x8F:
                cycles += 4;
                adc(*A, *(registers[low_nibble - 0x8]));
                pc++; break;

            case 0x96:
                cycles += 3;
            case 0x90:
            case 0x91:
            case 0x92:
            case 0x93:
            case 0x94:
            case 0x95:
            case 0x97:
                cycles += 4;
                sub(*(registers[low_nibble]));
                pc++; break;
            case 0x9E:
                cycles += 3;
            case 0x98:
            case 0x99:
            case 0x9A:
            case 0x9B:
            case 0x9C:
            case 0x9D:
            case 0x9F:
                cycles += 4;
                sbc(*A, *(registers[low_nibble - 0x8]));
                pc++; break;

            case 0xA6:
                cycles += 3;
            case 0xA0:
            case 0xA1:
            case 0xA2:
            case 0xA3:
            case 0xA4:
            case 0xA5:
            case 0xA7:
                cycles += 4;
                bitwise_and(*(registers[low_nibble]));
                pc++; break;
            case 0xAE:
                cycles += 3;
            case 0xA8:
            case 0xA9:
            case 0xAA:
            case 0xAB:
            case 0xAC:
            case 0xAD:
            case 0xAF:
                cycles += 4;
                bitwise_xor(*(registers[low_nibble - 0x8]));
                pc++; break;

            case 0xB6:
                cycles += 3;
            case 0xB0:
            case 0xB1:
            case 0xB2:
            case 0xB3:
            case 0xB4:
            case 0xB5:
            case 0xB7:
                cycles += 4;
                bitwise_or(*(registers[low_nibble]));
                pc++; break;
            case 0xBE:
                cycles += 3;
            case 0xB8:
            case 0xB9:
            case 0xBA:
            case 0xBB:
            case 0xBC:
            case 0xBD:
            case 0xBF:
                cycles += 4;
                cp(*(registers[low_nibble - 0x8]));
                pc++; break;

            case 0xC0: /* ret nz */
                if(!(get_flag(6))) {cycles += 11; pop(pc);}
                else {cycles += 5; pc++;}
                break;
            case 0xC1:
                cycles += 10;
                pop(BC.p);
                pc++; break;
            case 0xC2: /* jp nz, ** */
                cycles += 10;
                if(!(get_flag(6)))
                {
                    pc = get_operand(2);
                }
                else
                {
                    pc += 3;
                }
                break;
            case 0xC3: /* jp ** */
                cycles += 10;
                pc = get_operand(2);
                break;
            case 0xC4: /* call nz, ** */
                if(!(get_flag(6)))
                {
                    cycles += 17;
                    push(pc+3);
                    pc = get_operand(2);
                }else
                {
                    cycles += 10;
                    pc += 3;
                }
                break;
            case 0xC5:
                cycles += 11;
                push(BC.p);
                pc++; break;
            case 0xC6:
                cycles += 7;
                add(*A, get_operand(1));
                pc += 2; break;
            case 0xC7:
                cycles += 11;
                push(pc+1);
                pc = 0x00; break;
            case 0xC8: /* ret z */
                if(get_flag(6)) {cycles += 11; pop(pc);}
                else {cycles += 5; pc++;}
                break;
            case 0xC9:
                cycles += 10;
                pop(pc);
                break;
            case 0xCA: /* jp z, ** */
                cycles += 10;
                if(get_flag(6))
                {
                    pc = get_operand(2);
                }
                else
                {
                    pc += 3;
                }
                break;
            case 0xCB:
                pc++;
                interpret_bits(fetch(0)); /* Cycle incrementation inside of function */
                pc++; break;
            case 0xCC: /* call z, ** */
                if(get_flag(6))
                {
                    cycles += 17;
                    push(pc+3);
                    pc = get_operand(2);
                }else
                {
                    cycles += 10;
                    pc += 3;
                }
                break;
            case 0xCD: /* call ** */
                cycles += 17;
                push(pc+3);
                pc = get_operand(2);
                break;
            case 0xCE:
                cycles += 7;
                add(*A, get_operand(1) + get_flag(0));
                pc += 2; break;
            case 0xCF:
                cycles += 11;
                push(pc+1);
                pc = 0x08; break;

            case 0xD0: /* ret nc */
                if(!(get_flag(0))) {cycles += 11; pop(pc);}
                else {cycles += 5; pc++;}
                break;
            case 0xD1:
                cycles += 10;
                pop(DE.p);
                pc++; break;
            case 0xD2: /* jp nc, ** */
                cycles += 10;
                if(!(get_flag(0)))
                {
                    pc = get_operand(2);
                }
                else
                {
                    pc += 3;
                }
                break;
            case 0xD3: /* out (*), a */
                cycles += 11;
                OUT(get_operand(1), *A);
                pc += 2;break;
            case 0xD4: /* call nc, ** */
                if(!(get_flag(0)))
                {
                    cycles += 17;
                    push(pc+3);
                    pc = get_operand(2);
                }else
                {
                    cycles += 10;
                    pc += 3;
                }
                break;
            case 0xD5:
                cycles += 11;
                push(DE.p);
                pc++; break;
            case 0xD6:
                cycles += 7;
                sub(get_operand(1));
                pc += 2; break;
            case 0xD7:
                cycles += 11;
                push(pc+1);
                pc = 0x10; break;
            case 0xD8: /* ret c */
                if(get_flag(0)) {cycles += 11; pop(pc);}
                else {cycles += 5; pc++;} 
                break;
            case 0xD9:
                cycles += 4;
                std::swap(BC.p, BC_.p);
                std::swap(DE.p, DE_.p);
                std::swap(HL.p, HL_.p);
                pc++; break;
            case 0xDA: /* jp c, * */
                cycles += 10;
                if(get_flag(0))
                {
                    pc = get_operand(2);
                }
                else
                {
                    pc += 3;
                }
                break;
            case 0xDB: /* in a, (*) */
                cycles += 11;
                IN(*A, get_operand(1));
                pc += 2; break;
            case 0xDC: /* call c, * */
                if(get_flag(0))
                {
                    cycles += 17;
                    push(pc+3);
                    pc = get_operand(2);
                }else
                {
                    cycles += 10;
                    pc += 3;
                }
                break;
            case 0xDD:
                cycles += 19;
                interpret_ix(rom[++pc]);
                break;
            case 0xDE:
                cycles += 7;
                sub(get_operand(1) + get_flag(0));
                pc += 2; break;
            case 0xDF:
                cycles += 11;
                push(pc+1);
                pc = 0x18; break;

            case 0xE0: /* ret po */
                if(!get_flag(2)) {cycles += 11; pop(pc);}
                else {cycles += 5; pc++;}
                break;
            case 0xE1:
                cycles += 10;
                pop(HL.p);
                pc++; break;
            case 0xE2: /* jp po, ** */
                cycles += 10;
                if(!get_flag(2))
                {
                    pc = get_operand(2);
                }
                else{
                    pc += 3;
                }
                break;
            case 0xE3: /* ex (sp), hl */
                cycles += 19;
                memory[sp] = HL.r[0];
                memory[sp+1] = HL.r[1];
                pc++; break;
            case 0xE4: /* call po ** */
                if(!get_flag(2))
                {
                    cycles += 17;
                    push(pc+3);
                    pc = get_operand(2);
                }else
                {
                    cycles += 10;
                    pc += 3;
                }
                break;
            case 0xE5:
                cycles += 11;
                push(HL.p);
                pc++; break;
            case 0xE6:
                cycles += 7;
                bitwise_and(get_operand(1));
                pc += 2; break;
            case 0xE7: /* rst 20h */
                cycles += 11;
                push(pc+1);
                pc = 0x20; break;
            case 0xE8: /* ret pe */
                if(get_flag(2)) {cycles += 11; pop(pc);}
                else {cycles += 5; pc++;} 
                break;
            case 0xE9: /* jp (hl) */
                cycles += 4;
                pc = memory[HL.p];
                break;
            case 0xEA: /* jp pe, ** */
                cycles += 10;
                if(get_flag(2))
                    pc = get_operand(2);
                else pc += 3;
                break;
            case 0xEB:
                cycles += 4;
                std::swap(DE.p, HL.p);
                pc++; break;
            case 0xEC:
                if(get_flag(2))
                {
                    cycles += 17;
                    push(pc+3);
                    pc = get_operand(2);
                }else
                {
                    cycles += 10;
                    pc += 3;
                }
                break;
            case 0xED:
                pc++;
                interpret_extd(get_operand(1));
                break;
            case 0xEE:
                cycles += 7;
                bitwise_xor(get_operand(1));
                pc += 2; break;
            case 0xEF:
                cycles += 11;
                push(pc+1);
                pc = 0x28; break;

            case 0xF0:
                if(!get_flag(7)) {cycles += 11; pop(pc);}
                else {cycles += 5; pc++;}
                break;
            case 0xF1:
                cycles += 10;
                pop(AF.p);
                pc++; break;
            case 0xF2:
                cycles += 10;
                if(!get_flag(7))
                    pc = get_operand(2);
                else
                    pc += 3;
                break;
            case 0xF3: /* di */
                cycles += 4;
                di();
                pc++; break;
            case 0xF4:
                if(!get_flag(7))
                {
                    cycles += 17;
                    push(pc+3);
                    pc = get_operand(2);
                }else
                {
                    cycles += 10;
                    pc += 3;
                }
                break;
            case 0xF5:
                cycles += 11;
                push(AF.p);
                pc++; break;
            case 0xF6:
                cycles += 7;
                bitwise_or(get_operand(1));
                pc += 2; break;
            case 0xF7:
                cycles += 11;
                push(pc+1);
                pc = 0x30; break;
            case 0xF8:
                if(get_flag(7)) {cycles += 11; pop(pc);}
                else {cycles += 5; pc++;}
                break;
            case 0xF9:
                cycles += 6;
                ld(sp, HL.p);
                pc++; break;
            case 0xFA:
                cycles += 10;
                if(get_flag(7))
                    pc = get_operand(2);
                else
                    pc += 3;
                break;
            case 0xFB: /* ei */
                cycles += 4;
                di();
                pc++; execute(fetch(0)); /* During the execution of this instruction and the following instruction, maskable interrupts are disabled. */
                ei();
                pc++; break;
            case 0xFC:
                if(get_flag(7))
                {
                    cycles += 17;
                    push(pc+3);
                    pc = get_operand(2);
                }else
                {
                    cycles += 10;
                    pc += 3;
                }
                break;
            case 0xFD:
                // TODO IY
                break;
            case 0xFE:
                cycles += 7;
                cp(get_operand(1));
                pc += 2; break;
            case 0xFF:
                cycles += 11;
                push(pc+1);
                pc = 0x38; break;

            default:
                std::cout << std::hex << "Unrecognized instruction: " << (uint)opcode << std::endl;
                exit(EXIT_FAILURE); break;
        }
    }

    void Z80::interpret_extd(uint8_t opcode)
    {
        switch(opcode)
        {
            case 0x40: /* in b, (c) */
                IN(*B, *C);
                pc++; break;
            case 0x41:
                OUT(*C, *B);
                pc++; break;
            case 0x42:
                sbc(HL.p, BC.p);
                pc++; break;
            case 0x43:
                ld(memory[get_operand(2)], BC.p);
                pc += 3; break;
            case 0x44:
                *A = twoscomp(*A);
                pc++; break;
            case 0x45:
                pop(pc);
                iff1 = iff2;
                break;
            case 0x46:
                interrupt_mode = 0;
                pc++; break;
            case 0x47:
                ld(i, *A);
                pc++; break;
            case 0x48:
                IN(*C, *C);
                pc++; break;
            case 0x49:
                OUT(*C, *C);
                pc++; break;
            case 0x4A:
                adc(HL.p, BC.p);
                pc++; break;
            case 0x4B:
                ld(BC.p, memory[get_operand(2)]);
                pc += 3; break;
            case 0x4D: /* reti */
                ei();
                pop(pc);
                // Signals I/O device TODO
                break;
            case 0x4F:
                ld(r, *A);
                pc++; break;

            case 0x50:
                IN(*D, *C);
                pc++; break;
            case 0x51:
                OUT(*C, *B);
                pc++; break;
            case 0x52:
                sbc(HL.p, DE.p);
                pc++; break;
            case 0x53:
                ld(memory[get_operand(2)], DE.p);
                pc += 3; break;
            case 0x55:
                pop(pc);
                iff1 = iff2;
                break;
            case 0x56:
                interrupt_mode = 1;
                pc++; break;
            case 0x57:
                ld(*A, i);
                pc++; break;
            case 0x58:
                IN(*E, *C);
                pc++; break;
            case 0x59:
                OUT(*C, *E);
                pc++; break;
            case 0x5A:
                adc(HL.p, DE.p);
                pc++; break;
            case 0x5B:
                ld(DE.p, memory[get_operand(2)]);
                pc += 3; break;
            case 0x5D:
                pop(pc);
                iff1 = iff2;
                break;
            case 0x5E:
                interrupt_mode = 2;
                pc++; break;
            case 0x5F:
                ld(*A, r);
                pc++; break;

            case 0x60:
                IN(*H, *C);
                pc++; break;
            case 0x61:
                OUT(*C, *H);
                pc++; break;
            case 0x62:
                sbc(HL.p, HL.p);
                pc++; break;
            case 0x65:
                pop(pc);
                iff1 = iff2;
                break;
            case 0x66:
                interrupt_mode = 0;
                pc++; break;
            case 0x67: /* rrd */
                rrd();
                pc++; break;
            case 0x68:
                IN(*L, *C);
                pc++; break;
            case 0x69:
                OUT(*C, *L);
                pc++; break;
            case 0x6A:
                adc(HL.p, HL.p);
                pc++; break;
            case 0x6D:
                pop(pc);
                iff1 = iff2;
                break;
            case 0x6F:
                rld();
                pc++; break;

            case 0x72:
                sbc(HL.p, sp);
                pc++; break;
            case 0x73:
                ld(memory[get_operand(2)], sp);
                pc += 3; break;
            case 0x75:
                pop(pc);
                iff1 = iff2;
                break;
            case 0x76:
                interrupt_mode = 1;
                pc++; break;
            case 0x78:
                IN(*A, *C);
                pc++; break;
            case 0x79:
                OUT(*C, *A);
                pc++; break;
            case 0x7A:
                adc(HL.p, sp);
                pc++; break;
            case 0x7B:
                ld(sp, memory[get_operand(2)]);
                pc += 3; break;
            case 0x7D:
                pop(pc);
                iff1 = iff2;
                break;
            case 0x7E:
                interrupt_mode = 2;
                pc++; break;

            case 0xA0:
                ldi();
                pc++; break;
            case 0xA1:
                cpi();
                pc++; break;
            case 0xA2:
                ini();
                pc++; break;
            case 0xA3:
                outi();
                pc++; break;
            case 0xA8:
                ldd();
                pc++; break;
            case 0xA9:
                cpd();
                pc++; break;
            case 0xAA:
                ind();
                pc++; break;
            case 0xAB:
                outd();
                pc++; break;

            case 0xB0:
                ldir();
                pc++; break;
            case 0xB1:
                cpir();
                pc++; break;
            case 0xB2:
                inir();
                pc++; break;
            case 0xB3:
                otir();
                pc++; break;
            case 0xB8:
                lddr();
                pc++; break;
            case 0xB9:
                cpdr();
                pc++; break;
            case 0xBA:
                indr();
                pc++; break;
            case 0xBB:
                otdr();
                pc++; break;
        }
    }

    void Z80::interpret_bits(uint8_t opcode)
    {
        uint8_t* registers[] = {B, C, D, E, H, L, &(memory[HL.p]), A};

        uint8_t high_nibble = opcode >> 4;
        uint8_t low_nibble = opcode & 0xF;

        cycles += (low_nibble == 0x6 || low_nibble == 0x6 + 0x8) ? 15 : 8;
        switch(high_nibble)
        {
            case 0x0:
                if (low_nibble < 0x8) rlc(registers[low_nibble]);
                else rrc(registers[low_nibble - 0x8]);
                break;
            case 0x1:
                if (low_nibble < 0x8) rl(registers[low_nibble]);
                else rr(registers[low_nibble - 0x8]);
                break;
            case 0x2:
                if (low_nibble < 0x8) sla(registers[low_nibble]);
                else sra(registers[low_nibble - 0x8]);
                break;
            case 0x3:
                srl(registers[low_nibble - 0x8]);
                break;
            case 0x4:
            case 0x5:
            case 0x6:
            case 0x7:
                if (low_nibble < 0x8) bit(high_nibble * 2 - 8, registers[low_nibble]);
                else bit(high_nibble * 2 - 7, registers[low_nibble - 0x8]);
                break;
            case 0x8:
            case 0x9:
            case 0xA:
            case 0xB:
                if (low_nibble < 0x8) res(high_nibble - 8, registers[low_nibble]);
                else res(high_nibble - 7, registers[low_nibble - 0x8]);
                break;
            case 0xC:
            case 0xD:
            case 0xE:
            case 0xF:
                if (low_nibble < 0x8) set(high_nibble * 2 - 24, registers[low_nibble]);
                else set(high_nibble * 2 - 23, registers[low_nibble - 0x8]);
                break;
        }
    }

    void Z80::interpret_ix(uint8_t opcode)
    {
        uint8_t* registers[] = {B, C, D, E, H, L};
        uint8_t low_nibble = opcode & 0xF;
        uint8_t* ixx = &(memory[ix+get_operand(1)]);

        switch(opcode)
        {
            case 0x09:
                add(ix, BC.p);
                pc++; break;
            case 0x19:
                add(ix, DE.p);
                pc++; break;
            case 0x21:
                ld(ix, get_operand(2));
                pc += 3; break;
            case 0x22:
                ld(memory[get_operand(2)], ix);
                pc += 3; break;
            case 0x23:
                inc(ix);
                pc++; break;
            case 0x29:
                add(ix, ix);
                pc++; break;
            case 0x2A:
               ld(ix, memory[get_operand(2)]);
               pc += 3; break;
            case 0x2B:
                dec(ix);
                pc++; break;
            case 0x34:
                inc(*ixx);
                pc += 2; break;
            case 0x35:
                dec(*ixx);
                pc += 2; break;
            case 0x36:
                ld(*ixx, get_operand(2) << 8);
                pc += 2; break;
            case 0x39:
                add(ix, sp);
                pc++; break;
            case 0x46:
                ld(*B, *ixx);
                pc += 2; break;
            case 0x4E:
                ld(*C, *ixx);
                pc += 2; break;
            case 0x56:
                ld(*D, *ixx);
                pc += 2; break;
            case 0x5E:
                ld(*E, *ixx);
                pc += 2; break;
            case 0x66:
                ld(*H, *ixx);
                pc += 2; break;
            case 0x6E:
                ld(*L, *ixx);
                pc += 2; break;
            case 0x70:
            case 0x71:
            case 0x72:
            case 0x73:
            case 0x74:
            case 0x75:
                ld(*ixx, *registers[low_nibble]);
                pc += 2; break;
            case 0x77:
                ld(*ixx, *A);
                pc += 2; break;
            case 0x7E:
                ld(*A, *ixx);
                pc += 2; break;
            case 0x86:
                add(*A, *ixx);
                pc += 2; break;
            case 0x8E:
                adc(*A, *ixx);
                pc += 2; break;
            case 0x96:
                sub(*ixx);
                pc += 2; break;
            case 0x9E:
                sbc(*A, *ixx);
                pc += 2; break;
            case 0xA6:
                bitwise_and(*ixx);
                pc += 2; break;
            case 0xAE:
                bitwise_xor(*ixx);
                pc += 2; break;
            case 0xB6:
                bitwise_or(*ixx);
                pc += 2; break;
            case 0xBE:
                cp(*ixx);
                pc += 2; break;
            case 0xDD:
                // TODO IX BITS
                break;
            case 0xE1:
                pop(ix);
                pc++; break;
            case 0xE3:
                std::swap(ixl, memory[sp]);
                std::swap(ixh, memory[sp+1]);
                pc++; break;
            case 0xE5:
                push(ix);
                pc++; break;
            case 0xE9:
                pc = memory[ix];
                break;
            case 0xF9:
                ld(sp, ix);
                pc++; break;
        }
    }

    uint8_t Z80::fetch(int offset)
    {
        return rom[pc+offset];
    }

    void Z80::step()
    {
        uint8_t opcode;
        std::chrono::steady_clock::time_point t1;
        std::chrono::duration<double> time_span;

        t1 = std::chrono::steady_clock::now();
        for(cycles = 0; cycles<cpu_frequency/refresh_rate;) /* Number of cycles for one frame */
        {
            opcode = fetch(0);
            execute(opcode);
        }
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - t1);
        std::this_thread::sleep_for(std::chrono::microseconds(1000000/refresh_rate)-time_span);
    }

    void Z80::interrupt()
    {
        if(pins[17])
            pc++;
    }

    void Z80::ei()
    {
        iff1 = true;
        iff2 = true;
    }

    void Z80::di()
    {
        iff1 = false;
        iff2 = false;
    }

    void Z80::sub(unsigned int src)
    {
        arithmetic_sub(*A, src);
    }

    void Z80::bitwise_and(unsigned int src)
    {
        unsigned int result = *A & src;

        set_CF(false);
        set_NF(false);
        set_POF(parity_check(result));
        set_HF(true);
        set_ZF((result & 0xFF) == 0);
        set_SF(result & 0x80);

        *A = result;
    }

    void Z80::bitwise_xor(unsigned int src)
    {
        unsigned int result = *A ^ src;

        set_CF(false);
        set_NF(false);
        set_POF(parity_check(result));
        set_HF(false);
        set_ZF((result & 0xFF) == 0);
        set_SF(result & 0x80);

        *A = result;
    }

    void Z80::bitwise_or(unsigned int src)
    {
        unsigned int result = *A | src;
        set_CF(false);
        set_NF(false);
        set_POF(parity_check(result));
        set_HF(false);
        set_ZF((result & 0xFF) == 0);
        set_SF(result & 0x80);

        *A = result;
    }

    void Z80::cp(unsigned int src)
    {
        unsigned int result = *A - src;
        unsigned int half_result = (*A & 0xF) - (src & 0xF);

        set_CF(result > 255);
        set_NF(true);
        set_POF(twoscomp(result) > 255);
        set_F3(0x1 << 3 & result);
        set_HF(half_result & 0x10);
        set_F5(0x1 << 5 & result);
        set_ZF((result & 0xFF) == 0);
        set_SF(result & 0x80);

    }

    bool Z80::parity_check(unsigned int bin)
    {
        unsigned int c = 0;
        for(unsigned int i = 0; i<sizeof(bin)*8; ++i)
        {
            c += bin << i & 0x1;
        }

        return !(c % 2);
    }

    uint16_t Z80::get_operand(int offset)
    {
        if (offset == 1)
        {
            return fetch(1);
        }
        return fetch(2) << 8 | fetch(1);
    }

    uint8_t& Z80::get_memory(uint16_t address)
    {
        return memory[address];
    }

    void Z80::rlca()
    {
        uint8_t msb = *A & 0x80;
        *A = (*A << 1) | (msb >> 7);
        set_CF(bool(msb >> 7));

        set_HF(false);
        set_NF(false);
        
    }

    void Z80::rla()
    {
        uint8_t carry_flag = *F & 0x01;
        rlca();
        *A &= 0xFE; /* reset bit 0 */
        *A |= carry_flag;
    }

    void Z80::rrca()
    {
        uint8_t lsb = 0x01 & *A;
        *A = (*A >> 1) | (lsb << 7);
        set_CF(bool(lsb));
    }

    void Z80::rra()
    {
        uint8_t carry_flag = *F & 0x01;
        rrca();
        *A &= 0x7F; /* reset bit 7 */
        *A |= carry_flag << 7;
    }

    void Z80::djnz(int value)
    {
        cycles += 8;
        dec(*B);
        if(*B != 0)
        {
            cycles += 5;
            pc += value;
        }
        else
            pc += 2;
    }

    void Z80::cpl()
    {
        for(int i = 0; i<8; ++i)
        {
            uint8_t b = 0x1 << i;
            if(*A & b)
                *A &= 0xFF - b; /* change 1 to 0 */
            else
                *A |= b; /* change 0 to 1 */
        }
    }

    void Z80::daa()
    {
        /* Code from x86 DAA operation */
        uint8_t old_A = *A;
        uint8_t old_CF = get_flag(0);
        set_CF(false);

        if((*A & 0xF) > 9 || get_flag(4) == 1)
        {
            add(*A, *A+6);
            set_CF(old_CF || get_flag(0));
            set_HF(true);
        }else
            set_HF(false);

        if(old_A > 0x99 || old_CF == 1)
        {
            add(*A, *A+0x60);
            set_CF(true);
        }else
            set_CF(false);
    }

    void Z80::rrd()
    {
        uint8_t low_nibble = *A & 0xF;

        *A = (*A & 0xF0) | (memory[HL.p] & 0x0F);
        memory[HL.p] = (memory[HL.p] >> 4) | (low_nibble << 4);

        set_SF(*A & 0x80);
        set_ZF(*A == 0);
        set_HF(false);
        set_POF(parity_check(*A));
        set_NF(false);
        /* Carry flag is not affected */
    }

    void Z80::rld()
    {
        uint8_t high_nibble = memory[HL.p] >> 4;
        uint8_t low_nibble = *A & 0x0F;

        memory[HL.p] = (memory[HL.p] & 0x0F) | ((memory[HL.p] & 0x0F) << 4);
        *A = (*A & 0xF0) | high_nibble;
        memory[HL.p] = (memory[HL.p] & 0xF0) | low_nibble;

        set_SF(*A & 0x80);
        set_ZF(*A == 0);
        set_HF(false);
        set_POF(parity_check(*A));
        set_NF(false);
        /* Carry flag is not affected */

    }

    void Z80::ldi()
    {
        ld(memory[DE.p], memory[HL.p]);
        DE.p++;
        HL.p++;
        BC.p--;

        set_HF(false);
        set_POF(BC.p - 1 != 0);
        set_NF(false);
    }

    void Z80::cpi()
    {
        unsigned int result = *A - memory[HL.p];
        unsigned int half_result = (*A&0x0F) - (memory[HL.p]&0x0F);


        set_SF(result & 0x80);
        set_ZF((result&0xFF) == 0);
        set_HF(half_result&0x10);
        set_POF(BC.p - 1 != 0);
        set_NF(true);

        BC.p--;
        HL.p++;
    }

    void Z80::ini()
    {
        memory[HL.p] = ports[*C];

        set_ZF(*B - 1 == 0);
        set_NF(true);

        (*B)--;
        HL.p++;
    }

    void Z80::outi()
    {
        ports[*C] = memory[HL.p];

        set_ZF(*B - 1 == 0);
        set_NF(true);

        (*B)--;
        HL.p++;
    }

    void Z80::ldd()
    {
        memory[DE.p] = memory[HL.p];

        set_HF(false);
        set_POF(BC.p - 1 != 0);
        set_NF(false);

        HL.p--;
        DE.p--;
        BC.p--; /* Byte counter */
    }

    void Z80::cpd()
    {
        unsigned int result = *A - memory[HL.p];
        unsigned int half_result = (*A&0x0F) - (HL.p&0x0F);

        set_SF(result & 0x80);
        set_ZF(result == 0);
        set_HF(half_result&0x10);
        set_POF(BC.p - 1 != 0);
        set_NF(true);

        HL.p--;
        BC.p--;
    }

    void Z80::ind()
    {
        memory[HL.p] = ports[*C];

        set_ZF(*B - 1 == 0);
        set_NF(true);

        (*B)++;
        HL.p--;
    }

    void Z80::outd()
    {
        ports[*C] = memory[HL.p];

        set_ZF(*B - 1 == 0);
        set_NF(true);

        (*B)--;
        HL.p--;
    }

    void Z80::ldir()
    {
        do
        {
            ldi();
        }while(BC.p != 0);
    }

    void Z80::cpir()
    {
        do
        {
            cpi();
        }while(BC.p != 0 || *A != memory[HL.p]);
    }

    void Z80::inir()
    {
        do
        {
            ini();
        } while(*B != 0);
    }

    void Z80::otir()
    {
        do
        {
            outd();
        } while(*B != 0);
    }

    void Z80::lddr()
    {
        do
        {
            ldd();
        }while(BC.p != 0);
        set_POF(false);
    }

    void Z80::cpdr()
    {
        do
        {
            cpd();
        } while(BC.p != 0 || *A != memory[HL.p]);
        /* La documentation n'est pas claire, on ne sait pas si c'est un or ou un and pour la condition */
    }

    void Z80::indr()
    {
        do
        {
            ind();
        } while(*B != 0);
    }

    void Z80::otdr()
    {
        do
        {
            outd();
        } while(*B != 0);
    }

    void Z80::rlc(uint8_t* m)
    {
        set_CF(*m & 0x80);
        rl(m);
    }

    void Z80::rrc(uint8_t* m)
    {
        set_CF(0x01 & *m);
        rr(m);
    }

    void Z80::rl(uint8_t* m)
    {
        uint8_t msb = *m & 0x80;
        *m = (*m << 1) | (get_flag(0));
        set_CF(msb);

        set_SF(*m & 0x80);
        set_ZF(*m == 0);
        set_HF(false);
        set_POF(parity_check(*m));
        set_NF(false);

    }

    void Z80::rr(uint8_t* m)
    {
        uint8_t lsb = 0x01 & *m;
        *m = (*m >> 1) | (get_flag(0) << 7);
        set_CF(lsb);

        set_SF(*m & 0x80);
        set_ZF(*m == 0);
        set_HF(false);
        set_POF(parity_check(*m));
        set_NF(false);

    }

    void Z80::sla(uint8_t* m)
    {
        set_CF(0);
        rl(m);
    }

    void Z80::sra(uint8_t* m)
    {
        set_CF(1);
        rr(m);
    }

    void Z80::srl(uint8_t* m)
    {
        set_CF(0);
        rr(m);
    }

    void Z80::bit(uint8_t b, uint8_t* m)
    {
        set_ZF(*m & (0x1 << *m));
        set_HF(true);
        set_NF(false);
    }

    void Z80::res(uint8_t b, uint8_t* m)
    {
        *m &= ~(0x1 << b);
    }

    void Z80::set(uint8_t b, uint8_t* m)
    {
        *m |= (0x1 << b);
    }

    void Z80::pop(uint16_t& dst)
    {
       dst = memory[sp] << 8 | memory[sp+1];
       sp += 2;
    }

    void Z80::push(uint16_t src)
    {
        memory[sp-1] = src >> 8;
        memory[sp-2] = src & 0xFF;
        sp -= 2;
    }

    void Z80::set_flag(uint8_t flag, bool value)
    {
        *F &= 0x1 << flag ^ 0xFF; /* reset le flag en question */
        *F |= value << flag;
    }

    void Z80::set_CF(bool value)
    {
        set_flag(0, value);
    }

    void Z80::set_NF(bool value)
    {
        set_flag(1, value);
    }

    void Z80::set_POF(bool value)
    {
        set_flag(2, value);
    }

    void Z80::set_F3(bool value)
    {
        set_flag(3, value);
    }

    void Z80::set_HF(bool value)
    {
        set_flag(4, value);
    }

    void Z80::set_F5(bool value)
    {
        set_flag(5, value);
    }

    void Z80::set_ZF(bool value)
    {
        set_flag(6, value);
    }

    void Z80::set_SF(bool value)
    {
        set_flag(7, value);
    }

    unsigned int Z80::get_flag(unsigned int flag)
    {
        return *F >> flag & 0x1;
    }
}

#undef IN
#undef OUT
