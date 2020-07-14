#pragma once

#include <memory>
#include <string>

#include "Mesh.h"

namespace LSIS {

	namespace MeshLoader {

		std::shared_ptr<MeshData> LoadFromOBJ(const std::string& filepath);

		std::shared_ptr<MeshData> CreateRect(glm::vec2 size);
		std::shared_ptr<MeshData> CreateCube(float size);



	}

}