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
#define ADD(DST, SRC) DST = add(DST, SRC, sizeof(DST))
#define SUB(SRC) sub(SRC);
#define AND(SRC) bitwise_and(SRC)
#define XOR(SRC) bitwise_xor(SRC)

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

            uint add(uint dst, uint src, size_t type);
            void sub(uint src);
            void bitwise_and(uint src);
            void bitwise_xor(uint src);

            void rlca();
            void rla();
            void rrca();
            void rra();
            void djnz(int value);
            void cpl();

            void set_flag(uint8_t flag, bool value);
            void set_CF(bool value);
            void set_NF(bool value);
            void set_POF(bool value);
            void set_F3(bool value);
            void set_HF(bool value);
            void set_F5(bool value);
            void set_ZF(bool value);
            void set_SF(bool value);

            uint get_flag(uint flag);
            void flag_affect(uint result, int8_t flags[]);

            void swap(uint16_t* v1, uint16_t* v2);
            uint8_t onescomp(uint8_t bin);
            uint twoscomp(uint8_t bin);
            bool parity_check(uint bin);
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
                LD(BC.p, rom[pc+1] << 8 | rom[pc+2]);
                pc += 3; break;
            case 0x02: /* ld (bc), a */
                LD(memory[BC.p], *A);
                pc++; break;
            case 0x03: /* inc bc */
                INC(BC.p);
                pc++; break;
            case 0x04: /* inc b */
                INC(*B);
                pc++; break;
            case 0x05: /* dec b */
                DEC(*B);
                set_ZF(*B == 0);
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
                pc++; break;
            case 0x6F:
                LD(*L, *A);
                pc++; break;
                
            case 0x70:
                LD(memory[HL.p], *B);
                pc++; break;
            case 0x71:
                LD(memory[HL.p], *C);
                pc++; break;
            case 0x72:
                LD(memory[HL.p], *D);
                pc++; break;
            case 0x73:
                LD(memory[HL.p], *E);
                pc++; break;
            case 0x74:
                LD(memory[HL.p], *H);
                pc++; break;
            case 0x75:
                LD(memory[HL.p], *L);
                pc++; break;
            case 0x76:
                // TODO
                pc++; break;
            case 0x77:
                LD(memory[HL.p], *A);
                pc++; break;
            case 0x78:
                LD(*A, *B);
                pc++; break;
            case 0x79:
                LD(*A, *C);
                pc++; break;
            case 0x7A:
                LD(*A, *D);
                pc++; break;
            case 0x7B:
                LD(*A, *E);
                pc++; break;
            case 0x7C:
                LD(*A, *H);
                pc++; break;
            case 0x7D:
                LD(*A, *L);
                pc++; break;
            case 0x7E:
                LD(*A, memory[HL.p]);
                pc++; break;
            case 0x7F:
                LD(*A, *A);
                pc++; break;

            case 0x80:
                ADD(*A, *B);
                pc++; break;
            case 0x81:
                ADD(*A, *C);
                pc++; break;
            case 0x82:
                ADD(*A, *D);
                pc++; break;
            case 0x83:
                ADD(*A, *E);
                pc++; break;
            case 0x84:
                ADD(*A, *H);
                pc++; break;
            case 0x85:
                ADD(*A, *L);
                pc++; break;
            case 0x86:
                ADD(*A, memory[HL.p]);
                pc++; break;
            case 0x87:
                ADD(*A, *A);
                pc++; break;
            case 0x88:
                ADD(*A, *B + get_flag(0));
                pc++; break;
            case 0x89:
                ADD(*A, *C + get_flag(0));
                pc++; break;
            case 0x8A:
                ADD(*A, *D + get_flag(0));
                pc++; break;
            case 0x8B:
                ADD(*A, *E + get_flag(0));
                pc++; break;
            case 0x8C:
                ADD(*A, *H + get_flag(0));
                pc++; break;
            case 0x8D:
                ADD(*A, *L + get_flag(0));
                pc++; break;
            case 0x8E:
                ADD(*A, memory[HL.p] + get_flag(0));
                pc++; break;
            case 0x8F:
                ADD(*A, *A + get_flag(0));
                pc++; break;

            case 0x90:
                SUB(*B);
                pc++; break;
            case 0x91:
                SUB(*C);
                pc++; break;
            case 0x92:
                SUB(*D);
                pc++; break;
            case 0x93:
                SUB(*E);
                pc++; break;
            case 0x94:
                SUB(*H);
                pc++; break;
            case 0x95:
                SUB(*L);
                pc++; break;
            case 0x96:
                SUB(memory[HL.p]);
                pc++; break;
            case 0x97:
                SUB(*A);
                pc++; break;
            case 0x98:
                SUB(*B + get_flag(0));
                pc++; break;
            case 0x99:
                SUB(*C + get_flag(0));
                pc++; break;
            case 0x9A:
                SUB(*D + get_flag(0));
                pc++; break;
            case 0x9B:
                SUB(*E + get_flag(0));
                pc++; break;
            case 0x9C:
                SUB(*H + get_flag(0));
                pc++; break;
            case 0x9D:
                SUB(*L + get_flag(0));
                pc++; break;
            case 0x9E:
                SUB(memory[HL.p] + get_flag(0));
                pc++; break;
            case 0x9F:
                SUB(*A + get_flag(0));
                pc++; break;

            case 0xA0:
                AND(*B);
                pc++; break;
            case 0xA1:
                AND(*C);
                pc++; break;
            case 0xA2:
                AND(*D);
                pc++; break;
            case 0xA3:
                AND(*E);
                pc++; break;
            case 0xA4:
                AND(*H);
                pc++; break;
            case 0xA5:
                AND(*L);
                pc++; break;
            case 0xA6:
                AND(memory[HL.p]);
                pc++; break;
            case 0xA7:
                AND(*A);
                pc++; break;
            case 0xA8:
                XOR(*B);
                pc++; break;
            case 0xA9:
                XOR(*C);
                pc++; break;
            case 0xAA:
                XOR(*D);
                pc++; break;
            case 0xAB:
                XOR(*E);
                pc++; break;
            case 0xAC:
                XOR(*H);
                pc++; break;
            case 0xAD:
                XOR(*L);
                pc++; break;
            case 0xAE:
                XOR(memory[HL.p]);
                pc++; break;
            case 0xAF:
                XOR(*A);
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

    uint Z80::add(uint dst, uint src, size_t type)
    {
        if(type == 2) /* 16 bit operations don't affect flags */
            return dst + src;

        uint half_result = (dst&0x0F) + (src&0x0F);
        uint result = dst + src;

        set_CF(result > 255);
        set_NF(false);
        set_POF(twoscomp(result) > 255);
        set_F3(bool(0x1 << 3 & result));
        set_HF(bool(half_result & 0x10));
        set_F5(bool(0x1 << 5 & result));
        set_ZF(result & 0xFF == 0);
        set_SF(bool(twoscomp(result) & 0x80));

        return result;
    }

    void Z80::sub(uint src)
    {
        uint result = *A - src;
        uint half_result = (*A & 0x0F) + (src & 0x0F);

        set_CF(result > 255);
        set_NF(true);
        set_POF(twoscomp(result) > 255);
        set_F3(bool(0x1 << 3 & result));
        set_HF(bool(half_result & 0x10));
        set_F5(bool(0x1 << 5 & result));
        set_ZF(result & 0xFF == 0);
        set_SF(bool(twoscomp(result) & 0x80));

        *A = result;
    }

    void Z80::bitwise_and(uint src)
    {
        uint result = *A & src;

        set_CF(false);
        set_NF(false);
        set_POF(parity_check(result));
        set_HF(true);
        set_ZF(result == 0);
        set_SF(bool(twoscomp(result) & 0x80));

        *A = result;
    }

    void Z80::bitwise_xor(uint src)
    {
        uint result = *A ^ src;

        set_CF(false);
        set_NF(false);
        set_POF(parity_check(result));
        set_HF(false);
        set_ZF(result == 0);
        set_SF(bool(twoscomp(result) & 0x80));

        *A = result;
    }

    void Z80::swap(uint16_t* v1, uint16_t* v2)
    {
        uint16_t temp = *v1;
        *v1 = *v2;
        *v2 = temp;
    }

    uint8_t Z80::onescomp(uint8_t bin)
    {
        for(int i = 0; i<8; ++i)
        {
            uint8_t b = 0x1 << i;
            if(bin & b)
                bin &= 0xFF - b; /* change 1 to 0 */
            else
                bin |= b; /* change 0 to 1 */
        }

        return bin;
    }

    uint Z80::twoscomp(uint8_t bin)
    {
        return onescomp(bin)+1;
    }

    bool Z80::parity_check(uint bin)
    {
        uint c = 0;
        for(int i = 0; i<sizeof(bin)*8; ++i)
        {
            c += bin << i & 0x1;
        }

        return !(c % 2);
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

    uint Z80::get_flag(uint flag)
    {
        return *F << flag & 0x1;
    }
}

#endif
