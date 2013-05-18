#ifndef INCLUDED_TEST_REDECLARED_PASS_H
#define INCLUDED_TEST_REDECLARED_PASS_H

#include "redeclared_second.h"

namespace Pass
{
	const int TEST_REDECLARED1 = REDECLARED;
	const Redeclared TEST_REDECLARED2 = 0;
	const ForwardDeclared* FORWARD = 0;

	void test()
	{
		redeclared();
	}
}

#endif
