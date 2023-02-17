#ifndef _FILE_UTILS_H_F654FF32_7032_4271_B21A_D6322D0722E3
#define _FILE_UTILS_H_F654FF32_7032_4271_B21A_D6322D0722E3

#include <stdio.h>

//#include <algorithm>
#include <string>
#include <filesystem>
#include <regex>

#include <fstream>

namespace fileUtils {
    
    static std::string readFile( const std::string &filePath ) { 
        std::ifstream ifile{ filePath };
        return std::string( std::istreambuf_iterator< char >( ifile ), 
                            std::istreambuf_iterator< char >() );
    }

    static void writeFile( const std::string &filePath, const std::string& data ) { 
        //std::ofstream ofile{ filePath };
        //std::copy( data.begin(), data.end(), std::ostreambuf_iterator< char >( ofile ) );

        //std::ofstream ofile{ std::filesystem::absolute( std::filesystem::path( filePath ) ).string() };
        std::ofstream ofile{ filePath };
        ofile << data;
        ofile.close();
    }

	static void lukeFileWalker( const uint32_t level, 
								const std::filesystem::path& path, 
								std::vector<std::string>& filenames, 
								const std::regex& filter ) {
        for (const auto& entry : std::filesystem::directory_iterator( path )) {
            if (entry.is_directory()) {
                //printf( "dir: level %u, path '%s'\n", level, entry.path().string().c_str() );
                lukeFileWalker( level + 1, entry.path(), filenames, filter );
            } else {
                const std::string filename = entry.path().filename().string();
                //printf( "file: level %u, path '%s', file '%s'\n", level, entry.path().parent_path().string().c_str(), filename.c_str() );

                if (std::regex_match( filename, filter )) {
                    //printf( "    pushing back '%s'\n", entry.path().string().c_str() );
                    filenames.push_back( entry.path().string() );
                }
                else {
                    //printf( "    file '%s' does not match the regex - skipping it!\n", filename.c_str() ); 
                }
            }
        }
    }
	
}

#endif // _FILE_UTILS_H_F654FF32_7032_4271_B21A_D6322D0722E3