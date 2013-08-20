

#include "semantic.h"
#include "util.h"

#include "profiler.h"
#include "symbols.h"

#include "parser_symbols.h"

#include <iostream>

struct SemanticError
{
	SemanticError()
	{
#ifdef ALLOCATOR_DEBUG
		DEBUG_BREAK();
#endif
	}
};

#define SEMANTIC_ASSERT(condition) if(!(condition)) { throw SemanticError(); }

inline void semanticBreak()
{
}

void printDeclarations(const Scope::Declarations& declarations)
{
	std::cout << "{ ";
	for(Scope::Declarations::const_iterator i = declarations.begin(); i != declarations.end();)
	{
		std::cout << getValue((*i).second->getName());
		if(++i != declarations.end())
		{
			std::cout << ", ";
		}
	}
	std::cout << " }";
}

void printBases(const Types& bases)
{
	std::cout << "{ ";
	for(Types::const_iterator i = bases.begin(); i != bases.end();)
	{
		std::cout << getValue((*i).declaration->getName()) << ": ";
		Scope* scope = (*i).declaration->enclosed;
		if(scope != 0)
		{
			printDeclarations((*i).declaration->enclosed->declarations);
		}
		if(++i != bases.end())
		{
			std::cout << ", ";
		}
	}
	std::cout << " }";
}

void printScope(const Scope& scope)
{
	std::cout << getValue(scope.name) << ": ";
	std::cout << std::endl;
	std::cout << "  declarations: ";
	printDeclarations(scope.declarations);
	std::cout << std::endl;
	std::cout << "  bases: ";
	printBases(scope.bases);
	std::cout << std::endl;
	if(scope.parent != 0)
	{
		printScope(*scope.parent);
	}
}

void printName(const Scope& scope)
{
	if(scope.parent != 0)
	{
		printName(*scope.parent);
		std::cout << "::";
		std::cout << getValue(scope.name);
	}
}


struct IdentifierMismatch
{
	Identifier id;
	const char* expected;
	DeclarationPtr declaration;
	IdentifierMismatch()
	{
	}
	IdentifierMismatch(const Identifier& id, Declaration* declaration, const char* expected) :
		id(id), declaration(declaration), expected(expected)
	{
	}
};

const IdentifierMismatch IDENTIFIERMISMATCH_NULL = IdentifierMismatch(IDENTIFIER_NULL, 0, 0);
IdentifierMismatch gIdentifierMismatch = IDENTIFIERMISMATCH_NULL;

void printIdentifierMismatch(const IdentifierMismatch& e)
{
	printPosition(e.id.source);
	std::cout << "'" << getValue(e.id) << "' expected " << e.expected << ", " << (e.declaration == &gUndeclared ? "was undeclared" : "was declared here:") << std::endl;
	if(e.declaration != &gUndeclared)
	{
		printPosition(e.declaration->getName().source);
		std::cout << std::endl;
	}
}


inline void setDecoration(Identifier* id, const DeclarationInstance& declaration)
{
	SEMANTIC_ASSERT(declaration.name != 0);
	id->dec.p = &declaration;
}

inline bool isReturnTypeEqual(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	SYMBOLS_ASSERT(left.isFunction());
	SYMBOLS_ASSERT(right.isFunction());
	return isEqualInner(left, right);
}

inline bool isEquivalentSpecialization(const Declaration& declaration, const Declaration& other)
{
	return !(isComplete(declaration) && isComplete(other)) // if both are complete, assume that they have different argument lists!
		&& matchTemplateSpecialization(declaration, other.templateArguments);
}

inline bool isEquivalentTypedef(const Declaration& declaration, const Declaration& other)
{
	return getType(declaration) == getType(other);
}

inline bool isEquivalentTemplateParameter(const Type& left, const Type& right)
{
	extern Declaration gParam;
	if((left.declaration->type.declaration == &gParam)
		!= (right.declaration->type.declaration == &gParam))
	{
		return false;
	}
	return left.declaration->type.declaration == &gParam
		? isEqual(left, right)
		: isEqual(left.declaration->type, right.declaration->type);
}

inline bool isEquivalentTemplateParameters(const TemplateParameters& left, const TemplateParameters& right)
{
	if(std::distance(left.begin(), left.end()) != std::distance(right.begin(), right.end()))
	{
		return false;
	}
	TemplateParameters::const_iterator l = left.begin();
	for(TemplateParameters::const_iterator r = right.begin(); r != right.end(); ++l, ++r)
	{
		SYMBOLS_ASSERT(l != left.end());

		if(!isEquivalentTemplateParameter(*l, *r))
		{
			return false;
		}
	}
	SYMBOLS_ASSERT(l == left.end());
	return true;
}

inline bool isEquivalent(const Declaration& declaration, const Declaration& other)
{
	if(isClass(declaration)
		&& isClass(other))
	{
		// TODO: compare template-argument-lists of partial specializations
		return isSpecialization(declaration) == isSpecialization(other)
			&& (!isSpecialization(declaration) // both are not explicit/partial specializations
			|| isEquivalentSpecialization(declaration, other)); // both are specializations and have matching arguments

	}

	if(isEnum(declaration)
		|| isEnum(other))
	{
		return isEquivalentTypedef(declaration, other);
	}

	if(isClass(declaration)
		|| isClass(other))
	{
		return isEquivalentTypedef(declaration, other);
	}

	{
		SEMANTIC_ASSERT(declaration.type.unique != 0);
		SEMANTIC_ASSERT(other.type.unique != 0);
		UniqueTypeWrapper l(declaration.type.unique);
		UniqueTypeWrapper r(other.type.unique);
		if(l.isFunction())
		{
			// 13.2 [over.dcl] Two functions of the same name refer to the same function
			// if they are in the same scope and have equivalent parameter declarations.
			// TODO: also compare template parameter lists: <class, int> is not equivalent to <class, float>
			SYMBOLS_ASSERT(r.isFunction()); // TODO: non-fatal error: 'id' previously declared as non-function, second declaration is a function
			return declaration.isTemplate == other.isTemplate // early out
				&& isEquivalentTemplateParameters(declaration.templateParams, other.templateParams)
				// [over.load] Function declarations that differ only in the return type cannot be overloaded.
				&& (declaration.getName().value == gConversionFunctionId.value
					? isReturnTypeEqual(l, r) // return-types match
					// (only template overloads may differ in return type, return-type is not used to distinguish overloads, except for conversion-function)
					: isEquivalent(getParameterTypes(l.value), getParameterTypes(r.value))); // and parameter-types match
		}
		return true; // redeclaring an object (cannot be overloaded)
	}
	return false;
}

inline const DeclarationInstance* findRedeclared(const Declaration& declaration, const DeclarationInstance* overloaded)
{
	for(const DeclarationInstance* p = overloaded; p != 0; p = p->overloaded)
	{
		if(isEquivalent(declaration, *(*p)))
		{
			return p;
		}
	}
	return 0;
}


struct Overload
{
	const Declaration* declaration;
	const SimpleType* memberEnclosing;
	Overload(const Declaration* declaration, const SimpleType* memberEnclosing)
		: declaration(declaration), memberEnclosing(memberEnclosing)
	{
	}
};

inline bool operator==(const Overload& left, const Overload& right)
{
	return left.declaration == right.declaration
		&& left.memberEnclosing == right.memberEnclosing;
}

typedef std::vector<Overload> OverloadSet;


//-----------------------------------------------------------------------------
// Argument dependent lookup

struct KoenigAssociated
{
	typedef std::vector<Scope*> Namespaces;
	Namespaces namespaces;
	typedef std::vector<const SimpleType*> Classes;
	Classes classes;
};

void addAssociatedNamespace(KoenigAssociated& associated, Scope& scope)
{
	SEMANTIC_ASSERT(scope.type == SCOPETYPE_NAMESPACE);
	if(std::find(associated.namespaces.begin(), associated.namespaces.end(), &scope) == associated.namespaces.end())
	{
		associated.namespaces.push_back(&scope);
	}
}

void addAssociatedClass(KoenigAssociated& associated, const SimpleType& type)
{
	SEMANTIC_ASSERT(isClass(*type.declaration));
	if(std::find(associated.classes.begin(), associated.classes.end(), &type) == associated.classes.end())
	{
		associated.classes.push_back(&type);
	}
}

void addAssociatedEnclosingNamespace(KoenigAssociated& associated, const SimpleType& type)
{
	Scope* scope = getEnclosingNamespace(type.declaration->scope);
	if(scope != 0)
	{
		addAssociatedNamespace(associated, *scope);
	}
}

void addAssociatedClassAndNamespace(KoenigAssociated& associated, const SimpleType& classType)
{
	SEMANTIC_ASSERT(isClass(*classType.declaration));
	addAssociatedClass(associated, classType);
	addAssociatedEnclosingNamespace(associated, classType);
}

void addAssociatedClassRecursive(KoenigAssociated& associated, const SimpleType& classType)
{
	SEMANTIC_ASSERT(isClass(*classType.declaration));
	addAssociatedClassAndNamespace(associated, classType);
	for(UniqueBases::const_iterator i = classType.bases.begin(); i != classType.bases.end(); ++i)
	{
		const SimpleType* base = *i;
		addAssociatedClassRecursive(associated, *base); // TODO: check for cyclic base-class, prevent infinite recursion
	}
}

void addKoenigAssociated(KoenigAssociated& associated, const SimpleType& classType)
{
	if(classType.enclosing != 0)
	{
		addAssociatedClassAndNamespace(associated, *classType.enclosing);
	}
	addAssociatedClassRecursive(associated, classType);
}

void addKoenigAssociated(KoenigAssociated& associated, UniqueTypeWrapper type);

struct KoenigVisitor : TypeElementVisitor
{
	KoenigAssociated& associated;
	KoenigVisitor(KoenigAssociated& associated)
		: associated(associated)
	{
	}
#if 0
	virtual void visit(const Namespace& element)
	{
		SYMBOLS_ASSERT(false);
	}
#endif
	virtual void visit(const DependentType& element) // deduce from T, TT, TT<...>
	{
	}
	virtual void visit(const DependentTypename&)
	{
	}
	virtual void visit(const DependentNonType& element)
	{
	}
	virtual void visit(const TemplateTemplateArgument& element)
	{
		if(element.enclosing != 0)
		{
			addAssociatedClass(associated, *element.enclosing);
		}
	}
	virtual void visit(const NonType&)
	{
	}
	virtual void visit(const SimpleType& element)
	{
		// - If T is a fundamental type, its associated sets of namespaces and classes are both empty.
		if(isClass(*element.declaration))
		{
			// - If T is a class type (including unions), its associated classes are: the class itself; the class of which it is a
			//   member, if any; and its direct and indirect base classes. Its associated namespaces are the namespaces
			//   in which its associated classes are defined.
			addKoenigAssociated(associated, element);
		}
		else if(isEnum(*element.declaration))
		{
			// - If T is an enumeration type, its associated namespace is the namespace in which it is defined. If it is
			//   class member, its associated class is the member’s class; else it has no associated class.
			if(element.enclosing != 0)
			{
				addAssociatedClass(associated, *element.enclosing);
			}
			addAssociatedEnclosingNamespace(associated, element);
		}

		if(element.declaration->isTemplate)
		{
			// - If T is a template-id, its associated namespaces and classes are the namespace in which the template is
			//   defined; for member templates, the member template’s class; the namespaces and classes associated
			//   with the types of the template arguments provided for template type parameters (excluding template
			//   template parameters); the namespaces in which any template template arguments are defined; and the
			//   classes in which any member templates used as template template arguments are defined. [Note: nontype
			//   template arguments do not contribute to the set of associated namespaces. ]
			// TODO: does this apply to function-template id 'f<int>()' ?
			for(TemplateArgumentsInstance::const_iterator i = element.templateArguments.begin(); i != element.templateArguments.end(); ++i)
			{
				addKoenigAssociated(associated, *i);
			}
		}
	}
	// - If T is a pointer to U or an array of U, its associated namespaces and classes are those associated with U.
	virtual void visit(const PointerType&)
	{
	}
	virtual void visit(const ReferenceType&)
	{
	}
	virtual void visit(const ArrayType&)
	{
	}
	virtual void visit(const MemberPointerType& element)
	{
		// - If T is a pointer to a member function of a class X, its associated namespaces and classes are those associated
		//   with the function parameter types and return type, together with those associated with X.
		// - If T is a pointer to a data member of class X, its associated namespaces and classes are those associated
		//   with the member type together with those associated with X.
		addKoenigAssociated(associated, getSimpleType(element.type.value));
	}
	virtual void visit(const FunctionType& element)
	{
		// - If T is a function type, its associated namespaces and classes are those associated with the function
		//   parameter types and those associated with the return type.
		for(ParameterTypes::const_iterator i = element.parameterTypes.begin(); i != element.parameterTypes.end(); ++i)
		{
			addKoenigAssociated(associated, *i);
		}
	}
};


void addKoenigAssociated(KoenigAssociated& associated, UniqueTypeWrapper type)
{
	for(; type != gUniqueTypeNull; type.pop_front())
	{
		KoenigVisitor visitor(associated);
		type.value->accept(visitor);
	}
}

void addUniqueOverload(OverloadSet& result, const Overload& overload)
{
	if(std::find(result.begin(), result.end(), overload) == result.end())
	{
		result.push_back(overload);
	}
}

const SimpleType* findAssociatedClass(const KoenigAssociated& associated, const Declaration& declaration)
{
	SEMANTIC_ASSERT(isClass(declaration))
	for(KoenigAssociated::Classes::const_iterator i = associated.classes.begin(); i != associated.classes.end(); ++i)
	{
		const SimpleType* classType = *i;
		if(classType->declaration == &declaration)
		{
			return classType;
		}
	}
	return 0;
}

void addOverloaded(OverloadSet& result, const DeclarationInstance& declaration, const SimpleType* memberEnclosing)
{
	for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
	{
		if(p->specifiers.isFriend)
		{
			SEMANTIC_ASSERT(memberEnclosing == 0);
			continue; // ignore (namespace-scope) friend functions
		}
		addUniqueOverload(result, Overload(p, memberEnclosing));
	}
}

void addOverloaded(OverloadSet& result, const DeclarationInstance& declaration, const KoenigAssociated& associated = KoenigAssociated())
{
	for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
	{
		const SimpleType* memberEnclosing = 0;
		if(p->specifiers.isFriend)
		{
			Scope* enclosingClass = getEnclosingClass(p->enclosed);
			memberEnclosing = findAssociatedClass(associated, *getDeclaration(enclosingClass->name));
			if(memberEnclosing == 0)
			{
				continue; // friend should only be visible if member of an associated class
			}
		}
		addUniqueOverload(result, Overload(p, memberEnclosing));
	}
}

