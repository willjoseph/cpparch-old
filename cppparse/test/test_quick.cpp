
namespace GNamespace
{
	typedef int NType;
	NType nVar;
}

typedef GNamespace::NType GType;

GType gVar = GNamespace::nVar;

GType gFunc(GType param)
{
	typedef GType FType;
	FType lVar;
	if(FType lVar = gVar)
	{
	}
}

class GClass
{
	typedef GType CType;
	GClass(CType param) : mVar(param)
	{
	}
	CType mFunc(CType param)
	{
		typedef CType FType;
		FType fVar = param;
		if(FType cVar = mVar)
		{
			typedef FType LType;
			LType lVar;
		}

		class FClass
		{
			FClass(FType param) : mVar(param)
			{
			}
			FType mFunc(FType param)
			{
				typedef CType FType;
				FType fVar = param;
				if(FType cVar = mVar)
				{
					typedef FType LType;
					LType lVar;
				}
			}
			FType mVar;
		} fVar2;
	}
	CType mVar;

	class MClass
	{
		MClass(CType param) : mVar(param)
		{
		}
		CType mFunc(CType param)
		{
			typedef CType FType;
			FType fVar = param;
			if(FType cVar = mVar)
			{
				typedef FType LType;
				LType lVar;
			}
		}
		CType mVar;
	} mVar2;
} gVar2;
