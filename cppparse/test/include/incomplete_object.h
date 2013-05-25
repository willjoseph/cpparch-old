#ifndef INCLUDED_TEST_INCOMPLETE_OBJECT_H
#define INCLUDED_TEST_INCOMPLETE_OBJECT_H

struct S;
struct U;
namespace Object
{
	// [basic.types] objects shall not be defined to have an incomplete type
	S s; // defines an object with type 'S'
	struct C { S m; }; // defines an object with type 'S'
	void g(S* p) { if(S s = *p) {} } // defines an object with type 'S'
	S ca[1]; // defines an object with type 'array of S'

	U f(); // not a definition
	void f(U s); // not a definition
	void f(U[1]) {}; // equivalent to f(U*);

	extern U ia[][1]; // declares an object with type 'incomplete array of array U S'

	typedef U F(); // note if 'F' is passed as a template argument, it cannot be used as the type of a member in a class-template
	F fd; // not a definition, declares a function!

	U* upf; // defines an object with type 'pointer-to U'
	U (*pf)(); // defines an object with type 'pointer-to function-returning U'

	extern U u; // not a definition

	template<typename T>
	struct W
	{
		typedef T Type;
		static Type m; // type of 'm' is dependent
	};

	template<typename T>
	typename W<T>::Type W<T>::m; // type of 'm' is dependent

	S* pm = &W<S>::m; // causes instantiation of W<S>, defining 'm'
}

#endif
