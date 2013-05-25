
struct Base
{
};

struct S : Base
{
	S();
	S(int);

	int m;

	operator bool();
};

struct ParserContext;


int a[1];

#include "include/incomplete_object.h"
#include "include/incomplete_member.h"
#include "include/incomplete_sizeof.h"
#include "include/incomplete_sizeoftype.h"
#include "include/incomplete_cast.h"
#include "include/incomplete_new.h"
#include "include/incomplete_ptr_add.h"
#include "include/incomplete_ptr_diff.h"
#include "include/incomplete_pre_incr.h"
#include "include/incomplete_post_incr.h"
#include "include/incomplete_assign_add.h"

extern int a[];
namespace SizeofArray
{
	int size = sizeof(a);
}

namespace RvalueConversion
{
	// [basic.lval] Rvalues shall always have complete types
	// [basic.lval] Other builtin operators yield rvalues, and some expect them [overloaded operators require complete type]
	// [basic.lval] Whenever an lvalue appears in a context where an rvalue is expected, the lvalue is converted to an rvalue
	// [conv.lval] An lvalue of a non-function, non-array type T can be converted to an rvalue. If T is an incomplete type, a program that necessitates this conversion is ill formed.
	// [expr.ass] If the left operand is of class type, the class shall be complete.
	void f(S* p, S* q) { *p = *q; }
}

namespace Return
{
	// [basic.lval] The result of calling a function that does not return a reference is an rvalue
	S f();
	void g() { f(); }
}

namespace Construct
{
	// [expr.type.conv] A simple-type-specifier followed by a parenthesized expression-list constructs a value of the specified type.... If the simple type specifier specifies a class type, the class type shall be complete.
	const S& s = S(); // explicit type conversion
}

namespace Parameter
{
	// [expr.call] When a function is called, the parameters that have object type shall have completely-defined object type.
	void f(S s);
	void g(S* p) { f(*p); }
}

namespace Address
{
	// [expr.unary.op] The address of an object of incomplete type can be taken, but if the complete type of that object is a class type that declares operator&() as a member function, then the behaviour is undefined.
	S* p;
	S* q = &(*p); // should warn if S is incomplete?
}

namespace Delete
{
	// [expr.delete] If the object being deleted has incomplete class type at the point of deletion and the complete class has a non-trivial destructor or a deallocation function, the behaviour is undefined.
	void f(S* p) { delete p; } // should warn!
}

namespace Cast
{
	// [expr.cast] (possible unexpected behaviour if c-style-casting to/from pointer to incomplete type.)
	S* p;
	typedef struct B* P;
	P x = (P)p; // should warn!
	P y = P(p); // should warn!
}

namespace Definition
{
	// [dcl.fct] The type of the parameter or the return type for a function that is not a definition may be an incomplete class type.
	void f(S) {}
}

namespace BaseSpec
{
	// [class.derived] The class-name in a base-specifier shall not be an incompletely defined class.
	struct X : S {};
}

namespace Covariant
{
	// [class.virtual] (if return type of overriding virtual function is not identical to overridden function, it is covariant: return types cannot be incomplete)
	struct B { virtual Base* f(); };
	struct D : B { virtual S* f(); };
}

namespace Throw
{
	// [except.throw] The type of the throw-expression shall not be an incomplete type, or a pointer or reference to an incomplete type [other than void]
	void f() { throw S(); }
	void f(S* p) { throw p; }
	void f(S& r) { throw r; }
}

namespace Catch
{
	// [except.handle] The exception-declaration shall not denote an incomplete type. The exception-declaration shall not denote a pointer or reference to an incomplete type.
	void f()
	{
		try {} catch(S) {}
		try {} catch(S*) {}
		try {} catch(S&) {}
	}
}

namespace Spec
{
	// [except.spec] A type denoted in an exception-specification shall not denote an incomplete type. A type denoted in an exception-specification shall not denote a pointer or reference to an incomplete type
	void f1() throw(S);
	void f2() throw(S*);
	void f3() throw(S&);
}

