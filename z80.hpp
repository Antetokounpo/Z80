#ifndef Z80_H
#define Z80_H

#include<cstdint>
#include<iostream>
#include<fstream>
#include<cstring>
#include<cmath>
#include<algorithm>

#define DEC(O) O = sub(O, 1, sizeof(O))
#define SUB(SRC) sub(*A, SRC, 1);
#define OUT(DST, SRC) ports[DST] = SRC
#define IN(DST, SRC) DST = ports[SRC]
#define SBC(DST, SRC) sub(DST, SRC + get_flag(0), sizeof(DST))

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

            void interrupt();

        protected:
            /* Main registers */
            Register AF; uint8_t* A = &(AF.r[0]); uint8_t* F = &(AF.r[1]); /* Bit 	7 	6 	5 	4 	3 	2 	1 	0 */
            Register BC; uint8_t* B = &(BC.r[0]); uint8_t* C = &(BC.r[1]); /* Flag 	S 	Z 	F5 	H 	F3 	P/V N 	C */
            Register DE; uint8_t* D = &(DE.r[0]); uint8_t* E = &(DE.r[1]);
            Register HL; uint8_t* H = &(HL.r[0]); uint8_t* L = &(HL.r[1]);

            /* Alternate registers */
            Register AF_;
            Register BC_;
            Register DE_;
            Register HL_;

            /* Index registers */
            Register INDEX_X = {0}; uint8_t ixl = INDEX_X.r[0]; uint8_t ixh = INDEX_X.r[0]; uint16_t ix = INDEX_X.p; /* Index X */
            uint16_t iy; /* Index Y */
            uint16_t sp; /* Stack pointer */

            /* Other registers */
            uint8_t i; /* Interrupt vector */ 
            uint8_t r; /* Refresh counter */

            uint16_t pc = 0; /* Program counter */

            uint8_t memory[65536]; /* Random Access Memory */
            uint8_t* rom;          /* Read-Only Memory */
            uint rom_size;         /* Size of the ROM file */
            uint8_t ports[256];    /* I/O ports */

            bool pins[40]; /* I/O pins */

            /* Interrupt flip-flops */
            bool iff1 = false;
            bool iff2 = false;
            uint interrupt_mode = 0;

            template <class T, class U> void ld(T& dst, U src);
            template <class T, class U> void add(T& dst, U src);
            template <class T> void inc(T& dst);
            template <class T, class U> void adc(T& dst, U src);
            uint sub(uint dst, uint src, size_t type);
            void bitwise_and(uint src);
            void bitwise_xor(uint src);
            void bitwise_or(uint src);
            void cp(uint src);

            void rlca();
            void rla();
            void rrca();
            void rra();
            void djnz(int value);
            void cpl();
            void daa();
            void rrd();
            void rld();
            void ldi();
            void cpi();
            void ini();
            void outi();
            void ldd();
            void cpd();
            void ind();
            void outd();
            void ldir();
            void cpir();
            void inir();
            void otir();
            void lddr();
            void cpdr();
            void indr();
            void otdr();

            void rlc(uint8_t* m);
            void rrc(uint8_t* m);
            void rl(uint8_t* m);
            void rr(uint8_t* m);
            void sla(uint8_t* m);
            void sra(uint8_t* m);
            void srl(uint8_t* m);

            void bit(uint8_t b, uint8_t* m);
            void res(uint8_t b, uint8_t* m);
            void set(uint8_t b, uint8_t *m);

            void pop(uint16_t& dst);
            void push(uint16_t src);

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

            uint8_t onescomp(uint8_t bin);
            uint twoscomp(uint8_t bin);
            bool parity_check(uint bin);
            uint16_t get_operand(uint offset);

            void interpret_extd(uint8_t opcode);
            void interpret_bits(uint8_t opcode);
            void interpret_ix(uint8_t opcode);
    };

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
            getchar();
        #endif

        switch (opcode)
        {
            case 0x00: /* nop */
                pc++; break;
            case 0x01: /* ld bc, ** */
                ld(BC.p, get_operand(2));
                pc += 3; break;
            case 0x02: /* ld (bc), a */
                ld(memory[BC.p], *A);
                pc++; break;
            case 0x03: /* inc bc */
                inc(BC.p);
                pc++; break;
            case 0x04: /* inc b */
                inc(*B);
                pc++; break;
            case 0x05: /* dec b */
                DEC(*B);
                set_ZF(*B == 0);
                pc++; break;
            case 0x06: /* ld b, * */
                ld(*B, get_operand(1));
                pc += 2; break;
            case 0x07: /* rlca */
                rlca();
                pc++; break;
            case 0x08: /* ex af, af' */
                std::swap(AF.p, AF_.p);
                pc++; break;
            case 0x09: /* add hl, bc */
                add(HL.p, BC.p);
                pc++; break;
            case 0x0A: /* ld a, (bc) */
                ld(*A, memory[BC.p]);
                pc++; break;
            case 0x0B: /* dec bc */
                DEC(BC.p);
                pc++; break;
            case 0x0C: /* inc c */
                inc(*C);
                pc++; break;
            case 0x0D: /* dec c */
                DEC(*C);
                pc++; break;
            case 0x0E: /* ld c, * */
                ld(*C, get_operand(1));
                pc += 2; break;
            case 0x0F: /* rrca */
                rrca();
                pc++; break;
            
            case 0x10: /* djnz */
                djnz((int)get_operand(1));
                break;
            case 0x11: /* ld de, ** */
                ld(DE.p, get_operand(2));
                pc += 3; break;
            case 0x12: /* ld (de), a */
                ld(memory[DE.p], *A);
                pc++; break;
            case 0x13: /* inc de */
                inc(DE.p);
                pc++; break;
            case 0x14: /* inc d */
                inc(*D);
                pc++; break;
            case 0x15: /* dec d */
                DEC(*D);
                pc++; break;
            case 0x16: /* ld d, * */
                ld(*D, get_operand(1));
                pc += 2; break;
            case 0x17: /* rla */
                rla();
                pc++; break;
            case 0x18: /* jr * */
                pc += (int8_t)get_operand(1); break;
            case 0x19: /* add hl, de */
                add(HL.p, DE.p);
                pc++; break;
            case 0x1A: /* ld a, (de) */
                ld(*A, memory[DE.p]);
                pc++; break;
            case 0x1B: /* dec de */
                DEC(DE.p);
                pc++; break;
            case 0x1C: /* inc e */
                inc(*E);
                pc++; break;
            case 0x1D: /* dec e */
                DEC(*E);
                pc++; break;
            case 0x1E: /* ld e, * */
                ld(*E, get_operand(1));
                pc += 2; break;
            case 0x1F: /* rra */
                rra();
                pc++; break;
            
            case 0x20: /* jr nz, * */
                if(!(*F & 0x40)) pc += (int8_t)get_operand(1);
                else pc += 2;;
                break;
            case 0x21: /* ld hl, ** */
                ld(HL.p, get_operand(2));
                pc += 3; break;
            case 0x22: /* ld (**), hl */
                ld(memory[pc+1], HL.r[0]);
                ld(memory[pc+2], HL.r[1]);
                pc += 3; break;
            case 0x23: /* inc hl */
                inc(HL.p);
                pc++; break;
            case 0x24: /* inc h */
                inc(HL.r[0]);
                pc++; break;
            case 0x25: /* dec h */
                DEC(HL.r[0]);
                pc++; break;
            case 0x26: /* ld h, * */
                ld(HL.r[0], get_operand(1));
                pc += 2; break;
            case 0x27: /* daa */
                daa();
                break;
            case 0x28: /* jr z, * */
                if(*F & 0x40) pc += (int)get_operand(1);
                else pc += 2;
                break;
            case 0x29: /* add hl, hl */
                add(HL.p, HL.p);
                pc++; break;
            case 0x2A: /* ld hl, (**) */
                ld(HL.r[0], memory[pc+1]);
                ld(HL.r[1], memory[pc+2]);
                pc += 3; break;
            case 0x2B: /* dec hl */
                DEC(HL.p);
                pc++; break;
            case 0x2C: /* inc l */
                inc(*L);
                pc++; break;
            case 0x2D: /* dec l */
                DEC(*L);
                pc++; break;
            case 0x2E: /* ld l, * */
                ld(*L, get_operand(1));
                pc += 2; break;
            case 0x2F: /* cpl */
                cpl();
                pc++; break;
            
            case 0x30: /* jr nc, * */
                if(!((*F & 0x1))) pc += (int)get_operand(1);
                pc += 2; break;
            case 0x31: /* ld sp, ** */
                ld(sp, get_operand(2));
                pc += 3; break;
            case 0x32: /* ld (**), a */
                ld(memory[get_operand(2)], *A);
                pc += 3; break;
            case 0x33:
                inc(sp);
                pc++; break;
            case 0x34:
                inc(memory[HL.p]);
                pc++; break;
            case 0x35:
                DEC(memory[HL.p]);
                pc++; break;
            case 0x36:
                ld(memory[HL.p], get_operand(1));
                pc += 2; break;
            case 0x37: /* scf */
                set_CF(true);
                pc++; break;
            case 0x38: /* jr c, * */
                if(*F & 0x1) pc += (int)get_operand(1);
                pc += 2; break;
            case 0x39:
                add(HL.p, sp);
                pc++; break;
            case 0x3A:
                ld(*A, memory[get_operand(2)]);
                pc += 3; break;
            case 0x3B:
                DEC(sp);
                pc++; break;
            case 0x3C:
                inc(*A);
                pc++; break;
            case 0x3D:
                DEC(*A);
                pc++; break;
            case 0x3E:
                ld(*A, get_operand(1));
                pc += 2; break;
            case 0x3F: /* ccf */
                set_CF(!(*F & 0x1));
                pc++; break;
            
            case 0x40:
                ld(*B, *B);
                pc++; break;
            case 0x41:
                ld(*B, *C);
                pc++; break;
            case 0x42:
                ld(*B, *D);
                pc++; break;
            case 0x43:
                ld(*B, *E);
                pc++; break;
            case 0x44:
                ld(*B, *H);
                pc++; break;
            case 0x45:
                ld(*B, *L);
                pc++; break;
            case 0x46:
                ld(*B, memory[HL.p]);
                pc++; break;
            case 0x47:
                ld(*B, *A);
                pc++; break;
            case 0x48:
                ld(*C, *B);
                pc++; break;
            case 0x49:
                ld(*C, *C);
                pc++; break;
            case 0x4A:
                ld(*C, *D);
                pc++; break;
            case 0x4B:
                ld(*C, *E);
                pc++; break;
            case 0x4C:
                ld(*C, *H);
                pc++; break;
            case 0x4D:
                ld(*C, *L);
                pc++; break;
            case 0x4E:
                ld(*C, memory[HL.p]);
                pc++; break;
            case 0x4F:
                ld(*C, *A);
                pc++; break;
            
            case 0x50:
                ld(*D, *B);
                pc++; break;
            case 0x51:
                ld(*D, *C);
                pc++; break;
            case 0x52:
                ld(*D, *D);
                pc++; break;
            case 0x53:
                ld(*D, *E);
                pc++; break;
            case 0x54:
                ld(*D, *H);
                pc++; break;
            case 0x55:
                ld(*D, *L);
                pc++; break;
            case 0x56:
                ld(*D, memory[HL.p]);
                pc++; break;
            case 0x57:
                ld(*D, *A);
                pc++; break;
            case 0x58:
                ld(*E, *B);
                pc++; break;
            case 0x59:
                ld(*E, *C);
                pc++; break;
            case 0x5A:
                ld(*E, *D);
                pc++; break;
            case 0x5B:
                ld(*E, *E);
                pc++; break;
            case 0x5C:
                ld(*E, *H);
                pc++; break;
            case 0x5D:
                ld(*E, *L);
                pc++; break;
            case 0x5E:
                ld(*E, memory[HL.p]);
                pc++; break;
            case 0x5F:
                ld(*E, *A);
                pc++; break;

            case 0x60:
                ld(*H, *B);
                pc++; break;
            case 0x61:
                ld(*H, *C);
                pc++; break;
            case 0x62:
                ld(*H, *D);
                pc++; break;
            case 0x63:
                ld(*H, *E);
                pc++; break;
            case 0x64:
                ld(*H, *H);
                pc++; break;
            case 0x65:
                ld(*H, *L);
                pc++; break;
            case 0x66:
                ld(*H, memory[HL.p]);
                pc++; break;
            case 0x67:
                ld(*H, *A);
                pc++; break;
            case 0x68:
                ld(*L, *B);
                pc++; break;
            case 0x69:
                ld(*L, *C);
                pc++; break;
            case 0x6A:
                ld(*L, *D);
                pc++; break;
            case 0x6B:
                ld(*L, *E);
                pc++; break;
            case 0x6C:
                ld(*L, *H);
                pc++; break;
            case 0x6D:
                ld(*L, *L);
                pc++; break;
            case 0x6E:
                ld(*L, memory[HL.p]);
                pc++; break;
            case 0x6F:
                ld(*L, *A);
                pc++; break;
                
            case 0x70:
                ld(memory[HL.p], *B);
                pc++; break;
            case 0x71:
                ld(memory[HL.p], *C);
                pc++; break;
            case 0x72:
                ld(memory[HL.p], *D);
                pc++; break;
            case 0x73:
                ld(memory[HL.p], *E);
                pc++; break;
            case 0x74:
                ld(memory[HL.p], *H);
                pc++; break;
            case 0x75:
                ld(memory[HL.p], *L);
                pc++; break;
            case 0x76: /* halt */
                pins[17] = true;
                break;
            case 0x77:
                ld(memory[HL.p], *A);
                pc++; break;
            case 0x78:
                ld(*A, *B);
                pc++; break;
            case 0x79:
                ld(*A, *C);
                pc++; break;
            case 0x7A:
                ld(*A, *D);
                pc++; break;
            case 0x7B:
                ld(*A, *E);
                pc++; break;
            case 0x7C:
                ld(*A, *H);
                pc++; break;
            case 0x7D:
                ld(*A, *L);
                pc++; break;
            case 0x7E:
                ld(*A, memory[HL.p]);
                pc++; break;
            case 0x7F:
                ld(*A, *A);
                pc++; break;

            case 0x80:
                add(*A, *B);
                pc++; break;
            case 0x81:
                add(*A, *C);
                pc++; break;
            case 0x82:
                add(*A, *D);
                pc++; break;
            case 0x83:
                add(*A, *E);
                pc++; break;
            case 0x84:
                add(*A, *H);
                pc++; break;
            case 0x85:
                add(*A, *L);
                pc++; break;
            case 0x86:
                add(*A, memory[HL.p]);
                pc++; break;
            case 0x87:
                add(*A, *A);
                pc++; break;
            case 0x88:
                add(*A, *B + get_flag(0));
                pc++; break;
            case 0x89:
                add(*A, *C + get_flag(0));
                pc++; break;
            case 0x8A:
                add(*A, *D + get_flag(0));
                pc++; break;
            case 0x8B:
                add(*A, *E + get_flag(0));
                pc++; break;
            case 0x8C:
                add(*A, *H + get_flag(0));
                pc++; break;
            case 0x8D:
                add(*A, *L + get_flag(0));
                pc++; break;
            case 0x8E:
                add(*A, memory[HL.p] + get_flag(0));
                pc++; break;
            case 0x8F:
                add(*A, *A + get_flag(0));
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
                bitwise_and(*B);
                pc++; break;
            case 0xA1:
                bitwise_and(*C);
                pc++; break;
            case 0xA2:
                bitwise_and(*D);
                pc++; break;
            case 0xA3:
                bitwise_and(*E);
                pc++; break;
            case 0xA4:
                bitwise_and(*H);
                pc++; break;
            case 0xA5:
                bitwise_and(*L);
                pc++; break;
            case 0xA6:
                bitwise_and(memory[HL.p]);
                pc++; break;
            case 0xA7:
                bitwise_and(*A);
                pc++; break;
            case 0xA8:
                bitwise_xor(*B);
                pc++; break;
            case 0xA9:
                bitwise_xor(*C);
                pc++; break;
            case 0xAA:
                bitwise_xor(*D);
                pc++; break;
            case 0xAB:
                bitwise_xor(*E);
                pc++; break;
            case 0xAC:
                bitwise_xor(*H);
                pc++; break;
            case 0xAD:
                bitwise_xor(*L);
                pc++; break;
            case 0xAE:
                bitwise_xor(memory[HL.p]);
                pc++; break;
            case 0xAF:
                bitwise_xor(*A);
                pc++; break;

            case 0xB0:
                bitwise_or(*B);
                pc++; break;
            case 0xB1:
                bitwise_or(*C);
                pc++; break;
            case 0xB2:
                bitwise_or(*D);
                pc++; break;
            case 0xB3:
                bitwise_or(*E);
                pc++; break;
            case 0xB4:
                bitwise_or(*H);
                pc++; break;
            case 0xB5:
                bitwise_or(*L);
                pc++; break;
            case 0xB6:
                bitwise_or(memory[HL.p]);
                pc++; break;
            case 0xB7:
                bitwise_or(*A);
                pc++; break;
            case 0xB8:
                cp(*B);
                pc++; break;
            case 0xB9:
                cp(*C);
                pc++; break;
            case 0xBA:
                cp(*D);
                pc++; break;
            case 0xBB:
                cp(*E);
                pc++; break;
            case 0xBC:
                cp(*H);
                pc++; break;
            case 0xBD:
                cp(*L);
                pc++; break;
            case 0xBE:
                cp(memory[HL.p]);
                pc++; break;
            case 0xBF:
                cp(*A);
                pc++; break;

            case 0xC0: /* ret nz */
                if(!(get_flag(6))) pop(pc);
                else pc++;
                break;
            case 0xC1:
                pop(BC.p);
                pc++; break;
            case 0xC2:
                if(!(get_flag(6)))
                    pc = get_operand(2);
                else
                    pc += 3;
                break;
            case 0xC3:
                pc = get_operand(2);
                break;
            case 0xC4:
                if(!(get_flag(6)))
                {
                    push(pc+3);
                    pc = get_operand(2);
                }else
                    pc += 3;
                break;
            case 0xC5:
                push(BC.p);
                pc++; break;
            case 0xC6:
                add(*A, get_operand(1));
                pc += 2; break;
            case 0xC7:
                push(pc+1);
                pc = 0x00; break;
            case 0xC8:
                if(get_flag(6)) pop(pc);
                else pc++;
                break;
            case 0xC9:
                pop(pc);
                break;
            case 0xCA:
                if(get_flag(6))
                    pc = get_operand(2);
                else
                    pc += 3;
                break;
            case 0xCB:
                interpret_bits(rom[++pc]);
                pc++; break;
            case 0xCC:
                if(get_flag(6))
                {
                    push(pc+3);
                    pc = get_operand(2);
                }else
                    pc += 3;
                break;
            case 0xCD:
                push(pc+3);
                pc = get_operand(2);
                break;
            case 0xCE:
                add(*A, get_operand(1) + get_flag(0));
                pc += 2; break;
            case 0xCF:
                push(pc+1);
                pc = 0x08; break;

            case 0xD0:
                if(!(get_flag(0))) pop(pc);
                else pc++;
                break;
            case 0xD1:
                pop(DE.p);
                pc++; break;
            case 0xD2:
                if(!(get_flag(0)))
                    pc = get_operand(2);
                else
                    pc += 3;
                break;
            case 0xD3: /* out (*), a */
                OUT(get_operand(1), *A);
                pc += 2;break;
            case 0xD4:
                if(!(get_flag(0)))
                {
                    push(pc+3);
                    pc = get_operand(2);
                }else
                    pc += 3;
                break;
            case 0xD5:
                push(DE.p);
                pc++; break;
            case 0xD6:
                SUB(get_operand(1));
                pc += 2; break;
            case 0xD7:
                push(pc+1);
                pc = 0x10; break;
            case 0xD8:
                if(get_flag(0)) pop(pc);
                else pc++;
                break;
            case 0xD9:
                std::swap(BC.p, BC_.p);
                std::swap(DE.p, DE_.p);
                std::swap(HL.p, HL_.p);
                pc++; break;
            case 0xDA:
                if(get_flag(0))
                    pc = get_operand(2);
                 else
                    pc += 3;
                 break;
            case 0xDB: /* in a, (*) */
                IN(*A, get_operand(1));
                pc += 2; break;
            case 0xDC:
                if(get_flag(0))
                {
                    push(pc+3);
                    pc = get_operand(2);
                }else
                    pc += 3;
                break;
            case 0xDD:
                interpret_ix(rom[++pc]);
                break;
            case 0xDE:
                SUB(get_operand(1) + get_flag(0));
                pc += 2; break;
            case 0xDF:
                push(pc+1);
                pc = 0x18; break;

            case 0xE0:
                if(!get_flag(2)) pop(pc);
                else pc++;
                break;
            case 0xE1:
                pop(HL.p);
                pc++; break;
            case 0xE2:
                if(!get_flag(2))
                    pc = get_operand(2);
                else
                    pc += 3;
                break;
            case 0xE3:
                memory[sp] = HL.r[0];
                memory[sp+1] = HL.r[1];
                pc++; break;
            case 0xE4:
                if(!get_flag(2))
                {
                    push(pc+3);
                    pc = get_operand(2);
                }else
                    pc += 3;
                break;
            case 0xE5:
                push(HL.p);
                pc++; break;
            case 0xE6:
                bitwise_and(get_operand(1));
                pc += 2; break;
            case 0xE7:
                push(pc+1);
                pc = 0x20; break;
            case 0xE8:
                if(get_flag(2)) pop(pc);
                else pc++;
                break;
            case 0xE9: /* jp (hl) */
                 pc = memory[HL.p];
                 break;
            case 0xEA:
                if(get_flag(2))
                    pc = get_operand(2);
                break;
            case 0xEB:
                std::swap(DE.p, HL.p);
                pc++; break;
            case 0xEC:
                if(get_flag(2))
                {
                    push(pc+3);
                    pc = get_operand(2);
                }else
                    pc += 3;
                break;
            case 0xED:
                interpret_extd(rom[++pc]); /* Incremente le Program Counter en fetchant */
                break;
            case 0xEE:
                bitwise_xor(get_operand(1));
                pc += 2; break;
            case 0xEF:
                push(pc+1);
                pc = 0x28; break;

            case 0xF0:
                if(!get_flag(7)) pop(pc);
                else pc++;
                break;
            case 0xF1:
                pop(AF.p);
                pc++; break;
            case 0xF2:
                if(!get_flag(7))
                    pc = get_operand(2);
                else
                    pc += 3;
                break;
            case 0xF3: /* di */
                iff1 = false;
                iff2 = false;
                break;
            case 0xF4:
                if(!get_flag(7))
                {
                    push(pc+3);
                    pc = get_operand(2);
                }else
                    pc += 3;
                break;
            case 0xF5:
                push(AF.p);
                pc++; break;
            case 0xF6:
                bitwise_or(get_operand(1));
                pc += 2; break;
            case 0xF7:
                push(pc+1);
                pc = 0x30; break;
            case 0xF8:
                if(get_flag(7)) pop(pc);
                else pc++;
                break;
            case 0xF9:
                ld(sp, HL.p);
                pc++; break;
            case 0xFA:
                if(get_flag(7))
                    pc = get_operand(2);
                else
                    pc += 3;
                break;
            case 0xFB: /* ei */
                iff1 = true;
                iff2 = true;
                break;
            case 0xFC:
                if(get_flag(7))
                {
                    push(pc+3);
                    pc = get_operand(2);
                }else
                    pc += 3;
                break;
            case 0xFD:
                // TODO IY
                break;
            case 0xFE:
                cp(get_operand(1));
                pc += 2; break;
            case 0xFF:
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
                SBC(HL.p, BC.p);
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
                SBC(HL.p, DE.p);
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
                SBC(HL.p, HL.p);
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
                SBC(HL.p, sp);
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
                DEC(ix);
                pc++; break;
            case 0x34:
                inc(*ixx);
                pc += 2; break;
            case 0x35:
                DEC(*ixx);
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
                SUB(*ixx);
                pc += 2; break;
            case 0x9E:
                SBC(*A, *ixx);
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

    void Z80::step()
    {
        uint8_t opcode;
        if(pc > rom_size)
        {
            std::cout << "Program counter overflow" << std::endl;
            exit(EXIT_FAILURE);
        }
        opcode = rom[pc];
        execute(opcode);
    }

    void Z80::interrupt()
    {
        if(pins[17])
            pc++;
    }

    template <class T, class U> void Z80::ld(T& dst, U src)
    {
        dst = src;
    }

    template <class T, class U> void Z80::add(T& dst, U src)
    {
        if(sizeof(T) == 2) /* 16 bit operations don't affect flags */
            dst = dst + src;

        uint half_result = (dst&0x0F) + (src&0x0F);
        uint result = dst + src;

        set_CF(result > 255);
        set_NF(false);
        set_POF(twoscomp(result) > 255);
        set_F3(0x1 << 3 & result);
        set_HF(half_result & 0x10);
        set_F5(0x1 << 5 & result);
        set_ZF((result & 0xFF) == 0);
        set_SF(twoscomp(result) & 0x80);

        dst = result;
    }

    template <class T> void Z80::inc(T& dst)
    {
        add(dst, 1);
    }

    template <class T, class U> void Z80::adc(T& dst, U src)
    {
        add(dst, src+get_flag(0));
    }

    uint Z80::sub(uint dst, uint src, size_t type)
    {
        uint result = dst - src;
        if(type == 2)
            return result;
        uint half_result = (dst & 0x0F) - (src & 0x0F);

        set_CF(result > 255);
        set_NF(true);
        set_POF(twoscomp(result) > 255);
        set_F3(0x1 << 3 & result);
        set_HF(half_result & 0x10);
        set_F5(0x1 << 5 & result);
        set_ZF((result & 0xFF) == 0);
        set_SF(twoscomp(result) & 0x80);

        return result;
    }

    void Z80::bitwise_and(uint src)
    {
        uint result = *A & src;

        set_CF(false);
        set_NF(false);
        set_POF(parity_check(result));
        set_HF(true);
        set_ZF((result & 0xFF) == 0);
        set_SF(twoscomp(result) & 0x80);

        *A = result;
    }

    void Z80::bitwise_xor(uint src)
    {
        uint result = *A ^ src;

        set_CF(false);
        set_NF(false);
        set_POF(parity_check(result));
        set_HF(false);
        set_ZF((result & 0xFF) == 0);
        set_SF(twoscomp(result) & 0x80);

        *A = result;
    }

    void Z80::bitwise_or(uint src)
    {
        uint result = *A | src;
        set_CF(false);
        set_NF(false);
        set_POF(parity_check(result));
        set_HF(false);
        set_ZF((result & 0xFF) == 0);
        set_SF(twoscomp(result) & 0x80);

        *A = result;
    }

    void Z80::cp(uint src)
    {
        uint result = *A - src;
        uint half_result = (*A & 0xF) - (src & 0xF);

        set_CF(result > 255);
        set_NF(true);
        set_POF(twoscomp(result) > 255);
        set_F3(0x1 << 3 & result);
        set_HF(half_result & 0x10);
        set_F5(0x1 << 5 & result);
        set_ZF((result & 0xFF) == 0);
        set_SF(twoscomp(result) & 0x80);
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
        for(uint i = 0; i<sizeof(bin)*8; ++i)
        {
            c += bin << i & 0x1;
        }

        return !(c % 2);
    }

    uint16_t Z80::get_operand(uint offset)
    {
        if (offset == 1)
            return rom[pc+1];
        return rom[pc+1] << 8 | rom[pc+2];
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
        DEC(*B);
        if(*B != 0)
            pc += value;
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

        set_SF(twoscomp(*A) > 255);
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

        set_SF(twoscomp(*A) > 255);
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
        uint result = *A - memory[HL.p];
        uint half_result = (*A&0x0F) - (memory[HL.p]&0x0F);

        set_SF(twoscomp(result&0xFF) > 255);
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
        uint result = *A - memory[HL.p];
        uint half_result = (*A&0x0F) - (HL.p&0x0F);

        set_SF(twoscomp(result) > 255);
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

        set_SF(twoscomp(*m) > 255);
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

        set_SF(twoscomp(*m) > 255);
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
       dst = memory[sp+1] << 8 | memory[sp];
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

#undef DEC
#undef SUB
#undef OUT
#undef IN
#undef SBC

#endif
