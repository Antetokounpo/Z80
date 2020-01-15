#ifndef Z80_H
#define Z80_H

#include<cstdint>

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
            Z80();
            ~Z80() {}
            virtual void step();
            virtual uint8_t fetch(int offset);
            virtual bool load(const char* filename); /* Loads ROM */
            virtual void execute(uint8_t opcode);

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
            unsigned int rom_size; /* Size of the ROM file */
            uint8_t ports[256];    /* I/O ports */

            bool pins[40]; /* I/O pins */

            /* Interrupt flip-flops */
            bool iff1 = false;
            bool iff2 = false;
            unsigned int interrupt_mode = 0;

            void ei();
            void di();

            template <class T, class U> void ld(T& dst, U src);
            template <class T, class U> void add(T& dst, U src);
            template <class T> void inc(T& dst);
            template <class T, class U> void adc(T& dst, U src);
            template <class T, class U> void arithmetic_sub(T& dst, U src);
            void sub(unsigned int src);
            template <class T> void dec(T& dst);
            template <class T, class U> void sbc(T& dst, U src);
            void bitwise_and(unsigned int src);
            void bitwise_xor(unsigned int src);
            void bitwise_or(unsigned int src);
            void cp(unsigned int src);

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

            unsigned int get_flag(unsigned int flag);
            void flag_affect(unsigned int result, int8_t flags[]);

            template<class T> unsigned int onescomp(T bin);
            template<class T> unsigned int twoscomp(T bin);
            bool parity_check(unsigned int bin);
            uint16_t get_operand(int offset);
            uint8_t& get_memory(uint16_t address);

            void interpret_extd(uint8_t opcode);
            void interpret_bits(uint8_t opcode);
            void interpret_ix(uint8_t opcode);

            unsigned int cycles; /* Variable used to count CPU cycles used by instructions */
            unsigned int cpu_frequency; /* CPU frequency in Hz */
            unsigned int refresh_rate; /* Display refresh rate in Hz */
    };
}

#include "z80.tpp"

#endif
