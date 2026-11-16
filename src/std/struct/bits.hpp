#ifndef __BITS_HPP__
#define __BITS_HPP__

#define GET_BIT(buff, index) (((buff) >> (index)) & 1)

#include "std/types.h"
template<usize _Nb>
struct Bits {
    static constexpr usize _len = _Nb / 8;
    

    void SetBit(usize _index , u8 new_value ) {
        

    }
    
};







#endif
