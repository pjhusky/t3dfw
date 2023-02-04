#ifndef _STRINGUTILS_H_EA89F308_240F_4AE0_97B5_AFE55000B453
#define _STRINGUTILS_H_EA89F308_240F_4AE0_97B5_AFE55000B453

#include <sstream>

#include <tchar.h>

#define UTF8 0
#if ( UTF8 != 0 )
    #include "utfcpp/source/utf8.h" // solve C++17 UTF deprecation
#endif

namespace stringUtils {
    template < typename val_T >
    val_T min( const val_T& val1, const val_T& val2 ) {
        return ( val1 <= val2 ) ? val1 : val2;
    }

    template < typename val_T >
    val_T max( const val_T& val1, const val_T& val2 ) {
        return ( val1 >= val2 ) ? val1 : val2;
    }

#if ( UTF8 != 0 ) // solve C++17 UTF deprecation
    // https://codingtidbit.com/2020/02/09/c17-codecvt_utf8-is-deprecated/
    static inline std::string utf8_encode( const std::wstring& wStr )
    {
    #if 0
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes( wStr );
    #else
        std::string utf8line;

        if (wStr.empty())
            return utf8line;

    #ifdef _MSC_VER
        utf8::utf16to8( wStr.begin(), wStr.end(), std::back_inserter( utf8line ) );
    #else
        utf8::utf32to8( wStr.begin(), wStr.end(), std::back_inserter( utf8line ) );
    #endif
        return utf8line;
    #endif
    }

    // https://codingtidbit.com/2020/02/09/c17-codecvt_utf8-is-deprecated/
    static inline std::wstring utf8_decode( const std::string& sStr )
    {
    #if 0
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes( sStr );
    #else
        std::wstring wide_line;

        if (sStr.empty())
            return wide_line;

    #ifdef _MSC_VER
        utf8::utf8to16( sStr.begin(), sStr.end(), std::back_inserter( wide_line ) );
    #else
        utf8::utf8to32( sStr.begin(), sStr.end(), std::back_inserter( wide_line ) );
    #endif
        return wide_line;
    #endif
    }
#endif

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
