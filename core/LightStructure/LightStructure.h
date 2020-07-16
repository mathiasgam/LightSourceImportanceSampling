#pragma once

#include <memory>

#include "Light/Light.h"

namespace LSIS {

	// Interface class for light structures
	class LightStructure {
	public:
		virtual void AddLight(std::shared_ptr<Light> light) = 0;
		virtual void Build() = 0;
		virtual void Destroy() = 0;

		// standin method, should take in cl buffers to run a sampling kernel on the GPU
		virtual void SampleLights() = 0;

	private:
	};

}