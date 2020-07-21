workspace "LSIS"
	architecture "x64"
	cppdialect "c++17"
	startproject "PathTracer"
	systemversion "latest"

	defines {
		"APP_LSIS"
	}

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
IncludeDir["glfw"] = "Vendor/glfw/include"
IncludeDir["glad"] = "Vendor/glad/include"
IncludeDir["glm"] = "Vendor/glm/glm"
IncludeDir["stb_image"] = "Vendor/stb_image/"
IncludeDir["entt"] = "Vendor/entt/"

include "Vendor/glfw"
include "Vendor/glad"

project "Core"
	kind "StaticLib"
	location "Core"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
    pchsource "Core/pch.cpp"

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
		"opencl",
		"opengl32",
		"glfw",
		"glad"
	}

	includedirs {
		"$(OPENCL_PATH)/include",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{prj.name}",
	}

	defines {
		"GLFW_INCLUDE_NONE",
	}

project "PathTracer"
	kind "ConsoleApp"
	location "PathTracer"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
    pchsource "PathTracer/pch.cpp"

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.cpp"
	}

	libdirs {
		"Core"
	}

	links {
		"Core"
	}

	includedirs {
		"Core",
		"$(OPENCL_PATH)/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.entt}",
	}
