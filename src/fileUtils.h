#ifndef _FILE_UTILS_H_F654FF32_7032_4271_B21A_D6322D0722E3
#define _FILE_UTILS_H_F654FF32_7032_4271_B21A_D6322D0722E3

#include <stdio.h>
#include <string>
#include <filesystem>
#include <regex>

namespace fileUtils {
    
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