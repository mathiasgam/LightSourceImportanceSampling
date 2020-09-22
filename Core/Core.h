#pragma once

#include <memory>

#define CORE_ERROR(x, ...) { printf(x, __VA_ARGS__); __debugbreak(); }

#ifdef ENABLE_ASSERTS
	//#define LSIS_ASSERT(x, ...) { if (!(x)) { LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define CORE_ASSERT(x, ...) { if (!(x)) { CORE_ERROR("Assertion Failed: %s", __VA_ARGS__); __debugbreak(); } }
#else
	//#define ASSERT(x, ...)
	#define CORE_ASSERT(x, ...)
#endif // ENABLE_ASSERTS


#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

#define BIT(x) (1 << x)


namespace LSIS {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

}