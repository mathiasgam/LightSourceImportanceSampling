#pragma once

#include <vector>
#include <cinttypes>

#include "LightStructure.h"

namespace LSIS {

	class LightTree : public LightStructure {

		struct Node {
			uint32_t parent;
			uint32_t children;
		};

	public:
		LightTree(std::vector<Light> lights);
		virtual ~LightTree();

	private:
		std::vector<Node> m_nodes;
		std::vector<Light> m_lights;
	};

}