void argumentDependentLookup(OverloadSet& result, const Identifier& id, const Arguments& arguments)
{
	KoenigAssociated associated;
	// [basic.lookup.koenig]
	// For each argument type T in the function call, there is a set of zero or more associated namespaces and a set
	// of zero or more associated classes to be considered. The sets of namespaces and classes is determined
	// entirely by the types of the function arguments (and the namespace of any template template argument).
	// Typedef names and using-declarations used to specify the types do not contribute to this set.
	for(Arguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
	{
		UniqueTypeWrapper type = (*i).type;
		addKoenigAssociated(associated, type);
	}
	// TODO:
	// In addition, if the argument is the name or address of a set of overloaded functions and/or function templates,
	// its associated classes and namespaces are the union of those associated with each of the members of
	// the set: the namespace in which the function or function template is defined and the classes and namespaces
	// associated with its (non-dependent) parameter types and return type.
	for(KoenigAssociated::Namespaces::const_iterator i = associated.namespaces.begin(); i != associated.namespaces.end(); ++i)
	{
		// TODO: Any namespace-scope friend functions declared in associated classes are visible within their respective
		// namespaces even if they are not visible during an ordinary lookup.
		// All names except those of (possibly overloaded) functions and function templates are ignored.
		if(const DeclarationInstance* p = findDeclaration((*i)->declarations, id, IsFunctionName()))
		{
			const DeclarationInstance& declaration = *p;
			addOverloaded(result, *p, associated);
		}
	}
}

inline void addOverloads(OverloadResolver& resolver, const DeclarationInstance& declaration, const InstantiationContext& context)
{
	for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
	{
		addOverload(resolver, *p, context);
	}
}

inline void addOverloads(OverloadResolver& resolver, const OverloadSet& overloads, const InstantiationContext& context)
{
	for(OverloadSet::const_iterator i = overloads.begin(); i != overloads.end(); ++i)
	{
		const Overload& overload = *i;
		addOverload(resolver, *overload.declaration, setEnclosingType(context, overload.memberEnclosing));
	}
}

inline void printOverloads(OverloadResolver& resolver, const OverloadSet& overloads, const InstantiationContext& context)
{
	for(OverloadSet::const_iterator i = overloads.begin(); i != overloads.end(); ++i)
	{
		const Overload& overload = *i;
		addOverload(resolver, *overload.declaration, setEnclosingType(context, overload.memberEnclosing));
	
		const Declaration* p = overload.declaration;
		ParameterTypes parameters = addOverload(resolver, *p, context);
		printPosition(p->getName().source);
		std::cout << "(";
		bool separator = false;
		for(ParameterTypes::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			if(separator)
			{
				std::cout << ", ";
			}
			printType(*i);
			separator = true;
		}
		std::cout << ")" << std::endl;
	}
}

// source: where the overload resolution occurs (point of instantiation)
// enclosingType: the class of which the declaration is a member (along with all its overloads).
inline FunctionOverload findBestMatch(const OverloadSet& overloads, const TemplateArgumentsInstance* templateArguments, const Arguments& arguments, const InstantiationContext& context)
{
	SEMANTIC_ASSERT(!overloads.empty());
	OverloadResolver resolver(arguments, templateArguments, context);
	addOverloads(resolver, overloads, context);

	if(resolver.ambiguous != 0)
	{
#if 0
		std::cout << "overload resolution failed:" << std::endl;
		std::cout << "  ";
		printPosition(resolver.ambiguous->getName().position);
		printName(resolver.ambiguous);
		std::cout << std::endl;
		if(resolver.best.declaration != 0)
		{
			std::cout << "  ";
			printPosition(resolver.best.declaration->getName().position);
			printName(resolver.best.declaration);
			std::cout << std::endl;
		}
#endif
	}

	if(resolver.get().declaration == 0)
	{
		printPosition(context.source);
		std::cout << "overload resolution failed when matching arguments (";
		bool separator = false;
		for(Arguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
		{
			if(separator)
			{
				std::cout << ", ";
			}
			printType((*i).type);
			separator = true;
		}
		std::cout << ")" << std::endl;
		std::cout << "candidates for ";
		const Declaration* declaration = overloads.front().declaration;
		printName(declaration->scope);
		std::cout << getValue(declaration->getName());
		std::cout << std::endl;
		printOverloads(resolver, overloads, context);
	}

	return resolver.get();
}


inline void addBuiltInOperatorOverload(OverloadResolver& resolver, UniqueTypeWrapper type)
{
	const ParameterTypes& parameters = getParameterTypes(type.value);
	resolver.add(FunctionOverload(&gUnknown, popType(type)), parameters, false, 0);
}

inline void addBuiltInOperatorOverloads(OverloadResolver& resolver, BuiltInTypeArrayRange overloads)
{
	for(const BuiltInType* i = overloads.first; i != overloads.last; ++i)
	{
		BuiltInType overload = *i;
		const ParameterTypes& parameters = getParameterTypes(overload.value);
		if(resolver.arguments.size() != parameters.size())
		{
			continue;
		}
		addBuiltInOperatorOverload(resolver, overload);
	}
}

typedef std::vector<UserType> UserTypeArray;

extern BuiltInTypeArrayRange gIntegralTypesRange;
extern BuiltInTypeArrayRange gPromotedIntegralTypesRange;
extern BuiltInTypeArrayRange gArithmeticTypesRange;
extern BuiltInTypeArrayRange gPromotedArithmeticTypesRange;

inline void addBuiltInTypeConversions(UserTypeArray& conversions, BuiltInTypeArrayRange types)
{
	for(const BuiltInType* i = types.first; i != types.last; ++i)
	{
		conversions.push_back(UserType(*i));
	}
}

template<typename Op>
inline void forEachBase(const SimpleType& classType, Op op)
{
	SYMBOLS_ASSERT(classType.instantiated);
	op(classType);
	for(UniqueBases::const_iterator i = classType.bases.begin(); i != classType.bases.end(); ++i)
	{
		forEachBase(**i, op);
	}
}

template<typename T>
inline void addQualificationPermutations(UserTypeArray& conversions, UniqueTypeWrapper type, const T& pointerType)
{
	conversions.push_back(UserType(pushType(qualifyType(type, CvQualifiers(false, false)), pointerType)));
	conversions.push_back(UserType(pushType(qualifyType(type, CvQualifiers(true, false)), pointerType)));
	conversions.push_back(UserType(pushType(qualifyType(type, CvQualifiers(false, true)), pointerType)));
	conversions.push_back(UserType(pushType(qualifyType(type, CvQualifiers(true, true)), pointerType)));
}

inline void addQualificationPermutations(UserTypeArray& conversions, UniqueTypeWrapper type)
{
	addQualificationPermutations(conversions, type, PointerType());
}


struct AddPointerConversions
{
	UserTypeArray& conversions;
	CvQualifiers qualifiers;
	AddPointerConversions(UserTypeArray& conversions, CvQualifiers qualifiers)
		: conversions(conversions), qualifiers(qualifiers)
	{
	}
	void operator()(const SimpleType& classType) const
	{
		addQualificationPermutations(conversions, qualifyType(makeUniqueSimpleType(classType), qualifiers));
	}
};

struct AddMemberPointerConversions
{
	UserTypeArray& conversions;
	UniqueTypeWrapper type;
	AddMemberPointerConversions(UserTypeArray& conversions, UniqueTypeWrapper type)
		: conversions(conversions), type(type)
	{
	}
	void operator()(const SimpleType& classType) const
	{
		addQualificationPermutations(conversions, type, MemberPointerType(makeUniqueSimpleType(classType)));
	}
};

inline bool isPlaceholder(UniqueTypeWrapper type)
{
	type = removeReference(type);
	return type == gArithmeticPlaceholder
		|| type == gIntegralPlaceholder
		|| type == gPromotedIntegralPlaceholder
		|| type == gPromotedArithmeticPlaceholder
		|| type == gEnumerationPlaceholder
		|| type == gPointerToAnyPlaceholder
		|| type == gPointerToObjectPlaceholder
		|| type == gPointerToClassPlaceholder
		|| type == gPointerToFunctionPlaceholder
		|| type == gPointerToMemberPlaceholder;
}

// to: The placeholder parameter of the built-in operator.
// from: The type of the argument expression after lvalue-to-rvalue conversion, or the type yielded by the best conversion function.
inline void addBuiltInOperatorConversions(UserTypeArray& conversions, UniqueTypeWrapper to, UniqueTypeWrapper from, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(isPlaceholder(removeReference(to)));
	if(to.isReference())
	{
		// built-in operators that have reference parameters are always non-const, so must be an exact match.
		conversions.push_back(UserType(from));
		return;
	}
	if(isPointerPlaceholder(to))
	{
		SYMBOLS_ASSERT(from.isPointer());
		UniqueTypeWrapper type = popType(from);
		CvQualifiers qualifiers = type.value.getQualifiers();
		if(isClass(type)
			&& isComplete(type))
		{
			// the argument type 'pointer to class X' is convertible to a pointer to any base class of X
			const SimpleType& classType = getSimpleType(type.value);
			instantiateClass(classType, context);
			forEachBase(classType, AddPointerConversions(conversions, qualifiers));
		}
		else
		{
			addQualificationPermutations(conversions, type);
		}
		// the argument type 'pointer to X' is convertible to a pointer to void
		addQualificationPermutations(conversions, qualifyType(gVoid, qualifiers));
		return;
	}

	if(to == gPointerToMemberPlaceholder)
	{
		SYMBOLS_ASSERT(from.isMemberPointer());
		UniqueTypeWrapper type = popType(from);
		if(isComplete(from))
		{
			// the argument type 'pointer to member of class X of type Y' is convertible to a pointer to member of any base class of X of type Y
			const SimpleType& classType = getMemberPointerClass(from.value);
			instantiateClass(classType, context);
			forEachBase(classType, AddMemberPointerConversions(conversions, type));
		}
		else
		{
			addQualificationPermutations(conversions, type, getMemberPointerType(from.value));
		}
		return;
	}
	if(to == gEnumerationPlaceholder)
	{
		SYMBOLS_ASSERT(isEnum(from));
		conversions.push_back(UserType(from));
		// the argument type 'enumeration of type E' is also convertible to any arithmetic type 
		// drop through...
	}
	return addBuiltInTypeConversions(conversions, gArithmeticTypesRange);
}

typedef std::vector<UserTypeArray> ConversionPairs;
typedef TypeTuple<false, 1> Permutation1;
typedef TypeTuple<false, 2> Permutation2;
typedef std::vector<Permutation1> Permutation1Array;
typedef std::vector<Permutation2> Permutation2Array;

inline void addBuiltInOperatorPermutations(Permutation1Array& result, ConversionPairs& conversionPairs)
{
	SEMANTIC_ASSERT(!conversionPairs.empty());
	if(conversionPairs.size() == 1) // one argument was not a placeholder, or one argument is a null pointer constant expression
	{
		std::copy(conversionPairs[0].begin(), conversionPairs[0].end(), std::back_inserter(result));
	}
	else
	{
		SEMANTIC_ASSERT(conversionPairs.size() == 2);
		std::sort(conversionPairs[0].begin(), conversionPairs[0].end());
		std::sort(conversionPairs[1].begin(), conversionPairs[1].end());

		// find the union of both sets
		std::set_intersection(conversionPairs[0].begin(), conversionPairs[0].end(), conversionPairs[1].begin(), conversionPairs[1].end(), std::back_inserter(result));
	}
}

inline void addBuiltInOperatorPermutations(Permutation2Array& result, ConversionPairs& conversionPairs)
{
	SEMANTIC_ASSERT(!conversionPairs.empty());
	if(conversionPairs.size() == 1) // one argument was not a placeholder, or one argument is a null pointer constant expression
	{
		return;
	}
	else
	{
		SEMANTIC_ASSERT(conversionPairs.size() == 2);
		for(UserTypeArray::const_iterator i = conversionPairs[0].begin(); i != conversionPairs[0].end(); ++i)
		{
			UserType left = *i;
			for(UserTypeArray::const_iterator i = conversionPairs[0].begin(); i != conversionPairs[0].end(); ++i)
			{
				UserType right = *i;
				result.push_back(Permutation2(left, right));
			}
		}
	}
}

template<int N>
inline void addBuiltInOperatorOverloads(OverloadResolver& resolver, ArrayRange<BuiltInGenericType<N> > overloads)
{
	for(const BuiltInGenericType<N>* i = overloads.first; i != overloads.last; ++i)
	{
		BuiltInGenericType<N> overload = *i;
		const ParameterTypes& parameters = getParameterTypes(overload.value);
		if(resolver.arguments.size() != parameters.size())
		{
			continue;
		}

		ConversionPairs conversionPairs;
		conversionPairs.reserve(2);
		Arguments::const_iterator a = resolver.arguments.begin();
		ParameterTypes::const_iterator p = parameters.begin();
		for(; a != resolver.arguments.end(); ++a, ++p)
		{
			UniqueTypeWrapper to = *p;
			const Argument& from = *a;
			ImplicitConversion conversion = resolver.makeConversion(TargetType(to), from);
			if(!isValid(conversion)) // if the argument could not be converted
			{
				conversionPairs.clear();
				break;
			}
			if(!isPlaceholder(to))
			{
				continue;
			}
			if(isGeneralPointer(TargetType(to))
				&& isIntegral(from.type)) // if this argument is a null pointer constant expression
			{
				continue; // null pointer matches any pointer type, but does not give enough information to add built-in overloads
			}
			conversionPairs.push_back(UserTypeArray());
			UserTypeArray& conversions = conversionPairs.back();
			addBuiltInOperatorConversions(conversions, to, conversion.sequence.matched, resolver.context);
		}

		if(conversionPairs.empty()) // no built-in overloads can be matched by this argument list
		{
			continue;
		}

		typedef TypeTuple<false, N> Permutation;
		typedef std::vector<Permutation> Permutations;
		Permutations permutations;
		addBuiltInOperatorPermutations(permutations, conversionPairs);

		// TODO: limit qualification permutations for pointer / member-pointer to only those with equal or greater cv-qualification than type of argument expression
		for(typename Permutations::const_iterator i = permutations.begin(); i != permutations.end(); ++i)
		{
			Permutation permutation = *i;
			UserType substituted = overload.substitute(permutation);
			addBuiltInOperatorOverload(resolver, substituted);
		}
	}
}

extern BuiltInTypeArrayRange gUnaryPostIncOperatorTypes;
extern BuiltInTypeArrayRange gUnaryPreIncOperatorTypes;
extern BuiltInTypeArrayRange gUnaryArithmeticOperatorTypes;
extern BuiltInTypeArrayRange gUnaryIntegralOperatorTypes;
extern BuiltInTypeArrayRange gBinaryArithmeticOperatorTypes;
extern BuiltInTypeArrayRange gBinaryIntegralOperatorTypes;
extern BuiltInTypeArrayRange gRelationalArithmeticOperatorTypes;
extern BuiltInTypeArrayRange gShiftOperatorTypes;
extern BuiltInTypeArrayRange gAssignArithmeticOperatorTypes;
extern BuiltInTypeArrayRange gAssignIntegralOperatorTypes;
extern BuiltInTypeArrayRange gBinaryLogicalOperatorTypes;
extern BuiltInTypeArrayRange gUnaryLogicalOperatorTypes;

extern BuiltInGenericType1ArrayRange gPointerAddOperatorTypes;
extern BuiltInGenericType1ArrayRange gPointerSubtractOperatorTypes;
extern BuiltInGenericType1ArrayRange gSubscriptOperatorTypes;
extern BuiltInGenericType1ArrayRange gRelationalOperatorTypes;
extern BuiltInGenericType1ArrayRange gEqualityOperatorTypes;
extern BuiltInGenericType2ArrayRange gMemberPointerOperatorTypes;

// TODO:
// the built-in candidates include all of the candidate operator functions defined in 13.6
// that, compared to the given operator,
// - have the same operator name, and
// - accept the same number of operands, and
// - accept operand types to which the given operand or operands can be converted according to
//   13.3.3.1, and
// - do not have the same parameter type list as any non-template non-member candidate.
inline void addBuiltInOperatorOverloads(OverloadResolver& resolver, const Identifier& id)
{
	if(id.value == gOperatorPlusPlusId
		|| id.value == gOperatorMinusMinusId) // TODO: exclude 'bool' overloads for operator--
	{
		addBuiltInOperatorOverloads(resolver, gUnaryPreIncOperatorTypes);
		addBuiltInOperatorOverloads(resolver, gUnaryPostIncOperatorTypes);
	}
	else if(id.value == gOperatorStarId
		|| id.value == gOperatorDivideId)
	{
		addBuiltInOperatorOverloads(resolver, gBinaryArithmeticOperatorTypes);
	}
	else if(id.value == gOperatorPlusId
		|| id.value == gOperatorMinusId)
	{
		addBuiltInOperatorOverloads(resolver, gBinaryArithmeticOperatorTypes);
		addBuiltInOperatorOverloads(resolver, gUnaryArithmeticOperatorTypes); // +x, -x
	}
	else if(id.value == gOperatorComplId)
	{
		addBuiltInOperatorOverloads(resolver, gUnaryIntegralOperatorTypes); // ~x
	}
	else if(id.value == gOperatorLessId
		|| id.value == gOperatorGreaterId
		|| id.value == gOperatorLessEqualId
		|| id.value == gOperatorGreaterEqualId
		|| id.value == gOperatorEqualId
		|| id.value == gOperatorNotEqualId)
	{
		addBuiltInOperatorOverloads(resolver, gRelationalArithmeticOperatorTypes);
	}
	else if(id.value == gOperatorPercentId
		|| id.value == gOperatorAndId
		|| id.value == gOperatorXorId
		|| id.value == gOperatorOrId)
	{
		addBuiltInOperatorOverloads(resolver, gBinaryIntegralOperatorTypes);
	}
	else if(id.value == gOperatorShiftLeftId
		|| id.value == gOperatorShiftRightId)
	{
		addBuiltInOperatorOverloads(resolver, gShiftOperatorTypes);
	}
	else if(id.value == gOperatorAssignId
		|| id.value == gOperatorStarAssignId
		|| id.value == gOperatorDivideAssignId
		|| id.value == gOperatorPlusAssignId
		|| id.value == gOperatorMinusAssignId)
	{
		addBuiltInOperatorOverloads(resolver, gAssignArithmeticOperatorTypes);
	}
	else if(id.value == gOperatorPercentAssignId
		|| id.value == gOperatorShiftLeftAssignId
		|| id.value == gOperatorShiftRightAssignId
		|| id.value == gOperatorAndAssignId
		|| id.value == gOperatorXorAssignId
		|| id.value == gOperatorOrAssignId)
	{
		addBuiltInOperatorOverloads(resolver, gAssignIntegralOperatorTypes);
	}
	else if(id.value == gOperatorAndAndId
		|| id.value == gOperatorOrOrId)
	{
		addBuiltInOperatorOverloads(resolver, gBinaryLogicalOperatorTypes);
	}
	else if(id.value == gOperatorNotId)
	{
		addBuiltInOperatorOverloads(resolver, gUnaryLogicalOperatorTypes);
	}

	if(id.value == gOperatorPlusId)
	{
		addBuiltInOperatorOverloads(resolver, gPointerAddOperatorTypes);
	}
	else if(id.value == gOperatorMinusId)
	{
		addBuiltInOperatorOverloads(resolver, gPointerSubtractOperatorTypes);
	}
	else if(id.value == gOperatorSubscriptId)
	{
		addBuiltInOperatorOverloads(resolver, gSubscriptOperatorTypes);
	}
	else if(id.value == gOperatorLessId
		|| id.value == gOperatorGreaterId
		|| id.value == gOperatorLessEqualId
		|| id.value == gOperatorGreaterEqualId)
	{
		addBuiltInOperatorOverloads(resolver, gRelationalOperatorTypes);
	}
	else if(id.value == gOperatorEqualId
		|| id.value == gOperatorNotEqualId)
	{
		addBuiltInOperatorOverloads(resolver, gEqualityOperatorTypes);
	}
	else if(id.value == gOperatorArrowStarId)
	{
		addBuiltInOperatorOverloads(resolver, gMemberPointerOperatorTypes);
	}
}

inline FunctionOverload findBestOverloadedOperator(const Identifier& id, const Arguments& arguments, const InstantiationContext& context)
{
	Arguments::const_iterator i = arguments.begin();
	UniqueTypeWrapper left = (*i++).type;
	UniqueTypeWrapper right = i == arguments.end() ? gUniqueTypeNull : (*i).type;
	if(!isClass(left) && !isEnumeration(left)
		&& !isClass(right) && !isEnumeration(right)) // if the operand does not have class or enum type
	{
		return FunctionOverload(&gUnknown, gUniqueTypeNull);
	}
	// TODO: lookup for postfix operator++(int)

	// [over.match.oper]
	// If either operand has a type that is a class or an enumeration, a user-defined operator function might be
	// declared that implements this operator or a user-defined conversion can be necessary to convert the operand
	// to a type that is appropriate for a built-in operator. In this case, overload resolution is used to determine
	// which operator function or built-in operator is to be invoked to implement the operator.
	OverloadResolver resolver(arguments, 0, context);

	// For a unary operator @ with an operand of a type whose cv-unqualified version is T1, and for a binary operator
	// @ with a left operand of a type whose cv-unqualified version is T1 and a right operand of a type whose
	// cv-unqualified version is T2, three sets of candidate functions, designated member candidates, non-member
	// candidates and built-in candidates, are constructed as follows:	
	// - If T1 is a class type, the set of member candidates is the result of the qualified lookup of
	//   T1::operator@ (13.3.1.1.1); otherwise, the set of member candidates is empty.
	if(isClass(left)
		&& isComplete(left)) // can only find overloads if class is complete
	{
		const SimpleType& operand = getSimpleType(left.value);
		instantiateClass(operand, context); // searching for overloads requires a complete type
		LookupResultRef declaration = ::findDeclaration(operand, id, IsAny());
		if(declaration != 0)
		{
			const SimpleType* memberEnclosing = findEnclosingType(&operand, declaration->scope); // find the base class which contains the member-declaration
			SEMANTIC_ASSERT(memberEnclosing != 0);
			addOverloads(resolver, declaration, setEnclosingTypeSafe(context, memberEnclosing));
		}
	}
	// - The set of non-member candidates is the result of the unqualified lookup of operator@ in the context
	//   of the expression according to the usual rules for name lookup in unqualified function calls (3.4.2)
	//   except that all member functions are ignored. However, if no operand has a class type, only those nonmember
	//   functions in the lookup set that have a first parameter of type T1 or "reference to (possibly cv-qualified)
	//   T1", when T1 is an enumeration type, or (if there is a right operand) a second parameter of
	//   type T2 or "reference to (possibly cv-qualified) T2", when T2 is an enumeration type, are candidate
	//   functions.
	OverloadSet overloads;
	LookupResultRef declaration = findClassOrNamespaceMemberDeclaration(*context.enclosingScope, id, IsNonMemberName()); // look up non-member candidates in the enclosing scope (ignoring members)
	if(declaration != 0)
	{
		// TODO: ignore non-member candidates if no operand has a class type, unless one or more params has enum type
		addOverloaded(overloads, declaration);
	}
	argumentDependentLookup(overloads, id, arguments);
	addOverloads(resolver, overloads, setEnclosingType(context, 0));

	// TODO: 13.3.1.2: built-in operators for overload resolution
	// These are relevant either when the operand has a user-defined conversion to a non-class type, or is an enum that can be converted to an arithmetic type
	// TODO: ignore built-in overloads that have same signature as a non-member
	addBuiltInOperatorOverloads(resolver, id);

	return resolver.get();
}

inline UniqueTypeWrapper getBuiltInUnaryOperatorReturnType(Name operatorName, UniqueTypeWrapper type)
{
	if(operatorName == gOperatorAndId) // address-of
	{
		UniqueTypeId result = type;
		result.push_front(PointerType()); // produces a non-const pointer
		return result;
	}
	else if(operatorName == gOperatorStarId) // dereference
	{
		UniqueTypeId result = applyLvalueToRvalueConversion(type);
		SEMANTIC_ASSERT(!result.empty());
		// [expr.unary] The unary * operator performs indirection: the expression to which it is applied shall be a pointer to an
		// object type, or a pointer to a function type and the result is an lvalue referring to the object or function to
		// which the expression points.
		SEMANTIC_ASSERT(result.isPointer());
		result.pop_front();
		return result;
	}
	else if(operatorName == gOperatorPlusId
		|| operatorName == gOperatorMinusId)
	{
		if(!isFloating(type))
		{
			// TODO: check type is integral or enumeration
			return promoteToIntegralType(type);
		}
		return type;
	}
	else if(operatorName == gOperatorNotId)
	{
		return gBool;
	}
	else if(operatorName == gOperatorComplId)
	{
		// TODO: check type is integral or enumeration
		return promoteToIntegralType(type);
	}
	SEMANTIC_ASSERT(operatorName == gOperatorPlusPlusId || operatorName == gOperatorMinusMinusId);
	return type;
}

inline UniqueTypeWrapper typeOfUnaryExpression(Name operatorName, Argument operand, const InstantiationContext& context)
{
	Identifier id;
	id.value = operatorName;
	id.source = context.source;

	Arguments arguments(1, operand);
	FunctionOverload overload = findBestOverloadedOperator(id, arguments, context);
	if(overload.declaration == &gUnknown
		|| (overload.declaration == 0
			&& operatorName == gOperatorAndId)) // TODO: unary operator& has no built-in candidates  
	{
		if(operatorName == gOperatorAndId
			&& operand.isQualifiedNonStaticMemberName)
		{
			// [expr.unary.op]
			// The result of the unary & operator is a pointer to its operand. The operand shall be an lvalue or a qualified-id.
			// In the first case, if the type of the expression is "T," the type of the result is "pointer to T." In particular,
			// the address of an object of type "cv T" is "pointer to cv T," with the same cv-qualifiers.
			// For a qualified-id, if the member is a static member of type "T", the type of the result is plain "pointer to T."
			// If the member is a non-static member of class C of type T, the type of the result is "pointer to member of class C of type
			// T."
			UniqueTypeWrapper classType = makeUniqueSimpleType(*getIdExpression(operand).enclosing);
			UniqueTypeWrapper type = operand.type;
			type.push_front(MemberPointerType(classType)); // produces a non-const pointer
			return type;
		}
		else
		{
			return getBuiltInUnaryOperatorReturnType(operatorName, operand.type);
		}
	}
	else
	{
		SEMANTIC_ASSERT(overload.declaration != 0);
		SEMANTIC_ASSERT(overload.type != gUniqueTypeNull);
		return overload.type;
	}
}

inline UniqueTypeWrapper typeOfPostfixOperatorExpression(Name operatorName, Argument operand, const InstantiationContext& context)
{
	Identifier id;
	id.value = operatorName;
	id.source = context.source;

	ExpressionWrapper zero = ExpressionWrapper(makeUniqueExpression(IntegralConstantExpression(gSignedInt, IntegralConstant(0))), true);

	Arguments arguments;
	arguments.push_back(operand);
	arguments.push_back(Argument(zero, gSignedInt));
	FunctionOverload overload = findBestOverloadedOperator(id, arguments, context);
	if(overload.declaration == &gUnknown)
	{
		// [expr.post.incr] The type of the operand shall be an arithmetic type or a pointer to a complete object type.
		// The type of the result is the cv-unqualified version of the type of the operand.
		UniqueTypeWrapper type = operand.type;
		type.value.setQualifiers(CvQualifiers());
		requireCompleteObjectType(type, context);
		return operand.type;
	}
	else
	{
		SEMANTIC_ASSERT(overload.declaration != 0);
		SEMANTIC_ASSERT(overload.type != gUniqueTypeNull);
		return overload.type;
	}
}

typedef UniqueTypeWrapper (*BuiltInBinaryTypeOp)(UniqueTypeWrapper, UniqueTypeWrapper);

template<BuiltInBinaryTypeOp typeOp>
inline UniqueTypeWrapper typeOfBinaryExpression(Name operatorName, Argument left, Argument right, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(left.type != gUniqueTypeNull);
	SYMBOLS_ASSERT(right.type != gUniqueTypeNull);
	Identifier id;
	id.value = operatorName;
	id.source = context.source;
	FunctionOverload overload(&gUnknown, gUniqueTypeNull);
	if(!id.value.empty()) // if the operator can be overloaded
	{
		Arguments arguments;
		arguments.push_back(left);
		arguments.push_back(right);
		overload = findBestOverloadedOperator(id, arguments, context);
	}
	if(overload.declaration == &gUnknown
		|| (overload.declaration == 0 && id.value == gOperatorAssignId)) // TODO: declare implicit assignment operator
	{
		return  typeOp(left.type, right.type);
	}

	SEMANTIC_ASSERT(overload.declaration != 0);
	return overload.type;
}

inline bool isSpecialMember(const Declaration& declaration)
{
	return &declaration == gDestructorInstance.p
		|| &declaration == gCopyAssignmentOperatorInstance.p;
}

inline const SimpleType* getIdExpressionClass(const SimpleType* qualifying, const DeclarationInstance& declaration, const SimpleType* enclosingType)
{
	SEMANTIC_ASSERT(!isSpecialMember(*declaration))

	if(!isMember(*declaration)) // if the declaration is not a class member
	{
		return 0; // the declaration is at namespace-scope, therefore has no enclosing class
	}

	const SimpleType* idEnclosing = qualifying != 0 ? qualifying : enclosingType;

	SYMBOLS_ASSERT(idEnclosing != 0);
	// the identifier may name a type in a base-class of the qualifying type; findEnclosingType resolves this.
	idEnclosing = findEnclosingType(idEnclosing, declaration->scope); // it must be a member of (a base of) the qualifying class: find which one.
	SYMBOLS_ASSERT(idEnclosing != 0);

	return idEnclosing;
}

inline UniqueTypeWrapper typeOfIdExpression(const SimpleType* qualifying, const DeclarationInstance& declaration, const InstantiationContext& context)
{
	if(declaration == gDestructorInstance.p)
	{
		return pushType(gUniqueTypeNull, FunctionType()); // type of destructor is 'function taking no parameter and returning no type'
	}
	else if(declaration == gCopyAssignmentOperatorInstance.p)
	{
		const SimpleType* idEnclosing = qualifying != 0 ? qualifying : context.enclosingType;
		SYMBOLS_ASSERT(idEnclosing != 0);
		return makeCopyAssignmentOperatorType(*idEnclosing);
	}

	const SimpleType* idEnclosing = getIdExpressionClass(qualifying, declaration, context.enclosingType);
	// a member function template may have a type which depends on its template parameters
	return getUniqueType(declaration->type, setEnclosingType(context, idEnclosing), declaration->isTemplate);
}

UniqueTypeWrapper getOverloadedMemberOperatorType(UniqueTypeWrapper operand, const InstantiationContext& context)
{
	const SimpleType& classType = getSimpleType(operand.value);
	SEMANTIC_ASSERT(isClass(*classType.declaration)); // assert that this is a class type
	// [expr.ref] [the type of the operand-expression shall be complete]
	instantiateClass(classType, context); // searching for overloads requires a complete type

	Identifier id;
	id.value = gOperatorArrowId;
	id.source = context.source;

	ExpressionNodeGeneric<ExplicitTypeExpression> transientExpression = ExplicitTypeExpression(operand);
	Arguments arguments(1, Argument(ExpressionWrapper(&transientExpression, false), operand));
	OverloadResolver resolver(arguments, 0, context);

	LookupResultRef declaration = ::findDeclaration(classType, id, IsAny());
	if(declaration != 0)
	{
		const SimpleType* memberEnclosing = findEnclosingType(&classType, declaration->scope); // find the base class which contains the member-declaration
		SEMANTIC_ASSERT(memberEnclosing != 0);
		addOverloads(resolver, declaration, setEnclosingTypeSafe(context, memberEnclosing));
	}

	FunctionOverload result = resolver.get();
	SEMANTIC_ASSERT(result.declaration != 0);
	return result.type;
}

inline const SimpleType& getMemberOperatorType(Argument operand, bool isArrow, const InstantiationContext& context)
{
	UniqueTypeWrapper type = operand.type;
	if(isArrow)
	{
		while(isClass(type))
		{
			type = getOverloadedMemberOperatorType(type, context);
		}
	}

	bool isPointer = type.isPointer();
	SEMANTIC_ASSERT(isPointer == isArrow);
	if(isPointer)
	{
		type.pop_front();
	}
	// the left-hand side is (pointer-to) operand
	SEMANTIC_ASSERT(type.isSimple());
	const SimpleType& result = getSimpleType(type.value);
	SEMANTIC_ASSERT(isClass(*result.declaration)); // assert that this is a class type
	// [expr.ref] [the type of the operand-expression shall be complete]
	instantiateClass(result, context);
	return result;
}

// [class.copy] The implicitly-declared copy assignment operator for class X has the return type X&
inline UniqueTypeWrapper makeCopyAssignmentOperatorType(const SimpleType& classType)
{
	UniqueTypeWrapper type = makeUniqueSimpleType(classType);
	UniqueTypeWrapper parameter = type;
	parameter.value.setQualifiers(CvQualifiers(true, false));
	parameter.push_front(ReferenceType());
	type.push_front(ReferenceType());
	FunctionType function;
	function.parameterTypes.push_back(parameter);
	type.push_front(function);
	return type;
}

inline UniqueTypeWrapper binaryOperatorArithmeticType(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return usualArithmeticConversions(left, right);
}

inline UniqueTypeWrapper binaryOperatorAdditiveType(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	SEMANTIC_ASSERT(left != gUniqueTypeNull);
	SEMANTIC_ASSERT(right != gUniqueTypeNull);
	left = applyLvalueToRvalueConversion(left);
	right = applyLvalueToRvalueConversion(right);

	if(left.isPointer())
	{
		if(isIntegral(right)
			|| isEnumeration(right))
		{
			return left;
		}
		if(right.isPointer())
		{
			return gSignedLongLongInt; // TODO: ptrdiff_t
		}
	}
	return usualArithmeticConversions(left, right);
}

inline UniqueTypeWrapper makePointerCvUnion(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	CvQualifiers qualifiers = left.value.getQualifiers();
	qualifiers.isConst |= right.value.getQualifiers().isConst;
	qualifiers.isVolatile |= right.value.getQualifiers().isVolatile;

	UniqueTypeWrapper result = left;
	if((left.isPointer() && right.isPointer())
		|| (left.isMemberPointer() && right.isMemberPointer()
		&& getMemberPointerType(left.value).type == getMemberPointerType(right.value).type))
	{
		result = makePointerCvUnion(popType(left), popType(right));
		if(left.isPointer())
		{
			result.push_front(PointerType());
		}
		else
		{
			result.push_front(getMemberPointerType(left.value));
		}
	}
	else
	{
		SEMANTIC_ASSERT(left.value.getPointer() == right.value.getPointer()); // TODO: error: pointer types not similar
	}
	result.value.setQualifiers(qualifiers);
	return result;
}

inline UniqueTypeWrapper binaryOperatorPointerType(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	SEMANTIC_ASSERT(left.isPointer() || left.isMemberPointer());
	SEMANTIC_ASSERT(right.isPointer() || right.isMemberPointer());
	UniqueTypeWrapper result = left;
	// if one of the operands has type "pointer to cv1 void", then the other has type "pointer to cv2 T" and the composite
	// pointer type is "pointer to cv12 void", where cv12 is the union of cv1 and cv2.
	if(isVoidPointer(left)
		|| isVoidPointer(right))
	{
		SEMANTIC_ASSERT(left.isPointer() && right.isPointer());
		CvQualifiers qualifiers = left.value.getQualifiers();
		qualifiers.isConst |= right.value.getQualifiers().isConst;
		qualifiers.isVolatile |= right.value.getQualifiers().isVolatile;
		left.value.setQualifiers(qualifiers);
		left.push_front(PointerType());
		return left;
	}
	// Otherwise, the composite pointer type is a pointer type similar (4.4) to the type of one of the operands, with a cv-qualification signature
	// (4.4) that is the union of the cv-qualification signatures of the operand types.
	return makePointerCvUnion(left, right);
}

inline UniqueTypeWrapper getConditionalOperatorType(UniqueTypeWrapper leftType, UniqueTypeWrapper rightType)
{
	SEMANTIC_ASSERT(leftType != gUniqueTypeNull);
	SEMANTIC_ASSERT(rightType != gUniqueTypeNull);
	// [expr.cond]
	// If either the second or the third operand has type (possibly cv-qualified) void, then the lvalue-to-rvalue,
	// array-to-pointer, and function-to-pointer standard conversions are performed on the second and third operands,
	// and one of the following shall hold:
	//  - The second or the third operand (but not both) is a throw-expression; the result is of the type of
	//    the other and is an rvalue.
	//  - Both the second and the third operands have type void; the result is of type void and is an rvalue.
	//    [Note: this includes the case where both operands are throw-expressions.

	// TODO: technically not correct to remove toplevel qualifiers here, as it will change the overload resolution result when later choosing a conversion-function
	leftType.value.setQualifiers(CvQualifiers());
	rightType.value.setQualifiers(CvQualifiers());
	if(leftType == gVoid)
	{
		return rightType;
	}
	if(rightType == gVoid)
	{
		return leftType;
	}
	if(leftType == rightType)
	{
		// If the second and third operands are lvalues and have the same type, the result is of that type and is an lvalue.
		return leftType; // TODO: lvalueness
	}
	// Otherwise, the result is an rvalue.
	if(isClass(leftType) || isClass(rightType))
	{
		SEMANTIC_ASSERT(false); // TODO: user-defined conversions
		return gUniqueTypeNull;
	}
	// Lvalue-to-rvalue (4.1), array-to-pointer (4.2), and function-to-pointer (4.3) standard conversions are performed
	// on the second and third operands. After those conversions, one of the following shall hold:
	UniqueTypeWrapper left = applyLvalueToRvalueConversion(leftType);
	UniqueTypeWrapper right = applyLvalueToRvalueConversion(rightType);
	// - The second and third operands have the same type; the result is of that type.
	if(left == right)
	{
		return left;
	}
	// - The second and third operands have arithmetic or enumeration type; the usual arithmetic conversions
	// 	 are performed to bring them to a common type, and the result is of that type.
	if((isArithmetic(left) || isEnumeration(left))
		&& (isArithmetic(right) || isEnumeration(right)))
	{
		return binaryOperatorArithmeticType(left, right);
	}
	// - The second and third operands have pointer type, or one has pointer type and the other is a null pointer
	// 	 constant; pointer conversions (4.10) and qualification conversions (4.4) are performed to bring them to
	// 	 their composite pointer type (5.9). The result is of the composite pointer type.
	// - The second and third operands have pointer to member type, or one has pointer to member type and the
	// 	 other is a null pointer constant; pointer to member conversions (4.11) and qualification conversions
	// 	 (4.4) are performed to bring them to a common type, whose cv-qualification shall match the cvqualification
	// 	 of either the second or the third operand. The result is of the common type.
	bool leftPointer = left.isPointer() || left.isMemberPointer();
	bool rightPointer = right.isPointer() || right.isMemberPointer();
	SEMANTIC_ASSERT(leftPointer || rightPointer);
	// TODO: assert that other pointer is null-pointer-constant: must be deferred if expression is value-dependent
	if(leftPointer && !right.isPointer())
	{
		return left;
	}
	if(rightPointer && !left.isPointer())
	{
		return right;
	}
	SEMANTIC_ASSERT(leftPointer && rightPointer);
	return binaryOperatorPointerType(left, right);
}


inline UniqueTypeWrapper typeOfSubscriptExpression(Argument left, Argument right, const InstantiationContext& context)
{
	SEMANTIC_ASSERT(left.type != gUniqueTypeNull);
	if(isClass(left.type))
	{
		// [over.sub]
		// operator[] shall be a non-static member function with exactly one parameter.
		SEMANTIC_ASSERT(isComplete(left.type)); // TODO: non-fatal parse error
		const SimpleType& object = getSimpleType(left.type.value);
		instantiateClass(object, context); // searching for overloads requires a complete type
		Identifier tmp;
		tmp.value = gOperatorSubscriptId;
		tmp.source = context.source;
		LookupResultRef declaration = ::findDeclaration(object, tmp, IsAny());
		SEMANTIC_ASSERT(declaration != 0); // TODO: non-fatal error: expected array

		const SimpleType* memberEnclosing = findEnclosingType(&object, declaration->scope); // find the base class which contains the member-declaration
		SEMANTIC_ASSERT(memberEnclosing != 0);

		// The argument list submitted to overload resolution consists of the argument expressions present in the function
		// call syntax preceded by the implied object argument (E).
		Arguments arguments;
		arguments.push_back(left);
		arguments.push_back(right);

		OverloadSet overloads;
		addOverloaded(overloads, declaration, memberEnclosing);
		FunctionOverload overload = findBestMatch(overloads, 0, arguments, setEnclosingTypeSafe(context, memberEnclosing));
		SEMANTIC_ASSERT(overload.declaration != 0);
		return overload.type;
	}

	UniqueTypeWrapper type = left.type;
	SEMANTIC_ASSERT(type.isArray() || type.isPointer()); // TODO: non-fatal error: attempting to dereference non-array/pointer
	type.pop_front(); // dereference left-hand side
	// [expr.sub] The result is an lvalue of type T. The type "T" shall be a completely defined object type.
	requireCompleteObjectType(type, context);
	return type;
}

inline UniqueTypeWrapper typeOfFunctionCallExpression(Argument left, const Arguments& arguments, const InstantiationContext& context)
{
	ExpressionWrapper expression = left;
	UniqueTypeWrapper type = left.type;

	SEMANTIC_ASSERT(expression.p != 0);

	if(isDependentIdExpression(expression)) // if this is an expression of the form 'undeclared-id(args)', the name must be found via ADL 
	{
		SEMANTIC_ASSERT(!arguments.empty()); // check that the argument-list was not empty
		SEMANTIC_ASSERT(getDependentIdExpression(expression).templateArguments.empty()); // cannot be a template-id
		SEMANTIC_ASSERT(getDependentIdExpression(expression).qualifying == gUniqueTypeNull); // cannot be qualified

		Identifier id;
		id.value = getDependentIdExpression(expression).name;
		OverloadSet overloads;
		argumentDependentLookup(overloads, id, arguments);

		SEMANTIC_ASSERT(!overloads.empty()); // check that the declaration was found via ADL

		FunctionOverload overload = findBestMatch(overloads, 0, arguments, context);
		SEMANTIC_ASSERT(overload.declaration != 0);
#if 0 // TODO: find the corresponding declaration-instance for a name found via ADL
		{
			DeclarationInstanceRef instance = findLastDeclaration(getDeclaration(*id), overload.declaration);
			setDecoration(id, instance);
		}
#endif
		return overload.type;
	}

	type = removeReference(type);
	if(isClass(type))
	{
		// [over.call.object]
		// If the primary-expression E in the function call syntax evaluates to a class object of type "cv T", then the set
		// of candidate functions includes at least the function call operators of T. The function call operators of T are
		// obtained by ordinary lookup of the name operator() in the context of (E).operator().
		SEMANTIC_ASSERT(isComplete(type)); // TODO: non-fatal parse error
		const SimpleType& object = getSimpleType(type.value);
		instantiateClass(object, context); // searching for overloads requires a complete type
		Identifier tmp;
		tmp.value = gOperatorFunctionId;
		tmp.source = context.source;
		LookupResultRef declaration = ::findDeclaration(object, tmp, IsAny());
		SEMANTIC_ASSERT(declaration != 0); // TODO: non-fatal error: expected function

		const SimpleType* memberEnclosing = findEnclosingType(&object, declaration->scope); // find the base class which contains the member-declaration
		SEMANTIC_ASSERT(memberEnclosing != 0);

		// The argument list submitted to overload resolution consists of the argument expressions present in the function
		// call syntax preceded by the implied object argument (E).
		Arguments augmentedArguments;
		augmentedArguments.push_back(left);
		augmentedArguments.insert(augmentedArguments.end(), arguments.begin(), arguments.end());

		OverloadSet overloads;
		addOverloaded(overloads, declaration, memberEnclosing);

		SEMANTIC_ASSERT(!overloads.empty());

		FunctionOverload overload = findBestMatch(overloads, 0, augmentedArguments, setEnclosingTypeSafe(context, memberEnclosing));
		SEMANTIC_ASSERT(overload.declaration != 0);
#if 0 // TODO: record which overload was chosen, for dependency-tracking
		{
			DeclarationInstanceRef instance = findLastDeclaration(declaration, overload.declaration);
			setDecoration(&declaration->getName(), instance);
		}
#endif
		return overload.type;
	}

	if(type.isPointer()) // if this is a pointer to function
	{
		type.pop_front();
		SEMANTIC_ASSERT(type.isFunction());
		return popType(type);
	}

	SEMANTIC_ASSERT(type.isFunction());

	bool isClassMemberAccess = isClassMemberAccessExpression(expression);
	bool isNamed = isClassMemberAccess
		|| isIdExpression(expression);

	if(!isNamed) // if the left-hand expression does not contain an (optionally parenthesised) id-expression (and is not a class which supports 'operator()')
	{
		// the call does not require overload resolution
		SEMANTIC_ASSERT(type.isFunction());
		return popType(type); // get the return type: T
	}

	const IdExpression& idExpression = getIdExpression(
		isClassMemberAccess ? getClassMemberAccessExpression(expression).right : expression);
	DeclarationInstanceRef declaration = idExpression.declaration;
	const TemplateArgumentsInstance& templateArguments = idExpression.templateArguments;

	// [over.call.func] Call to named function
	SEMANTIC_ASSERT(declaration.p != 0);

	// if this is a qualified member-function-call, the class type of the object-expression
	const SimpleType* memberClass = isClassMemberAccess ? getObjectExpression(getClassMemberAccessExpression(expression).left).classType : 0;

	if(declaration.p == &gDestructorInstance)
	{
		return gUniqueTypeNull;
	}

	if(declaration.p == &gCopyAssignmentOperatorInstance)
	{
		// [class.copy] If the class definition does not explicitly declare a copy assignment operator, one is declared implicitly.
		// TODO: ignore using-declaration with same id.
		// TODO: check correct lookup behaviour: base-class copy-assign should always be hidden by derived.
		// TODO: correct argument type depending on base class copy-assign declarations.

		// either the call is qualified or 'this' is valid
		SEMANTIC_ASSERT(memberClass != 0 || context.enclosingType != 0);
		SEMANTIC_ASSERT(memberClass == 0 || memberClass != &gDependentSimpleType);

		return popType(type);
	}

	// the identifier names an overloadable function

	SEMANTIC_ASSERT(declaration != &gDependentObject); // the id-expression should not be dependent

	// if this is a member-function-call, the type of the class containing the member
	const SimpleType* memberEnclosing = getIdExpressionClass(idExpression.enclosing, idExpression.declaration, memberClass != 0 ? memberClass : context.enclosingType);

	ExpressionNodeGeneric<ObjectExpression> transientExpression = ObjectExpression(0);
	Arguments augmentedArguments;
	if(isMember(*declaration))
	{
		// either the call is qualified, 'this' is valid, or the member is static
		SEMANTIC_ASSERT(memberClass != 0 || context.enclosingType != 0 || isStatic(*declaration));
		SEMANTIC_ASSERT(memberClass == 0 || memberClass != &gDependentSimpleType);

		const SimpleType& classType = memberClass != 0
			? *memberClass // qualified-function-call (member access expression)
			// unqualified function call
			: context.enclosingType != 0
			// If the keyword 'this' is in scope and refers to the class of that member function, or a derived class thereof,
			// then the function call is transformed into a normalized qualified function call using (*this) as the postfix-expression
			// to the left of the . operator.
			? *context.enclosingType // implicit '(*this).'
			// If the keyword 'this' is not in scope or refers to another class, then name resolution found a static member of some
			// class T. In this case, all overloaded declarations of the function name in T become candidate functions and
			// a contrived object of type T becomes the implied object argument
			: *memberEnclosing;
		transientExpression = ObjectExpression(&classType);
		augmentedArguments.push_back(Argument(ExpressionWrapper(&transientExpression, false), makeUniqueSimpleType(classType)));
	}

	augmentedArguments.insert(augmentedArguments.end(), arguments.begin(), arguments.end());

	OverloadSet overloads;
	addOverloaded(overloads, declaration, memberEnclosing);
	if(!isMember(*declaration))
	{
		// [basic.lookup.koenig]
		// If the ordinary unqualified lookup of the name finds the declaration of a class member function, the associated
		// namespaces and classes are not considered. Otherwise the set of declarations found by the lookup of
		// the function name is the union of the set of declarations found using ordinary unqualified lookup and the set
		// of declarations found in the namespaces and classes associated with the argument types.
		argumentDependentLookup(overloads, declaration->getName(), augmentedArguments);
	}

	SEMANTIC_ASSERT(!overloads.empty());

	// TODO: handle empty template-argument list '<>'. If specified, overload resolution should ignore non-templates
	FunctionOverload overload = findBestMatch(overloads, templateArguments.empty() ? 0 : &templateArguments, augmentedArguments, context);
	SEMANTIC_ASSERT(overload.declaration != 0);
#if 0 // TODO: record which overload was chosen, for dependency-tracking
	{
		// TODO: this will give the wrong result if the declaration was found via ADL and is in a different namespace
		DeclarationInstanceRef instance = findLastDeclaration(declaration, overload.declaration);
		setDecoration(id, instance);
	}
#endif
	SEMANTIC_ASSERT(!::isDependent(overload.type));
	return overload.type;
}



inline bool isIntegralConstant(UniqueTypeWrapper type)
{
	return type.isSimple()
		&& type.value.getQualifiers().isConst
		&& (isIntegral(type)
		|| isEnumeration(type));
}




struct WalkerContext : public TreeAllocator<int>
{
	Scope global;
	Declaration globalDecl;
	TypeRef globalType;
	std::size_t declarationCount;
	UniqueTypeWrapper typeInfoType;

	WalkerContext(const TreeAllocator<int>& allocator) :
		TreeAllocator<int>(allocator),
		global(allocator, gGlobalId, SCOPETYPE_NAMESPACE),
		globalDecl(allocator, 0, gGlobalId, TYPE_NULL, &global),
		globalType(Type(&globalDecl, allocator), allocator),
		declarationCount(0)
	{
	}
};

typedef std::list< DeferredParse<struct WalkerBase, struct WalkerState> > DeferredSymbolsList;

struct DeferredSymbols
{
	DeferredSymbolsList first;
	DeferredSymbolsList second;

	void splice(DeferredSymbols& other)
	{
		first.splice(first.end(), other.first);
		second.splice(second.end(), other.second);
	}
	bool empty() const
	{
		return first.empty() && second.empty();
	}
};


typedef bool (*IdentifierFunc)(const Declaration& declaration);
const char* getIdentifierType(IdentifierFunc func);


struct WalkerState
	: public ContextBase
{
	typedef WalkerState State;

	WalkerContext& context;
	ScopePtr enclosing;
	const SimpleType* enclosingType;
	Dependent enclosingDependent;
	TypePtr qualifying_p;
	DeclarationPtr qualifyingScope;
	const SimpleType* qualifyingClass;
	const SimpleType* memberClass;
	UniqueTypeWrapper memberType;
	ExpressionWrapper objectExpression; // the lefthand side of a class member access expression
	SafePtr<const TemplateParameters> templateParams;
	ScopePtr templateParamScope;
	DeferredSymbols* deferred;
	std::size_t templateDepth;
	bool isExplicitInstantiation;

	WalkerState(WalkerContext& context)
		: context(context)
		, enclosing(0)
		, enclosingType(0)
		, qualifying_p(0)
		, qualifyingScope(0)
		, qualifyingClass(0)
		, memberClass(0)
		, templateParams(0)
		, templateParamScope(0)
		, deferred(0)
		, templateDepth(0)
		, isExplicitInstantiation(false)
	{
	}
	const WalkerState& getState() const
	{ 
		return *this;
	}
	Location getLocation() const
	{
		return Location(parser->get_source(), context.declarationCount);
	}
	InstantiationContext getInstantiationContext() const
	{
		return InstantiationContext(getLocation(), enclosingType, enclosing);
	}

	UniqueTypeWrapper getTypeInfoType()
	{
		if(context.typeInfoType == gUniqueTypeNull)
		{
			Identifier stdId = makeIdentifier(parser->context.makeIdentifier("std"));
			LookupResultRef declaration = ::findDeclaration(context.global, stdId, IsNestedName());
			SEMANTIC_ASSERT(declaration != 0);
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			SEMANTIC_ASSERT(declaration->enclosed->type == SCOPETYPE_NAMESPACE);
			Identifier typeInfoId = makeIdentifier(parser->context.makeIdentifier("type_info"));
			declaration = ::findDeclaration(*declaration->enclosed, typeInfoId);
			SEMANTIC_ASSERT(declaration != 0);
			SEMANTIC_ASSERT(isClass(*declaration));
			Type type(declaration, context);
			context.typeInfoType = makeUniqueType(type, InstantiationContext(Location(), 0, 0), false);
			context.typeInfoType.value.setQualifiers(CvQualifiers(true, false));
		}
		return context.typeInfoType;
	}

	bool objectExpressionIsDependent() const 
	{
		return objectExpression.p != 0
			&& objectExpression.isTypeDependent
			&& memberClass != 0;
	}
	bool allowNameLookup() const
	{
		if(isDependent(qualifying_p))
		{
			return false;
		}
		if(objectExpressionIsDependent())
		{
			return false;
		}
		return true;
	}
	LookupResult lookupQualified(const Identifier& id, bool isDeclarator, LookupFilter filter = IsAny())
	{
		return isDeclarator
			? findDeclaratorDeclaration(id, filter)
			: lookupQualified(id, filter);
	}
	LookupResult findDeclaratorDeclaration(const Identifier& id, LookupFilter filter = IsAny())
	{
		SEMANTIC_ASSERT(getQualifyingScope() != 0);
		LookupResult result;
		if(result.append(::findDeclaration(*getQualifyingScope(), id, filter)))
		{
			return result;
		}
		result.filtered = &gUndeclaredInstance;
		return result;
	}
	LookupResult lookupQualified(const Identifier& id, LookupFilter filter = IsAny())
	{
		SEMANTIC_ASSERT(getQualifyingScope() != 0);
		LookupResult result;
		// [basic.lookup.qual]
		if(qualifyingClass != 0)
		{
			instantiateClass(*qualifyingClass, getInstantiationContext());
			if(result.append(::findDeclaration(*qualifyingClass, id, filter)))
			{
				return result;
			}
		}
		else if(result.append(::findNamespaceDeclaration(*getQualifyingScope(), id, filter)))
		{
#ifdef LOOKUP_DEBUG
			std::cout << "HIT: qualified" << std::endl;
#endif
			return result;
		}
		result.filtered = &gUndeclaredInstance;
		return result;
	}
	LookupResult findDeclaration(const Identifier& id, LookupFilter filter = IsAny(), bool isUnqualifiedId = false)
	{
		ProfileScope profile(gProfileLookup);
#ifdef LOOKUP_DEBUG
		std::cout << "lookup: " << getValue(id) << " (" << getIdentifierType(filter) << ")" << std::endl;
#endif
		LookupResult result;
		if(getQualifyingScope() != 0)
		{
			return lookupQualified(id, filter);
		}
		else
		{
			bool isQualified = objectExpression.p != 0
				&& memberClass != 0;
			SYMBOLS_ASSERT(!(isUnqualifiedId && objectExpression.isTypeDependent)); // in case of unqualified-id, should check allowNameLookup before calling
			if(isQualified
				&& !objectExpression.isTypeDependent)
			{
				// [basic.lookup.classref]
				SYMBOLS_ASSERT(memberClass != &gDependentSimpleType);
				if(result.append(::findDeclaration(*memberClass, id, filter)))
				{
#ifdef LOOKUP_DEBUG
					std::cout << "HIT: member" << std::endl;
#endif
					return result;
				}
				// else if we're parsing a nested-name-specifier prefix, drop through, look up in the current context
			}

			if(!isQualified || !isUnqualifiedId)
			{
				if(templateParamScope != 0)
				{
					// this occurs when looking up template parameters during parse of (but before the point of declaration of) a template class/function, 
					if(result.append(::findDeclaration(*templateParamScope, id, filter)))
					{
#ifdef LOOKUP_DEBUG
						std::cout << "HIT: templateParamScope" << std::endl;
#endif
						return result;
					}
				}
				if(result.append(::findClassOrNamespaceMemberDeclaration(*enclosing, id, filter)))
				{
#ifdef LOOKUP_DEBUG
					std::cout << "HIT: unqualified" << std::endl;
#endif
					return result;
				}
			}
		}
#ifdef LOOKUP_DEBUG
		std::cout << "FAIL" << std::endl;
#endif
		result.filtered = &gUndeclaredInstance;
		return result;
	}

	const DeclarationInstance& pointOfDeclaration(
		const TreeAllocator<int>& allocator,
		Scope* parent,
		Identifier& name,
		const TypeId& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateParameters& params = TEMPLATEPARAMETERS_NULL,
		bool isSpecialization = false,
		const TemplateArguments& arguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = Dependent())
	{
		SEMANTIC_ASSERT(parent != 0);
		SEMANTIC_ASSERT(templateParameter == INDEX_INVALID || ::isTemplate(*parent));
		SEMANTIC_ASSERT(isTemplate || params.empty());
		SEMANTIC_ASSERT(isClassKey(*type.declaration) || !hasTemplateParamDefaults(params)); // 14.1-9: a default template-arguments may be specified in a class template declaration/definition (not for a function or class-member)
		SEMANTIC_ASSERT(!isSpecialization || isTemplate); // only a template can be a specialization
		SEMANTIC_ASSERT(!isTemplate || isSpecialization || !params.empty()); // only a specialization may have an empty template parameter clause <>

		parser->context.allocator.deferredBacktrack(); // flush cached parse-tree

		static size_t uniqueId = 0;

		SEMANTIC_ASSERT(!name.value.empty());
		Declaration declaration(allocator, parent, name, type, enclosed, specifiers, isTemplate, params, isSpecialization, arguments, templateParameter, valueDependent);
		SEMANTIC_ASSERT(!isTemplate || (isClass(declaration) || isFunction(declaration) || declaration.templateParameter != INDEX_INVALID)); // only a class, function or template-parameter can be a template
		declaration.uniqueId = ++uniqueId;
		DeclarationInstance instance;
		const DeclarationInstance* existing = 0;
		if(!isAnonymous(declaration)) // unnamed class/struct/union/enum
		{
			LookupFilter filter = IsAny();
			if(type.declaration == &gCtor)
			{
				filter = IsConstructor(); // find existing constructor declaration
			}
			existing = ::findDeclaration(parent->declarations, name, filter);
		}
		/* 3.4.4-1
		An elaborated-type-specifier (7.1.6.3) may be used to refer to a previously declared class-name or enum-name
		even though the name has been hidden by a non-type declaration (3.3.10).
		*/
		if(existing != 0)
		{
			instance = *existing;
			try
			{
				const Declaration& primary = getPrimaryDeclaration(*instance, declaration);
				if(&primary == instance)
				{
					return *existing;
				}
			}
			catch(DeclarationError& e)
			{
				printPosition(name.source);
				std::cout << "'" << name.value.c_str() << "': " << e.description << std::endl;
				printPosition(instance->getName().source);
				throw SemanticError();
			}

			if(isFunction(declaration)
				&& instance->isTemplateName)
			{
				// quick hack - if any template overload of a function has been declared, all subsequent declarations are template names
				declaration.isTemplateName = true;
			}

			declaration.overloaded = findOverloaded(instance); // the new declaration refers to the existing declaration

			instance.p = 0;
			instance.overloaded = existing;
			instance.redeclared = findRedeclared(declaration, existing);
			if(instance.redeclared != 0)
			{
				instance.p = *instance.redeclared;
				if(isClass(declaration)
					&& declaration.isTemplate)
				{
					TemplateParameters tmp(context);
					tmp.swap(instance->templateParams);
					instance->templateParams = declaration.templateParams;
					if(declaration.isSpecialization) // this is a partial-specialization
					{
						SEMANTIC_ASSERT(!hasTemplateParamDefaults(declaration.templateParams)); // TODO: non-fatal error: partial-specialization may not have default template-arguments
					}
					else
					{
						SEMANTIC_ASSERT(!declaration.templateParams.empty());
						mergeTemplateParamDefaults(*instance, tmp);
					}
				}
				if(isClass(declaration)
					&& isIncomplete(*instance)) // if this class-declaration was previously forward-declared
				{
					instance->enclosed = declaration.enclosed; // complete it
					instance->setName(declaration.getName()); // make this the definition
				}
			}
		}
		if(instance.p == 0)
		{
			instance.p = allocatorNew(context, Declaration());
			instance->swap(declaration);
		}

		instance.name = &name;
		instance.visibility = context.declarationCount++;
		const DeclarationInstance& result = parent->declarations.insert(instance);
		parent->declarationList.push_back(instance);
		return result;
	}

	TreeAllocator<int> getAllocator()
	{
#ifdef TREEALLOCATOR_LINEAR
		return parser->context.allocator;
#else
		return DebugAllocator<int>();
#endif
	}

	void pushScope(Scope* scope)
	{
		SEMANTIC_ASSERT(findScope(enclosing, scope) == 0);
		scope->parent = enclosing;
		enclosing = scope;
	}

	void addBase(Declaration* declaration, const Type& base)
	{
#if 0
		if(getUnderlyingType(base).declaration == declaration)
		{
			return; // TODO: implement template-instantiation, and disallow inheriting from current-instantiation
		}
#endif
		declaration->enclosed->bases.push_front(base);
	}

	Declaration* getDeclaratorQualifying()
	{
		if(qualifying_p == TypePtr(0))
		{
			return 0;
		}
		Declaration* declaration = qualifying_p->declaration;
		if(isNamespace(*declaration))
		{
			return declaration;
		}
		SEMANTIC_ASSERT(isClass(*declaration)); // TODO: non-fatal error: declarator names must not be typedef names
		// only declarator names may be dependent
		if(declaration->isTemplate) // TODO: template partial specialization
		{
			Declaration* specialization = findTemplateSpecialization(declaration, qualifying_p->templateArguments);
			if(specialization != 0)
			{
				return specialization;
			}
			return findPrimaryTemplate(declaration);
		}
		return declaration; 
	}

	Scope* getQualifyingScope()
	{
		if(qualifyingScope == 0)
		{
			return 0;
		}
		SEMANTIC_ASSERT(qualifyingScope->enclosed != 0);
		return qualifyingScope->enclosed;
	}

	void clearQualifying()
	{
		qualifying_p = 0;
		qualifyingScope = 0;
		qualifyingClass = 0;
		memberType = gUniqueTypeNull;
		memberClass = 0;
		objectExpression = ExpressionWrapper();
	}

	const TemplateParameters& getTemplateParams() const
	{
		if(templateParams == 0)
		{
			return TEMPLATEPARAMETERS_NULL;
		}
		return *templateParams;
	}

	void clearTemplateParams()
	{
		templateParamScope = 0;
		templateParams = 0;
	}

	template<typename T>
	void reportIdentifierMismatch(T* symbol, const Identifier& id, Declaration* declaration, const char* expected)
	{
		result = 0;
#if 0
		gIdentifierMismatch = IdentifierMismatch(id, declaration, expected);
#endif
	}

	Scope* getEtsScope() const
	{
		Scope* scope = enclosing;
		for(; !enclosesEts(scope->type); scope = scope->parent)
		{
		}
		return scope;
	}

	Scope* getFriendScope() const
	{
		SEMANTIC_ASSERT(enclosing->type == SCOPETYPE_CLASS);
		Scope* scope = enclosing;
		for(; scope->type != SCOPETYPE_NAMESPACE; scope = scope->parent)
		{
			if(scope->type == SCOPETYPE_LOCAL)
			{
				return enclosing; // friend declaration in a local class lives in class scope
			}
		}
		return scope;
	}

	Scope* getClassScope() const
	{
		return ::getEnclosingClass(enclosing);
	}

	void printScope()
	{
#if 1
		if(templateParamScope != 0)
		{
			std::cout << "template-params:" << std::endl;
			::printScope(*templateParamScope);
		}
#endif
		if(getQualifyingScope() != 0)
		{
			std::cout << "qualifying:" << std::endl;
			::printScope(*getQualifyingScope());
		}
		else
		{
			std::cout << "enclosing:" << std::endl;
			::printScope(*enclosing);
		}
	}


	bool isDependent(Declaration* dependent) const
	{
		return ::isDependentImpl(dependent, enclosing, templateParamScope);
	}
	bool isDependent(const Type& type) const
	{
		return isDependent(type.dependent);
	}
	bool isDependent(const Types& bases) const
	{
		DeclarationPtr dependent(0);
		setDependent(dependent, bases);
		return isDependent(dependent);
	}
	bool isDependent(const TypePtr& qualifying) const
	{
		DeclarationPtr dependent(0);
		setDependent(dependent, qualifying.get());
		return isDependent(dependent);
	}
	bool isDependent(const TemplateArguments& arguments) const
	{
		DeclarationPtr dependent(0);
		setDependent(dependent, arguments);
		return isDependent(dependent);
	}
	bool isDependent(const Dependent& dependent) const
	{
		return isDependent(static_cast<Declaration*>(dependent));
	}
	// the dependent-scope is the outermost template-definition
	void setDependent(DeclarationPtr& dependent, Declaration* candidate) const
	{
		SEMANTIC_ASSERT(dependent == DeclarationPtr(0) || isDependent(dependent));
		if(!isDependent(candidate))
		{
			return;
		}
		SEMANTIC_ASSERT(candidate->scope->type != SCOPETYPE_NAMESPACE);
		if(dependent != 0
			&& findScope(candidate->scope, dependent->scope)) // if the candidate template-parameter's template-definition is within the current dependent-scope
		{
			return; // already dependent on outer template
		}
		dependent = candidate; // the candidate template-parameter is within the current dependent-scope
	}
	void setDependentEnclosingTemplate(DeclarationPtr& dependent, Declaration* enclosingTemplate) const
	{
		if(enclosingTemplate != 0)
		{
			SEMANTIC_ASSERT(enclosingTemplate->isTemplate);
			// 'declaration' is a class that is dependent because it is a (possibly specialized) member of an enclosing template class
			SEMANTIC_ASSERT(enclosingTemplate->isSpecialization || !enclosingTemplate->templateParams.empty());
			if(!enclosingTemplate->templateParams.empty()) // if the enclosing template class is not an explicit specialization
			{
				// depend on the template parameter(s) of the enclosing template class
				setDependent(dependent, enclosingTemplate->templateParams.back().declaration);
			}
		}
	}
	void setDependent(DeclarationPtr& dependent, Declaration& declaration) const
	{
		if(declaration.templateParameter != INDEX_INVALID)
		{
			setDependent(dependent, &declaration);
		}
		else if(declaration.specifiers.isTypedef)
		{
			setDependent(dependent, declaration.type.dependent);
		}
		else if(isClass(declaration)
			&& isComplete(declaration))
		{
			setDependent(dependent, declaration.enclosed->bases);
		}

		setDependentEnclosingTemplate(dependent, findEnclosingClassTemplate(&declaration));

		setDependent(dependent, declaration.valueDependent);
	}
	void setDependent(DeclarationPtr& dependent, const Type* qualifying) const
	{
		if(qualifying == 0)
		{
			return;
		}
		setDependent(dependent, qualifying->dependent);
		setDependent(dependent, qualifying->qualifying.get());
	}
	void setDependent(DeclarationPtr& dependent, const Qualifying& qualifying) const
	{
		setDependent(dependent, qualifying.get());
	}
	void setDependent(DeclarationPtr& dependent, const Types& bases) const
	{
		for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
		{
			setDependent(dependent, (*i).dependent);
		}
	}
	void setDependent(DeclarationPtr& dependent, const TemplateArguments& arguments) const
	{
		for(TemplateArguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
		{
			setDependent(dependent, (*i).type.dependent);
			setDependent(dependent, (*i).valueDependent);
		}
	}
	void setDependent(DeclarationPtr& dependent, const Parameters& parameters) const
	{
		for(Parameters::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			setDependent(dependent, (*i).declaration->type.dependent);
		}
	}
	void setDependent(Type& type, Declaration* declaration) const
	{
		setDependent(type.dependent, declaration);
	}
	void setDependent(Type& type) const
	{
		setDependent(type.dependent, *type.declaration);
	}

	void addDependentName(Dependent& dependent, Declaration* declaration)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, *declaration);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
	void addDependentType(Dependent& dependent, Declaration* declaration)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, declaration->type.dependent);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
	void addDependent(Dependent& dependent, const Type& type)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, type.dependent);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
	void addDependent(Dependent& dependent, Scope* scope)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, scope->bases);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
	void addDependent(Dependent& dependent, Dependent& other)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, other);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
};


