

struct S
{
};

#include "include/incomplete_sizeof.h"
#include "include/incomplete_sizeoftype.h"


// [basic.types] objects shall not be declared to have an incomplete type
S s;

// [expr.add] [pointer arithmetic requires completely defined object type]
S* q = p + 1;
S* q = p - 1;
ptrdiff_t d = p - q;

// [basic.lval] Rvalues shall always have complete types
// [basic.lval] Other builtin operators yield rvalues, and some expect them [overloaded operators require complete type]
// [basic.lval] Whenever an lvalue appears in a context where an rvalue is expected, the lvalue is converted to an rvalue

// [basic.lval] The result of calling a function that does not return a reference is an rvalue
S f();
void g() { f(); }

// [expr.type.conv] A simple-type-specifier followed by a parenthesized expression-list constructs a value of the specified type.... If the simple type specifier specifies a class type, the class type shall be complete.
const S& s = S(); // explicit type conversion

// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
const S& s = S(0); // function-style-cast creates a temporary
const S& s = (S)0; // C-style-cast creates a temporary
const S& s = static_cast<S>(0); // static-cast creates a temporary

// [conv.lval] An lvalue of a non-function, non-array type T can be converted to an rvalue. If T is an incomplete type, a program that necessitates this conversion is ill formed.

// [expr.ref] [the type of the object-expression shall be complete]
void f(S* p) { p->m = 0; }
void f(S& p) { p.m = 0; }

// [expr.call] When a function is called, the parameters that have object type shall have completely-defined object type.
void f(S s);
void g() { f(*p); }

// [expr.unary.op] The address of an object of incomplete type can be taken, but if the complete type of that object is a class type that declares operator&() as a member function, then the behaviour is undefined.
S* q = &(*p); // should warn if S is incomplete?

// [expr.delete] If the object being deleted has incomplete class type at the point of deletion and the complete class has a non-trivial destructor or a deallocation function, the behaviour is undefined.
void f() { delete p; } // should warn!

// [expr.cast] (possible unexpected behaviour if c-style-casting to/from pointer to incomplete type.)
typedef struct X* P;
P x = (P)p; // should warn!
P y = P(p); // should warn!

// [dcl.fct] The type of the parameter or the return type for a function that is not a definition may be an incomplete class type.
void f(S) {}
S f() {}

// [class.derived] The class-name in a base-specifier shall not be an incompletely defined class.
struct X : S {};

// [class.virtual] (if return type of overriding virtual function is not identical to overridden function, it is covariant: return types cannot be incomplete)
struct B { virtual S* f(); };
struct D : B { virtual X* f(); };

// [except.throw] The type of the throw-expression shall not be an incomplete type, or a pointer or reference to an incomplete type [other than void]
void f(S* p) { throw p; }
void f(S* p) { throw *p; }
void f(S& r) { throw r; }

// [except.handle] The exception-declaration shall not denote an incomplete type. The exception-declaration shall not denote a pointer or reference to an incomplete type.
void f() { try {} catch(S) {} }
void f() { try {} catch(S*) {} }
void f() { try {} catch(S&) {} }

// [except.spec] A type denoted in an exception-specification shall not denote an incomplete type. A type denoted in an exception-specification shall not denote a pointer or reference to an incomplete type
void f() throw(S);
void f() throw(S*);
void f() throw(S&);

