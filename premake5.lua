workspace "LSIS"
	architecture "x64"
	cppdialect "c++17"
	startproject "LSIS"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

	filter "system:windows"
        defines {
		    "LSIS_PLATFORM_WIN",
	    }
        
    filter "action:vs*"
        vectorextensions "AVX"

	filter "configurations:Release"
		buildoptions "/MT"
        defines { "NDEBUG" }
		optimize "On"
		runtime "Release"

	filter "configurations:Dist"
		buildoptions "/MT"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"

  	filter "configurations:Debug"
  		defines {
            "DEBUG",
            "CH_ENABLE_ASSERTS"
		}
		buildoptions "/MTd"
		symbols "on"
		runtime "Debug"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["glfw"] = "vendor/glfw/include"
IncludeDir["glad"] = "vendor/glad/include"
IncludeDir["glm"] = "vendor/glm/glm"

include "vendor/glfw"
include "vendor/glad"

project "Core"
	kind "ConsoleApp"
	location "core"
	language "C++"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")


	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.c",
		"%{prj.name}/**.cpp",
		"%{prj.name}/**.cl",
		"%{prj.name}/**.vert",
		"%{prj.name}/**.frag",
	}

	libdirs {
		"$(OPENCL_PATH)/lib/x64"
	}

	links
	{
		"glfw",
		"glad",
		"opengl32",
		"opencl",
	}

	includedirs {
		"$(OPENCL_PATH)/include",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.glm}",
		"%{prj.name}/src"
	}

	defines {
		"GLFW_INCLUDE_NONE"
	}