inline const char* getIdentifierType(IdentifierFunc func)
{
	if(func == isTypeName)
	{
		return "type-name";
	}
	if(func == isNamespaceName)
	{
		return "namespace-name";
	}
	if(func == isTemplateName)
	{
		return "template-name";
	}
	if(func == isNestedName)
	{
		return "nested-name";
	}
	return "<unknown>";
}


struct WalkerBase : public WalkerState
{
	typedef WalkerBase Base;

	WalkerBase(WalkerContext& context)
		: WalkerState(context)
	{
	}
	WalkerBase(const WalkerState& state)
		: WalkerState(state)
	{
	}
	Scope* newScope(const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
	{
		return allocatorNew(context, Scope(context, name, type));
	}

	template<typename T>
	ExpressionWrapper makeExpression(const T& value, bool isConstant = false, bool isTypeDependent = false, bool isValueDependent = false)
	{
		ExpressionNode* node = isConstant ? makeUniqueExpression(value) : allocatorNew(context, ExpressionNodeGeneric<T>(value));
		return ExpressionWrapper(node, isConstant, isTypeDependent, isValueDependent);
	}

	void disableBacktrack()
	{
		parser->addBacktrackCallback(makeBacktrackErrorCallback());
	}

	// Causes /p declaration to be undeclared when backtracking.
	// In practice this only happens for the declaration in an elaborated-type-specifier.
	void trackDeclaration(const DeclarationInstance& declaration)
	{
		parser->addBacktrackCallback(makeUndeclareCallback(&declaration));
	}

	Declaration* declareClass(Scope* parent, Identifier* id, bool isSpecialization, TemplateArguments& arguments)
	{
		Scope* enclosed = newScope(makeIdentifier("$class"), SCOPETYPE_CLASS);
		DeclarationInstanceRef declaration = pointOfDeclaration(context, parent, id == 0 ? parent->getUniqueName() : *id, TYPE_CLASS, enclosed, DeclSpecifiers(), templateParams != 0, getTemplateParams(), isSpecialization, arguments);
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		if(id != 0)
		{
			setDecoration(id, declaration);
		}
		enclosed->name = declaration->getName();
		return declaration;
	}

	Declaration* declareObject(Scope* parent, Identifier* id, const TypeId& type, Scope* enclosed, DeclSpecifiers specifiers, size_t templateParameter, const Dependent& valueDependent)
	{
		// 7.3.1.2 Namespace member definitions
		// Paragraph 3
		// Every name first declared in a namespace is a member of that namespace. If a friend declaration in a non-local class
		// first declares a class or function (this implies that the name of the class or function is unqualified) the friend
		// class or function is a member of the innermost enclosing namespace.
		if(specifiers.isFriend // is friend
			&& parent == enclosing) // is unqualified
		{
			parent = getFriendScope();
		}

		bool isTemplate = templateParams != 0;
		bool isExplicitSpecialization = isTemplate && templateParams->empty();
		DeclarationInstanceRef declaration = pointOfDeclaration(context, parent, *id, type, enclosed, specifiers, isTemplate, getTemplateParams(), isExplicitSpecialization, TEMPLATEARGUMENTS_NULL, templateParameter, valueDependent); // 3.3.1.1
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		if(id != &gAnonymousId)
		{
			setDecoration(id, declaration);
		}

		if(declaration->templateParamScope == 0)
		{
			declaration->templateParamScope = templateParamScope; // required by findEnclosingType
		}

		// the type of an object is required to be complete
		// a member's type must be instantiated before the point of declaration of the member, to prevent the member being found by name lookup during the instantiation
		SEMANTIC_ASSERT(type.unique != 0);
		UniqueTypeWrapper uniqueType = UniqueTypeWrapper(type.unique);
		// NOTE: these checks must occur after the declaration because an out-of-line definition of a static member is otherwise not known to be static
		if(parent->type == SCOPETYPE_CLASS // just members, for now
			&& !uniqueType.isFunction() // member functions are not instantiated when class is implicitly instantiated
			&& !isStatic(*declaration) // static members are not instantiated when class is implicitly instantiated
			&& type.declaration != &gCtor) // ignore constructor
		{
			SimpleType* enclosingClass = const_cast<SimpleType*>(getEnclosingType(enclosingType));
			std::size_t size = 0;
			if(!type.isDependent)
			{
				if(!(parent->type == SCOPETYPE_CLASS && isStatic(*declaration)) // ignore static member
					&& !specifiers.isTypedef // ignore typedef
					&& (uniqueType.isSimple() || uniqueType.isArray()))
				{
					// TODO: accurate sizeof
					size = requireCompleteObjectType(uniqueType, getInstantiationContext());
				}
			}
			else if(enclosingClass != 0
				&& templateParams == 0) // ignore template member functions, for now
			{
				enclosingClass->children.push_back(uniqueType);
				// TODO: check compliance: the point of instantiation of a type used in a member declaration is the point of declaration of the member
				// .. along with the point of instantiation of types required when naming the member type. e.g. A<T>::B m; B<A<T>::value> m;
				enclosingClass->childLocations.push_back(getLocation());
			}
			if(enclosingClass != 0)
			{
				enclosingClass->size += size;
			}
		}

		return declaration;
	}

	bool declareEts(Type& type, Identifier* forward)
	{
		if(isClassKey(*type.declaration))
		{
			SEMANTIC_ASSERT(forward != 0);
			/* 3.3.1-6
			if the elaborated-type-specifier is used in the decl-specifier-seq or parameter-declaration-clause of a
			function defined in namespace scope, the identifier is declared as a class-name in the namespace that
			contains the declaration; otherwise, except as a friend declaration, the identifier is declared in the
			smallest non-class, non-function-prototype scope that contains the declaration.
			*/
			DeclarationInstanceRef declaration = pointOfDeclaration(context, getEtsScope(), *forward, TYPE_CLASS, 0);
			
			trackDeclaration(declaration);
			setDecoration(forward, declaration);
			type = declaration;
			return true;
		}
		return false;
	}

	bool consumeTemplateParams(const Qualifying& qualifying)
	{
		if(qualifying.empty())
		{
			return false;
		}
		const Type& type = qualifying.back();
		if(!type.declaration->isTemplate) // if the qualifying type is not a template
		{
			return consumeTemplateParams(type.qualifying);
		}
		Declaration* primary = findPrimaryTemplate(type.declaration);
		SEMANTIC_ASSERT(primary->templateParamScope->templateDepth <= templateDepth); // TODO: non-fatal error: not enough template-parameter-clauses in class declaration
		return primary->templateParamScope->templateDepth == templateDepth;
	}

	LookupResultRef lookupTemplate(const Identifier& id, LookupFilter filter)
	{
		if(!isDependent(qualifying_p))
		{
			return LookupResultRef(findDeclaration(id, filter));
		}
		return gDependentTemplateInstance;
	}

	void addDependentOverloads(Dependent& dependent, Declaration* declaration)
	{
		for(Declaration* p = declaration; p != 0; p = p->overloaded)
		{
			setDependent(dependent, p->type.dependent);
		}
	}
	static UniqueTypeWrapper binaryOperatorIntegralType(UniqueTypeWrapper left, UniqueTypeWrapper right)
	{
		SEMANTIC_ASSERT(!isFloating(left));
		SEMANTIC_ASSERT(!isFloating(right));
		return usualArithmeticConversions(left, right);
	}

	template<typename T>
	void makeUniqueTypeImpl(T& type)
	{
		SYMBOLS_ASSERT(type.unique == 0); // type must not be uniqued twice
		type.isDependent = isDependent(type)
			|| objectExpressionIsDependent(); // this occurs when uniquing the dependent type name in a nested name-specifier in a class-member-access expression
		type.unique = makeUniqueType(type, getInstantiationContext(), type.isDependent).value;
	}
	void makeUniqueTypeSafe(Type& type)
	{
		makeUniqueTypeImpl(type);
	}
	void makeUniqueTypeSafe(TypeId& type)
	{
		makeUniqueTypeImpl(type);
	}
	UniqueTypeWrapper getUniqueTypeSafe(const TypeId& type)
	{
		SEMANTIC_ASSERT(type.unique != 0); // type must have previously been uniqued by makeUniqueTypeImpl
		return type.isDependent ? gUniqueTypeNull : UniqueTypeWrapper(type.unique);
	}
};

struct WalkerQualified : public WalkerBase
{
	Qualifying qualifying;
	WalkerQualified(const WalkerState& state)
		: WalkerBase(state), qualifying(context)
	{
	}

