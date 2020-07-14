#pragma once

#include <vector>
#include <cinttypes>

#include <Scene/Light.h>

namespace LSIS {

	class LightTree {

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