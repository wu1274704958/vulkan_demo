#include "core/object.hpp"

namespace vkd {

	uint32_t Object::component_count() { return components.size(); }
	void Object::adjust_locat(size_t i, int offset)
	{
		for (; i < components.size(); ++i)
		{
			size_t ty_id = typeid(*components[i]).hash_code();
			locator[ty_id] += offset;
		}
	}
}