	void setQualifyingGlobal()
	{
		SEMANTIC_ASSERT(qualifying.empty());
		qualifying_p = context.globalType.get_ref();
		qualifyingScope = qualifying_p->declaration;
		qualifyingClass = 0;
	}

	void swapQualifying(const Type& type, bool isDeclarator = false)
	{
#if 0 // allow incomplete types as qualifying, for nested-name-specifier in ptr-operator (defining member-function-ptr)
		if(type.declaration->enclosed == 0)
		{
			// TODO
			//printPosition(symbol->id->value.position);
			std::cout << "'" << getValue(type.declaration->name) << "' is incomplete, declared here:" << std::endl;
			printPosition(type.declaration->getName().position);
			throw SemanticError();
		}
#endif
		Qualifying tmp(type, context);
		swapQualifying(tmp, isDeclarator);
	}
	void swapQualifying(Qualifying& other, bool isDeclarator = false)
	{
		qualifying.swap(other);
		qualifying_p = qualifying.get_ref();
		if(isDeclarator)
		{
			qualifyingScope = getDeclaratorQualifying();
		}
		else if(qualifying_p != TypePtr(0))
		{
			Declaration* declaration = qualifying_p->declaration;
			if(isNamespace(*declaration))
			{
				qualifyingScope = declaration;
			}
			else if(isDependent(qualifying_p))
			{
				qualifyingScope = 0;
			}
			else
			{
				qualifyingClass = &getSimpleType(getUniqueType(*qualifying_p, getInstantiationContext(), isDeclarator).value);
				qualifyingScope = qualifyingClass->declaration;
			}
		}
	}
};




#define TREEWALKER_WALK(walker, symbol) SYMBOL_WALK(walker, symbol);
#define TREEWALKER_WALK_SRC(walker, symbol) Source source_ = parser->get_source(); TREEWALKER_WALK(walker, symbol); symbol->source = source_
#define TREEWALKER_LEAF(symbol) TREEWALKER_WALK(*this, symbol)
#define TREEWALKER_LEAF_SRC(symbol) TREEWALKER_WALK_SRC(*this, symbol)

#define TREEWALKER_DEFAULT PARSERCONTEXT_DEFAULT


#define TREEWALKER_WALK_CACHED_DATA(walker, symbol, data) \
	if(!parser->cacheLookup((symbol), (data))) \
	{ \
		CachedSymbols::Key key = parser->context.position; \
		TREEWALKER_WALK(walker, symbol); \
		parser->cacheStore(key, (symbol), (data)); \
	} \
	else \
	{ \
		result = symbol; \
	}

#define TREEWALKER_WALK_CACHED(walker, symbol) TREEWALKER_WALK_CACHED_DATA(walker, symbol, walker)
#define TREEWALKER_LEAF_CACHED(symbol) int _dummy; TREEWALKER_WALK_CACHED_DATA(*this, symbol, _dummy)


bool isUnqualified(cpp::elaborated_type_specifier_default* symbol)
{
	return symbol != 0
		&& symbol->isGlobal.value.empty()
		&& symbol->context.p == 0;
}


struct Walker
{


struct IsHiddenNamespaceName
{
	DeclarationPtr hidingType; // valid if the declaration is hidden by a type name

	IsHiddenNamespaceName()
		: hidingType(0)
	{
	}

	bool operator()(const Declaration& declaration)
	{
		if(isNamespaceName(declaration))
		{
			return true;
		}
		if(hidingType == 0
			&& isTypeName(declaration))
		{
			hidingType = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		return false;
	}
};

struct NamespaceNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	LookupResultRef declaration;
	IsHiddenNamespaceName filter;
	NamespaceNameWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		declaration = findDeclaration(symbol->value, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
		}
		setDecoration(&symbol->value, declaration);
	}
};

struct TemplateArgumentListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateArgument argument;
	TemplateArguments arguments;

	TemplateArgumentListWalker(const WalkerState& state)
		: WalkerBase(state), argument(context), arguments(context)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		argument.type.swap(walker.type);
		argument.source = getLocation();
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.expression.isTemplateArgumentAmbiguity)
		{
			// [temp.arg] In a template argument, an ambiguity between a typeid and an expression is resolved to a typeid
			result = 0; // fail parse, will retry for a type-id
			return;
		}
		addDependent(argument.valueDependent, walker.valueDependent);
		argument.type = &gNonType;
		argument.expression = walker.expression;
		argument.source = getLocation();
	}
	void visit(cpp::template_argument_list* symbol)
	{
		TemplateArgumentListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		arguments.swap(walker.arguments);
		arguments.push_front(walker.argument); // allocates last element first!
	}
};

struct OverloadableOperatorWalker : public WalkerBase
{
	Name name;
	OverloadableOperatorWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	template<typename T>
	void visit(T* symbol)
	{
		TREEWALKER_LEAF(symbol);
		name = getOverloadableOperatorId(symbol);
	}
	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
	}
};

struct OperatorFunctionIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Name name;
	OperatorFunctionIdWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::overloadable_operator* symbol)
	{
		OverloadableOperatorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		name = walker.name;
	}
};

