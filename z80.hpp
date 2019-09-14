#ifndef Z80_H
#define Z80_H

#include<cstdint>
#include<iostream>
#include<fstream>
#include<cstring>
#include<cmath>

#define LD(DST, SRC) DST = SRC
#define INC(O) O += 1
#define DEC(O) O -= 1
#define ADD(DST, SRC) DST += SRC

namespace Z80
{
    union Register
    {
        uint16_t p;   /* pair of registers */
        uint8_t r[2]; /* separate registers */
    };

    class Z80
    {
        public:
            Z80(){}
            ~Z80(){}
            virtual void step();
            virtual bool load(const char* filename); /* Loads ROM */
            void execute(uint8_t opcode);

        protected:
            /* Main registers */
            Register AF; uint8_t* A = &(AF.r[0]); uint8_t* F = &(AF.r[1]); /* Bit 	7 	6 	5 	4 	3 	2 	1 	0 */
            Register BC; uint8_t* B = &(AF.r[0]); uint8_t* C = &(AF.r[1]); /* Flag 	S 	Z 	F5 	H 	F3 	P/V N 	C */
            Register DE; uint8_t* D = &(AF.r[0]); uint8_t* E = &(AF.r[1]);
            Register HL; uint8_t* H = &(AF.r[0]); uint8_t* L = &(AF.r[1]);

            /* Alternate registers */
            Register AF_;
            Register BC_;
            Register DE_;
            Register HL_;

            /* Index registers */
            uint16_t ix; /* Index X */
            uint16_t iy; /* Index Y */
            uint16_t sp; /* Stack pointer */

            /* Other registers */
            uint8_t i; /* Interrupt vector */ 
            uint8_t r; /* Refresh counter */

            uint16_t pc = 0; /* Program counter */

            uint8_t memory[65536]; /* Random Access Memory */
            uint8_t* rom;          /* Read-Only Memory */

            void rlca();
            void rla();
            void rrca();
            void rra();
            void djnz(int value);
            void cpl();

            void set_flag(uint8_t flag, bool value);
            void set_CF(bool value);
            void set_OF(bool value);

            void swap(uint16_t* v1, uint16_t* v2);
    };

