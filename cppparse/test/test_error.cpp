
void function(int arg, ...)
{
	va_list args;
	(args = p); 
}

void function(void(*)(void));

class A
{
public:
	explicit A()
		: a(0)
	{
	}
	__thiscall ~A()
	{
	}
	A& operator=(const A&)
	{
		return *this;
	}
};
