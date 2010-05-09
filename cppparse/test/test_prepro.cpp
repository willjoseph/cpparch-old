
int f(int)
{
	return 0;
}

int a[1];
int* b[1];

void f()
{
            f(
                    (   (   f(0) 
                        >>  a[
                                *(b[f(1)] - f(0))
                            ]
                        >>  f(0)
                        )
                    |   a[
                            *(b[f(1)] - f(0))
                        ]
                    )
				);
}



#if 1
#include <boost/wave/util/interpret_pragma.hpp>
#endif

#if 0
#include <boost/spirit/home/classic/tree/common.hpp>
#endif
