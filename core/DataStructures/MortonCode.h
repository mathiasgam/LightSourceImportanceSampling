#pragma once

#include "glm.hpp"

#include <vector>
#include <cassert>
#include <bitset>
#include <iostream>

class MortonCode
{

};

class MortonCode3 : MortonCode {
private:

public:
	uint64_t data;
	MortonCode3();
	MortonCode3(glm::vec3 p);

	bool operator < (const MortonCode3& other) const;
	bool operator > (const MortonCode3& other) const;

private:
	static inline uint64_t SplitBy3(uint64_t);
};

inline std::ostream& operator<<(std::ostream& os, const MortonCode3& code)
{
	os << "(" << std::bitset<64>(code.data) << ")";
	return os;
}

