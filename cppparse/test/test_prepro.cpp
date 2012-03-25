
template<class _Ty> inline
void swap(_Ty& _Left, _Ty& _Right)
{	// exchange values stored at _Left and _Right
	_Ty _Tmp = _Left;
	_Left = _Right, _Right = _Tmp;
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
