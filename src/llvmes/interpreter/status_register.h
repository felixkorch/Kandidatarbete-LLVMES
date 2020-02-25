namespace llvmes {
class StatusRegister {
    template <unsigned int N>
    class Bit {
        unsigned int &data;

       public:
        constexpr Bit(unsigned int &data) : data(data)
        {
        }

        unsigned int &operator=(bool value)
        {
            if (value)
                return data |= (1 << N);
            return data &= ~(1 << N);
        }

        constexpr operator bool() const
        {
            return data & (1 << N);
        }
    };

   public:
    unsigned int data;
    Bit<0> C;
    Bit<1> Z;
    Bit<2> I;
    Bit<3> D;
    Bit<4> B;
    Bit<5> Unused;
    Bit<6> V;
    Bit<7> N;

    constexpr StatusRegister()
        : data(0),
          C(data),
          Z(data),
          I(data),
          D(data),
          B(data),
          Unused(data),
          V(data),
          N(data)
    {
    }

    constexpr StatusRegister(const StatusRegister &other)
        : data(other.data),
          C(data),
          Z(data),
          I(data),
          D(data),
          B(data),
          Unused(data),
          V(data),
          N(data)
    {
    }

    constexpr StatusRegister(unsigned int value)
        : data(value),
          C(data),
          Z(data),
          I(data),
          D(data),
          B(data),
          Unused(data),
          V(data),
          N(data)
    {
    }

    constexpr operator unsigned int() const
    {
        return data;
    }

    unsigned int &operator=(unsigned int newValue)
    {
        return data = newValue;
    }

    StatusRegister &operator=(StatusRegister &other)
    {
        data = other.data;
        return *this;
    }
};
}  // namespace llvmes