struct TemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	TemplateArguments arguments;
	bool isTemplate; // true if the template-id is preceded by 'template'
	TemplateIdWalker(const WalkerState& state, bool isTemplate = false)
		: WalkerBase(state), id(0), arguments(context), isTemplate(isTemplate)
	{
	}
	template<typename T>
	void verifyTemplateName(T* symbol)
	{
		// [temp.names]
		// After name lookup (3.4) finds that a name is a template-name or that an operator-function-id or a literal-operator-
		// id refers to a set of overloaded functions any member of which is a function template if this is
		// followed by a <, the < is always taken as the delimiter of a template-argument-list and never as the less-than
		// operator.
		if(!isTemplate // if the name is not preceded by 'template'
			&& !isDependent(qualifying_p)) // and the name is not qualified by a dependent type
		{
			if(qualifyingClass == 0
				&& getQualifyingScope() != 0
				&& getQualifyingScope()->type == SCOPETYPE_CLASS) // special case for declarator qualified by type-name
			{
				// TODO: unify lookup for declarator qualified by type name - store dependent qualifyingClass
				return; // don't bother checking for a template-name, the following cannot be 'less-than'.. we hope.
			}

			// search qualifying type/namespace, object-expression and/or enclosing scope depending on context
			LookupResultRef declaration = findDeclaration(symbol->value, IsAny());
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "template-name");
			}
		}
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
		return verifyTemplateName(symbol);
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		Source source = parser->get_source();
		OperatorFunctionIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		symbol->value.value = walker.name;
		symbol->value.source = source;
		id = &symbol->value;
		return verifyTemplateName(symbol);
	}
	void visit(cpp::template_argument_clause* symbol)
	{
		clearQualifying();
		TemplateArgumentListWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		arguments.swap(walker.arguments);
	}
};

struct UnqualifiedIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isIdentifier;
	bool isTemplate;
	UnqualifiedIdWalker(const WalkerState& state, bool isTemplate = false)
		: WalkerBase(state), id(0), arguments(context), isIdentifier(false), isTemplate(isTemplate)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
		isIdentifier = true;
		if(allowNameLookup())
		{
			declaration = findDeclaration(*id, IsAny(), true);
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK_CACHED(walker, symbol);
		if(allowNameLookup())
		{
			LookupResultRef declaration = findDeclaration(*walker.id, IsAny(), true);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			this->declaration = declaration;
		}
		id = walker.id;
		arguments.swap(walker.arguments);
	}
	void visit(cpp::template_id_operator_function* symbol)
	{
		TemplateIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK_CACHED(walker, symbol);
		if(allowNameLookup())
		{
			LookupResultRef declaration = findDeclaration(*walker.id, IsAny(), true);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			this->declaration = declaration;
		}
		id = walker.id;
		arguments.swap(walker.arguments);
	}
	void visit(cpp::operator_function_id* symbol)
	{
		Source source = parser->get_source();
		OperatorFunctionIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		symbol->value.value = walker.name;
		symbol->value.source = source;
		id = &symbol->value;
		if(allowNameLookup())
		{
			declaration = findDeclaration(*id, IsAny(), true);
			if(declaration == &gUndeclared
				&& id->value == gOperatorAssignId)
			{
				// TODO: declare operator= if not already declared
				declaration = gCopyAssignmentOperatorInstance;
			}
		}
	}
	void visit(cpp::conversion_function_id* symbol)
	{
		Source source = parser->get_source();
		TypeIdWalker walker(getState());
#if 0
		// do not look up type-name within qualifying scope: e.g. x.operator T(): T is looked up scope of entire postfix-expression, not within 'x'
		walker.clearQualifying();
#endif
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		symbol->value = gConversionFunctionId;
		symbol->value.source = source;
		id = &symbol->value;
		if(allowNameLookup())
		{
			declaration = findDeclaration(*id, IsAny(), true);
		}
	}
	void visit(cpp::destructor_id* symbol)
	{
		// TODO: can destructor-id be dependent?
		TREEWALKER_LEAF(symbol);
		id = &symbol->name->value;
		if(objectExpression.p == 0)
		{
			// destructor id can only appear in class member access expression
			return reportIdentifierMismatch(symbol, *id, declaration, "class member access expression");
		}
		declaration = gDestructorInstance;
	}
};

struct QualifiedIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isTemplate;
	QualifiedIdWalker(const WalkerState& state)
		: WalkerQualified(state), id(0), arguments(context), isTemplate(false)
	{
	}

	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
	}
	void visit(cpp::qualified_id_suffix* symbol)
	{
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
	}
};

struct IdExpressionWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	/* 14.6.2.2-3
	An id-expression is type-dependent if it contains:
	- an identifier that was declared with a dependent type,
	- a template-id that is dependent,
	- a conversion-function-id that specifies a dependent type,
	- a nested-name-specifier or a qualified-id that names a member of an unknown specialization
	*/
	ExpressionWrapper expression;
	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	Dependent typeDependent;
	Dependent valueDependent;
	bool isIdentifier;
	bool isUndeclared;
	bool isTemplate;
	IdExpressionWalker(const WalkerState& state, bool isTemplate = false)
		: WalkerQualified(state), id(0), arguments(context), isIdentifier(false), isUndeclared(false), isTemplate(isTemplate)
	{
	}
	void visit(cpp::qualified_id_default* symbol)
	{
		// TODO
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		// TODO
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::qualified_id* symbol)
	{
		TREEWALKER_LEAF(symbol);
		// [temp.dep.expr] An id-expression is type-dependent if it contains:- a nested-name-specifier that contains a class-name that names a dependent type
		setDependent(typeDependent, qualifying.get());
		setDependent(valueDependent, qualifying.get()); // it's clearly value-dependent too, because name lookup must be deferred
	}
	void visit(cpp::unqualified_id* symbol)
	{
		// TODO
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
		isIdentifier = walker.isIdentifier;
	}
	bool commit()
	{
		UniqueTypeWrapper qualifyingType = makeUniqueQualifying(qualifying, getInstantiationContext(), isDependent(qualifying.get_ref()));

		TemplateArgumentsInstance templateArguments;
		makeUniqueTemplateArguments(arguments, templateArguments, getInstantiationContext(), isDependent(arguments));

		expression = ExpressionWrapper();

		if(isDependent(typeDependent)
			|| objectExpressionIsDependent())
		{
			setDecoration(id, gDependentObjectInstance);

			expression = makeExpression(DependentIdExpression(id->value, qualifyingType, templateArguments),
				true, // TODO: expression depending on template parameter may or may not be an integral constant expression
				true,
				true
			);
		}
		else if(isIdentifier // the expression is 'identifier'
			&& declaration == &gUndeclared) // the identifier was not previously declared
		{
			// defer name-lookup: this may be the id-expression in a dependent call to named function, to be found by ADL
			isUndeclared = true;
			setDecoration(id, gDependentObjectInstance);

			expression = makeExpression(DependentIdExpression(id->value, gUniqueTypeNull, TemplateArgumentsInstance()));
		}
		else
		{
			SEMANTIC_ASSERT(declaration != 0);

			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				return false;
			}

			// [temp.dep.expr] An id-expression is type-dependent if it contains:- an identifier that was declared with a dependent type
			addDependentType(typeDependent, declaration);
			// [temp.dep.expr] An id-expression is type-dependent if it contains: - a template-id that is dependent
			setDependent(typeDependent, arguments); // the id-expression may have an explicit template argument list
			// [temp.dep.expr] An id-expression is type-dependent if it contains: - an identifier associated by name lookup with one or more declarations declared with a dependent type,
			addDependentOverloads(typeDependent, declaration);

			// [temp.dep.constexpr] An identifier is value-dependent if it is:- a name declared with a dependent type
			addDependentType(valueDependent, declaration);
			// [temp.dep.constexpr] An identifier is value-dependent if it is:- the name of a non-type template parameter,
			// - a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
			addDependentName(valueDependent, declaration); // adds 'declaration' if it names a non-type template-parameter; adds a dependent initializer


			setDecoration(id, declaration);

			SEMANTIC_ASSERT(!isDependent(qualifying.get_ref()));

			const SimpleType* qualifyingClass = qualifyingType == gUniqueTypeNull ? 0 : &getSimpleType(qualifyingType.value);
			SEMANTIC_ASSERT(qualifyingClass == this->qualifyingClass);

			SEMANTIC_ASSERT(declaration->templateParameter == INDEX_INVALID || qualifying.empty()); // template params cannot be qualified
			expression = declaration->templateParameter == INDEX_INVALID
				// TODO: check compliance: id-expression cannot be compared for equivalence unless it names a non-type template-parameter
				? makeExpression(IdExpression(declaration, qualifyingClass, templateArguments), false, isDependent(typeDependent), isDependent(valueDependent))
				: makeExpression(NonTypeTemplateParameter(declaration), true, isDependent(typeDependent), isDependent(valueDependent));

			expression.isNonStaticMemberName = isMember(*declaration) && !isStatic(*declaration);
			expression.isQualifiedNonStaticMemberName = expression.isNonStaticMemberName && qualifyingType != gUniqueTypeNull;
		}
		return true;
	}
};

struct ExplicitTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TypeId type;
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWrapper expression;
	ExplicitTypeExpressionWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context)
	{
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		if(type.declaration == 0)
		{
			type = getFundamentalType(walker.fundamental);
		}
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type);
	}
	void visit(cpp::typename_specifier* symbol)
	{
		TypenameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type);
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		type.swap(walker.type);
		addDependent(typeDependent, type);
	}
	void visit(cpp::new_type* symbol)
	{
		NewTypeWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		addDependent(typeDependent, type);
		addDependent(typeDependent, walker.valueDependent);
		makeUniqueTypeSafe(type);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::cast_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct ArgumentListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Arguments arguments;
	Dependent typeDependent;
	Dependent valueDependent;
	ArgumentListWalker(const WalkerState& state)
		: WalkerBase(state)
	{
		clearQualifying();
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		arguments.push_back(Argument(walker.expression, walker.type));
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct LiteralWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ExpressionWrapper expression;
	LiteralWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::numeric_literal* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::numeric_literal::UNKNOWN) // workaround for boost::wave issue: T_PP_NUMBER exists in final token stream
		{
			symbol->id = isFloatingLiteral(symbol->value.value.c_str()) ? cpp::numeric_literal::FLOATING : cpp::numeric_literal::INTEGER;
		}
		expression = makeUniqueExpression(parseNumericLiteral(symbol));
	}
	void visit(cpp::string_literal* symbol)
	{
		TREEWALKER_LEAF(symbol);
		expression = makeUniqueExpression(IntegralConstantExpression(getStringLiteralType(symbol), IntegralConstant()));
	}
};

#if 0
struct DependentPrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	LookupResultRef declaration;
	IdentifierPtr id;
	Dependent typeDependent;
	DependentPrimaryExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}
	void visit(cpp::id_expression* symbol)
	{
		// this handles the rare case that the unqualified-id in the id-expression names an undeclared identifier
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		if(walker.isIdentifier // expression is 'identifier'
			&& (declaration == &gUndeclared // identifier was not previously declared
				|| (isObject(*declaration) && declaration->scope->type == SCOPETYPE_NAMESPACE))) // identifier was previously declared in namespace-scope
		{
			// defer name-lookup: this identifier may be a dependent-name.
			id = walker.id;
		}
		else
		{
			if(declaration != 0)
			{
				if(declaration == &gUndeclared
					|| !isObject(*declaration))
				{
					return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name");
				}
				setDecoration(walker.id, declaration);
				addDependentType(typeDependent, declaration); // an id-expression is type-dependent if it contains an identifier that was declared with a dependent type
			}
			else if(walker.id != 0)
			{
				setDecoration(walker.id, gDependentObjectInstance);
				if(!walker.qualifying.empty())
				{
					setDependent(typeDependent, walker.qualifying.get());
				}
			}
		}
	}
	void visit(cpp::primary_expression_parenthesis* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
};

// walks an argument-dependent-lookup function-call expression: postfix-expression ( expression-list. )
struct DependentPostfixExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	LookupResultRef declaration;
	IdentifierPtr id;
	Dependent typeDependent;
	DependentPostfixExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}
	void visit(cpp::primary_expression* symbol)
	{
		DependentPrimaryExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::postfix_expression_call* symbol)
	{
		ArgumentListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		if(id != 0)
		{
			if(!isDependent(walker.typeDependent))
			{
				if(declaration != 0)
				{
					if(declaration == &gUndeclared
						|| !isObject(*declaration))
					{
						return reportIdentifierMismatch(symbol, *id, declaration, "object-name");
					}
					addDependentType(typeDependent, declaration);
					setDecoration(id, declaration);
				}
			}
			else
			{
				setDecoration(id, gDependentObjectInstance);
			}
		}
	}
};
#endif

struct PrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	UniqueTypeId type;
	ExpressionWrapper expression;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	TemplateArguments arguments; // only valid when the expression is a (qualified) template-id
	const SimpleType* idEnclosing; // may be valid when the above id-expression is a qualified-id
	Dependent typeDependent;
	Dependent valueDependent;
	bool isUndeclared;
	PrimaryExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0), arguments(context), idEnclosing(0), isUndeclared(false)
	{
	}
	void visit(cpp::literal* symbol)
	{
		LiteralWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		expression = walker.expression;
		type = typeOfExpression(expression, getInstantiationContext());
		SEMANTIC_ASSERT(!type.empty());
	}
	void visit(cpp::id_expression* symbol)
	{
		Source source = parser->get_source();
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(!walker.commit())
		{
			return reportIdentifierMismatch(symbol, *walker.id, walker.declaration, "object-name");
		}
		id = walker.id;
		arguments.swap(walker.arguments);
		type = gUniqueTypeNull;
		LookupResultRef declaration = walker.declaration;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = walker.expression;
		isUndeclared = walker.isUndeclared;

		SEMANTIC_ASSERT(memberClass == 0); // assert that the id-expression is not part of a class-member-access

		if(isUndeclared)
		{
			type = gUniqueTypeNull;
			idEnclosing = 0;
		}
		else if(expression.p != 0
			&& !isDependent(typeDependent))
		{
			UniqueTypeWrapper qualifyingType = makeUniqueQualifying(walker.qualifying, getInstantiationContext());
			const SimpleType* qualifyingClass = qualifyingType == gUniqueTypeNull ? 0 : &getSimpleType(qualifyingType.value);
			type = typeOfIdExpression(qualifyingClass, declaration, getInstantiationContext());
			idEnclosing = isSpecialMember(*declaration) ? 0 : getIdExpressionClass(qualifyingClass, declaration, enclosingType);

			if(!type.isFunction()) // if the id-expression refers to a function, overload resolution depends on the parameter types; defer evaluation of type
			{
				// [expr.const]
				// An integral constant-expression can involve only ... enumerators, const variables or static
				// data members of integral or enumeration types initialized with constant expressions, non-type template
				// parameters of integral or enumeration types
				expression.isConstant |= (isIntegralConstant(type) && declaration->initializer.isConstant); // TODO: determining whether the expression is constant depends on the type of the expression!
			}
		}
	}
	void visit(cpp::primary_expression_parenthesis* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		expression = walker.expression;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::primary_expression_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		SEMANTIC_ASSERT(enclosingType != 0);
		// TODO: cv-qualifiers: change enclosingType to a UniqueType<SimpleType>
		type = UniqueTypeWrapper(pushUniqueType(gUniqueTypes, makeUniqueSimpleType(*enclosingType).value, PointerType()));
		/* 14.6.2.2-2
		'this' is type-dependent if the class type of the enclosing member function is dependent
		*/
		addDependent(typeDependent, enclosingDependent);
		expression = makeExpression(ExplicitTypeExpression(type), false, isDependent(typeDependent));
		setExpressionType(symbol, type);
	}
};

struct PostfixExpressionMemberWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	ExpressionWrapper expression;
	LookupResultRef declaration;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	TemplateArguments arguments; // only used if the identifier is a template-name
	Dependent typeDependent;
	Dependent valueDependent;
	bool isTemplate;
	PostfixExpressionMemberWalker(const WalkerState& state)
		: WalkerQualified(state), id(0), arguments(context), isTemplate(false)
	{
	}
	void visit(cpp::member_operator* symbol)
	{
		TREEWALKER_LEAF(symbol);
		bool isArrow = symbol->id == cpp::member_operator::ARROW;

		memberClass = &gDependentSimpleType;
		SEMANTIC_ASSERT(objectExpression.p != 0);
		if(!objectExpression.isTypeDependent) // if the type of the object expression is not dependent
		{
			// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
			UniqueTypeWrapper operand = removeReference(memberType);
			memberClass = &getMemberOperatorType(Argument(expression, operand), isArrow, getInstantiationContext());

			objectExpression = makeExpression(ObjectExpression(memberClass));
		}
#if 0
		else
		{
			objectExpression = makeExpression(DependentObjectExpression(objectExpression, isArrow));
		}
#endif
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		bool isObjectName = walker.commit();
		SEMANTIC_ASSERT(isObjectName); // TODO: non-fatal error: expected object name
		id = walker.id;
		arguments.swap(walker.arguments);
		declaration = walker.declaration;
		swapQualifying(walker.qualifying);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = walker.expression;
	}
};

struct TypeTraitsIntrinsicWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent valueDependent;
	UniqueTypeWrapper first;
	UniqueTypeWrapper second;
	TypeTraitsIntrinsicWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		// debugging
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		walker.commit();
		addDependent(valueDependent, walker.type);

		UniqueTypeWrapper type = UniqueTypeWrapper(walker.type.unique);
		setExpressionType(symbol, type);

		(first == gUniqueTypeNull ? first : second) = type;
	}
};

struct PostfixExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	UniqueTypeId type;
	ExpressionWrapper expression;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	TemplateArguments arguments; // only valid when the expression is a (qualified) template-id
	const SimpleType* idEnclosing; // may be valid when the above id-expression is a qualified-id
	Dependent typeDependent;
	Dependent valueDependent;
	bool isUndeclared;
	PostfixExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0), arguments(context), idEnclosing(0), isUndeclared(false)
	{
	}
	void clearMemberType()
	{
		memberType = gUniqueTypeNull;
		objectExpression = ExpressionWrapper();
	}
	void updateMemberType()
	{
		memberClass = 0;
		if(type.isFunction())
		{
			memberType = gUniqueTypeNull;
			objectExpression = ExpressionWrapper();
		}
		else
		{
			memberType = type;
			objectExpression = makeExpression(ExplicitTypeExpression(type),
				false, isDependent(typeDependent), isDependent(valueDependent)
			);
			SEMANTIC_ASSERT(objectExpression.isTypeDependent || !::isDependent(type));
		}
	}
	void visit(cpp::primary_expression* symbol)
	{
		PrimaryExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		id = walker.id;
		arguments.swap(walker.arguments);
		idEnclosing = walker.idEnclosing;
		isUndeclared = walker.isUndeclared;
		setExpressionType(symbol, type);
		updateMemberType();
	}
#if 0
	// prefix
	void visit(cpp::postfix_expression_disambiguate* symbol)
	{
		// this is reached only if the lookup of the identifier in the primary-expression failed.
		// TODO: 
		/* 14.6.2-1
		In an expression of the form:
		postfix-expression ( expression-list. )
		where the postfix-expression is an unqualified-id but not a template-id, the unqualified-id denotes a dependent
		name if and only if any of the expressions in the expression-list is a type-dependent expression (
		*/
		DependentPostfixExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: type = &gDependent;
		addDependent(typeDependent, walker.typeDependent);
		updateMemberType();
	}
#endif
	void visit(cpp::postfix_expression_construct* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		type = getUniqueTypeSafe(walker.type);
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, getInstantiationContext());
		expression = makeExpression(CastExpression(type, walker.expression), walker.expression.isConstant, isDependent(typeDependent), false);
		expression.isTemplateArgumentAmbiguity = symbol->args == 0;
		setExpressionType(symbol, type);
		updateMemberType();
	}
	void visit(cpp::postfix_expression_cast* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		type = getUniqueTypeSafe(walker.type);
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, getInstantiationContext());
		if(symbol->op->id != cpp::cast_operator::DYNAMIC)
		{
			Dependent tmp(walker.typeDependent);
			addDependent(valueDependent, tmp);
		}
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = makeExpression(CastExpression(type, walker.expression), walker.expression.isConstant, isDependent(typeDependent), isDependent(valueDependent)); // TODO: can this be value-dependent?
		setExpressionType(symbol, type);
		updateMemberType();
	}
	void visit(cpp::postfix_expression_typeid* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: operand type required to be complete?
		type = getTypeInfoType();
		expression = makeExpression(ExplicitTypeExpression(type));
		updateMemberType();
	}
	void visit(cpp::postfix_expression_typeidtype* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: operand type required to be complete?
		type = getTypeInfoType();
		expression = makeExpression(ExplicitTypeExpression(type));
		updateMemberType();
	}

	// suffix
	void visit(cpp::postfix_expression_subscript* symbol)
	{
		ExpressionWalker walker(getState());
		walker.clearQualifying(); // the expression in [] is looked up in the context of the entire postfix expression
		TREEWALKER_WALK_SRC(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		id = 0; // don't perform overload resolution for a[i](x);
		if(isDependent(typeDependent))
		{
			type = gUniqueTypeNull;
		}
		else
		{
			SEMANTIC_ASSERT(type != gUniqueTypeNull);
			// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
			type = removeReference(type);
			type = typeOfSubscriptExpression(
				Argument(expression, type),
				Argument(walker.expression, walker.type),
				getInstantiationContext());
		}
		expression = makeExpression(SubscriptExpression(expression, walker.expression), false, isDependent(typeDependent), isDependent(valueDependent));
		setExpressionType(symbol, type);
		updateMemberType();
	}
	void visit(cpp::postfix_expression_call* symbol)
	{
		ArgumentListWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		setExpressionType(symbol, type);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		
		if(!isDependent(typeDependent)
			&& expression.isNonStaticMemberName // if the id-expression names a nonstatic member
			&& memberClass == 0) // and this is not a class-member-access expression
		{
			SEMANTIC_ASSERT(enclosingType != 0); // TODO: check that the id-expression is found in the context of a non-static member (i.e. 'this' is valid)
			// [class.mfct.nonstatic] An id-expression (that is not part of a class-member-access expression, and is found in the context of a nonstatic member)
			// that names a nonstatic member is transformed to a class-member-access expression prefixed by (*this)

			addDependent(typeDependent, enclosingDependent);
			// when a nonstatic member name is used in a function call, overload resolution is dependent on the type of the implicit object parameter
		}


		if(isDependent(typeDependent)) // if either the argument list or the id-expression are dependent
			// TODO: check valueDependent too?
		{
			if(id != 0)
			{
				id->dec.deferred = true;
			}
			type = gUniqueTypeNull;
			expression = ExpressionWrapper();
		}
		else
		{
			{ // consistency checking
				SEMANTIC_ASSERT(isUndeclared == isDependentIdExpression(expression));
				SEMANTIC_ASSERT(!isUndeclared || getDependentIdExpression(expression).name == id->value);

				TemplateArgumentsInstance templateArguments;
				makeUniqueTemplateArguments(arguments, templateArguments, getInstantiationContext());

				if(isUndeclared)
				{
					SEMANTIC_ASSERT(templateArguments.empty());
				}
				else
				{
					bool isCallToNamedFunction = expression.p != 0
						&& (isIdExpression(expression)
						|| isClassMemberAccessExpression(expression));
					SEMANTIC_ASSERT((id != 0) == (isCallToNamedFunction || (expression.p != 0 && isNonTypeTemplateParameter(expression))));
					if(!isCallToNamedFunction)
					{
						SEMANTIC_ASSERT(templateArguments.empty());
					}
					else
					{
						bool isCallToNamedMemberFunction = isClassMemberAccessExpression(expression);
						const IdExpression& idExpression = getIdExpression(
							isCallToNamedMemberFunction ? getClassMemberAccessExpression(expression).right : expression);
						SEMANTIC_ASSERT(idExpression.declaration.p == &getDeclaration(*id));
						SEMANTIC_ASSERT(idExpression.templateArguments == templateArguments);

						if(type.isFunction())
						{
							const SimpleType* tmp = isCallToNamedMemberFunction ? getObjectExpression(getClassMemberAccessExpression(expression).left).classType : 0;
							SEMANTIC_ASSERT(memberClass == tmp);

							if(!isSpecialMember(*idExpression.declaration))
							{
								const SimpleType* tmp = getIdExpressionClass(idExpression.enclosing, idExpression.declaration, memberClass != 0 ? memberClass : enclosingType);
								SEMANTIC_ASSERT(idEnclosing == tmp);
							}
						}
					}
				}
			}
			type = typeOfFunctionCallExpression(Argument(expression, type), walker.arguments, getInstantiationContext());
			expression = makeExpression(FunctionCallExpression(expression, walker.arguments), false, isDependent(typeDependent), isDependent(valueDependent));
		}
		// TODO: set of pointers-to-function
		id = 0; // don't perform overload resolution for a(x)(x);
		idEnclosing = 0;
		updateMemberType();
		isUndeclared = false; // for an expression of the form 'undeclared-id(args)'
	}

	void visit(cpp::postfix_expression_member* symbol)
	{
		PostfixExpressionMemberWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		setExpressionType(symbol, type);
		id = walker.id; // perform overload resolution for a.m(x);
		arguments.swap(walker.arguments);
		type = gUniqueTypeNull;
		LookupResultRef declaration = walker.declaration;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);

		expression = ExpressionWrapper();

		if(walker.expression.p != 0
			&& !isDependent(typeDependent))
		{
			SEMANTIC_ASSERT(objectExpression.p != 0);
			SEMANTIC_ASSERT(!objectExpression.isTypeDependent); // the object-expression should not be dependent
			SEMANTIC_ASSERT(walker.memberClass != 0);
			SEMANTIC_ASSERT(walker.memberClass != &gDependentSimpleType);

			// TODO: [expr.ref] inherit const/volatile from object-expression type if member is non-static
			UniqueTypeWrapper qualifyingType = makeUniqueQualifying(walker.qualifying, getInstantiationContext());
			const SimpleType* qualifyingClass = qualifyingType == gUniqueTypeNull ? 0 : &getSimpleType(qualifyingType.value);
			type = typeOfIdExpression(qualifyingClass, declaration, setEnclosingTypeSafe(getInstantiationContext(), walker.memberClass));
			idEnclosing = isSpecialMember(*declaration) ? 0 : getIdExpressionClass(qualifyingClass, declaration, walker.memberClass);

			expression = makeExpression(ClassMemberAccessExpression(walker.objectExpression, walker.expression),
				false, isDependent(typeDependent), isDependent(valueDependent)
			);
		}

		updateMemberType();

		if(type.isFunction())
		{
			// type determination is deferred until overload resolution is complete
			memberClass = walker.memberClass; // store the type of the implied object argument in a qualified function call.
		}
	}
	void visit(cpp::postfix_expression_destructor* symbol)
	{
		TREEWALKER_LEAF_SRC(symbol);
		setExpressionType(symbol, type);
		type = gVoid; // TODO: should this be null-type?
		id = 0;
		expression = ExpressionWrapper();
		// TODO: name-lookup for destructor name
		clearMemberType();
	}
	void visit(cpp::postfix_operator* symbol)
	{
		TREEWALKER_LEAF_SRC(symbol);
		if(isDependent(typeDependent))
		{
			type = gUniqueTypeNull;
			expression = ExpressionWrapper();
		}
		else
		{
			type = removeReference(type);
			type = typeOfPostfixOperatorExpression(
				getOverloadedOperatorId(symbol),
				Argument(expression, type),
				getInstantiationContext());

			expression = makeExpression(PostfixOperatorExpression(getOverloadedOperatorId(symbol), expression));
		}
		setExpressionType(symbol, type);
		id = 0;
		updateMemberType();
	}
	void visit(cpp::postfix_expression_typetraits_unary* symbol)
	{
		TypeTraitsIntrinsicWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		type = gBool;
		UnaryTypeTraitsOp operation = getUnaryTypeTraitsOp(symbol->trait);
		Name name = getTypeTraitName(symbol);
		expression = makeExpression(TypeTraitsUnaryExpression(name, operation, walker.first), true, false, isDependent(valueDependent));
	}
	void visit(cpp::postfix_expression_typetraits_binary* symbol)
	{
		TypeTraitsIntrinsicWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		type = gBool;
		BinaryTypeTraitsOp operation = getBinaryTypeTraitsOp(symbol->trait);
		Name name = getTypeTraitName(symbol);
		expression = makeExpression(TypeTraitsBinaryExpression(name, operation, walker.first, walker.second), true, false, isDependent(valueDependent));
	}
};

