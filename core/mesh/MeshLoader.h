#pragma once

#include <memory>
#include <string>

#include "Mesh.h"

namespace LSIS {

	namespace MeshLoader {

		std::shared_ptr<Mesh> LoadFromOBJ(const std::string& filepath);
		std::shared_ptr<Mesh> LoadFromFile(const std::string& filepath);

		std::shared_ptr<Mesh> CreateRect(glm::vec2 size);
		std::shared_ptr<Mesh> CreateCube(float size);



	}

}