    bool Z80::load(const char* filename)
    {
        std::streampos size;
        char* buffer;

        std::ifstream file(filename, std::ios::binary|std::ios::ate);
        if(file.is_open()){
            size = file.tellg();
            buffer = new char[size];

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
            std::cout << std::hex << "opcode: " << (uint)opcode << std::endl;
        #endif

        switch (opcode)
        {
            case 0x00: /* nop */
                pc++; break;
            case 0x01: /* ld bc, ** */
                LD(BC.r[0], rom[pc+1]);
                LD(BC.r[1], rom[pc+2]);
                pc += 3; break;
            case 0x02: /* ld (bc), a */
                LD(memory[BC.p], *A);
                pc++; break;
            case 0x03: /* inc bc */
                INC(BC.p);
                pc++; break;
            case 0x04:
                INC(*B);
                pc++; break;
            case 0x05: /* dec b */
                DEC(BC.r[0]);
                pc++; break;
            case 0x06: /* ld b, * */
                LD(*B, rom[pc+1]);
                pc += 2; break;
            case 0x07: /* rlca */
                rlca();
                pc++; break;
            case 0x08: /* ex af, af' */
                swap(&(AF.p), &(AF_.p));
                pc++; break;
            case 0x09: /* add hl, bc */
                ADD(HL.p, BC.p);
                pc++; break;
            case 0x0A: /* ld a, (bc) */
                LD(*A, memory[BC.p]);
                pc++; break;
            case 0x0B: /* dec bc */
                DEC(BC.p);
                pc++; break;
            case 0x0C: /* inc c */
                INC(*C);
                pc++; break;
            case 0x0D: /* dec c */
                DEC(*C);
                pc++; break;
            case 0x0E: /* ld c, * */
                LD(*C, rom[pc+1]);
                pc += 2; break;
            case 0x0F: /* rrca */
                rrca();
                pc++; break;
            
            case 0x10: /* djnz */
                djnz((int)rom[pc+1]);
                break;
            case 0x11: /* ld de, ** */
                LD(DE.r[0], rom[pc+1]);
                LD(DE.r[1], rom[pc+2]);
                pc += 3; break;
            case 0x12: /* ld (de), a */
                LD(memory[DE.p], *A);
                pc++; break;
            case 0x13: /* inc de */
                INC(DE.p);
                pc++; break;
            case 0x14: /* inc d */
                INC(*D);
                pc++; break;
            case 0x15: /* dec d */
                DEC(*D);
                pc++; break;
            case 0x16: /* ld d, * */
                LD(*D, rom[pc+1]);
                pc += 2; break;
            case 0x17: /* rla */
                rla();
                pc++; break;
            case 0x18: /* jr * */
                pc += (int)rom[pc+1]; break;
            case 0x19: /* add hl, de */
                ADD(HL.p, DE.p);
                pc++; break;
            case 0x1A: /* ld a, (de) */
                LD(*A, memory[DE.p]);
                pc++; break;
            case 0x1B: /* dec de */
                DEC(DE.p);
                pc++; break;
            case 0x1C: /* inc e */
                INC(*E);
                pc++; break;
            case 0x1D: /* dec e */
                DEC(*E);
                pc++; break;
            case 0x1E: /* ld e, * */
                LD(*E, rom[pc+1]);
                pc += 2; break;
            case 0x1F: /* rra */
                rra();
                pc++; break;
            
            case 0x20: /* jr nz, * */
                if(!(*F & 0x40)) pc += (int)rom[pc+1];
                break;
            case 0x21: /* ld hl, ** */
                LD(HL.r[0], rom[pc+1]);
                LD(HL.r[1], rom[pc+2]);
                pc += 3; break;
            case 0x22: /* ld (**), hl */
                LD(memory[pc+1], HL.r[0]);
                LD(memory[pc+2], HL.r[1]);
                pc += 3; break;
            case 0x23: /* inc hl */
                INC(HL.p);
                pc++; break;
            case 0x24: /* inc h */
                INC(HL.r[0]);
                pc++; break;
            case 0x25: /* dec h */
                DEC(HL.r[0]);
                pc++; break;
            case 0x26: /* ld h, * */
                LD(HL.r[0], rom[pc+1]);
                pc += 2; break;
            case 0x27: /* daa */
                // TODO
                break;
            case 0x28: /* jr z, * */
                if(*F & 0x40) pc += (int)rom[pc+1];
                break;
            case 0x29: /* add hl, hl */
                ADD(HL.p, HL.p);
                pc++; break;
            case 0x2A: /* ld hl, (**) */
                LD(HL.r[0], memory[pc+1]);
                LD(HL.r[1], memory[pc+2]);
                pc += 3; break;
            case 0x2B: /* dec hl */
                DEC(HL.p);
                pc++; break;
            case 0x2C: /* inc l */
                INC(*L);
                pc++; break;
            case 0x2D: /* dec l */
                DEC(*L);
                pc++; break;
            case 0x2E: /* ld l, * */
                LD(*L, rom[pc+1]);
                pc += 2; break;
            case 0x2F: /* cpl */
                cpl();
                pc++; break;
            
            case 0x30: /* jr nc, * */
                if(!((*F & 0x1))) pc += (int)rom[pc+1];
                pc += 2; break;
            case 0x31: /* ld sp, ** */
                LD(sp, rom[pc+1] << 8 | rom[pc+2]);
                pc += 3; break;
            case 0x32: /* ld (**), a */
                LD(memory[rom[pc+1] << 8 | rom[pc+2]], *A);
                pc += 3; break;
            case 0x33:
                INC(sp);
                pc++; break;
            case 0x34:
                INC(memory[HL.p]);
                pc++; break;
            case 0x35:
                DEC(memory[HL.p]);
                pc++; break;
            case 0x36:
                LD(memory[HL.p], rom[pc+1]);
                pc += 2; break;
            case 0x37: /* scf */
                set_CF(true);
                pc++; break;
            case 0x38: /* jr c, * */
                if(*F & 0x1) pc += (int)rom[pc+1];
                pc += 2; break;
            case 0x39:
                ADD(HL.p, sp);
                pc++; break;
            case 0x3A:
                LD(*A, memory[rom[pc+1] << 8 | rom[pc+2]]);
                pc += 3; break;
            case 0x3B:
                DEC(sp);
                pc++; break;
            case 0x3C:
                INC(*A);
                pc++; break;
            case 0x3D:
                DEC(*A);
                pc++; break;
            case 0x3E:
                LD(*A, rom[pc+1]);
                pc += 2; break;
            case 0x3F: /* ccf */
                set_CF(!(*F & 0x1));
                pc++; break;
            
            case 0x40:
                LD(*B, *B);
                pc++; break;
            case 0x41:
                LD(*B, *C);
                pc++; break;
            case 0x42:
                LD(*B, *D);
                pc++; break;
            case 0x43:
                LD(*B, *E);
                pc++; break;
            case 0x44:
                LD(*B, *H);
                pc++; break;
            case 0x45:
                LD(*B, *L);
                pc++; break;
            case 0x46:
                LD(*B, memory[HL.p]);
                pc++; break;
            case 0x47:
                LD(*B, *A);
                pc++; break;
            case 0x48:
                LD(*C, *B);
                pc++; break;
            case 0x49:
                LD(*C, *C);
                pc++; break;
            case 0x4A:
                LD(*C, *D);
                pc++; break;
            case 0x4B:
                LD(*C, *E);
                pc++; break;
            case 0x4C:
                LD(*C, *H);
                pc++; break;
            case 0x4D:
                LD(*C, *L);
                pc++; break;
            case 0x4E:
                LD(*C, memory[HL.p]);
                pc++; break;
            case 0x4F:
                LD(*C, *A);
                pc++; break;
            
            case 0x50:
                LD(*D, *B);
                pc++; break;
            case 0x51:
                LD(*D, *C);
                pc++; break;
            case 0x52:
                LD(*D, *D);
                pc++; break;
            case 0x53:
                LD(*D, *E);
                pc++; break;
            case 0x54:
                LD(*D, *H);
                pc++; break;
            case 0x55:
                LD(*D, *L);
                pc++; break;
            case 0x56:
                LD(*D, memory[HL.p]);
                pc++; break;
            case 0x57:
                LD(*D, *A);
                pc++; break;
            case 0x58:
                LD(*E, *B);
                pc++; break;
            case 0x59:
                LD(*E, *C);
                pc++; break;
            case 0x5A:
                LD(*E, *D);
                pc++; break;
            case 0x5B:
                LD(*E, *E);
                pc++; break;
            case 0x5C:
                LD(*E, *H);
                pc++; break;
            case 0x5D:
                LD(*E, *L);
                pc++; break;
            case 0x5E:
                LD(*E, memory[HL.p]);
                pc++; break;
            case 0x5F:
                LD(*E, *A);
                pc++; break;

            case 0x60:
                LD(*H, *B);
                pc++; break;
            case 0x61:
                LD(*H, *C);
                pc++; break;
            case 0x62:
                LD(*H, *D);
                pc++; break;
            case 0x63:
                LD(*H, *E);
                pc++; break;
            case 0x64:
                LD(*H, *H);
                pc++; break;
            case 0x65:
                LD(*H, *L);
                pc++; break;
            case 0x66:
                LD(*H, memory[HL.p]);
                pc++; break;
            case 0x67:
                LD(*H, *A);
                pc++; break;
            case 0x68:
                LD(*L, *B);
                pc++; break;
            case 0x69:
                LD(*L, *C);
                pc++; break;
            case 0x6A:
                LD(*L, *D);
                pc++; break;
            case 0x6B:
                LD(*L, *E);
                pc++; break;
            case 0x6C:
                LD(*L, *H);
                pc++; break;
            case 0x6D:
                LD(*L, *L);
                pc++; break;
            case 0x6E:
                LD(*L, memory[HL.p]);
                pc++; break;
            case 0x6F:
                LD(*L, *A);
                pc++; break;
                
            
            default:
                std::cout << std::hex << "Unrecognized instruction: " << (uint)opcode << std::endl;
                exit(EXIT_FAILURE); break;
        }
    }

    void Z80::step()
    {
        uint8_t opcode;
        opcode = rom[pc];
        execute(opcode);
    }

    void Z80::swap(uint16_t* v1, uint16_t* v2)
    {
        uint16_t temp = *v1;
        *v1 = *v2;
        *v2 = temp;
    }

    void Z80::rlca()
    {
        uint8_t msb = *A & 0x80;
        *A = (*A << 1) | (msb >> 7);
        set_CF(bool(msb >> 7));
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
        DEC(*B);
        if(*B != 0)
            pc += value;
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

    void Z80::set_flag(uint8_t flag, bool value)
    {
        *F &= 0x1 << flag ^ 0xFF;
        *F |= (uint)value;
    }

    void Z80::set_CF(bool value)
    {
        set_flag(0, value);
    }

    void Z80::set_OF(bool value)
    {
        set_flag(1, value);
    }
}

#endif
