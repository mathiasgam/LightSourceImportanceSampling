#pragma once

#include "LightStructure.h"

namespace LSIS {

	class LightArray : public LightStructure {
	public:
		LightArray();
		virtual ~LightArray();

		virtual void AddLight(std::shared_ptr<Light> light) override;
		virtual void Build() override;
		virtual void Destroy() override;

		// Uses power sampling to sample light sources
		virtual void SampleLights() override;


	private:

	};

}