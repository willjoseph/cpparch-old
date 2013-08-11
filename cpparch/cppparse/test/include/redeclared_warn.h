#ifndef INCLUDED_TEST_REDECLARED_WARN_H
#define INCLUDED_TEST_REDECLARED_WARN_H

namespace Warn
{
	const int TEST_REDECLARED1 = REDECLARED; // using 'REDECLARED' without (in)direct inclusion
	const Redeclared TEST_REDECLARED2 = 0; // using 'Redeclared' without (in)direct inclusion
	const ForwardDeclared* FORWARD = 0; // using 'ForwardDeclared' without (in)direct inclusion

	void test()
	{
		redeclared(); // using 'redeclared' without (in)direct inclusion
	}
}

#endif
