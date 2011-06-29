
#if 1
// size_t

#if Wp64
typedef __w64 unsigned int size_t;
#else
typedef unsigned int size_t;
#endif


// New and Delete Operators (C++ only)
extern __declspec(unmanaged) void * __cdecl operator new(size_t);
extern __declspec(unmanaged) void __cdecl operator delete(void *);

// Exception Handling (C++ only)
extern "C" __declspec(unmanaged) int __cdecl atexit(void (__cdecl *)(void));

#pragma pack(push, ehdata, 4)

typedef struct _PMD
{
	int mdisp;
	int pdisp;
	int vdisp;
} _PMD;

typedef void (*_PMFN)(void);

#pragma warning(disable:4200)
#pragma pack(push, _TypeDescriptor, 8)
typedef struct _TypeDescriptor
{
	const void * pVFTable;
	void * spare;
	char name[];
} _TypeDescriptor;
#pragma pack(pop, _TypeDescriptor)
#pragma warning(default:4200)

typedef const struct _s__CatchableType {
	unsigned int properties;
	_TypeDescriptor *pType;
	_PMD thisDisplacement;
	int sizeOrOffset;
	_PMFN copyFunction;
} _CatchableType;

#pragma warning(disable:4200)
typedef const struct _s__CatchableTypeArray {
	int nCatchableTypes;
	_CatchableType *arrayOfCatchableTypes[];
} _CatchableTypeArray;
#pragma warning(default:4200)

typedef const struct _s__ThrowInfo {
	unsigned int attributes;
	_PMFN pmfnUnwind;
	int (__cdecl*pForwardCompat)(...);
	_CatchableTypeArray *pCatachableTypeArray;
} _ThrowInfo;

__declspec (noreturn) extern "C" void __stdcall _CxxThrowException(void* pExceptionObject, _ThrowInfo* pThrowInfo);
extern "C" int __cdecl __CxxExceptionFilter(void*, void*, int, void *);
extern "C" int __cdecl __CxxRegisterExceptionObject(void *exception, void *storage);
extern "C" int __cdecl __CxxDetectRethrow(void *exception);
extern "C" int __cdecl __CxxQueryExceptionSize(void);
extern "C" void __cdecl __CxxUnregisterExceptionObject(void *storage, int rethrow);

#pragma pack(pop, ehdata)

// Run-Time Type Information (C++ only)
#pragma pack(push, rttidata, 4)

typedef const struct _s__RTTIBaseClassDescriptor {
	_TypeDescriptor *pTypeDescriptor;
	unsigned long numContainedBases;
	_PMD where;
	unsigned long attributes;
} __RTTIBaseClassDescriptor;

#pragma warning(disable:4200)
typedef const struct _s__RTTIBaseClassArray {
	__RTTIBaseClassDescriptor *arrayOfBaseClassDescriptors[];
} __RTTIBaseClassArray;
#pragma warning(default:4200)

typedef const struct _s__RTTIClassHierarchyDescriptor {
	unsigned long signature;
	unsigned long attributes;
	unsigned long numBaseClasses;
	__RTTIBaseClassArray *pBaseClassArray;
} __RTTIClassHierarchyDescriptor;

typedef const struct _s__RTTICompleteObjectLocator {
	unsigned long signature;
	unsigned long offset;
	unsigned long cdOffset;
	_TypeDescriptor *pTypeDescriptor;
	__RTTIClassHierarchyDescriptor *pClassDescriptor;
} __RTTICompleteObjectLocator;

typedef const class type_info &__RTtypeidReturnType;

extern "C" void* __cdecl __RTDynamicCast (
	void*,
	long,
	void*,
	void*,
	int) throw (...);
extern "C" void* __cdecl __RTtypeid (void*) throw (...);
extern "C" void* __cdecl __RTCastToVoid (void*) throw (...);

#pragma pack(pop, rttidata)



// GUID (C++ only)
struct __s_GUID {
	unsigned long Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
};

typedef const struct _GUID &__rcGUID_t;

// Intrinsics
extern "C"
__declspec(unmanaged) void __cdecl __debugbreak(void);
extern "C"

#if defined(_NATIVE_WCHAR_T_DEFINED)
__declspec(unmanaged) void __cdecl __annotation(const wchar_t *, ...);
#else
__declspec(unmanaged) void __cdecl __annotation(const unsigned short *, ...);
#endif

#endif


#include _CPPP_SRC
