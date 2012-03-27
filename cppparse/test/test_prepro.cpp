
void f(int a);

void (*pf(int a)) (int)
{
	int i = a;
	return f;
}


void overloaded(double)
{
}

void overloaded(int)
{
}

void f()
{
	overloaded(1 * 3 * (9 * 8.2));
}




template<typename T>
struct Tmpl
{
};

typedef Tmpl<const struct IncludeDependencyNode*> IncludeDependencyNodes;

struct IncludeDependencyNode : public IncludeDependencyNodes
{
};