struct SizeofTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent valueDependent;
	ExpressionWrapper expression;
	SizeofTypeExpressionWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		walker.commit();
		// [temp.dep.expr] Expressions of the following form [sizeof(T)] are never type-dependent (because the type of the expression cannot be dependent)
		// [temp.dep.constexpr] Expressions of the following form [sizeof(T)] are value-dependent if ... the type-id is dependent
		addDependent(valueDependent, walker.type);

		UniqueTypeId type = getUniqueTypeSafe(walker.type);
		setExpressionType(symbol, type);

		expression = makeExpression(SizeofTypeExpression(type), true, false, isDependent(valueDependent));
	}
};

struct ConditionalExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWrapper left;
	ExpressionWrapper right;
	UniqueTypeWrapper leftType;
	UniqueTypeWrapper rightType;
	ConditionalExpressionWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		left = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		leftType = walker.type;
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		right = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		rightType = walker.type;
	}
};

struct ExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	UniqueTypeId type;
	ExpressionWrapper expression;
	/* 14.6.2.2-1
	...an expression is type-dependent if any subexpression is type-dependent.
	*/
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}

	// this path handles the right-hand side of a binary expression
	// it is assumed that 'type' already contains the type of the left-hand side
	template<BuiltInBinaryTypeOp typeOp, typename T>
	void walkBinaryExpression(T*& symbol)
	{
		// TODO: SEMANTIC_ASSERT(walker.type.declaration != 0);
		ExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		// TODO: SEMANTIC_ASSERT(type.declaration != 0 && walker.type.declaration != 0);
		BinaryIceOp iceOp = getBinaryIceOp(symbol);
		ExpressionWrapper leftExpression = expression;
		expression = makeExpression(BinaryExpression(getOverloadedOperatorId(symbol), iceOp, typeOfBinaryExpression<typeOp>, expression, walker.expression),
			expression.isConstant && walker.expression.isConstant && iceOp != 0,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		if(!expression.isTypeDependent)
		{
			UniqueTypeWrapper left = removeReference(type);
			UniqueTypeWrapper right = removeReference(walker.type);
			type = typeOfBinaryExpression<typeOp>(getOverloadedOperatorId(symbol),
				Argument(leftExpression, left), Argument(walker.expression, right),
				getInstantiationContext());
			SYMBOLS_ASSERT(type != gUniqueTypeNull);
		}
		ExpressionType<T>::set(symbol, type);
	}
	template<typename T>
	void walkBinaryArithmeticExpression(T* symbol)
	{
		walkBinaryExpression<binaryOperatorArithmeticType>(symbol);
	}
	template<typename T>
	void walkBinaryAdditiveExpression(T* symbol)
	{
		walkBinaryExpression<binaryOperatorAdditiveType>(symbol);
	}
	template<typename T>
	void walkBinaryIntegralExpression(T* symbol)
	{
		walkBinaryExpression<binaryOperatorIntegralType>(symbol);
	}
	template<typename T>
	void walkBinaryBooleanExpression(T* symbol)
	{
		walkBinaryExpression<binaryOperatorBoolean>(symbol);
	}
	void visit(cpp::assignment_expression_suffix* symbol)
	{
		// 5.1.7 Assignment operators
		// the type of an assignment expression is that of its left operand
		walkBinaryExpression<binaryOperatorAssignment>(symbol);
	}
	void visit(cpp::conditional_expression_suffix* symbol)
	{
		ConditionalExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = makeExpression(TernaryExpression(conditional, expression, walker.left, walker.right),
			expression.isConstant && walker.left.isConstant && walker.right.isConstant,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		if(!expression.isTypeDependent)
		{
			type = getConditionalOperatorType(removeReference(walker.leftType), removeReference(walker.rightType));
		}
	}
	void visit(cpp::logical_or_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::logical_and_expression_default* symbol)
	{
		walkBinaryBooleanExpression(symbol);
	}
	void visit(cpp::inclusive_or_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::exclusive_or_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::and_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::equality_expression_default* symbol)
	{
		walkBinaryBooleanExpression(symbol);
	}
	void visit(cpp::relational_expression_default* symbol)
	{
		walkBinaryBooleanExpression(symbol);
	}
	void visit(cpp::shift_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::additive_expression_default* symbol)
	{
		walkBinaryAdditiveExpression(symbol);
	}
	void visit(cpp::multiplicative_expression_default* symbol)
	{
		walkBinaryArithmeticExpression(symbol);
	}
	void visit(cpp::pm_expression_default* symbol)
	{
		walkBinaryExpression<binaryOperatorMemberPointer>(symbol);
		id = 0; // not a parenthesised id-expression, expression is not 'call to named function' [over.call.func]
	}
#if 0
	void visit(cpp::assignment_expression_default* symbol)
	{
		TREEWALKER_LEAF_SRC(symbol);
		setExpressionType(symbol, type);
	}
#endif
	void visit(cpp::assignment_expression* symbol) // expression_list, assignment_expression_suffix, conditional_expression_suffix
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		// [expr.comma] The type and value of the result are the type and value of the right operand
		expression = walker.expression;
		type = walker.type;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		setExpressionType(symbol, type);
	}
	void visit(cpp::expression_list* symbol) // a comma-separated list of assignment_expression
	{
		TREEWALKER_LEAF(symbol);
		setExpressionType(symbol, type);
	}
	void visit(cpp::postfix_expression* symbol)
	{
		PostfixExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		if(walker.isUndeclared)
		{
			return reportIdentifierMismatch(symbol, *id, &gUndeclared, "object-name");
		}
		id = walker.id;
		type.swap(walker.type);
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		//setDependent(dependent, walker.dependent); // TODO:
		setExpressionType(symbol, type);
	}
	void visit(cpp::unary_expression_op* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = 0; // not a parenthesised id-expression, expression is not 'call to named function' [over.call.func]
		if(!isDependent(typeDependent))
		{
			SEMANTIC_ASSERT(type != gUniqueTypeNull);
			SEMANTIC_ASSERT(!::isDependent(type)); // can't resolve operator overloads if type is dependent
			SEMANTIC_ASSERT(getQualifyingScope() == 0);
			SEMANTIC_ASSERT(!(objectExpression.p != 0 && memberClass != 0)); // unary expression should not occur during class-member-access

			type = removeReference(type);
			type = typeOfUnaryExpression(
				getOverloadedOperatorId(symbol->op),
				Argument(expression, type),
				getInstantiationContext());
			// TODO: decorate parse-tree with declaration
		}
		else
		{
			type = gUniqueTypeNull;
		}


		UnaryIceOp iceOp = getUnaryIceOp(symbol);
		expression = makeExpression(UnaryExpression(getOverloadedOperatorId(symbol->op), iceOp, expression),
			expression.isConstant && iceOp != 0,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		setExpressionType(symbol, type);
	}
	/* 14.6.2.2-3
	Expressions of the following forms are type-dependent only if the type specified by the type-id, simple-type-specifier
	or new-type-id is dependent, even if any subexpression is type-dependent:
	- postfix-expression-construct
	- new-expression
	- postfix-expression-cast
	- cast-expression
	*/
	/* temp.dep.constexpr
	Expressions of the following form are value-dependent if either the type-id or simple-type-specifier is dependent or the
	expression or cast-expression is value-dependent:
	simple-type-specifier ( expression-listopt )
	static_cast < type-id > ( expression )
	const_cast < type-id > ( expression )
	reinterpret_cast < type-id > ( expression )
	( type-id ) cast-expression
	*/
	void visit(cpp::new_expression_placement* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		type = getUniqueTypeSafe(walker.type);
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		requireCompleteObjectType(type, getInstantiationContext());
		type.push_front(PointerType());
		addDependent(typeDependent, walker.typeDependent);
		expression = makeExpression(ExplicitTypeExpression(type), false, isDependent(typeDependent));
		setExpressionType(symbol, type);
	}
	void visit(cpp::new_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		type = getUniqueTypeSafe(walker.type);
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		requireCompleteObjectType(type, getInstantiationContext());
		type.push_front(PointerType());
		addDependent(typeDependent, walker.typeDependent);
		expression = makeExpression(ExplicitTypeExpression(type), false, isDependent(typeDependent));
		setExpressionType(symbol, type);
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		type = getUniqueTypeSafe(walker.type);
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, getInstantiationContext());
		Dependent tmp(walker.typeDependent);
		addDependent(valueDependent, tmp);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = makeExpression(CastExpression(type, walker.expression), walker.expression.isConstant, isDependent(typeDependent), isDependent(valueDependent));
		setExpressionType(symbol, type);
	}
	/* 14.6.2.2-4
	Expressions of the following forms are never type-dependent (because the type of the expression cannot be
	dependent):
	literal
	postfix-expression . pseudo-destructor-name
	postfix-expression -> pseudo-destructor-name
	sizeof unary-expression
	sizeof ( type-id )
	sizeof ... ( identifier )
	alignof ( type-id )
	typeid ( expression )
	typeid ( type-id )
	::opt delete cast-expression
	::opt delete [ ] cast-expression
	throw assignment-expressionopt
	*/
	// TODO: destructor-call is not dependent
	/* temp.dep.constexpr
	Expressions of the following form are value-dependent if the unary-expression is type-dependent or the type-id is dependent
	(even if sizeof unary-expression and sizeof ( type-id ) are not type-dependent):
	sizeof unary-expression
	sizeof ( type-id )
	*/
	void visit(cpp::unary_expression_sizeof* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		// [temp.dep.expr] Expressions of the following form [sizeof(expr)] are never type-dependent (because the type of the expression cannot be dependent)
		// [temp.dep.constexpr] Expressions of the following form [sizeof(expr)] are value-dependent if the unary-expression is type-dependent
		addDependent(valueDependent, walker.typeDependent);
		type = gUnsignedInt;
		setExpressionType(symbol, type);
		expression = makeExpression(SizeofExpression(walker.expression), true, false, isDependent(valueDependent));
	}
	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		SizeofTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		type = gUnsignedInt;
		setExpressionType(symbol, type);
		expression = walker.expression;
	}
	void visit(cpp::delete_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = gVoid; // TODO: check compliance: type of delete-expression
		setExpressionType(symbol, type);
		expression = ExpressionWrapper();
	}
	void visit(cpp::throw_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = gVoid; // [except] A throw-expression is of type void.
		expression = ExpressionWrapper();
	}
};

struct IsHiddenTypeName
{
	DeclarationPtr nonType; // valid if the declaration is hidden by a non-type name
	DeclarationPtr hidingNamespace; // valid if the declaration is hidden by a namespace name

	IsHiddenTypeName()
		: nonType(0), hidingNamespace(0)
	{
	}

	bool operator()(const Declaration& declaration)
	{
		if(isTypeName(declaration))
		{
			return true;
		}
		if(nonType == 0
			&& isAny(declaration))
		{
			nonType = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		if(hidingNamespace == 0
			&& isNamespaceName(declaration))
		{
			hidingNamespace = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		return false;
	}
};


struct TypeNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	IsHiddenTypeName filter; // allows type-name to be parsed without knowing whether it is the prefix of a nested-name-specifier (in which case it cannot be hidden by a non-type name)
	bool isTypename; // true if a type is expected in this context; e.g. following 'typename', preceding '::'
	TypeNameWalker(const WalkerState& state, bool isTypename = false)
		: WalkerBase(state), type(0, context), isTypename(isTypename)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		LookupResultRef declaration = gDependentTypeInstance;
		if(!isDependent(qualifying_p))
		{
			declaration = findDeclaration(symbol->value, makeLookupFilter(filter));
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "type-name");
			}
 		}
		else if(!isTypename)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, symbol->value, &gUndeclared, "typename");
		}
		type.id = &symbol->value;
		type.declaration = declaration;
		if(type.declaration->isTemplate)
		{
			type.isImplicitTemplateId = true; // this is either a template-name or an implicit template-id
			const SimpleType* enclosingTemplate = findEnclosingTemplate(enclosingType, type.declaration);
			if(enclosingTemplate != 0) // if this is the name of an enclosing class-template definition (which may be an explicit/partial specialization)
			{
				type.isEnclosingClass = true; // this is an implicit template-id
				type.declaration = enclosingTemplate->declaration; // the type should refer to the enclosing class-template definition (which may be an explicit/partial specialization)
			}
		}
		setDecoration(&symbol->value, declaration);
		setDependent(type);
#if 1 // temp hack, imitate previous isDependent behaviour
		if(type.declaration->isTemplate
			&& type.declaration->templateParameter == INDEX_INVALID) // ignore template-template-parameter
		{
			if(type.declaration->isSpecialization)
			{
				setDependent(type.dependent, type.declaration->templateArguments);
			}
			else
			{
				SEMANTIC_ASSERT(!type.declaration->templateParams.empty());
				setDependent(type.dependent, *type.declaration->templateParams.front().declaration); // depend on first template param
			}
		}
#endif
	}

	void visit(cpp::simple_template_id* symbol)
	{
		//ProfileScope profile(gProfileTemplateId);

		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		LookupResultRef declaration = lookupTemplate(*walker.id, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "type-name");
		}
		if(declaration == &gDependentTemplate
			&& !isTypename)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}

		setDecoration(walker.id, declaration);
		type.id = walker.id;
		type.declaration = findOverloaded(declaration); // NOTE: stores the declaration from which all explicit/partial specializations are visible via 'Declaration::overloaded'
		type.templateArguments.swap(walker.arguments);
		setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of its arguments are dependent
	}
};

struct NestedNameSpecifierSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool isDeclarator;
	bool isTemplate;
	NestedNameSpecifierSuffixWalker(const WalkerState& state, bool isDeclarator = false)
		: WalkerBase(state), type(0, context), isDeclarator(isDeclarator), isTemplate(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		LookupResultRef declaration = gDependentNestedInstance;
		if(isDeclarator
			|| !isDependent(qualifying_p))
		{
			declaration = lookupQualified(symbol->value, isDeclarator, IsNestedName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "nested-name");
			}
		}
		type = declaration;
		type.id = &symbol->value;
		setDecoration(&symbol->value, declaration);
		if(declaration != &gDependentNested)
		{
			setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK_CACHED(walker, symbol);
		LookupResultRef declaration = gDependentNestedTemplateInstance;
		if(isDeclarator
			|| !isDependent(qualifying_p))
		{
			declaration = lookupQualified(*walker.id, isDeclarator, IsNestedName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "nested-name");
			}
		}
		type = declaration;
		type.id = walker.id;
		type.templateArguments.swap(walker.arguments);
		if(declaration != &gDependentNestedTemplate)
		{
			setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		}
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of its arguments are dependent
	}
};

// basic.lookup.qual
// During the lookup for a name preceding the :: scope resolution operator, object, function, and enumerator names are ignored.
struct NestedNameSpecifierPrefixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool isDeclarator;
	NestedNameSpecifierPrefixWalker(const WalkerState& state, bool isDeclarator = false)
		: WalkerBase(state), type(0, context), isDeclarator(isDeclarator)
	{
	}

#if 0 // for debugging parse-tree cache
	void visit(cpp::nested_name* symbol)
	{
		NestedNameSpecifierPrefixWalker walker(getState(), isDeclarator);
		TREEWALKER_WALK_CACHED(walker, symbol);
		type.swap(walker.type);
	}
#endif
	void visit(cpp::namespace_name* symbol)
	{
		NamespaceNameWalker walker(getState()); // considers only namespace names
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.hidingType != 0) // if the namespace name we found is hidden by a type name
		{
			return reportIdentifierMismatch(symbol, walker.filter.hidingType->getName(), walker.filter.hidingType, "namespace-name");
		}
		type.declaration = walker.declaration;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState(), true); // considers only type names
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.hidingNamespace != 0) // if the type name we found is hidden by a namespace name
		{
			return reportIdentifierMismatch(symbol, walker.filter.hidingNamespace->getName(), walker.filter.hidingNamespace, "type-name");
		}
		if(isDeclarator
			&& !isClass(*walker.type.declaration))
		{
			// the prefix of the nested-name-specifier in a qualified declarator-id must be a class-name (not a typedef)
			return reportIdentifierMismatch(symbol, walker.type.declaration->getName(), walker.type.declaration, "class-name");
		}
		type.swap(walker.type);
	}
};

struct NestedNameSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	bool isDeclarator;
	NestedNameSpecifierWalker(const WalkerState& state, bool isDeclarator = false)
		: WalkerQualified(state), isDeclarator(isDeclarator)
	{
	}
	void visit(cpp::nested_name_specifier_prefix* symbol)
	{
		NestedNameSpecifierPrefixWalker walker(getState(), isDeclarator);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		makeUniqueTypeSafe(walker.type);
		swapQualifying(walker.type, isDeclarator);
		//disableBacktrack();
	}
	void visit(cpp::nested_name_specifier_suffix_template* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(getState(), isDeclarator);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		walker.type.qualifying.swap(qualifying);
		setDependent(walker.type.dependent, walker.type.qualifying);
		makeUniqueTypeSafe(walker.type);
		swapQualifying(walker.type, isDeclarator);
		//disableBacktrack();
	}
	void visit(cpp::nested_name_specifier_suffix_default* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(getState(), isDeclarator);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		walker.type.qualifying.swap(qualifying);
		setDependent(walker.type.dependent, walker.type.qualifying);
		makeUniqueTypeSafe(walker.type);
		swapQualifying(walker.type, isDeclarator);
		//disableBacktrack();
	}
};

struct TypeSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	unsigned fundamental;
	TypeSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context), fundamental(0)
	{
	}
	void visit(cpp::simple_type_specifier_name* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		fundamental = walker.fundamental;
	}
	void visit(cpp::simple_type_specifier_template* symbol) // X::template Y<Z>
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		fundamental = walker.fundamental;
	}
	void visit(cpp::type_name* symbol) // simple_type_specifier_name
	{
		TypeNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.nonType != 0)
		{
			// 3.3.7: a type-name can be hidden by a non-type name in the same scope (this rule applies to a type-specifier)
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol) // simple_type_specifier_name | simple_type_specifier_template
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::simple_template_id* symbol) // simple_type_specifier_template
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		// [temp]
		// A class template shall not have the same name as any other template, class, function, variable, enumeration,
		// enumerator, namespace, or type in the same scope
		LookupResultRef declaration = lookupTemplate(*walker.id, IsAny());
		if(declaration == &gUndeclared
			|| !isTypeName(*declaration)
			|| !isTemplateName(*declaration))
		{
			return reportIdentifierMismatch(symbol, *walker.id, declaration, "class-template-name");
		}
		if(declaration == &gDependentTemplate)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}
		setDecoration(walker.id, declaration);
		type.declaration = declaration;
		type.templateArguments.swap(walker.arguments);
		type.qualifying.swap(qualifying);
		setDependent(type); // a template-id is dependent if the template-name is a template-parameter
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of the template arguments are dependent
		setDependent(type.dependent, type.qualifying);
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		fundamental = combineFundamental(0, symbol->id);
	}
};

struct UnqualifiedDeclaratorIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	UnqualifiedDeclaratorIdWalker(const WalkerState& state)
		: WalkerBase(state), id(&gAnonymousId), conversionType(0, context)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
	}
	void visit(cpp::template_id* symbol) 
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		id = walker.id;
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		Source source = parser->get_source();
		FilePosition position = parser->get_position();
		OperatorFunctionIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		symbol->value.value = walker.name;
		symbol->value.source = source;
		id = &symbol->value;
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		Source source = parser->get_source();
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		symbol->value = gConversionFunctionId;
		symbol->value.source = source;
		id = &symbol->value;
		conversionType.swap(walker.type);
	}
	void visit(cpp::destructor_id* symbol) 
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->name->value;
	}
};

struct QualifiedDeclaratorIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	QualifiedDeclaratorIdWalker(const WalkerState& state)
		: WalkerQualified(state), id(&gAnonymousId)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState(), true); // in a template member definition, the qualifying nested-name-specifier may be dependent on a template-parameter
		TREEWALKER_WALK(walker, symbol); // no need to cache: the nested-name-specifier is not a shared-prefix
		swapQualifying(walker.qualifying, true);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
	}
};

