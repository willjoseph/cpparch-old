
#include <typeinfo>
// [expr.typeid] The result of a typeid expression is an lvalue of static type const std::type_info

namespace N301
{

	struct S
	{
	};

	const char* q = typeid(S).name() + 1;

	const std::type_info* p = &typeid(S);
}
