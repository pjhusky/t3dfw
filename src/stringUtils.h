#ifndef _STRINGUTILS_H_EA89F308_240F_4AE0_97B5_AFE55000B453
#define _STRINGUTILS_H_EA89F308_240F_4AE0_97B5_AFE55000B453

#include <sstream>

#include <tchar.h>

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

    // https://stackoverflow.com/questions/46840919/calling-stdto-string-or-stdto-wstring-based-on-given-typedef
    template<class Char>
    struct string_traits;
    template<>
    struct string_traits<char>{
        using string=std::string;
        template<class T>
        static string convert_to_string(T&& t){
            using std::to_string;
            return to_string(std::forward<T>(t));
        }
    };
    template<>
    struct string_traits<wchar_t>{
        using string=std::wstring;
        template<class T>
        static string convert_to_string(T&& t){
            using std::to_wstring;
            return to_wstring(std::forward<T>(t));
        }
    };

    template <typename TChar = TCHAR, typename T>
    std::basic_string<TChar>
        ToString(const T& Arg) {
        return string_traits<TChar>::convert_to_string(Arg);
    }

}

#endif // _STRINGUTILS_H_EA89F308_240F_4AE0_97B5_AFE55000B453