struct DeclaratorIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	DeclaratorIdWalker(const WalkerState& state)
		: WalkerQualified(state), id(&gAnonymousId), conversionType(0, context)
	{
	}
	void visit(cpp::qualified_id_default* symbol)
	{
		QualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		swapQualifying(walker.qualifying, true);
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		QualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		swapQualifying(walker.qualifying, true);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		conversionType.swap(walker.conversionType);
	}
};

struct ParameterDeclarationListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Parameters parameters;

	ParameterDeclarationListWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::parameter_declaration* symbol)
	{
		ParameterDeclarationWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		if(!isVoidParameter(walker.declaration->type))
		{
			parameters.push_back(Parameter(walker.declaration, walker.defaultArgument));
		}
	}
};

struct ParameterDeclarationClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Parameters parameters;

	ParameterDeclarationClauseWalker(const WalkerState& state)
		: WalkerBase(state)
	{
		pushScope(newScope(makeIdentifier("$prototype"), SCOPETYPE_PROTOTYPE));
		if(templateParamScope != 0)
		{
			// insert the template-parameter scope to enclose the declarator scope
			templateParamScope->parent = enclosing->parent;
			enclosing->parent = templateParamScope;
			enclosing->templateDepth = templateParamScope->templateDepth;
		}
		clearTemplateParams();
	}

	void visit(cpp::parameter_declaration_list* symbol)
	{
		ParameterDeclarationListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		parameters = walker.parameters;
	}
	void visit(cpp::terminal<boost::wave::T_ELLIPSIS> symbol)
	{
		parameters.isEllipsis = true;
	}
};

struct ExceptionSpecificationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ExceptionSpecificationWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct CvQualifierSeqWalker : WalkerBase
{
	TREEWALKER_DEFAULT;

	CvQualifiers qualifiers;
	CvQualifierSeqWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::cv_qualifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::cv_qualifier::CONST)
		{
			qualifiers.isConst = true;
		}
		else if(symbol->id == cpp::cv_qualifier::VOLATILE)
		{
			qualifiers.isVolatile = true;
		}
	}

};

struct PtrOperatorWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	CvQualifiers qualifiers;
	PtrOperatorWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::cv_qualifier_seq* symbol)
	{
		CvQualifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		qualifiers = walker.qualifiers;
	}
};

struct DeclaratorFunctionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ScopePtr paramScope;
	Parameters parameters;
	CvQualifiers qualifiers;
	DeclaratorFunctionWalker(const WalkerState& state)
		: WalkerBase(state), paramScope(0)
	{
	}

	void visit(cpp::parameter_declaration_clause* symbol)
	{
		ParameterDeclarationClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		paramScope = walker.enclosing; // store reference for later resumption
		parameters = walker.parameters;
	}
	void visit(cpp::exception_specification* symbol)
	{
		ExceptionSpecificationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::cv_qualifier_seq* symbol)
	{
		CvQualifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		qualifiers = walker.qualifiers;
	}
};

struct DeclaratorArrayWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent valueDependent;
	ArrayRank rank;
	ExpressionWrapper expression;
	DeclaratorArrayWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::terminal<boost::wave::T_LEFTBRACKET> symbol)
	{
		// we may parse multiple pairs of brackets: omitted constant-expression indicates an array of unknown size
		expression = ExpressionWrapper();
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		// [temp.dep.constexpr] An identifier is value-dependent if it is:- a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
		addDependent(valueDependent, walker.valueDependent);
		expression = walker.expression;
	}
	void visit(cpp::terminal<boost::wave::T_RIGHTBRACKET> symbol)
	{
		rank.push_back(expression);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	UniqueTypeWrapper qualifying;
	ScopePtr paramScope;
	Dependent valueDependent;
	TypeSequence typeSequence;
	CvQualifiers qualifiers;
	Qualifying memberPointer;
	Dependent dependent; // track which template parameters the declarator's type depends on. e.g. 'T::* memberPointer', 'void f(T)'
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	DeclaratorWalker(const WalkerState& state)
		: WalkerBase(state), id(&gAnonymousId), paramScope(0), typeSequence(context), memberPointer(context), conversionType(0, context)
	{
	}
	void pushPointerType(cpp::ptr_operator* op)
	{
		if(op->key->id == cpp::ptr_operator_key::REF)
		{
			typeSequence.push_front(DeclaratorReferenceType());
		}
		else
		{
			if(memberPointer.empty())
			{
				typeSequence.push_front(DeclaratorPointerType(qualifiers));
			}
			else
			{
				typeSequence.push_front(DeclaratorMemberPointerType(memberPointer.back(), qualifiers));
				setDependent(dependent, memberPointer);
			}
		}
	}

	void visit(cpp::ptr_operator* symbol)
	{
		PtrOperatorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		qualifiers = walker.qualifiers;
		memberPointer.swap(walker.qualifying);
	}
	template<typename T>
	void walkDeclaratorPtr(T* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol); // if parse fails, state of typeSeqence is not modified.
		walker.pushPointerType(symbol->op);
		id = walker.id;
		qualifying = walker.qualifying;
		enclosing = walker.enclosing;
		paramScope = walker.paramScope;
		templateParams = walker.templateParams;
		addDependent(dependent, walker.dependent);
		addDependent(valueDependent, walker.valueDependent);
		addDependent(enclosingDependent, walker.enclosingDependent);
		SYMBOLS_ASSERT(typeSequence.empty());
		typeSequence = walker.typeSequence;

		qualifiers = walker.qualifiers;
		memberPointer.swap(walker.memberPointer);
		conversionType.swap(walker.conversionType);
	}
	void visit(cpp::declarator_ptr* symbol)
	{
		return walkDeclaratorPtr(symbol);
	}
	void visit(cpp::abstract_declarator_ptr* symbol)
	{
		return walkDeclaratorPtr(symbol);
	}
	void visit(cpp::new_declarator_ptr* symbol)
	{
		return walkDeclaratorPtr(symbol);
	}
	void visit(cpp::conversion_declarator* symbol)
	{
		return walkDeclaratorPtr(symbol);
	}
	void visit(cpp::declarator_id* symbol)
	{
		DeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		qualifying = walker.qualifying.empty() || isNamespace(*walker.qualifying.back().declaration)
			? gUniqueTypeNull : UniqueTypeWrapper(walker.qualifying.back().unique);

		if(walker.getQualifyingScope()
			&& enclosing->type != SCOPETYPE_CLASS) // //TODO: in 'class C { friend void Q::N(X); };' X should be looked up in the scope of Q rather than C (if Q is a class)
		{
			enclosing = walker.getQualifyingScope(); // names in declarator suffix (array-size, parameter-declaration) are looked up in declarator-id's qualifying scope
		}

		if(qualifying != gUniqueTypeNull)// if the declarator is qualified by a class-name
		{
			// represents the type of 'this'
			enclosingType = &getSimpleType(qualifying.value);
			if(enclosingType->declaration->isTemplate // if the declarator is qualified with a template-id
				&& !enclosingType->declaration->templateParams.empty()) // and the template is not an explicit-specialization
			{
				// 'this' is dependent within a template-definition (except for an explicit-specialization)
				// NOTE: depends on state of 'enclosing', modified above!
				setDependent(enclosingDependent, enclosingType->declaration->templateParams.back().declaration);
			}
		}


		if(templateParams != 0
			&& !templateParams->empty()
			&& consumeTemplateParams(walker.qualifying))
		{
			templateParams = 0;
		}

		if(walker.conversionType.declaration != 0)
		{
			conversionType.swap(walker.conversionType);
		}
		addDependent(dependent, conversionType); // TODO: check compliance: conversion-function declarator-id is dependent if it contains a dependent type?
	}
	template<typename T>
	void walkDeclaratorArray(T* symbol)
	{
		DeclaratorArrayWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		// [temp.dep.type] A type is dependent if it is - an array type [...] whose size is specified by a constant expression that is value-dependent
		addDependent(dependent, walker.valueDependent);
		addDependent(valueDependent, walker.valueDependent);
		typeSequence.push_front(DeclaratorArrayType(walker.rank));
	}
	void visit(cpp::declarator_suffix_array* symbol)
	{
		return walkDeclaratorArray(symbol);
	}
	void visit(cpp::declarator_suffix_function* symbol)
	{
		DeclaratorFunctionWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		if(paramScope == 0) // only interested in the innermost parameter-list
		{
			paramScope = walker.paramScope;
		}
		typeSequence.push_front(DeclaratorFunctionType(walker.parameters, walker.qualifiers));
		setDependent(dependent, walker.parameters);
	}
	void visit(cpp::new_declarator_suffix* symbol)
	{
		return walkDeclaratorArray(symbol);
	}
	void visit(cpp::expression* symbol) // in direct_new_declarator
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	template<typename T>
	void walkDeclarator(T* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		qualifying = walker.qualifying;
		enclosing = walker.enclosing;
		paramScope = walker.paramScope;
		templateParams = walker.templateParams;
		addDependent(dependent, walker.dependent);
		addDependent(valueDependent, walker.valueDependent);
		addDependent(enclosingDependent, walker.enclosingDependent);
		SYMBOLS_ASSERT(typeSequence.empty());
		typeSequence = walker.typeSequence;
		conversionType.swap(walker.conversionType);
	}
	void visit(cpp::direct_abstract_declarator* symbol)
	{
		return walkDeclarator(symbol); // if parse fails, state of typeSeqence is not modified. e.g. type-id: int((int))
	}
	void visit(cpp::direct_abstract_declarator_parenthesis* symbol)
	{
		return walkDeclarator(symbol); // if parse fails, state of typeSeqence is not modified. e.g. function-style-cast type-id followed by parenthesised expression: int(*this)
	}
	void visit(cpp::direct_new_declarator* symbol)
	{
		return walkDeclarator(symbol);
	}
	void visit(cpp::declarator* symbol)
	{
		return walkDeclarator(symbol);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		return walkDeclarator(symbol);
	}
	void visit(cpp::new_declarator* symbol)
	{
		return walkDeclarator(symbol);
	}
};

struct BaseSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	BaseSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context)
	{
	}

	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_name* symbol)
	{
		/* [class.derived]
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		TypeNameWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		makeUniqueTypeSafe(type);
	}
};

struct ClassHeadWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	ScopePtr parent;
	TemplateArguments arguments;
	bool isUnion;
	bool isSpecialization;
	ClassHeadWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), parent(enclosing), arguments(context), isUnion(false), isSpecialization(false)
	{
	}

	void visit(cpp::class_key* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		isUnion = symbol->id == cpp::class_key::UNION;
	}
	void visit(cpp::identifier* symbol) // class_name
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);

		// resolve the (possibly dependent) qualifying scope
		if(walker.getDeclaratorQualifying() != 0)
		{
			parent = walker.getDeclaratorQualifying()->enclosed; // class is declared in scope of qualifying class/namespace
		}

		if(templateParams != 0
			&& !templateParams->empty()
			&& consumeTemplateParams(walker.qualifying))
		{
			templateParams = 0;
		}
	}
	void visit(cpp::simple_template_id* symbol) // class_name
	{
		TemplateIdWalker walker(getState(), true); // TODO: specifying isTemplate as a temporary workaround: name lookup of qualified class-name currently fails.
		TREEWALKER_WALK_CACHED(walker, symbol);
		// TODO: don't declare anything - this is a template (partial) specialisation
		id = walker.id;
		arguments.swap(walker.arguments);
		isSpecialization = true;
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until ':' is discovered
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = declareClass(parent, id, isSpecialization, arguments);
	}
	void visit(cpp::base_specifier* symbol) 
	{
		BaseSpecifierWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		if(walker.type.declaration != 0) // declaration == 0 if base-class is dependent
		{
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			addBase(declaration, walker.type);
		}
		SEMANTIC_ASSERT(walker.type.unique != 0);
		setExpressionType(symbol, walker.type.isDependent ? gUniqueTypeNull : UniqueTypeWrapper(walker.type.unique));
	}
};

struct UsingDeclarationWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	bool isTypename;
	UsingDeclarationWalker(const WalkerState& state)
		: WalkerQualified(state), isTypename(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(!isTypename
			&& !isDependent(qualifying_p))
		{
			LookupResultRef declaration = walker.declaration;
			if(declaration == &gUndeclared
				|| !(isObject(*declaration) || isTypeName(*declaration)))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name or type-name");
			}

			setDecoration(walker.id, declaration); // refer to the primary declaration of this name, rather than the one declared by this using-declaration
			
			DeclarationInstance instance(declaration);
			instance.name = walker.id;
			instance.overloaded = declaration.p;
			instance.redeclared = declaration.p;
			DeclarationInstanceRef redeclaration = enclosing->declarations.insert(instance);
			enclosing->declarationList.push_back(instance);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(redeclaration);
#endif

		}
		else
		{
			// TODO: introduce typename into enclosing namespace
			setDecoration(walker.id, gDependentTypeInstance);
		}
	}
	void visit(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
		isTypename = true;
	}
};

struct UsingDirectiveWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	UsingDirectiveWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::namespace_name* symbol)
	{
		/* basic.lookup.udir
		When looking up a namespace-name in a using-directive or namespace-alias-definition, only namespace
		names are considered.
		*/
		NamespaceNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(!findScope(enclosing, walker.declaration->enclosed))
		{
			enclosing->usingDirectives.push_back(walker.declaration->enclosed);
		}
	}
};

struct NamespaceAliasDefinitionWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	NamespaceAliasDefinitionWalker(const WalkerState& state)
		: WalkerQualified(state), id(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(id == 0) // first identifier
		{
			id = &symbol->value;
		}
		else // second identifier
		{
			LookupResultRef declaration = findDeclaration(symbol->value, IsNamespaceName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
			}

			// TODO: check for conflicts with earlier declarations
			declaration = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, declaration->enclosed);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif
			setDecoration(id, declaration);
		}
	}
};

struct MemberDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	MemberDeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::member_template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_implicit* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_nested* symbol)
	{
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};


struct ClassSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	TemplateArguments arguments;
	DeferredSymbols deferred;
	bool isUnion;
	bool isSpecialization;
	ClassSpecifierWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), arguments(context), isUnion(false), isSpecialization(false)
	{
	}

	void visit(cpp::class_head* symbol)
	{
		ClassHeadWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		isUnion = walker.isUnion;
		isSpecialization = walker.isSpecialization;
		arguments.swap(walker.arguments);
		enclosing = walker.parent;
		templateParams = walker.templateParams; // template-params may have been consumed by qualifying template-name
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until '{' is discovered
		if(declaration == 0)
		{
			// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
			declaration = declareClass(enclosing, id, isSpecialization, arguments);
		}

		/* basic.scope.class-1
		The potential scope of a name declared in a class consists not only of the declarative region following
		the name’s point of declaration, but also of all function bodies, brace-or-equal-initializers of non-static
		data members, and default arguments in that class (including such things in nested classes).
		*/
		SEMANTIC_ASSERT(declaration->enclosed != 0);

		pushScope(declaration->enclosed);
		if(templateParamScope != 0)
		{
			// insert the template-parameter scope to enclose the class scope
			templateParamScope->parent = enclosing->parent;
			enclosing->parent = templateParamScope; // required when looking up template-parameters from within a template class
		}
		if(declaration->isTemplate)
		{
			enclosing->templateDepth = templateDepth; // indicates that this is a template
		}
		declaration->templateParamScope = templateParamScope; // required by findEnclosingType

		Type type(declaration, context);
		type.id = &declaration->getName();
		setDependent(type);
		if(declaration->isTemplate)
		{
			setDependentEnclosingTemplate(type.dependent, declaration);
		}
		type.isDependent = isDependent(type);
		type.isImplicitTemplateId = declaration->isTemplate;
		type.isEnclosingClass = true;
		bool isExplicitSpecialization = isSpecialization && declaration->templateParams.empty();
		bool allowDependent = type.isDependent || (declaration->isTemplate && !isExplicitSpecialization); // prevent uniquing of template-arguments in implicit template-id
		declaration->type.isDependent = type.isDependent;
		declaration->type.unique = makeUniqueType(type, getInstantiationContext(), allowDependent).value;
		enclosingType = &getSimpleType(declaration->type.unique);
		const_cast<SimpleType*>(enclosingType)->declaration = declaration; // if this is a specialization, use the specialization instead of the primary template
		instantiateClass(*enclosingType, InstantiationContext(getLocation(), 0, 0), allowDependent); // instantiate non-dependent base classes

		addDependent(enclosingDependent, type);

		clearTemplateParams();
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(getState());
		if(WalkerState::deferred == 0)
		{
			walker.deferred = &deferred;
		}
		TREEWALKER_WALK(walker, symbol);
	}
	inline bool hasCopyAssignmentOperator(const Declaration& declaration)
	{
		Identifier id;
		id.value = gOperatorAssignId;
		const DeclarationInstance* result = ::findDeclaration(declaration.enclosed->declarations, id);
		if(result == 0)
		{
			return false;
		}
		for(const Declaration* p = findOverloaded(*result); p != 0; p = p->overloaded)
		{
			if(p->isTemplate)
			{
				continue; // TODO: check compliance: copy-assignment-operator cannot be a template?
			}

		}
		return false;
	}
	void visit(cpp::terminal<boost::wave::T_RIGHTBRACE> symbol)
	{
		declaration->isComplete = true;
		// If the class definition does not explicitly declare a copy assignment operator, one is declared implicitly.
		// The implicitly-declared copy assignment operator for a class X will have the form
		//   X& X::operator=(const X&)
		// TODO: correct constness of parameter
		// TODO: this must occur at point of instantiation

		parseDeferred(deferred.first, *this);
		parseDeferred(deferred.second, *this);
	}
};

struct EnumeratorDefinitionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	bool isInitialized;
	EnumeratorDefinitionWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), isInitialized(false)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		/* 3.1-4
		The point of declaration for an enumerator is immediately after its enumerator-definition.
		*/
		// TODO: give enumerators a type
		DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, symbol->value, TYPE_ENUMERATOR, 0, DeclSpecifiers());
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(instance);
#endif
		setDecoration(&symbol->value, instance);
		declaration = instance;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		declaration->initializer = walker.expression;
		isInitialized = true;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
};

struct EnumSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	ExpressionWrapper value;
	EnumSpecifierWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}

	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer declaration until '{' resolves ambiguity between enum-specifier and elaborated-type-specifier
		if(id != 0)
		{
			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_ENUM, 0);
			setDecoration(id, instance);
			declaration = instance;
		}
		// [dcl.enum] If the first enumerator has no initializer, the value of the corresponding constant is zero.
		value = makeUniqueExpression(IntegralConstantExpression(gSignedInt, IntegralConstant(0))); // TODO: [dcl.enum] underlying type of enumerator
	}

	void visit(cpp::enumerator_definition* symbol)
	{
		if(declaration == 0)
		{
			// unnamed enum
			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, enclosing->getUniqueName(), TYPE_ENUM, 0);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(instance);
#endif
			declaration = instance;
		}
		EnumeratorDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		walker.declaration->type = declaration; // give the enumerator the type of its enumeration
		walker.declaration->type.qualifiers = CvQualifiers(true, false); // an enumerator may be used in an integral constant expression
		setDependent(walker.declaration->type); // the enumeration type is dependent if it is a member of a class template
		makeUniqueTypeSafe(walker.declaration->type);
		if(walker.isInitialized)
		{
			SEMANTIC_ASSERT(isDependent(walker.declaration->valueDependent) || walker.declaration->initializer.isConstant);
			value = walker.declaration->initializer;
		}
		else
		{
			walker.declaration->initializer = value;
		}
		// [dcl.enum] An enumerator-definition without an initializer gives the enumerator the value obtained by increasing the value of the previous enumerator by one.
		ExpressionWrapper one = makeUniqueExpression(IntegralConstantExpression(gSignedInt, IntegralConstant(1)));
		value = makeExpression(BinaryExpression(Name("+"), operator+, 0, value, one), // TODO: type of enumerator
			true, value.isTypeDependent, value.isValueDependent
		);
	}
};

struct ElaboratedTypeSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	DeclarationPtr key;
	Type type;
	IdentifierPtr id;
	ElaboratedTypeSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), key(0), type(0, context), id(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::elaborated_type_specifier_default* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		id = walker.id;

		if(!isUnqualified(symbol)
			|| !isClassKey(*type.declaration))
		{
			id = 0;
		}
	}
	void visit(cpp::elaborated_type_specifier_template* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		id = walker.id;
	}
	void visit(cpp::nested_name_specifier* symbol) // elaborated_type_specifier_default | elaborated_type_specifier_template
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_key* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		key = &gClass;
	}
	void visit(cpp::enum_key* symbol)
	{
		TREEWALKER_LEAF(symbol);
		key = &gEnum;
	}
	void visit(cpp::simple_template_id* symbol) // elaborated_type_specifier_default | elaborated_type_specifier_template
	{
		SEMANTIC_ASSERT(key == &gClass);
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		// 3.4.4-2: when looking up 'identifier' in elaborated-type-specifier, ignore any non-type names that have been declared. 
		LookupResultRef declaration = lookupTemplate(*walker.id, IsTypeName());
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "type-name");
		}
		if(declaration == &gDependentTemplate)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}
		setDecoration(walker.id, declaration);
		id = walker.id;
		type.declaration = declaration;
		type.templateArguments.swap(walker.arguments);
		type.qualifying.swap(qualifying);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		/* 3.4.4-2
		If the elaborated-type-specifier has no nested-name-specifier ...
		... the identifier is looked up according to 3.4.1 but ignoring any non-type names that have been declared. If
		the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
		declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
		the class-key and this lookup does not find a previously declared type-name ...
		the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
		*/
		id = &symbol->value;
		LookupResultRef declaration = findDeclaration(symbol->value, IsTypeName());
		if(declaration == &gUndeclared // if there is no existing declaration
			|| isTypedef(*declaration) // or the existing declaration is a typedef
			|| declaration->isTemplate // or the existing declaration is a template class
			|| templateParams != 0 // or we are forward-declaring a template class
			|| (key == &gClass && declaration->scope == getEtsScope())) // or this is a forward-declaration of a class/struct
		{
			if(key != &gClass)
			{
				SEMANTIC_ASSERT(key == &gEnum);
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier refers to undefined enum" << std::endl;
				throw SemanticError();
			}
			type = key;
		}
		else
		{
#if 0 // elaborated type specifier cannot refer to a template in a different scope - this case will be treated as a redeclaration
			// template<typename T> class C
			if(declaration->isSpecialization) // if the lookup found a template explicit/partial-specialization
			{
				SEMANTIC_ASSERT(declaration->isTemplate);
				declaration = findPrimaryTemplateLastDeclaration(declaration); // the name is a plain identifier, not a template-id, therefore the name refers to the primary template
			}
#endif
			setDecoration(&symbol->value, declaration);
			/* [dcl.type.elab]
			3.4.4 describes how name lookup proceeds for the identifier in an elaborated-type-specifier. If the identifier
			resolves to a class-name or enum-name, the elaborated-type-specifier introduces it into the declaration the
			same way a simple-type-specifier introduces its type-name. If the identifier resolves to a typedef-name, the
			elaborated-type-specifier is ill-formed.
			*/
#if 0 // allow hiding a typedef with a forward-declaration
			if(isTypedef(*declaration))
			{
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier refers to a typedef" << std::endl;
				printPosition(declaration->getName().source);
				throw SemanticError();
			}
#endif
			/* 7.1.6.3-3
			The class-key or enum keyword present in the elaborated-type-specifier shall agree in kind with the declaration
			to which the name in the elaborated-type-specifier refers.
			*/
			if(declaration->type.declaration != key)
			{
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier key does not match declaration" << std::endl;
				printPosition(declaration->getName().source);
				throw SemanticError();
			}
			type = declaration;
		}
	}
};

struct TypenameSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	TypenameSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context)
	{
	}

	void visit(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		// TODO
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
	}
};

