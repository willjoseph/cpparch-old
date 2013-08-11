#ifndef INCLUDED_TEST_REDECLARED_LOCAL_H
#define INCLUDED_TEST_REDECLARED_LOCAL_H

struct ForwardDeclared;
extern const int REDECLARED;
typedef int Redeclared;
void redeclared();

namespace Local
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
