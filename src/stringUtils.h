#ifndef _STRINGUTILS_H_EA89F308_240F_4AE0_97B5_AFE55000B453
#define _STRINGUTILS_H_EA89F308_240F_4AE0_97B5_AFE55000B453

#include <sstream>

namespace stringUtils {
    template < typename val_T >
    val_T min( const val_T& val1, const val_T& val2 ) {
        return ( val1 <= val2 ) ? val1 : val2;
    }

    template < typename val_T >
    val_T max( const val_T& val1, const val_T& val2 ) {
        return ( val1 >= val2 ) ? val1 : val2;
    }

    template < typename val_T >
    static val_T convStrTo( const std::string& str ) {
        std::stringstream numStream{ str };
        val_T val;
        numStream >> val;
        return val;
    }

    template < typename val_T >
    static void convStrTo( val_T& num, const std::string& str ) {
        printf( "convStrTo() got string %s\n", str.c_str() );
        std::stringstream numStream{ str };
        numStream >> num;
        printf( "after conversion: %f\n", num );
    }
}

#endif // _STRINGUTILS_H_EA89F308_240F_4AE0_97B5_AFE55000B453