struct DeclSpecifierSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	unsigned fundamental;
	DeclSpecifiers specifiers;
	CvQualifiers qualifiers;
	IdentifierPtr forward;
	bool isUnion;
	DeclSpecifierSeqWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context), fundamental(0), forward(0), isUnion(false)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		if(type.declaration == 0)
		{
			fundamental = walker.fundamental;
			type = getFundamentalType(fundamental);
		}
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		fundamental = combineFundamental(fundamental, symbol->id);
		type = getFundamentalType(fundamental);
	}
	void visit(cpp::elaborated_type_specifier* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		forward = walker.id;
		type.swap(walker.type);
	}
	void visit(cpp::typename_specifier* symbol)
	{
		TypenameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
	}
	void visit(cpp::class_specifier* symbol)
	{
		ClassSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
		setDependent(type); // a class-specifier is dependent if it declares a nested class of a template class
		templateParams = walker.templateParams;
		isUnion = walker.isUnion;
	}
	void visit(cpp::enum_specifier* symbol)
	{
		EnumSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
	}
	void visit(cpp::decl_specifier_default* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::decl_specifier_default::TYPEDEF)
		{
			specifiers.isTypedef = true;
		}
		else if(symbol->id == cpp::decl_specifier_default::FRIEND)
		{
			specifiers.isFriend = true;
		}
	}
	void visit(cpp::storage_class_specifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::storage_class_specifier::STATIC)
		{
			specifiers.isStatic = true;
		}
		else if(symbol->id == cpp::storage_class_specifier::EXTERN)
		{
			specifiers.isExtern = true;
		}
	}
	void visit(cpp::cv_qualifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::cv_qualifier::CONST)
		{
			qualifiers.isConst = true;
		}
		else if(symbol->id == cpp::cv_qualifier::VOLATILE)
		{
			qualifiers.isVolatile = true;
		}
	}
};

struct TryBlockWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TryBlockWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct LabeledStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	LabeledStatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		// TODO: goto label
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct StatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	StatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
	}
	void visit(cpp::try_block* symbol)
	{
		TryBlockWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::namespace_alias_definition* symbol)
	{
		NamespaceAliasDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::selection_statement* symbol)
	{
		ControlStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::iteration_statement* symbol)
	{
		ControlStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::expression_statement* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::jump_statement_return* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::jump_statement_goto* symbol)
	{
		TREEWALKER_LEAF(symbol);
		// TODO
	}
	void visit(cpp::labeled_statement* symbol)
	{
		LabeledStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_directive* symbol)
	{
		UsingDirectiveWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ControlStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ControlStatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	void visit(cpp::condition_init* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
	}
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct CompoundStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	CompoundStatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct HandlerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	HandlerWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_CATCH> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	void visit(cpp::exception_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct HandlerSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	HandlerSeqWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::handler* symbol)
	{
		HandlerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct QualifiedTypeNameWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	QualifiedTypeNameWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_name* symbol)
	{
		TypeNameWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
	}
};

struct MemInitializerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	MemInitializerWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::mem_initializer_id_base* symbol)
	{
		QualifiedTypeNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		SEMANTIC_ASSERT(getQualifyingScope() == 0);
		LookupResultRef declaration = findDeclaration(symbol->value);
		if(declaration == &gUndeclared
			|| !isObject(*declaration))
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "object-name");
		}
		setDecoration(&symbol->value, declaration);
	}
	void visit(cpp::expression_list* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct MemberDeclaratorBitfieldWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	MemberDeclaratorBitfieldWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
	}
};

struct TypeIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TypeId type;
	TypeIdWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context)
	{
	}
	// called when parse of type-id is complete (necessary because trailing abstract-declarator is optional)
	void commit()
	{
		makeUniqueTypeSafe(type);
	}
	void visit(cpp::terminal<boost::wave::T_OPERATOR> symbol) // conversion_function_id
	{
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifiers = walker.qualifiers;
		declareEts(type, walker.forward);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.typeSequence = walker.typeSequence;
		// [temp.dep.type] A type is dependent if it is a compound type constructed from any dependent type
		setDependent(type.dependent, walker.dependent);
	}
	void visit(cpp::conversion_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		walker.pushPointerType(symbol->op);
		type.typeSequence = walker.typeSequence;
		// [temp.dep.type] A type is dependent if it is a compound type constructed from any dependent type
		setDependent(type.dependent, walker.dependent);
	}
};

struct NewTypeWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TypeId type;
	Dependent valueDependent;
	NewTypeWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context)
	{
	}
	void visit(cpp::terminal<boost::wave::T_OPERATOR> symbol) 
	{
		// for debugging purposes
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifiers = walker.qualifiers;
		declareEts(type, walker.forward);
	}
	void visit(cpp::new_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		type.typeSequence = walker.typeSequence;
		setDependent(type.dependent, walker.dependent);
		// new T
		// new T*
		// new T[variable]
		// new T[variable][constant]
		// new T*[variable]
		// new T C::*
	}
};

struct IsTemplateName
{
	WalkerState& context;
	IsTemplateName(WalkerState& context) : context(context)
	{
	}
	bool operator()(Identifier& id) const
	{
		LookupResultRef declaration = context.findDeclaration(id);
		return declaration != &gUndeclared && isTemplateName(*declaration);
	}
};

struct InitializerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ExpressionWrapper expression;
	Dependent valueDependent;
	InitializerWalker(const WalkerState& state) : WalkerBase(state)
	{
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct SimpleDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	ScopePtr parent;
	IdentifierPtr id;
	TypeId type;
	Dependent typeDependent;
	ScopePtr enclosed;
	DeclSpecifiers specifiers;
	IdentifierPtr forward;

	DeferredSymbols deferred;

	Dependent valueDependent;
	size_t templateParameter;
	bool isParameter;
	bool isUnion;
	bool isConversionFunction;
	CvQualifiers conversionFunctionQualifiers;

	SimpleDeclarationWalker(const WalkerState& state, bool isParameter = false, size_t templateParameter = INDEX_INVALID) : WalkerBase(state),
		declaration(0),
		parent(0),
		id(0),
		type(&gCtor, context),
		enclosed(0),
		forward(0),
		templateParameter(templateParameter),
		isParameter(isParameter),
		isUnion(false),
		isConversionFunction(false)
	{
	}

	// commit the declaration to the enclosing scope.
	// invoked when no further ambiguities remain.
	void commit()
	{
		if(id != 0)
		{
			if(isConversionFunction)
			{
				type.typeSequence.push_back(DeclaratorFunctionType(Parameters(), conversionFunctionQualifiers));
			}
			DeclarationPtr tmpDependent = type.dependent;
			setDependent(type.dependent, typeDependent);
			makeUniqueTypeSafe(type);
			if(enclosed == 0
				&& templateParamScope != 0)
			{
				templateParamScope->parent = parent;
				enclosed = templateParamScope; // for a static-member-variable definition, store template-params with different names than those in the class definition
			}
			declaration = declareObject(parent, id, type, enclosed, specifiers, templateParameter, valueDependent);

			enclosing = parent;

			if(enclosed != 0) // if the declaration has a parameter-declaration-clause
			{
				enclosed->name = declaration->getName();
				enclosing = enclosed; // subsequent declarations are contained by the parameter-scope - see 3.3.2-1: parameter scope
			}
			clearTemplateParams();

			id = 0;

			// clear state that was modified while committing 
			type.unique = 0;
			type.isDependent = false;
			type.dependent = tmpDependent;
		}
	}

	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifiers = walker.qualifiers;
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		specifiers = walker.specifiers;
		forward = walker.forward;
		templateParams = walker.templateParams;
		isUnion = walker.isUnion;
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifiers = walker.qualifiers;
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		forward = walker.forward;
		templateParams = walker.templateParams;
		isUnion = walker.isUnion;
	}
	void visit(cpp::function_specifier* symbol) // in constructor_definition/member_declaration_implicit -> function_specifier_seq
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::function_specifier::EXPLICIT)
		{
			specifiers.isExplicit = true;
		}
	}

	void visit(cpp::declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		if(isParameter)
		{
			walker.declareEts(type, forward);
		}

		if(WalkerState::deferred != 0)
		{
			// while parsing simple-declaration-named declarator which contains deferred default-argument expression,
			// on finding '{', we rewind and try parsing function-definition.
			// In this situation, 'deferred' contains the reference to the deferred expression.
			walker.deferred = &deferred;
		}

		TREEWALKER_WALK_CACHED(walker, symbol);
		parent = walker.enclosing; // if the id-expression in the declarator is a qualified-id, this is the qualifying scope
		id = walker.id;
		enclosed = walker.paramScope;
		type.typeSequence = walker.typeSequence;
		addDependent(typeDependent, walker.dependent);
		/* temp.dep.constexpr
		An identifier is value-dependent if it is:
			- a name declared with a dependent type,
			- the name of a non-type template parameter,
			- a constant with effective literal type and is initialized with an expression that is value-dependent.
		*/
		addDependent(valueDependent, walker.valueDependent);

		if(walker.qualifying != gUniqueTypeNull)
		{
			SEMANTIC_ASSERT(walker.qualifying.isSimple());
			enclosingType = &getSimpleType(walker.qualifying.value);
			enclosingDependent = walker.enclosingDependent; // not using addDependent, workaround for issue when 'enclosing' is not (yet) referring to qualifying class in declarator 'S<T>::f()' 
		}
		templateParams = walker.templateParams; // template-params may have been consumed by qualifying template-name

		if(walker.conversionType.declaration != 0)
		{
			// [class.conv.fct] The type of a conversion function is "function taking no parameter returning conversion-type-id."
			// take the cv-qualifiers from the declarator
			conversionFunctionQualifiers = getDeclaratorFunctionType(type.typeSequence.get()).qualifiers;
			type.swap(walker.conversionType);
			isConversionFunction = true;
		}
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		if(isParameter)
		{
			walker.declareEts(type, forward);
		}
		TREEWALKER_WALK(walker, symbol);
		enclosed = walker.paramScope;
		type.typeSequence = walker.typeSequence;
		addDependent(typeDependent, walker.dependent);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		MemberDeclaratorBitfieldWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.id != 0)
		{
			DeclarationPtr tmpDependent = type.dependent;
			setDependent(type.dependent, typeDependent);
			makeUniqueTypeSafe(type);

			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *walker.id, type, 0, specifiers); // 3.3.1.1
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(instance);
#endif
			setDecoration(walker.id, instance);
			declaration = instance;

			// clear state that was modified while committing 
			type.unique = 0;
			type.isDependent = false;
			type.dependent = tmpDependent;
		}
	}
	void visit(cpp::terminal<boost::wave::T_ASSIGN> symbol) // begins initializer_default
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_LEFTPAREN> symbol) // begins initializer_parenthesis
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_TRY> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol) // begins function_body
	{
		commit();
		SEMANTIC_ASSERT(declaration != 0);
		declaration->isFunctionDefinition = true;

		// symbols may be deferred during attempt to parse shared-prefix declarator: f(int i = j)
		// first parsed as member_declaration_named, backtracks on reaching '{'
		if(WalkerState::deferred != 0
			&& !deferred.empty())
		{
			WalkerState::deferred->splice(deferred);
		}
	}
	void visit(cpp::terminal<boost::wave::T_COMMA> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_SEMICOLON> symbol)
	{
		commit();

		// symbols may be deferred during attempt to parse shared-prefix declarator: f(int i = j)
		// first parsed as member_declaration_named, backtracks on reaching '{'
		if(WalkerState::deferred != 0
			&& !deferred.empty())
		{
			WalkerState::deferred->splice(deferred);
		}
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol) // in member_declarator_bitfield, or ctor_initializer
	{
		commit();
	}

	void visit(cpp::default_argument* symbol)
	{
		// We cannot correctly skip a template-id in a default-argument if it refers to a template declared later in the class.
		// This is considered to be correct: http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#325
		if(WalkerState::deferred != 0
			&& templateParameter == INDEX_INVALID) // don't defer parse of default-argument for non-type template-parameter
		{
			result = defer(WalkerState::deferred->first, *this, makeSkipDefaultArgument(IsTemplateName(*this)), symbol);
		}
		else
		{
			TREEWALKER_LEAF(symbol);
		}
	}
	// handle assignment-expression(s) in initializer
	void visit(cpp::assignment_expression* symbol) // condition_init
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(!isParameter // parameters cannot be constants
			&& declaration != 0) // declaration is 0 during deferred parse of default-argument
		{
			declaration->initializer = walker.expression;
			addDependent(declaration->valueDependent, walker.valueDependent);
		}
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	void visit(cpp::initializer_clause* symbol) // initializer_default
	{
		InitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	void visit(cpp::expression_list* symbol) // initializer_parenthesis
	{
		InitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
	void visit(cpp::constant_expression* symbol) // member_declarator_bitfield
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}

	void visit(cpp::statement_seq_wrapper* symbol)
	{
		// NOTE: we must ensure that symbol-table modifications within the scope of this function are undone on parse fail
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
		if(WalkerState::deferred != 0)
		{
			result = defer(WalkerState::deferred->second, *this, skipBraced, symbol);
			if(result == 0)
			{
				return;
			}
		}
		else
		{
			TREEWALKER_LEAF(symbol);
		}
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::mem_initializer* symbol)
	{
		MemInitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::mem_initializer_clause* symbol)
	{
		if(WalkerState::deferred != 0)
		{
			result = defer(WalkerState::deferred->second, *this, skipMemInitializerClause, symbol);
		}
		else // in case of an out-of-line constructor-definition
		{
			TREEWALKER_LEAF(symbol);
		}
	}

	struct DeclareEtsGuard
	{
		Type* p;
		TypeSequence* typeSequence;
		DeclareEtsGuard(SimpleDeclarationWalker& walker)
		{
			p = walker.declareEts(walker.type, walker.forward) ? &walker.type : 0;
			typeSequence = &walker.type.typeSequence;
		}
		~DeclareEtsGuard()
		{
			if(p != 0)
			{
				*p = &gClass;
			}
			if(typeSequence != 0)
			{
				typeSequence->clear(); // if declaration parse failed, don't keep a reference to the (deleted) type-sequence
			}
		}
		void hit()
		{
			p = 0;
			typeSequence = 0;
		}
	};

	void visit(cpp::simple_declaration_named* symbol)
	{
		DeclareEtsGuard guard(*this); // the point of declaration for the ETS in the decl-specifier-seq is just before the declarator
		TREEWALKER_LEAF(symbol);
		guard.hit();
	}
	void visit(cpp::member_declaration_named* symbol)
	{
		DeclareEtsGuard guard(*this);
		TREEWALKER_LEAF(symbol);
		guard.hit();
	}
	void visit(cpp::function_definition* symbol)
	{
		DeclareEtsGuard guard(*this);
		TREEWALKER_LEAF(symbol);
		guard.hit();
	}
	void visit(cpp::type_declaration_suffix* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(forward != 0) // declare the name found in elaborated-type-specifier parse
		{
			bool isSpecialization = !isClassKey(*type.declaration);
			if(isSpecialization
				&& (specifiers.isFriend
					|| isExplicitInstantiation))
			{
				// friend class C<int>; // friend
				// template class C<int>; // explicit instantiation
			}
			else if(specifiers.isFriend)
			{
				// friend class C;
			}
			else
			{
				// class C;
				// template<class T> class C;
				// template<> class C<int>;
				// template<class T> class C<T*>;
				DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *forward, TYPE_CLASS, 0, DeclSpecifiers(), templateParams != 0, getTemplateParams(), isSpecialization, type.templateArguments);
#ifdef ALLOCATOR_DEBUG
				trackDeclaration(instance);
#endif
				setDecoration(forward, instance);
				declaration = instance;
				if(declaration->templateParamScope == 0)
				{
					declaration->templateParamScope = templateParamScope; // required by findEnclosingType
				}
			}
			type = TypeId(declaration, context); // TODO: is this necessary?
		}
		else if(declaration != 0
			&& declaration->isTemplate
			&& templateParams != 0) // if not an explicit-specialization
		{
			// template<class T> class C;
			SEMANTIC_ASSERT(!declaration->isSpecialization);
			mergeTemplateParamDefaults(*declaration, *templateParams);
		}

		if(isUnion
			&& isAnonymous(*declaration))
		{
			/* class.union-2
			The names of the members of an anonymous union
			shall be distinct from the names of any other entity in the scope in which the anonymous union is declared.
			For the purpose of name lookup, after the anonymous union definition, the members of the anonymous union
			are considered to have been defined in the scope in which the anonymous union is declared.
			*/
			// TODO: verify that member names are distinct
			for(Scope::Declarations::iterator i = declaration->enclosed->declarations.begin(); i != declaration->enclosed->declarations.end(); ++i)
			{
				Declaration& member = *(*i).second;
				if(isAnonymous(member))
				{
					member.setName(enclosing->getUniqueName());
					if(member.enclosed != 0)
					{
						member.enclosed->name = member.getName();
					}
				}
				else
				{
					const DeclarationInstance* holder = ::findDeclaration(enclosing->declarations, member.getName());
					if(holder != 0)
					{
						Declaration* declaration = *holder;
						printPosition(member.getName().source);
						std::cout << "'" << member.getName().value.c_str() << "': anonymous union member already declared" << std::endl;
						printPosition(declaration->getName().source);
						throw SemanticError();
					}
				}
				member.scope = enclosing;
				Identifier* id = &member.getName();
				enclosing->declarations.insert(DeclarationInstance(&member, context.declarationCount++));
				enclosing->declarationList.push_back(&member);
			}
			declaration->enclosed = 0;
		}
	}
};

struct ParameterDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	size_t templateParameter;
	cpp::default_argument* defaultArgument;
	bool isParameter;

	ParameterDeclarationWalker(const WalkerState& state, bool isParameter = false, size_t templateParameter = INDEX_INVALID)
		: WalkerBase(state), templateParameter(templateParameter), defaultArgument(0), isParameter(isParameter)
	{
	}
	void visit(cpp::parameter_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		declaration = walker.declaration;
		defaultArgument = symbol->init;
	}
	void visit(cpp::parameter_declaration_abstract* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK(walker, symbol);
		walker.parent = enclosing;
		walker.id = &gAnonymousId;
		walker.commit();
		declaration = walker.declaration;
		defaultArgument = symbol->init;
	}
};

struct TypeParameterWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	DeclarationPtr declaration;
	TemplateArgument argument; // the default argument for this param
	TemplateParameters params; // the template parameters for this param (if template-template-param)
	size_t templateParameter;
	TypeParameterWalker(const WalkerState& state, size_t templateParameter)
		: WalkerBase(state), id(&gAnonymousId), declaration(0), argument(context), params(context), templateParameter(templateParameter)
	{
	}
	void commit()
	{
		SEMANTIC_ASSERT(declaration == 0); // may only be called once, after parse of type-parameter succeeds
		DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_PARAM, 0, DECLSPEC_TYPEDEF, !params.empty(), params, false, TEMPLATEARGUMENTS_NULL, templateParameter);
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(instance);
#endif
		if(id != &gAnonymousId)
		{
			setDecoration(id, instance);
		}
		declaration = instance;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::type_id* symbol)
	{
		SEMANTIC_ASSERT(params.empty());
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		argument.type.swap(walker.type);
	}
	void visit(cpp::template_parameter_clause* symbol)
	{
		TemplateParameterClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
	}
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		LookupResultRef declaration = walker.declaration;
		if(declaration != 0) // TODO: error, lookup failed?
		{
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			setDecoration(walker.id, declaration);
		}
	}
};

struct TemplateParameterListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateParameter param;
	TemplateParameters params;
	size_t count;
	TemplateParameterListWalker(const WalkerState& state, size_t count)
		: WalkerBase(state), param(context), params(context), count(count)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		TypeParameterWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		param = walker.declaration;
		setDependent(param);
		makeUniqueTypeSafe(param);
		param.argument.swap(walker.argument);
		++count;
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		TypeParameterWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		SEMANTIC_ASSERT( walker.declaration != 0);
		param = walker.declaration;
		setDependent(param);
		makeUniqueTypeSafe(param);
		param.argument.swap(walker.argument);
		++count;
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		ParameterDeclarationWalker walker(getState(), false, count);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.declaration != 0);
		param = walker.declaration;
		if(walker.defaultArgument != 0)
		{
			addDependent(param.argument.valueDependent, walker.declaration->valueDependent);
			param.argument.type = &gNonType;
			param.argument.expression = walker.declaration->initializer;
		}
		++count;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
		params.push_front(walker.param);
	}
};

struct TemplateParameterClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateParameters params;
	TemplateParameterClauseWalker(const WalkerState& state)
		: WalkerBase(state), params(context)
	{
		// collect template-params into a new scope
		if(templateParamScope != 0)
		{
			pushScope(templateParamScope); // the existing template parameter scope encloses the new scope
		}
		pushScope(newScope(makeIdentifier("$template"), SCOPETYPE_TEMPLATE));
		clearTemplateParams();
		enclosing->templateDepth = templateDepth;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState(), 0);
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
		params.push_front(walker.param);
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	TemplateParameters params;
	TemplateDeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), params(context)
	{
		++templateDepth;
		templateParams = &TEMPLATEPARAMETERS_NULL; // explicit specialization has empty template params: template<> struct S;
	}
	void visit(cpp::template_parameter_clause* symbol)
	{
		TemplateParameterClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		templateParamScope = walker.enclosing;
		enclosing = walker.enclosing->parent;
		params.swap(walker.params);
		templateParams = &params;
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
};

struct ExplicitInstantiationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	ExplicitInstantiationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isExplicitInstantiation = true;
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
};

struct DeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	DeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::linkage_specification* symbol)
	{
		NamespaceWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::namespace_definition* symbol)
	{
		NamespaceWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::namespace_alias_definition* symbol)
	{
		NamespaceAliasDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::general_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	// occurs in for-init-statement
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::constructor_definition* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::explicit_instantiation* symbol)
	{
		ExplicitInstantiationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::explicit_specialization* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_directive* symbol)
	{
		UsingDirectiveWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct NamespaceWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	NamespaceWalker(WalkerContext& context)
		: WalkerBase(context), declaration(0), id(0)
	{
		pushScope(&context.global);
	}

	NamespaceWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		if(id != 0)
		{
			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, 0);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(instance);
#endif
			setDecoration(id, instance);
			declaration = instance;
			if(declaration->enclosed == 0)
			{
				declaration->enclosed = newScope(*id, SCOPETYPE_NAMESPACE);
			}
			pushScope(declaration->enclosed);
		}
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		IncludeEvents events = parser->get_events();
		TREEWALKER_WALK_SRC(walker, symbol);
		symbol->events = events;
	}
};

};



TreeAllocator<int> getAllocator(ParserContext& context)
{
#ifdef TREEALLOCATOR_LINEAR
	return context.allocator;
#else
	return DebugAllocator<int>();
#endif
}


cpp::declaration_seq* parseFile(ParserContext& context)
{
	gUniqueNames.clear();
	gUniqueTypes.clear();
	gUniqueExpressions.clear();

	WalkerContext& globals = *new WalkerContext(getAllocator(context));
	Walker::NamespaceWalker& walker = *new Walker::NamespaceWalker(globals);
	ParserGeneric<Walker::NamespaceWalker> parser(context, walker);

	cpp::symbol_sequence<cpp::declaration_seq> result(NULL);
	try
	{
		ProfileScope profile(gProfileParser);
		PARSE_SEQUENCE(parser, result);
	}
	catch(ParseError&)
	{
	}
	catch(SemanticError&)
	{
		printPosition(parser.context.getErrorPosition());
		std::cout << "caught SemanticError" << std::endl;
		throw;
	}
	catch(SymbolsError&)
	{
		printPosition(parser.context.getErrorPosition());
		std::cout << "caught SymbolsError" << std::endl;
		throw;
	}
	catch(TypeError& e)
	{
		e.report();
	}
	if(!context.finished())
	{
		printError(parser);
	}
	dumpProfile(gProfileIo);
	dumpProfile(gProfileWave);
	dumpProfile(gProfileParser);
	dumpProfile(gProfileLookup);
	dumpProfile(gProfileDiagnose);
	dumpProfile(gProfileAllocator);
	dumpProfile(gProfileIdentifier);
	dumpProfile(gProfileTemplateId);

	return result;
}

cpp::statement_seq* parseFunction(ParserContext& context)
{
	gUniqueNames.clear();
	gUniqueTypes.clear();
	gUniqueExpressions.clear();

	WalkerContext& globals = *new WalkerContext(getAllocator(context));
	Walker::CompoundStatementWalker& walker = *new Walker::CompoundStatementWalker(globals);
	ParserGeneric<Walker::CompoundStatementWalker> parser(context, walker);

	cpp::symbol_sequence<cpp::statement_seq> result(NULL);
	try
	{
		ProfileScope profile(gProfileParser);
		PARSE_SEQUENCE(parser, result);
	}
	catch(ParseError&)
	{
	}
	catch(TypeError& e)
	{
		e.report();
	}
	if(!context.finished())
	{
		printError(parser);
	}
	return result;
}


