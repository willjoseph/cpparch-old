#ifndef INCLUDED_TEST_INCOMPLETE_MEMBER_H
#define INCLUDED_TEST_INCOMPLETE_MEMBER_H

struct S;
namespace Member
{
	// [expr.ref] [the type of the object-expression shall be complete]
	void f(S* p) { p->m = 0; }
	void f(S& p) { p.m = 0; }
}

#endif
