--
-- run: premake5 vs2022
-- then open the VS2022 solution file
--
-- additional info when using Qt
-- https://github.com/dcourtois/premake-qt
--
--
-- sample invocation for building from the command line:
-- then open VS command prompt from the start menu "x64 Native Tools Command Prompt for VS 2017"
-- msbuild OrientationMatchingWorkspace.sln -target:OrientationMatchingProject -property:Configuration=Release -property:Platform="Win64"
--

--
-- NOTE: on macOS gmake2 doesn't work (moc error)
--    https://github.com/dcourtois/premake-qt/issues/15
--    https://github.com/premake/premake-core/issues/1398
-- for macOS, run: premake5 gmake
-- then, run make config=Release_macos
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

-- workspace "T3DFW_Workspace"
-- 	location "../"

-- 	-- https://premake.github.io/docs/characterset/
-- 	-- characterset ("MBCS")

-- 	--
-- 	-- setup your solution's configuration here ...
-- 	--
-- 	configurations { "Debug", "Release" }
-- 	platforms { "Win64", "macOS" }

-- 	filter { "platforms:Win64" }
-- 		system "Windows"
-- 		architecture "x86_64"		

-- 	filter { "platforms:macOS" }
-- 		system "macosx"
-- 		architecture "x86_64"		

-- 	filter { "system:Windows" }
-- 		systemversion "latest" -- To use the latest version of the SDK available
-- 		--	systemversion "10.0.10240.0" -- To specify the version of the SDK you want
	
-- 		-- https://stackoverflow.com/questions/40667910/optimizing-a-single-file-in-a-premake-generated-vs-solution
-- 	filter { "configurations:Debug" }
-- 		optimize "Debug"
-- 		defines { "DEBUG", "TRACE" }
-- 		buildoptions{ "-fdiagnostics-color=always" }
-- 	filter { "configurations:Release" }
-- 		optimize "Speed"
-- 		defines { "NDEBUG" }

-- 	filter {}

	-- cdialect "C99"
	-- cppdialect "C++20"

	function incorporateGlfw (GFX_API_DIR)
		filter { "platforms:macosx" }
			linkoptions "-v -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit"
			local GLFW_BASE_DIR = GFX_API_DIR .. "glfw-3.3.8.macosx.WIN64/"
			local GLFW_LIB_DIR = GLFW_BASE_DIR .. "lib/"
			libdirs { GLFW_LIB_DIR }

		filter {"platforms:macosx", "gmake"}
			buildoptions {"-F /Library/Frameworks"}
			linkoptions {"-F /Library/Frameworks"}
			local GLFW_BASE_DIR = GFX_API_DIR .. "glfw-3.3.8.macosx.WIN64/"
			local GLFW_LIB_DIR = GLFW_BASE_DIR .. "lib/"
			libdirs { GLFW_LIB_DIR }
			
		filter { "platforms:Win*", "action:vs*" }
			local GLFW_BASE_DIR = GFX_API_DIR .. "glfw-3.3.8.bin.WIN64/"
			local GLFW_LIB_DIR = GLFW_BASE_DIR .. "lib-vc2022"
			libdirs { GLFW_LIB_DIR }
			links { "glfw3" }
			
		filter { "platforms:Win*", "action:gmake*", "toolset:gcc" }
			local GLFW_BASE_DIR = GFX_API_DIR .. "glfw-3.3.8.bin.WIN64/"
			local GLFW_LIB_DIR = GLFW_BASE_DIR .. "lib-mingw-w64"
			libdirs { GLFW_LIB_DIR }
			--links { "glfw3" }
			
		filter "configurations:Debug"
			symbols "On"

		filter {}

		return GLFW_BASE_DIR
	end
	
	local PRJ_LOCATION = "%{prj.location}/"
	local T3DFW_BASE_DIR = PRJ_LOCATION
	local T3DFW_EXTERNAL_DIR = T3DFW_BASE_DIR .. "external/"
	local T3DFW_SRC_DIR = T3DFW_BASE_DIR .. "src/"
	
	local LINALG_DIR = T3DFW_SRC_DIR .. "math/"
	local GFX_API_DIR = T3DFW_SRC_DIR .. "gfxAPI/"
	local GLAD_BASE_DIR = GFX_API_DIR .. "glad/"
	
	local STB_BASE_DIR = T3DFW_EXTERNAL_DIR .. "stb/"
	local STL_READER_DIR = T3DFW_EXTERNAL_DIR .. "stl_reader/"
	
	local NATIVEFILEDIALOG_DIR = T3DFW_EXTERNAL_DIR .. "nativefiledialog/"
	
	local IMGUI_DIR = T3DFW_EXTERNAL_DIR .. "imgui/"
	
	-- main project	
	project "T3DFW_LIB_Project"

		-- objdir "obj/%{cfg.platform}_%{cfg.buildcfg}"
		-- objdir "!obj/$(Platform)_$(Configuration)"

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
			-- VS also links these two libs, but they seem to not be necessary... "odbc32.lib" "odbccp32.lib" 
			-- buildoptions {"-fopenmp"} -- NOT NEEDED IF DEFINED AT PROJECT SCOPE AS "openmp "On""
			-- linkoptions {"lgomp"}
			defines { "UNIX", "_USE_MATH_DEFINES" }
				
		filter {"platforms:Win*", "vs*"}
			defines { "_USE_MATH_DEFINES" }
			
		filter {}

		-- filter { "configurations:Debug" }
			-- optimize "Debug"
			-- symbols "On"

		-- filter { "configurations:Debug", "platforms:Win*", "action:gmake*", "toolset:gcc" }		
			-- buildoptions { "-g" }
			-- https://stackoverflow.com/questions/54969270/how-to-use-gcc-for-compilation-and-debug-on-vscode
			-- buildoptions { "-gdwarf-4 -g3" }

		filter {}

		includedirs { 
			_SCRIPT_DIR,
			PRJ_LOCATION,
			GLFW_BASE_DIR .. "include/",
			GLAD_BASE_DIR .. "include/", 
			LINALG_DIR,
			GFX_API_DIR,
			STB_BASE_DIR, 
			STL_READER_DIR, 
			NATIVEFILEDIALOG_DIR .. "include/",
			IMGUI_DIR,
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
		-- excludes {  }
		
		-- https://premake.github.io/docs/defines/
		-- "_MBCS", 
		defines { "_WIN32", "WIN32", "_WINDOWS" }

	
		filter { "platforms:Win*" }
			excludes { NATIVEFILEDIALOG_DIR .. "nfd_gtk.c", NATIVEFILEDIALOG_DIR .. "nfd_zenity.c" }

			-- configuration "macosx"
		filter { "platforms:macosx" }
			excludes { NATIVEFILEDIALOG_DIR .. "nfd_win.c" }

		-- 
		-- disable filters, so this is valid for all projects
		-- 
		filter {} 


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
 