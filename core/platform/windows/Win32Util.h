#pragma once

#ifdef LSIS_PLATFORM_WIN

#include "glm.hpp"
#include "GLFW/glfw3.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

namespace LSIS {
	inline void SetWin32Border(GLFWwindow* window, bool b) {
		HWND hwnd = glfwGetWin32Window(window);
		LONG style = GetWindowLongA(hwnd, GWL_STYLE);
		style = b ? style | WS_BORDER : style & (~WS_BORDER);
		SetWindowLongA(hwnd, GWL_STYLE, style);
	}
}

#endif // LSIS_PLATFORM_WINDOWS