#pragma once

#include <memory>

/*
#ifdef CH_ENABLE_ASSERTS
	#define CH_ASSERT(x, ...) { if (!(x)) { CH_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define CH_CORE_ASSERT(x, ...) { if (!(x)) { CH_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define CH_ASSERT(x, ...)
	#define CH_CORE_ASSERT(x, ...)
#endif // CH_ENABLE_ASSERTS
*/

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

#define BIT(x) (1 << x)


namespace LSIS {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

}