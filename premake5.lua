--
-- run: premake5 vs2022
-- then open the VS2022 solution file
--
--
-- sample invocation for building from the command line:
-- then open VS command prompt from the start menu "x64 Native Tools Command Prompt for VS 2017"
-- msbuild <SolutionFilename>.sln -target:<ProjectName> -property:Configuration=Release -property:Platform="Win64"
--
--
-- NOTE: minGW makefiles:
--
-- > premake5 gmake2
-- > mingw32-make -f Makefile config=release_win64 CC=gcc
--
-- the CC=gcc define is needed to prevent this error: "'cc' is not recognized as an internal or external command"
--

--
-- run: 'premake5 clean' to delete all temporary/generated files
--


--
-- workspace is roughly a VS solution; contains projects
--

	function incorporateGlfw (gfxApiDir)
		filter { "platforms:macosx", "action:xcode*" }
			linkoptions "-v -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit"
			local GLFW_BASE_DIR = path.normalize( path.join( gfxApiDir, "glfw-3.3.8.macosx.WIN64" ) )
			local GLFW_LIB_DIR = path.normalize( path.join( GLFW_BASE_DIR, "lib" ) )
			libdirs { GLFW_LIB_DIR }

		filter {"platforms:macosx", "action:gmake*"}
			buildoptions {"-F /Library/Frameworks"}
			linkoptions {"-F /Library/Frameworks"}
			local GLFW_BASE_DIR = path.normalize( path.join( gfxApiDir, "glfw-3.3.8.macosx.WIN64" ) )
			local GLFW_LIB_DIR = path.normalize( path.join( GLFW_BASE_DIR, "lib" ) )
			libdirs { GLFW_LIB_DIR }
			
		filter { "platforms:Win*", "action:vs*" }
			local GLFW_BASE_DIR = path.normalize( path.join( gfxApiDir, "glfw-3.3.8.bin.WIN64" ) )
			local GLFW_LIB_DIR = path.normalize( path.join( GLFW_BASE_DIR, "lib-vc2022" ) )
			libdirs { GLFW_LIB_DIR }
			links { "glfw3" }
			
		filter { "platforms:Win*", "action:gmake*", "toolset:gcc" }
			local GLFW_BASE_DIR = path.normalize( path.join( gfxApiDir, "glfw-3.3.8.bin.WIN64" ) )
			local GLFW_LIB_DIR = path.normalize( path.join( GLFW_BASE_DIR, "lib-mingw-w64" ) )
			libdirs { GLFW_LIB_DIR }
			--links { "glfw3" }
			--buildoptions { "-fpermissive" }

		filter {}

		return GLFW_BASE_DIR
	end
	
	local PRJ_LOCATION = path.normalize( "%{prj.location}" )
	local T3DFW_BASE_DIR = PRJ_LOCATION
	local T3DFW_EXTERNAL_DIR = path.normalize( path.join( T3DFW_BASE_DIR, "external" ) )
	local T3DFW_SRC_DIR = path.normalize( path.join( T3DFW_BASE_DIR, "src" ) )
	
	local LINALG_DIR = path.normalize( path.join( T3DFW_SRC_DIR, "math" ) )
	local GFX_API_DIR = path.normalize( path.join( T3DFW_SRC_DIR, "gfxAPI" ) )
	local GLAD_BASE_DIR = path.normalize( path.join( GFX_API_DIR, "glad" ) )
	
	local STB_BASE_DIR = path.normalize( path.join( T3DFW_EXTERNAL_DIR, "stb" ) )
	local STL_READER_DIR = path.normalize( path.join( T3DFW_EXTERNAL_DIR, "stl_reader" ) )
	
	local NATIVEFILEDIALOG_DIR = path.normalize( path.join( T3DFW_EXTERNAL_DIR, "nativefiledialog" ) )
	local TINYPROCESS_DIR = path.normalize( path.join( T3DFW_EXTERNAL_DIR, "tiny-process-library" ) )
	
	local IMGUI_DIR = path.normalize( path.join( T3DFW_EXTERNAL_DIR, "imgui" ) )
	local IMGUIZMO_QUAT_DIR = path.normalize( path.join( T3DFW_EXTERNAL_DIR, "imGuIZMO.quat" ) )
	
	-- main project	
	project "T3DFW_LIB_Project"

		local allDefinesProject = { "_USE_MATH_DEFINES", "IMGUIZMO_IMGUI_FOLDER="..IMGUI_DIR.."/" }
		if _ACTION == "gmake2" then
			table.insert( allDefinesProject, "UNIX" )
		elseif string.match( _ACTION, 'vs*') then
			table.insert( allDefinesProject, "_WIN32" )
			table.insert( allDefinesProject, "WIN32" )
			table.insert( allDefinesProject, "_WIN64" )
			table.insert( allDefinesProject, "_AMD64_" )
			table.insert( allDefinesProject, "_WINDOWS" )
			table.insert( allDefinesProject, "_CRT_SECURE_NO_WARNINGS" )
		end

		openmp "On" -- ALTERNATIVELY per filter: buildoptions {"-fopenmp"}
	
		cdialect "C99"
		cppdialect "C++20"

		local GLFW_BASE_DIR = incorporateGlfw(GFX_API_DIR)
			
		filter { "platforms:Win*", "action:gmake*", "toolset:gcc" }
			-- NEED TO LINK STATICALLY AGAINST libgomp.a AS WELL: https://stackoverflow.com/questions/30394848/c-openmp-undefined-reference-to-gomp-loop-dynamic-start
			links { 
				"kernel32", 
				"user32", 
				"comdlg32", 
				"advapi32", 
				"shell32", 
				"uuid", 
				"glfw3", -- BEFORE gdi32 AND opengl32
				"gdi32", 
				"opengl32", 
				"Dwmapi", 
				"ole32", 
				"oleaut32", 
				"gomp",
				--"bufferoverflowu", -- https://stackoverflow.com/questions/21627607/gcc-linker-error-undefined-reference-to-security-cookie
			}
			-- VS also links these two libs, but they seem to be unnecessary... "odbc32.lib" "odbccp32.lib" 
			--undefines { "__STRICT_ANSI__" } -- for simdb wvsprintfA() - https://stackoverflow.com/questions/3445312/swprintf-and-vswprintf-not-declared
			--cppdialect "gnu++20" -- https://github.com/assimp/assimp/issues/4190, https://premake.github.io/docs/cppdialect/

		filter "configurations:Debug*"
			symbols "On"
			targetsuffix "_d"
			
		filter {}

		includedirs { 
			T3DFW_EXTERNAL_DIR,
			_SCRIPT_DIR,
			PRJ_LOCATION,
			path.normalize( path.join( GLFW_BASE_DIR, "include" ) ),
			path.normalize( path.join( GLAD_BASE_DIR, "include" ) ), 
			LINALG_DIR,
			GFX_API_DIR,
			STB_BASE_DIR, 
			STL_READER_DIR, 
			path.normalize( path.join( NATIVEFILEDIALOG_DIR, "include" ) ),
			TINYPROCESS_DIR,
			IMGUI_DIR,
			IMGUIZMO_QUAT_DIR, 
			path.normalize( path.join( IMGUIZMO_QUAT_DIR, "imguizmo_quat" ) ),
			-- path.normalize( path.join( IMGUIZMO_QUAT_DIR, "libs" ) ),
		}	
		
		shaderincludedirs { "src/shaders" }  
		
		--
		-- setup your project's configuration here ...
		--
		kind "StaticLib"
		language "C++"

		-- add files to project
		files { 
			"**.h", 
			"**.hpp", 
			"**.cpp", 
			"**.c", 
			"**.inl", 
			"premake5.lua", 
			"**.bat", 
			"**.glsl",
		}
		--removefiles { "externals/tiny-process-library/tests/**" }
		excludes { path.join( TINYPROCESS_DIR, "tests/**" ), path.join( IMGUIZMO_QUAT_DIR, "example/**" ), path.join( IMGUIZMO_QUAT_DIR, "libs/imgui/examples/**" ), path.join( IMGUIZMO_QUAT_DIR, "old_examples/**" ), path.join( IMGUIZMO_QUAT_DIR, "examples/**" ), path.join( IMGUIZMO_QUAT_DIR, "basic_examples/**" ) }
		-- excludes { path.join( IMGUIZMO_QUAT_DIR, "libs/glad/**" ) }
		excludes { path.join( IMGUIZMO_QUAT_DIR, "libs/**" ) }
		
		removefiles { path.join( TINYPROCESS_DIR, "tests/**" ), path.join( IMGUIZMO_QUAT_DIR, "example/**" ), path.join( IMGUIZMO_QUAT_DIR, "libs/imgui/examples/**" ), path.join( IMGUIZMO_QUAT_DIR, "old_examples/**" ), path.join( IMGUIZMO_QUAT_DIR, "examples/**" ), path.join( IMGUIZMO_QUAT_DIR, "basic_examples/**" ) }
		removefiles { path.join( IMGUIZMO_QUAT_DIR, "commons/utils/framework*" ), path.join( IMGUIZMO_QUAT_DIR, "commons/utils/dbg*" ), path.join( IMGUIZMO_QUAT_DIR, "commons/utils/sdl2wgpu*" ) }
		-- removefiles { path.join( IMGUIZMO_QUAT_DIR, "libs/glad/**" ) }
		removefiles { path.join( IMGUIZMO_QUAT_DIR, "libs/**" ) }
		

		filter { "platforms:Win*" }
			excludes { path.join( NATIVEFILEDIALOG_DIR, "nfd_gtk.c" ), path.join( NATIVEFILEDIALOG_DIR, "nfd_zenity.c" ) }
			removefiles { path.join( TINYPROCESS_DIR, "process_unix.cpp" ), path.join( TINYPROCESS_DIR, "examples.cpp" ) }
			--defines { "_WIN32", "WIN32", "_WIN64", "_AMD64_", "_WINDOWS" }

		-- configuration "macosx"
		filter { "platforms:macosx" }
			excludes { path.join( NATIVEFILEDIALOG_DIR, "nfd_win.c" ) }


		--
		-- apply defines
		-- 
		filter { "configurations:Debug*" }
			defines { allDefinesProject, "DEBUG", "TRACE" }
		filter { "configurations:Release*" }
			defines { allDefinesProject, "NDEBUG" }

		-- 
		-- disable filters, so this is valid for all projects
		-- 
		filter {} 


	-- externalproject "TinyProcessProject"
	-- 	location "external/tiny-process-library/tiny-process-library.vcxproj"
	-- 	uuid "9CDC89A2-DEF3-4CAC-83DF-BED879282BC5"
	-- 	kind "StaticLib"
	-- 	language "C++"
	-- 	buildcommands { "echo mkdir", "echo external/tiny-process-library/" }
	-- 	buildinputs { "external/tiny-process-library//CMakeLists.txt" }
	-- 	buildoutputs { "" }

-- NOTE: this way we could trigger cleanup code as well:
-- if _ACTION == 'clean' then
-- 	print("clean action T3DFW!")
-- end
				

-- https://stackoverflow.com/questions/33307155/how-to-clean-projects-with-premake5
-- Clean Function --
newaction {
	trigger     = "clean",
	description = "clean T3DFW",
	onProject 	= function(prj)
		print("Clean action for project " .. prj.name)
		print("project " .. prj.name .. ", dir: " .. prj.location )
		print("cleaning T3DFW ...")
		os.rmdir( prj.location .. "/obj" )
		os.rmdir( prj.location .. "/bin" )
		os.remove( prj.location .. "/*.sln" )
		os.remove( prj.location .. "/*.vcxproj" )
		os.remove( prj.location .. "/*.vcxproj.*" )
	end,	
}
 