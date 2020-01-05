namespace Z80
{
    template <class T, class U>
    void Z80::ld(T& dst, U src)
    {
        cycles += 1;
        dst = src;
    }

    template <class T, class U>
    void Z80::add(T& dst, U src)
    {
        if(sizeof(T) == 2)
        {
            unsigned int result = dst + src;
            unsigned int half_result = (dst&0xFF) + (src&0xFF);

            set_HF(half_result & 0x1000);
            set_NF(false);
            set_CF(result > 0xFFFF);

            dst = result;

            cycles += 3;
            return;
        }

        unsigned int half_result = (dst&0x0F) + (src&0x0F);
        unsigned int result = dst + src;

        set_CF(result > 255);
        set_NF(false);
        set_POF(twoscomp(result) > 255);
        set_F3(0x1 << 3 & result);
        set_HF(half_result & 0x10);
        set_F5(0x1 << 5 & result);
        set_ZF((result & 0xFF) == 0);
        set_SF(result & 0x80);

        dst = result;

        cycles += 1;
    }

    template <class T>
    void Z80::inc(T& dst)
    {
        if(sizeof(T) == 2)
        {
            dst += 1;
            cycles += 1;
            return;
        }
        add(dst, 1);
    }

    template <class T, class U>
    void Z80::adc(T& dst, U src)
    {
        add(dst, src+get_flag(0));
        if(sizeof(T) == 2)
        {
            set_SF(dst & 0x800);
            set_ZF(dst == 0);
            set_POF(twoscomp(dst) > 0xFFFF);
            cycles += 1;
        }
    }

    template <class T, class U>
    void Z80::arithmetic_sub(T& dst, U src)
    {
        unsigned int result = dst - src;
        if(sizeof(T) == 2)
            dst = result;
        unsigned int half_result = (dst & 0x0F) - (src & 0x0F);

        set_CF(result > 255);
        set_NF(true);
        set_POF(twoscomp(result) > 255);
        set_F3(0x1 << 3 & result);
        set_HF(half_result & 0x10);
        set_F5(0x1 << 5 & result);
        set_ZF((result & 0xFF) == 0);
        set_SF(result & 0x80);

        dst = result;
    }

    template <class T>
    void Z80::dec(T& dst)
    {
        arithmetic_sub(dst, 1);
    }

    template <class T, class U>
    void Z80::sbc(T& dst, U src)
    {
        arithmetic_sub(dst, src+get_flag(0));
    }

    template<class T>
    unsigned int Z80::onescomp(T bin)
    {
        for(unsigned int i = 0; i<sizeof(bin)*8; ++i)
        {
            uint8_t b = 0x1 << i;
            if(bin & b)
                bin &= 0xFF - b; /* change 1 to 0 */
            else
                bin |= b; /* change 0 to 1 */
        }

        return bin;
    }

    template<class T>
    unsigned int Z80::twoscomp(T bin)
    {
        return onescomp(bin)+1;
    }
}