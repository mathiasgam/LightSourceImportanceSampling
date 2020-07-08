#pragma once

#include "Window.h"
#include "scene/Scene.h"

#include "glm.hpp"
#include "CL/cl.h"

#include <iostream>
#include <memory>

namespace LSIS {

	namespace Application {

		void Init();
		void Run();

		void OnKeyPressed(int key);
		void OnKeuReleased(int key);

	}

}