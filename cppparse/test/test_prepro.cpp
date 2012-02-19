namespace N112
{
	// Assume f and g have not yet been defined.
	void h(int);
	template <class T> void f2(T);
	namespace A {
		class X {
			friend void f(X); // A::f(X) is a friend
		};
		// A::f, A::g and A::h are not visible here
		X x;
		void g() { f(x); } // definition of A::g
		void f(X) { /* ... */} // definition of A::f
		void h(int) { /* ... */ } // definition of A::h
		// A::f, A::g and A::h are visible here and known to be friends
	}
}