

#include "semantic.h"
#include "util.h"

#include "profiler.h"
#include "symbols.h"

#include "parser_symbols.h"

#include <iostream>

#include <list> // deferred-parse


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
	SEMANTIC_ASSERT(id != &gAnonymousId);
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

inline UniqueTypeWrapper binaryOperatorAssignment(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return left;
}

inline UniqueTypeWrapper binaryOperatorComma(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return right;
}

inline UniqueTypeWrapper binaryOperatorBoolean(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return gBool;
}

inline UniqueTypeWrapper binaryOperatorMemberPointer(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return popType(right);
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

bool isUnqualified(cpp::elaborated_type_specifier_default* symbol)
{
	return symbol != 0
		&& symbol->isGlobal.value.empty()
		&& symbol->context.p == 0;
}




struct SemaContext : public TreeAllocator<int>
{
	ParserContext& parserContext;
	Scope global;
	Declaration globalDecl;
	TypeRef globalType;
	std::size_t declarationCount;
	UniqueTypeWrapper typeInfoType;

	SemaContext(ParserContext& parserContext, const TreeAllocator<int>& allocator) :
		TreeAllocator<int>(allocator),
		parserContext(parserContext),
		global(allocator, gGlobalId, SCOPETYPE_NAMESPACE),
		globalDecl(allocator, 0, gGlobalId, TYPE_NULL, &global),
		globalType(Type(&globalDecl, allocator), allocator),
		declarationCount(0)
	{
	}
};

typedef std::list< DeferredParse<struct SemaState> > DeferredSymbolsList;

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


struct SemaState
	: public ContextBase
{
	typedef SemaState State;

	SemaContext& context;
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
	DeferredSymbols* enclosingDeferred;
	std::size_t templateDepth;
	bool isExplicitInstantiation;

	SemaState(SemaContext& context)
		: context(context)
		, enclosing(0)
		, enclosingType(0)
		, qualifying_p(0)
		, qualifyingScope(0)
		, qualifyingClass(0)
		, memberClass(0)
		, templateParams(0)
		, templateParamScope(0)
		, enclosingDeferred(0)
		, templateDepth(0)
		, isExplicitInstantiation(false)
	{
	}
	const SemaState& getState() const
	{ 
		return *this;
	}
	Location getLocation() const
	{
		return Location(context.parserContext.get_source(), context.declarationCount);
	}
	InstantiationContext getInstantiationContext() const
	{
		return InstantiationContext(getLocation(), enclosingType, enclosing);
	}

	UniqueTypeWrapper getTypeInfoType()
	{
		if(context.typeInfoType == gUniqueTypeNull)
		{
			Identifier stdId = makeIdentifier(context.parserContext.makeIdentifier("std"));
			LookupResultRef declaration = ::findDeclaration(context.global, stdId, IsNestedName());
			SEMANTIC_ASSERT(declaration != 0);
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			SEMANTIC_ASSERT(declaration->enclosed->type == SCOPETYPE_NAMESPACE);
			Identifier typeInfoId = makeIdentifier(context.parserContext.makeIdentifier("type_info"));
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

		context.parserContext.allocator.deferredBacktrack(); // flush cached parse-tree

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
		return context.parserContext.allocator;
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
		declaration->enclosed->bases.push_front(base);
	}

	Declaration* getDeclaratorQualifying() const
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
	bool reportIdentifierMismatch(T* symbol, const Identifier& id, Declaration* declaration, const char* expected)
	{
#if 0
		gIdentifierMismatch = IdentifierMismatch(id, declaration, expected);
#endif
		return false;
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
	void addDependent(Dependent& dependent, const Dependent& other)
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




struct Args0
{
};

template<typename A1>
struct Args1
{
	A1 a1;
	Args1(A1 a1) : a1(a1)
	{
	}
};

template<typename A1, typename A2>
struct Args2
{
	A1 a1;
	A2 a2;
	Args2(A1 a1, A2 a2) : a1(a1), a2(a2)
	{
	}
};

struct InvokeNone
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		return true;
	}
};

struct InvokeChecked
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		return walker.action(symbol);
	}
};

struct InvokeUnchecked
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		walker.action(symbol);
		return true;
	}
};

struct InvokeCheckedResult
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		return walker.action(symbol, result);
	}
};

struct InvokeUncheckedResult
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		walker.action(symbol, result);
		return true;
	}
};


struct CommitNull
{
};

struct CommitEnable
{
};


template<typename Inner, typename Commit = CommitNull, typename Args = Args0>
struct SemaPush : Args
{
	typedef Args ArgsType;
	SemaPush(const Args& args)
		: Args(args)
	{
	}
};

template<typename Inner, typename Commit>
struct SemaPush<Inner, Commit, Args0>
{
	typedef SemaPush ArgsType;
};

template<typename SemaT, typename Inner, typename Commit>
Inner makeInnerWalker(SemaT& walker, const SemaPush<Inner, Commit, Args0>& args)
{
	return Inner(walker.getState());
}

template<typename SemaT, typename Inner, typename Commit, typename A1>
Inner makeInnerWalker(SemaT& walker, const SemaPush<Inner, Commit, Args1<A1> >& args)
{
	return Inner(walker.getState(), args.a1);
}

template<typename SemaT, typename Inner, typename Commit, typename A1, typename A2>
Inner makeInnerWalker(SemaT& walker, const SemaPush<Inner, Commit, Args2<A1, A2> >& args)
{
	return Inner(walker.getState(), args.a1, args.a2);
}

template<typename T, T m>
struct SfinaeNonType
{
	typedef void Type;
};



template<typename SemaT, LexTokenId ID, typename U = void>
struct HasAction
{
	static const bool value = false;
};

template<typename SemaT, LexTokenId ID>
struct HasAction<SemaT, ID, typename SfinaeNonType<void(SemaT::*)(cpp::terminal<ID>), &SemaT::action>::Type>
{
	static const bool value = true;
};


template<typename SemaT, LexTokenId ID>
typename EnableIf<!HasAction<SemaT, ID>::value>::Type
	semaAction(SemaT& walker, cpp::terminal<ID>)
{
	// do nothing
}

template<typename SemaT, LexTokenId ID>
typename EnableIf<HasAction<SemaT, ID>::value>::Type
	semaAction(SemaT& walker, cpp::terminal<ID> symbol)
{
	walker.action(symbol);
}


template<typename SemaT, typename Inner, typename Args>
void semaCommit(SemaT& walker, const SemaPush<Inner, CommitNull, Args>& inner)
{
	// do nothing
}

template<typename SemaT, typename Inner, typename Args>
void semaCommit(SemaT& walker, const SemaPush<Inner, CommitEnable, Args>& inner)
{
	walker.commit();
}

struct Once
{
	bool done;
	Once()
		: done(false)
	{
	}
	void operator()()
	{
		SEMANTIC_ASSERT(!done);
		done = true;
	}
	void test() const
	{
		SEMANTIC_ASSERT(done);
	}
};

struct SemaIdentity
{
	typedef SemaIdentity ArgsType;
};

template<typename SemaT>
SemaT& makeInnerWalker(SemaT& walker, const SemaIdentity&)
{
	return walker;
}

template<typename SemaT>
void semaCommit(SemaT& walker, const SemaIdentity& inner)
{
	// do nothing
}

struct Nothing
{
};

struct AnnotateNull
{
	typedef Nothing Data;
	static Data makeData(const Token& token)
	{
		return Nothing();
	}
	template<typename T>
	static void annotate(T* symbol, const Nothing&)
	{
	}
};

struct AnnotateSrc
{
	typedef Source Data;
	static Data makeData(const Token& token)
	{
		return token.source;
	}
	template<typename T>
	static void annotate(T* symbol, const Source& source)
	{
		symbol->source = source;
	}
};

struct AnnotateId
{
	typedef Source Data;
	static Data makeData(const Token& token)
	{
		return token.source;
	}
	template<typename T>
	static void annotate(T* symbol, const Source& source)
	{
		symbol->source = source;
	}
};

struct SourceEvents : Source, IncludeEvents
{
	SourceEvents(const Source& source, const IncludeEvents& events)
		: Source(source), IncludeEvents(events)
	{
	}
};

struct AnnotateTop
{
	typedef SourceEvents Data;
	static Data makeData(const Token& token)
	{
		return SourceEvents(token.source, token.events);
	}
	static void annotate(cpp::declaration* symbol, const Data& data)
	{
		symbol->source = data;
		symbol->events = data;
	}
};



template<typename Inner, typename Annotate = AnnotateNull, typename Invoke = InvokeUncheckedResult, typename Cache = DisableCache, typename Defer = DeferDefault>
struct SemaPolicyGeneric : Inner, Annotate, Invoke, Cache, Defer
{
	typedef typename Inner::ArgsType ArgsType;
	SemaPolicyGeneric(const ArgsType& args = ArgsType())
		: Inner(args)
	{
	}
	const Inner& getInnerPolicy() const
	{
		return *this;
	}
	typedef Annotate AnnotateType;
	const Annotate& getAnnotatePolicy() const
	{
		return *this;
	}
	const Cache& getCachePolicy() const
	{
		return *this;
	}
	const Defer& getDeferPolicy() const
	{
		return *this;
	}
	const Invoke& getActionPolicy() const
	{
		return *this;
	}
};

#ifdef _WIN32
#define SEMA_INLINE __forceinline 
#endif

typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeNone> SemaPolicyNone;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeUnchecked> SemaPolicyIdentity;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateSrc, InvokeUnchecked> SemaPolicySrc;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeChecked> SemaPolicyIdentityChecked;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeUnchecked, EnableCache> SemaPolicyIdentityCached;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeChecked, EnableCache> SemaPolicyIdentityCachedChecked;
template<typename SemaT>
struct SemaPolicyPush : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateNull> {};
template<typename SemaT>
struct SemaPolicyPushCommit : SemaPolicyGeneric<SemaPush<SemaT, CommitEnable>, AnnotateNull> {};
template<typename SemaT>
struct SemaPolicyPushSrc : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateSrc> {};
template<typename SemaT>
struct SemaPolicyPushId : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateId> {};
template<typename SemaT>
struct SemaPolicyPushIdCommit : SemaPolicyGeneric<SemaPush<SemaT, CommitEnable>, AnnotateId> {};
template<typename SemaT>
struct SemaPolicyPushTop : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateTop> {};
template<typename SemaT>
struct SemaPolicyPushSrcChecked : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateSrc, InvokeCheckedResult> {};
template<typename SemaT>
struct SemaPolicyPushIdChecked : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateId, InvokeCheckedResult> {};
template<typename SemaT>
struct SemaPolicyPushChecked : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateNull, InvokeCheckedResult> {};
template<typename SemaT>
struct SemaPolicyPushCached : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateNull, InvokeUncheckedResult, CachedWalk> {};
template<typename SemaT>
struct SemaPolicyPushCachedChecked : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateNull, InvokeCheckedResult, CachedWalk> {};
template<typename SemaT>
struct SemaPolicyPushBool : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<bool> >, AnnotateNull, InvokeUncheckedResult, DisableCache>
{
	SemaPolicyPushBool(bool value) : SemaPolicyGeneric(Args1<bool>(value))
	{
	}
};
template<typename SemaT>
struct SemaPolicyPushCheckedBool : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<bool> >, AnnotateNull, InvokeCheckedResult, DisableCache>
{
	SemaPolicyPushCheckedBool(bool value) : SemaPolicyGeneric(Args1<bool>(value))
	{
	}
};
template<typename SemaT>
struct SemaPolicyPushCachedBool : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<bool> >, AnnotateNull, InvokeUncheckedResult, CachedWalk>
{
	SemaPolicyPushCachedBool(bool value) : SemaPolicyGeneric(Args1<bool>(value))
	{
	}
};
template<typename SemaT>
struct SemaPolicyPushCachedCheckedBool : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<bool> >, AnnotateNull, InvokeCheckedResult, CachedWalk>
{
	SemaPolicyPushCachedCheckedBool(bool value) : SemaPolicyGeneric(Args1<bool>(value))
	{
	}
};
template<typename SemaT>
struct SemaPolicyPushIndexCommit : SemaPolicyGeneric<SemaPush<SemaT, CommitEnable, Args1<std::size_t> >, AnnotateNull, InvokeUncheckedResult, DisableCache>
{
	SemaPolicyPushIndexCommit(std::size_t value) : SemaPolicyGeneric(Args1<std::size_t>(value))
	{
	}
};

template<typename SemaT, typename Defer>
struct SemaPolicyPushDeferred : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args0>, AnnotateNull, InvokeUncheckedResult, DisableCache, Defer>
{
};




#define SEMA_POLICY(Symbol, Policy) \
	SEMA_INLINE Policy makePolicy(Symbol*) \
	{ \
		return Policy(); \
	}

#define SEMA_POLICY_ARGS(Symbol, Policy, args) \
	SEMA_INLINE Policy makePolicy(Symbol*) \
	{ \
		return Policy(args); \
	}

#define SEMA_BOILERPLATE \
	template<typename T> \
	SemaPolicyNone makePolicy(T* symbol) \
	{ \
		return SemaPolicyNone(); \
	}


struct SemaDeclSpecifierSeqResult
{
	Type type;
	IdentifierPtr forward; // if this is an elaborated-type-specifier, the 'identifier'
	CvQualifiers qualifiers;
	DeclSpecifiers specifiers;
	bool isUnion;
	SemaDeclSpecifierSeqResult(Declaration* declaration, const TreeAllocator<int>& allocator)
		: type(declaration, allocator), forward(0), isUnion(false)
	{
	}
};

struct SemaDeclarationArgs
{
	bool isParameter;
	size_t templateParameter;
	SemaDeclarationArgs(bool isParameter = false, size_t templateParameter = INDEX_INVALID)
		: isParameter(isParameter), templateParameter(templateParameter)
	{
	}
};

template<typename SemaT>
struct SemaPolicyParameterDeclaration : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<SemaDeclarationArgs> > >
{
	SemaPolicyParameterDeclaration(SemaDeclarationArgs value) : SemaPolicyGeneric(value)
	{
	}
};

struct SemaExpressionResult
{
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	UniqueTypeId type;
	ExpressionWrapper expression;
	/* 14.6.2.2-1
	...an expression is type-dependent if any subexpression is type-dependent.
	*/
	Dependent typeDependent;
	Dependent valueDependent;

	SemaExpressionResult()
		: id(0)
	{
	}
};

struct SemaTypeIdResult
{
	TypeId type;
	Once committed;
	SemaTypeIdResult(const TreeAllocator<int>& allocator)
		: type(0, allocator)
	{
	}
};

struct SemaTemplateParameterClauseResult
{
	TemplateParameters params;
	SemaTemplateParameterClauseResult(const TreeAllocator<int>& allocator)
		: params(allocator)
	{
	}
};

struct SemaTypeSpecifierResult
{
	Type type;
	unsigned fundamental;
	SemaTypeSpecifierResult(const TreeAllocator<int>& allocator)
		: type(0, allocator), fundamental(0)
	{
	}
};

struct SemaTypenameSpecifierResult
{
	Type type;
	SemaTypenameSpecifierResult(const TreeAllocator<int>& allocator)
		: type(0, allocator)
	{
	}
};

struct SemaNewTypeResult
{
	TypeId type;
	Dependent valueDependent;
	SemaNewTypeResult(const TreeAllocator<int>& allocator)
		: type(0, allocator)
	{
	}
};

struct SemaSimpleDeclarationResult
{
	DeclarationPtr declaration; // the result of the declaration
	cpp::default_argument* defaultArgument; // parsing of this symbol will be deferred if this is a member-declaration

	SemaSimpleDeclarationResult()
		: declaration(0), defaultArgument(0)
	{
	}
};

struct SemaDeclarationResult
{
	DeclarationPtr declaration;

	SemaDeclarationResult()
		: declaration(0)
	{
	}
};

typedef SemaDeclarationResult SemaNamespaceResult;



struct SemaBase : public SemaState
{
	typedef SemaBase Base;

	SemaBase(SemaContext& context)
		: SemaState(context)
	{
	}
	SemaBase(const SemaState& state)
		: SemaState(state)
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

	void addBacktrackCallback(const BacktrackCallback& callback)
	{
		context.parserContext.allocator.addBacktrackCallback(context.parserContext.allocator.position, callback);
	}

	void disableBacktrack()
	{
		addBacktrackCallback(makeBacktrackErrorCallback());
	}

	// Causes /p declaration to be undeclared when backtracking.
	// In practice this only happens for the declaration in an elaborated-type-specifier.
	void trackDeclaration(const DeclarationInstance& declaration)
	{
		addBacktrackCallback(makeUndeclareCallback(&declaration));
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

struct SemaQualifyingResult
{
	Qualifying qualifying;
	SemaQualifyingResult(const TreeAllocator<int>& allocator)
		: qualifying(allocator)
	{
	}
};

struct SemaQualified : public SemaBase, SemaQualifyingResult
{
	SemaQualified(const SemaState& state)
		: SemaBase(state), SemaQualifyingResult(context)
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
	void swapQualifying(const Qualifying& other, bool isDeclarator = false)
	{
		qualifying = other;
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


struct SemaNamespaceName : public SemaBase
{
	SEMA_BOILERPLATE;

	LookupResultRef declaration;
	IsHiddenNamespaceName filter;
	SemaNamespaceName(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCachedChecked)
	bool action(cpp::identifier* symbol)
	{
		declaration = findDeclaration(symbol->value, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
		}
		setDecoration(&symbol->value, declaration);
		return true;
	}
};

struct SemaTemplateArgumentList : public SemaBase
{
	SEMA_BOILERPLATE;

	TemplateArgument argument;
	TemplateArguments arguments;
	Once committed;

	SemaTemplateArgumentList(const SemaState& state)
		: SemaBase(state), argument(context), arguments(context)
	{
		clearQualifying();
	}
	void commit()
	{
		committed();
		arguments.push_front(argument); // allocates last element first!
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		argument.type = walker.type;
		argument.source = getLocation();
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPushChecked<struct SemaExpression>)
	bool action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		if(walker.expression.isTemplateArgumentAmbiguity)
		{
			// [temp.arg] In a template argument, an ambiguity between a typeid and an expression is resolved to a typeid
			return false; // fail parse, will retry for a type-id
		}
		addDependent(argument.valueDependent, walker.valueDependent);
		argument.type = &gNonType;
		argument.expression = walker.expression;
		argument.source = getLocation();
		return true;
	}
	SEMA_POLICY(cpp::template_argument_list, SemaPolicyPushCommit<struct SemaTemplateArgumentList>)
	void action(cpp::template_argument_list* symbol, const SemaTemplateArgumentList& walker)
	{
		walker.committed.test();
		arguments = walker.arguments;
	}
};

struct SemaOverloadableOperator : public SemaBase
{
	Name name;
	SemaOverloadableOperator(const SemaState& state)
		: SemaBase(state)
	{
	}
	template<typename T>
	SemaPolicyIdentity makePolicy(T symbol)
	{
		return SemaPolicyIdentity();
	}
	template<typename T>
	void action(T* symbol)
	{
		name = getOverloadableOperatorId(symbol);
	}
	template<LexTokenId id>
	void action(cpp::terminal<id> symbol)
	{
	}
};

struct SemaOperatorFunctionId : public SemaBase
{
	SEMA_BOILERPLATE;

	Name name;
	SemaOperatorFunctionId(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::overloadable_operator, SemaPolicyPush<struct SemaOverloadableOperator>)
	void action(cpp::overloadable_operator* symbol, SemaOverloadableOperator& walker)
	{
		name = walker.name;
	}
};

struct SemaTemplateId : public SemaBase
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	TemplateArguments arguments;
	bool isTemplate; // true if the template-id is preceded by 'template'
	SemaTemplateId(const SemaState& state, bool isTemplate = false)
		: SemaBase(state), id(0), arguments(context), isTemplate(isTemplate)
	{
	}
	template<typename T>
	bool verifyTemplateName(T* symbol)
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
				return true; // don't bother checking for a template-name, the following cannot be 'less-than'.. we hope.
			}

			// search qualifying type/namespace, object-expression and/or enclosing scope depending on context
			LookupResultRef declaration = findDeclaration(symbol->value, IsAny());
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "template-name");
			}
		}
		return true;
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCachedChecked)
	bool action(cpp::identifier* symbol)
	{
		id = &symbol->value;
		return verifyTemplateName(symbol);
	}
	SEMA_POLICY(cpp::operator_function_id, SemaPolicyPushIdChecked<struct SemaOperatorFunctionId>)
	bool action(cpp::operator_function_id* symbol, SemaOperatorFunctionId& walker) 
	{
		symbol->value.value = walker.name;
		id = &symbol->value;
		return verifyTemplateName(symbol);
	}
	SEMA_POLICY(cpp::template_argument_clause, SemaPolicyPushCached<struct SemaTemplateArgumentList>)
	void action(cpp::template_argument_clause* symbol, SemaTemplateArgumentList& walker)
	{
		arguments = walker.arguments;
	}
};

struct SemaUnqualifiedId : public SemaBase
{
	SEMA_BOILERPLATE;

	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isIdentifier;
	bool isTemplate;
	SemaUnqualifiedId(const SemaState& state, bool isTemplate = false)
		: SemaBase(state), id(0), arguments(context), isIdentifier(false), isTemplate(isTemplate)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
		isIdentifier = true;
		if(allowNameLookup())
		{
			declaration = findDeclaration(*id, IsAny(), true);
		}
	}
	SEMA_POLICY_ARGS(cpp::simple_template_id, SemaPolicyPushCachedCheckedBool<struct SemaTemplateId>, isTemplate)
	bool action(cpp::simple_template_id* symbol, SemaTemplateId& walker)
	{
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
		arguments = walker.arguments;
		return true;
	}
	SEMA_POLICY_ARGS(cpp::template_id_operator_function, SemaPolicyPushCachedCheckedBool<struct SemaTemplateId>, isTemplate)
	bool action(cpp::template_id_operator_function* symbol, SemaTemplateId& walker)
	{
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
		arguments = walker.arguments;
		return true;
	}
	SEMA_POLICY(cpp::operator_function_id, SemaPolicyPushId<struct SemaOperatorFunctionId>)
	void action(cpp::operator_function_id* symbol, const SemaOperatorFunctionId& walker)
	{
		symbol->value.value = walker.name;
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
	SEMA_POLICY(cpp::conversion_function_id, SemaPolicyPushIdCommit<struct SemaTypeId>)
	void action(cpp::conversion_function_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		symbol->value = gConversionFunctionId;
		id = &symbol->value;
		if(allowNameLookup())
		{
			declaration = findDeclaration(*id, IsAny(), true);
		}
	}
	SEMA_POLICY(cpp::destructor_id, SemaPolicyIdentityChecked)
	bool action(cpp::destructor_id* symbol)
	{
		// TODO: can destructor-id be dependent?
		id = &symbol->name->value;
		if(objectExpression.p == 0)
		{
			// destructor id can only appear in class member access expression
			return reportIdentifierMismatch(symbol, *id, declaration, "class member access expression");
		}
		declaration = gDestructorInstance;
		return true;
	}
};

struct SemaQualifiedId : public SemaQualified
{
	SEMA_BOILERPLATE;

	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isTemplate;
	SemaQualifiedId(const SemaState& state)
		: SemaQualified(state), id(0), arguments(context), isTemplate(false)
	{
	}

	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}

	SEMA_POLICY_ARGS(cpp::unqualified_id, SemaPolicyPushBool<struct SemaUnqualifiedId>, isTemplate)
	void action(cpp::unqualified_id* symbol, const SemaUnqualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
	}
	SEMA_POLICY_ARGS(cpp::qualified_id_suffix, SemaPolicyPushBool<struct SemaUnqualifiedId>, isTemplate)
	void action(cpp::qualified_id_suffix* symbol, const SemaUnqualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
	}
};


struct SemaIdExpression : public SemaQualified
{
	SEMA_BOILERPLATE;

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
	SemaIdExpression(const SemaState& state, bool isTemplate = false)
		: SemaQualified(state), id(0), arguments(context), isIdentifier(false), isUndeclared(false), isTemplate(isTemplate)
	{
	}
	SEMA_POLICY(cpp::qualified_id_default, SemaPolicyPush<struct SemaQualifiedId>)
	void action(cpp::qualified_id_default* symbol, const SemaQualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::qualified_id_global, SemaPolicyPush<struct SemaQualifiedId>)
	void action(cpp::qualified_id_global* symbol, const SemaQualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::qualified_id, SemaPolicyIdentity)
	void action(cpp::qualified_id* symbol)
	{
		// [temp.dep.expr] An id-expression is type-dependent if it contains:- a nested-name-specifier that contains a class-name that names a dependent type
		setDependent(typeDependent, qualifying.get());
		setDependent(valueDependent, qualifying.get()); // it's clearly value-dependent too, because name lookup must be deferred
	}
	SEMA_POLICY_ARGS(cpp::unqualified_id, SemaPolicyPushBool<struct SemaUnqualifiedId>, isTemplate)
	void action(cpp::unqualified_id* symbol, const SemaUnqualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
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

struct SemaExplicitTypeExpression : public SemaBase
{
	SEMA_BOILERPLATE;

	TypeId type;
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWrapper expression;
	SemaExplicitTypeExpression(const SemaState& state)
		: SemaBase(state), type(0, context)
	{
	}
	SEMA_POLICY(cpp::simple_type_specifier, SemaPolicyPush<struct SemaTypeSpecifier>)
	void action(cpp::simple_type_specifier* symbol, const SemaTypeSpecifierResult& walker)
	{
		type = walker.type;
		if(type.declaration == 0)
		{
			type = getFundamentalType(walker.fundamental);
		}
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type);
	}
	SEMA_POLICY(cpp::typename_specifier, SemaPolicyPush<struct SemaTypenameSpecifier>)
	void action(cpp::typename_specifier* symbol, const SemaTypenameSpecifierResult& walker)
	{
		type = walker.type;
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type);
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		type = walker.type;
		addDependent(typeDependent, type);
	}
	SEMA_POLICY(cpp::new_type, SemaPolicyPush<struct SemaNewType>)
	void action(cpp::new_type* symbol, const SemaNewTypeResult& walker)
	{
		type = walker.type;
		addDependent(typeDependent, type);
		addDependent(typeDependent, walker.valueDependent);
		makeUniqueTypeSafe(type);
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
	SEMA_POLICY(cpp::cast_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::cast_expression* symbol, const SemaExpressionResult& walker)
	{
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct SemaArgumentList : public SemaBase
{
	SEMA_BOILERPLATE;

	Arguments arguments;
	Dependent typeDependent;
	Dependent valueDependent;
	SemaArgumentList(const SemaState& state)
		: SemaBase(state)
	{
		clearQualifying();
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		arguments.push_back(Argument(walker.expression, walker.type));
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct SemaSubscript : public SemaBase
{
	SEMA_BOILERPLATE;

	ExpressionWrapper expression;
	UniqueTypeWrapper type;
	Dependent typeDependent;
	Dependent valueDependent;
	SemaSubscript(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_LEFTBRACKET>)
	{
		clearQualifying(); // the expression in [] is looked up in the context of the entire postfix expression
	}
	SEMA_POLICY(cpp::expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression* symbol, const SemaExpressionResult& walker)
	{
		expression = walker.expression;
		type = walker.type;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
};


struct SemaLiteral : public SemaBase
{
	SEMA_BOILERPLATE;

	ExpressionWrapper expression;
	SemaLiteral(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::numeric_literal, SemaPolicyIdentity)
	void action(cpp::numeric_literal* symbol)
	{
		if(symbol->id == cpp::numeric_literal::UNKNOWN) // workaround for boost::wave issue: T_PP_NUMBER exists in final token stream
		{
			symbol->id = isFloatingLiteral(symbol->value.value.c_str()) ? cpp::numeric_literal::FLOATING : cpp::numeric_literal::INTEGER;
		}
		expression = makeUniqueExpression(parseNumericLiteral(symbol));
	}
	SEMA_POLICY(cpp::string_literal, SemaPolicyIdentity)
	void action(cpp::string_literal* symbol)
	{
		expression = makeUniqueExpression(IntegralConstantExpression(getStringLiteralType(symbol), IntegralConstant()));
	}
};

struct SemaPrimaryExpression : public SemaBase
{
	SEMA_BOILERPLATE;

	UniqueTypeId type;
	ExpressionWrapper expression;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	TemplateArguments arguments; // only valid when the expression is a (qualified) template-id
	const SimpleType* idEnclosing; // may be valid when the above id-expression is a qualified-id
	Dependent typeDependent;
	Dependent valueDependent;
	bool isUndeclared;
	SemaPrimaryExpression(const SemaState& state)
		: SemaBase(state), id(0), arguments(context), idEnclosing(0), isUndeclared(false)
	{
	}
	SEMA_POLICY(cpp::literal, SemaPolicyPush<struct SemaLiteral>)
	void action(cpp::literal* symbol, const SemaLiteral& walker)
	{
		expression = walker.expression;
		type = typeOfExpression(expression, getInstantiationContext());
		SEMANTIC_ASSERT(!type.empty());
	}
	SEMA_POLICY(cpp::id_expression, SemaPolicyPushChecked<struct SemaIdExpression>)
	bool action(cpp::id_expression* symbol, SemaIdExpression& walker)
	{
		if(!walker.commit())
		{
			return reportIdentifierMismatch(symbol, *walker.id, walker.declaration, "object-name");
		}
		id = walker.id;
		arguments = walker.arguments;
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
		return true;
	}
	SEMA_POLICY(cpp::primary_expression_parenthesis, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::primary_expression_parenthesis* symbol, const SemaExpressionResult& walker)
	{
		type = walker.type;
		expression = walker.expression;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
	SEMA_POLICY(cpp::primary_expression_builtin, SemaPolicyIdentity)
	void action(cpp::primary_expression_builtin* symbol)
	{
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

struct SemaPostfixExpressionMember : public SemaQualified
{
	SEMA_BOILERPLATE;

	ExpressionWrapper expression;
	LookupResultRef declaration;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	TemplateArguments arguments; // only used if the identifier is a template-name
	Dependent typeDependent;
	Dependent valueDependent;
	bool isTemplate;
	SemaPostfixExpressionMember(const SemaState& state)
		: SemaQualified(state), id(0), arguments(context), isTemplate(false)
	{
	}
	SEMA_POLICY(cpp::member_operator, SemaPolicyIdentity)
	void action(cpp::member_operator* symbol)
	{
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
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	SEMA_POLICY_ARGS(cpp::id_expression, SemaPolicyPushBool<struct SemaIdExpression>, isTemplate)
	void action(cpp::id_expression* symbolm, SemaIdExpression& walker)
	{
		bool isObjectName = walker.commit();
		SEMANTIC_ASSERT(isObjectName); // TODO: non-fatal error: expected object name
		id = walker.id;
		arguments = walker.arguments;
		declaration = walker.declaration;
		swapQualifying(walker.qualifying);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = walker.expression;
	}
};

struct SemaTypeTraitsIntrinsic : public SemaBase
{
	SEMA_BOILERPLATE;

	Dependent valueDependent;
	UniqueTypeWrapper first;
	UniqueTypeWrapper second;
	SemaTypeTraitsIntrinsic(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		// debugging
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		addDependent(valueDependent, walker.type);

		UniqueTypeWrapper type = UniqueTypeWrapper(walker.type.unique);
		setExpressionType(symbol, type);

		(first == gUniqueTypeNull ? first : second) = type;
	}
};

struct SemaPostfixExpression : public SemaBase
{
	SEMA_BOILERPLATE;

	UniqueTypeId type;
	ExpressionWrapper expression;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	TemplateArguments arguments; // only valid when the expression is a (qualified) template-id
	const SimpleType* idEnclosing; // may be valid when the above id-expression is a qualified-id
	Dependent typeDependent;
	Dependent valueDependent;
	bool isUndeclared;
	SemaPostfixExpression(const SemaState& state)
		: SemaBase(state), id(0), arguments(context), idEnclosing(0), isUndeclared(false)
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
	SEMA_POLICY(cpp::primary_expression, SemaPolicyPush<struct SemaPrimaryExpression>)
	void action(cpp::primary_expression* symbol, const SemaPrimaryExpression& walker)
	{
		type = walker.type;
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		id = walker.id;
		arguments = walker.arguments;
		idEnclosing = walker.idEnclosing;
		isUndeclared = walker.isUndeclared;
		setExpressionType(symbol, type);
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_construct, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::postfix_expression_construct* symbol, const SemaExplicitTypeExpression& walker)
	{
		addDependent(typeDependent, walker.typeDependent);
		type = getUniqueTypeSafe(walker.type);
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, getInstantiationContext());
		expression = makeExpression(CastExpression(type, walker.expression), walker.expression.isConstant, isDependent(typeDependent), false);
		expression.isTemplateArgumentAmbiguity = symbol->args == 0;
		setExpressionType(symbol, type);
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_cast, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::postfix_expression_cast* symbol, const SemaExplicitTypeExpression& walker)
	{
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
	SEMA_POLICY(cpp::postfix_expression_typeid, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::postfix_expression_typeid* symbol, const SemaExpressionResult& walker)
	{
		// TODO: operand type required to be complete?
		type = getTypeInfoType();
		expression = makeExpression(ExplicitTypeExpression(type));
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_typeidtype, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::postfix_expression_typeidtype* symbol, const SemaTypeIdResult& walker)
	{
		// TODO: operand type required to be complete?
		type = getTypeInfoType();
		expression = makeExpression(ExplicitTypeExpression(type));
		updateMemberType();
	}

	// suffix
	SEMA_POLICY(cpp::postfix_expression_subscript, SemaPolicyPushSrc<struct SemaSubscript>)
	void action(cpp::postfix_expression_subscript* symbol, const SemaSubscript& walker)
	{
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
	SEMA_POLICY(cpp::postfix_expression_call, SemaPolicyPushSrc<struct SemaArgumentList>)
	void action(cpp::postfix_expression_call* symbol, const SemaArgumentList& walker)
	{
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

	SEMA_POLICY(cpp::postfix_expression_member, SemaPolicyPushSrc<struct SemaPostfixExpressionMember>)
	void action(cpp::postfix_expression_member* symbol, const SemaPostfixExpressionMember& walker)
	{
		setExpressionType(symbol, type);
		id = walker.id; // perform overload resolution for a.m(x);
		arguments = walker.arguments;
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
	SEMA_POLICY(cpp::postfix_expression_destructor, SemaPolicySrc)
	void action(cpp::postfix_expression_destructor* symbol)
	{
		setExpressionType(symbol, type);
		type = gVoid; // TODO: should this be null-type?
		id = 0;
		expression = ExpressionWrapper();
		// TODO: name-lookup for destructor name
		clearMemberType();
	}
	SEMA_POLICY(cpp::postfix_operator, SemaPolicySrc)
	void action(cpp::postfix_operator* symbol)
	{
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
	SEMA_POLICY(cpp::postfix_expression_typetraits_unary, SemaPolicyPush<struct SemaTypeTraitsIntrinsic>)
	void action(cpp::postfix_expression_typetraits_unary* symbol, const SemaTypeTraitsIntrinsic& walker)
	{
		addDependent(valueDependent, walker.valueDependent);
		type = gBool;
		UnaryTypeTraitsOp operation = getUnaryTypeTraitsOp(symbol->trait);
		Name name = getTypeTraitName(symbol);
		expression = makeExpression(TypeTraitsUnaryExpression(name, operation, walker.first), true, false, isDependent(valueDependent));
	}
	SEMA_POLICY(cpp::postfix_expression_typetraits_binary, SemaPolicyPush<struct SemaTypeTraitsIntrinsic>)
	void action(cpp::postfix_expression_typetraits_binary* symbol, const SemaTypeTraitsIntrinsic& walker)
	{
		addDependent(valueDependent, walker.valueDependent);
		type = gBool;
		BinaryTypeTraitsOp operation = getBinaryTypeTraitsOp(symbol->trait);
		Name name = getTypeTraitName(symbol);
		expression = makeExpression(TypeTraitsBinaryExpression(name, operation, walker.first, walker.second), true, false, isDependent(valueDependent));
	}
};

struct SemaSizeofTypeExpression : public SemaBase
{
	SEMA_BOILERPLATE;

	Dependent valueDependent;
	ExpressionWrapper expression;
	SemaSizeofTypeExpression(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		// [temp.dep.expr] Expressions of the following form [sizeof(T)] are never type-dependent (because the type of the expression cannot be dependent)
		// [temp.dep.constexpr] Expressions of the following form [sizeof(T)] are value-dependent if ... the type-id is dependent
		addDependent(valueDependent, walker.type);

		UniqueTypeId type = getUniqueTypeSafe(walker.type);
		setExpressionType(symbol, type);

		expression = makeExpression(SizeofTypeExpression(type), true, false, isDependent(valueDependent));
	}
};

struct SemaConditionalExpression : public SemaBase
{
	SEMA_BOILERPLATE;

	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWrapper left;
	ExpressionWrapper right;
	UniqueTypeWrapper leftType;
	UniqueTypeWrapper rightType;
	SemaConditionalExpression(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression* symbol, const SemaExpressionResult& walker)
	{
		left = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		leftType = walker.type;
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		right = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		rightType = walker.type;
	}
};

struct SemaExpression : SemaBase, SemaExpressionResult
{
	SEMA_BOILERPLATE;

	SemaExpression(const SemaState& state)
		: SemaBase(state)
	{
	}

	// this path handles the right-hand side of a binary expression
	// it is assumed that 'type' already contains the type of the left-hand side
	template<BuiltInBinaryTypeOp typeOp, typename T>
	void walkBinaryExpression(T*& symbol, const SemaExpressionResult& walker)
	{
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
	void walkBinaryArithmeticExpression(T* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorArithmeticType>(symbol, walker);
	}
	template<typename T>
	void walkBinaryAdditiveExpression(T* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorAdditiveType>(symbol, walker);
	}
	template<typename T>
	void walkBinaryIntegralExpression(T* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorIntegralType>(symbol, walker);
	}
	template<typename T>
	void walkBinaryBooleanExpression(T* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorBoolean>(symbol, walker);
	}
	SEMA_POLICY(cpp::assignment_expression_suffix, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::assignment_expression_suffix* symbol, const SemaExpressionResult& walker)
	{
		// 5.1.7 Assignment operators
		// the type of an assignment expression is that of its left operand
		walkBinaryExpression<binaryOperatorAssignment>(symbol, walker);
	}
	SEMA_POLICY(cpp::conditional_expression_suffix, SemaPolicyPush<struct SemaConditionalExpression>)
	void action(cpp::conditional_expression_suffix* symbol, const SemaConditionalExpression& walker)
	{
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
	SEMA_POLICY(cpp::logical_or_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::logical_or_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::logical_and_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::logical_and_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryBooleanExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::inclusive_or_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::inclusive_or_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::exclusive_or_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::exclusive_or_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::and_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::and_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::equality_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::equality_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryBooleanExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::relational_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::relational_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryBooleanExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::shift_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::shift_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::additive_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::additive_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryAdditiveExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::multiplicative_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::multiplicative_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryArithmeticExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::pm_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::pm_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorMemberPointer>(symbol, walker);
		id = 0; // not a parenthesised id-expression, expression is not 'call to named function' [over.call.func]
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker) // expression_list, assignment_expression_suffix, conditional_expression_suffix
	{
		// [expr.comma] The type and value of the result are the type and value of the right operand
		expression = walker.expression;
		type = walker.type;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		setExpressionType(symbol, type);
	}
	SEMA_POLICY(cpp::expression_list, SemaPolicyIdentity)
	void action(cpp::expression_list* symbol) // a comma-separated list of assignment_expression
	{
		setExpressionType(symbol, type);
	}
	SEMA_POLICY(cpp::postfix_expression, SemaPolicyPushSrcChecked<struct SemaPostfixExpression>)
	bool action(cpp::postfix_expression* symbol, const SemaPostfixExpression& walker)
	{
		if(walker.isUndeclared)
		{
			return reportIdentifierMismatch(symbol, *id, &gUndeclared, "object-name");
		}
		id = walker.id;
		type = walker.type;
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		//setDependent(dependent, walker.dependent); // TODO:
		setExpressionType(symbol, type);
		return true;
	}
	SEMA_POLICY(cpp::unary_expression_op, SemaPolicyIdentity)
	void action(cpp::unary_expression_op* symbol)
	{
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
	SEMA_POLICY(cpp::new_expression_placement, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::new_expression_placement* symbol, const SemaExplicitTypeExpression& walker)
	{
		type = getUniqueTypeSafe(walker.type);
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		requireCompleteObjectType(type, getInstantiationContext());
		type.push_front(PointerType());
		addDependent(typeDependent, walker.typeDependent);
		expression = makeExpression(ExplicitTypeExpression(type), false, isDependent(typeDependent));
		setExpressionType(symbol, type);
	}
	SEMA_POLICY(cpp::new_expression_default, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::new_expression_default* symbol, const SemaExplicitTypeExpression& walker)
	{
		type = getUniqueTypeSafe(walker.type);
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		requireCompleteObjectType(type, getInstantiationContext());
		type.push_front(PointerType());
		addDependent(typeDependent, walker.typeDependent);
		expression = makeExpression(ExplicitTypeExpression(type), false, isDependent(typeDependent));
		setExpressionType(symbol, type);
	}
	SEMA_POLICY(cpp::cast_expression_default, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::cast_expression_default* symbol, const SemaExplicitTypeExpression& walker)
	{
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
	SEMA_POLICY(cpp::unary_expression_sizeof, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::unary_expression_sizeof* symbol, const SemaExpressionResult& walker)
	{
		// [temp.dep.expr] Expressions of the following form [sizeof(expr)] are never type-dependent (because the type of the expression cannot be dependent)
		// [temp.dep.constexpr] Expressions of the following form [sizeof(expr)] are value-dependent if the unary-expression is type-dependent
		addDependent(valueDependent, walker.typeDependent);
		type = gUnsignedInt;
		setExpressionType(symbol, type);
		expression = makeExpression(SizeofExpression(walker.expression), true, false, isDependent(valueDependent));
	}
	SEMA_POLICY(cpp::unary_expression_sizeoftype, SemaPolicyPushSrc<struct SemaSizeofTypeExpression>)
	void action(cpp::unary_expression_sizeoftype* symbol, const SemaSizeofTypeExpression& walker)
	{
		addDependent(valueDependent, walker.valueDependent);
		type = gUnsignedInt;
		setExpressionType(symbol, type);
		expression = walker.expression;
	}
	SEMA_POLICY(cpp::delete_expression, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::delete_expression* symbol, const SemaExpressionResult& walker)
	{
		type = gVoid; // TODO: check compliance: type of delete-expression
		setExpressionType(symbol, type);
		expression = ExpressionWrapper();
	}
	SEMA_POLICY(cpp::throw_expression, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::throw_expression* symbol, const SemaExpressionResult& walker)
	{
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


struct SemaTypeName : public SemaBase
{
	SEMA_BOILERPLATE;

	Type type;
	IsHiddenTypeName filter; // allows type-name to be parsed without knowing whether it is the prefix of a nested-name-specifier (in which case it cannot be hidden by a non-type name)
	bool isTypename; // true if a type is expected in this context; e.g. following 'typename', preceding '::'
	SemaTypeName(const SemaState& state, bool isTypename = false)
		: SemaBase(state), type(0, context), isTypename(isTypename)
	{
	}

	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCachedChecked)
	bool action(cpp::identifier* symbol)
	{
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
			const SimpleType* enclosingTemplate = findEnclosingPrimaryTemplate(enclosingType, type.declaration);
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
		return true;
	}

	SEMA_POLICY(cpp::simple_template_id, SemaPolicyPushCachedChecked<struct SemaTemplateId>)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateId& walker)
	{
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
		type.templateArguments = walker.arguments;
		setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of its arguments are dependent
		return true;
	}
};

struct SemaNestedNameSpecifierSuffix : public SemaBase
{
	SEMA_BOILERPLATE;

	Type type;
	bool isDeclarator;
	bool isTemplate;
	SemaNestedNameSpecifierSuffix(const SemaState& state, bool isDeclarator = false)
		: SemaBase(state), type(0, context), isDeclarator(isDeclarator), isTemplate(false)
	{
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCachedChecked)
	bool action(cpp::identifier* symbol)
	{
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
		return true;
	}
	SEMA_POLICY_ARGS(cpp::simple_template_id, SemaPolicyPushCachedCheckedBool<struct SemaTemplateId>, isTemplate)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateId& walker)
	{
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
		type.templateArguments = walker.arguments;
		if(declaration != &gDependentNestedTemplate)
		{
			setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		}
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of its arguments are dependent
		return true;
	}
};

// basic.lookup.qual
// During the lookup for a name preceding the :: scope resolution operator, object, function, and enumerator names are ignored.
struct SemaNestedNameSpecifierPrefix : public SemaBase
{
	SEMA_BOILERPLATE;

	Type type;
	bool isDeclarator;
	SemaNestedNameSpecifierPrefix(const SemaState& state, bool isDeclarator = false)
		: SemaBase(state), type(0, context), isDeclarator(isDeclarator)
	{
	}

#if 0 // for debugging parse-tree cache
	SEMA_POLICY_ARGS(cpp::nested_name, SemaPolicyPushCachedBool<struct SemaNestedNameSpecifierPrefix>, isDeclarator)
	void action(cpp::nested_name* symbol, const SemaNestedNameSpecifierPrefix& walker)
	{
		type = walker.type;
	}
#endif
	SEMA_POLICY(cpp::namespace_name, SemaPolicyPushChecked<struct SemaNamespaceName>) // considers only namespace names
	bool action(cpp::namespace_name* symbol, const SemaNamespaceName& walker)
	{
		if(walker.filter.hidingType != 0) // if the namespace name we found is hidden by a type name
		{
			return reportIdentifierMismatch(symbol, walker.filter.hidingType->getName(), walker.filter.hidingType, "namespace-name");
		}
		type.declaration = walker.declaration;
		return true;
	}
	SEMA_POLICY_ARGS(cpp::type_name, SemaPolicyPushCheckedBool<struct SemaTypeName>, true) // considers only type names
	bool action(cpp::type_name* symbol, const SemaTypeName& walker)
	{
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
		type = walker.type;
		return true;
	}
};

struct SemaNestedNameSpecifier : public SemaQualified
{
	SEMA_BOILERPLATE;

	bool isDeclarator;
	SemaNestedNameSpecifier(const SemaState& state, bool isDeclarator = false)
		: SemaQualified(state), isDeclarator(isDeclarator)
	{
	}
	SEMA_POLICY_ARGS(cpp::nested_name_specifier_prefix, SemaPolicyPushBool<struct SemaNestedNameSpecifierPrefix>, isDeclarator)
	void action(cpp::nested_name_specifier_prefix* symbol, const SemaNestedNameSpecifierPrefix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		Type type = walker.type;
		makeUniqueTypeSafe(type);
		swapQualifying(type, isDeclarator);
		//disableBacktrack();
	}
	SEMA_POLICY_ARGS(cpp::nested_name_specifier_suffix_template, SemaPolicyPushBool<struct SemaNestedNameSpecifierSuffix>, isDeclarator)
	void action(cpp::nested_name_specifier_suffix_template* symbol, const SemaNestedNameSpecifierSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		Type type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		makeUniqueTypeSafe(type);
		swapQualifying(type, isDeclarator);
		//disableBacktrack();
	}
	SEMA_POLICY_ARGS(cpp::nested_name_specifier_suffix_default, SemaPolicyPushBool<struct SemaNestedNameSpecifierSuffix>, isDeclarator)
	void action(cpp::nested_name_specifier_suffix_default* symbol, const SemaNestedNameSpecifierSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		Type type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		makeUniqueTypeSafe(type);
		swapQualifying(type, isDeclarator);
		//disableBacktrack();
	}
};

struct SemaTypeSpecifier : public SemaQualified, SemaTypeSpecifierResult
{
	SEMA_BOILERPLATE;

	SemaTypeSpecifier(const SemaState& state)
		: SemaQualified(state), SemaTypeSpecifierResult(context)
	{
	}
	SEMA_POLICY(cpp::simple_type_specifier_name, SemaPolicyPush<struct SemaTypeSpecifier>)
	void action(cpp::simple_type_specifier_name* symbol, const SemaTypeSpecifierResult& walker)
	{
		type = walker.type;
		fundamental = walker.fundamental;
	}
	SEMA_POLICY(cpp::simple_type_specifier_template, SemaPolicyPush<struct SemaTypeSpecifier>)
	void action(cpp::simple_type_specifier_template* symbol, const SemaTypeSpecifierResult& walker) // X::template Y<Z>
	{
		type = walker.type;
		fundamental = walker.fundamental;
	}
	SEMA_POLICY(cpp::type_name, SemaPolicyPushChecked<struct SemaTypeName>)
	bool action(cpp::type_name* symbol, const SemaTypeName& walker) // simple_type_specifier_name
	{
		if(walker.filter.nonType != 0)
		{
			// 3.3.7: a type-name can be hidden by a non-type name in the same scope (this rule applies to a type-specifier)
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		return true;
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker) // simple_type_specifier_name | simple_type_specifier_template
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::simple_template_id, SemaPolicyPushCachedChecked<struct SemaTemplateId>)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateId& walker) // simple_type_specifier_template
	{
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
		type.templateArguments = walker.arguments;
		type.qualifying.swap(qualifying);
		setDependent(type); // a template-id is dependent if the template-name is a template-parameter
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of the template arguments are dependent
		setDependent(type.dependent, type.qualifying);
		return true;
	}
	SEMA_POLICY(cpp::simple_type_specifier_builtin, SemaPolicyIdentity)
	void action(cpp::simple_type_specifier_builtin* symbol)
	{
		fundamental = combineFundamental(0, symbol->id);
	}
};

struct SemaUnqualifiedDeclaratorId : public SemaBase
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	SemaUnqualifiedDeclaratorId(const SemaState& state)
		: SemaBase(state), id(&gAnonymousId), conversionType(0, context)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::template_id, SemaPolicyPushCached<struct SemaTemplateId>)
	void action(cpp::template_id* symbol, const SemaTemplateId& walker) 
	{
		id = walker.id;
	}
	SEMA_POLICY(cpp::operator_function_id, SemaPolicyPushId<struct SemaOperatorFunctionId>)
	void action(cpp::operator_function_id* symbol, const SemaOperatorFunctionId& walker) 
	{
		symbol->value.value = walker.name;
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::conversion_function_id, SemaPolicyPushId<struct SemaTypeId>)
	void action(cpp::conversion_function_id* symbol, const SemaTypeIdResult& walker) 
	{
		// note: no commit here, because type-id will be uniqued later on..
		symbol->value = gConversionFunctionId;
		id = &symbol->value;
		conversionType = walker.type;
	}
	SEMA_POLICY(cpp::destructor_id, SemaPolicyIdentity)
	void action(cpp::destructor_id* symbol) 
	{
		id = &symbol->name->value;
	}
};

struct SemaQualifiedDeclaratorId : public SemaQualified
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	SemaQualifiedDeclaratorId(const SemaState& state)
		: SemaQualified(state), id(&gAnonymousId)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	// in a template member definition, the qualifying nested-name-specifier may be dependent on a template-parameter
	// no need to cache: the nested-name-specifier is not a shared-prefix
	SEMA_POLICY_ARGS(cpp::nested_name_specifier, SemaPolicyPushBool<struct SemaNestedNameSpecifier>, true)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying, true);
	}
	SEMA_POLICY(cpp::unqualified_id, SemaPolicyPush<struct SemaUnqualifiedDeclaratorId>)
	void action(cpp::unqualified_id* symbol, const SemaUnqualifiedDeclaratorId& walker)
	{
		id = walker.id;
	}
};

struct SemaDeclaratorId : public SemaQualified
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	SemaDeclaratorId(const SemaState& state)
		: SemaQualified(state), id(&gAnonymousId), conversionType(0, context)
	{
	}
	SEMA_POLICY(cpp::qualified_id_default, SemaPolicyPush<struct SemaQualifiedDeclaratorId>)
	void action(cpp::qualified_id_default* symbol, const SemaQualifiedDeclaratorId& walker)
	{
		id = walker.id;
		swapQualifying(walker.qualifying, true);
	}
	SEMA_POLICY(cpp::qualified_id_global, SemaPolicyPush<struct SemaQualifiedDeclaratorId>)
	void action(cpp::qualified_id_global* symbol, const SemaQualifiedDeclaratorId& walker)
	{
		id = walker.id;
		swapQualifying(walker.qualifying, true);
	}
	SEMA_POLICY(cpp::unqualified_id, SemaPolicyPush<struct SemaUnqualifiedDeclaratorId>)
	void action(cpp::unqualified_id* symbol, const SemaUnqualifiedDeclaratorId& walker)
	{
		id = walker.id;
		conversionType = walker.conversionType;
	}
};

struct SemaParameterDeclarationList : public SemaBase
{
	SEMA_BOILERPLATE;

	Parameters parameters;

	SemaParameterDeclarationList(const SemaState& state)
		: SemaBase(state)
	{
	}

	SEMA_POLICY_ARGS(cpp::parameter_declaration, SemaPolicyPushBool<struct SemaSimpleDeclaration>, true)
	void action(cpp::parameter_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
		if(!isVoidParameter(walker.declaration->type))
		{
			parameters.push_back(Parameter(walker.declaration, walker.defaultArgument));
		}
	}
};

struct SemaParameterDeclarationClause : public SemaBase
{
	SEMA_BOILERPLATE;

	Parameters parameters;

	SemaParameterDeclarationClause(const SemaState& state)
		: SemaBase(state)
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

	SEMA_POLICY(cpp::parameter_declaration_list, SemaPolicyPush<struct SemaParameterDeclarationList>)
	void action(cpp::parameter_declaration_list* symbol, const SemaParameterDeclarationList& walker)
	{
		parameters = walker.parameters;
	}
	void action(cpp::terminal<boost::wave::T_ELLIPSIS> symbol)
	{
		parameters.isEllipsis = true;
	}
};

struct SemaExceptionSpecification : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaExceptionSpecification(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
	}
};

struct SemaCvQualifierSeq : SemaBase
{
	SEMA_BOILERPLATE;

	CvQualifiers qualifiers;
	SemaCvQualifierSeq(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::cv_qualifier, SemaPolicyIdentity)
	void action(cpp::cv_qualifier* symbol)
	{
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

struct SemaPtrOperator : public SemaQualified
{
	SEMA_BOILERPLATE;

	CvQualifiers qualifiers;
	SemaPtrOperator(const SemaState& state)
		: SemaQualified(state)
	{
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::cv_qualifier_seq, SemaPolicyPush<struct SemaCvQualifierSeq>)
	void action(cpp::cv_qualifier_seq* symbol, const SemaCvQualifierSeq& walker)
	{
		qualifiers = walker.qualifiers;
	}
};

struct SemaDeclaratorFunction : public SemaBase
{
	SEMA_BOILERPLATE;

	ScopePtr paramScope;
	Parameters parameters;
	CvQualifiers qualifiers;
	SemaDeclaratorFunction(const SemaState& state)
		: SemaBase(state), paramScope(0)
	{
	}

	SEMA_POLICY(cpp::parameter_declaration_clause, SemaPolicyPush<struct SemaParameterDeclarationClause>)
	void action(cpp::parameter_declaration_clause* symbol, const SemaParameterDeclarationClause& walker)
	{
		paramScope = walker.enclosing; // store reference for later resumption
		parameters = walker.parameters;
	}
	SEMA_POLICY(cpp::exception_specification, SemaPolicyPush<struct SemaExceptionSpecification>)
	void action(cpp::exception_specification* symbol, const SemaExceptionSpecification& walker)
	{
	}
	SEMA_POLICY(cpp::cv_qualifier_seq, SemaPolicyPush<struct SemaCvQualifierSeq>)
	void action(cpp::cv_qualifier_seq* symbol, const SemaCvQualifierSeq& walker)
	{
		qualifiers = walker.qualifiers;
	}
};

struct SemaDeclaratorArray : public SemaBase
{
	SEMA_BOILERPLATE;

	Dependent valueDependent;
	ArrayRank rank;
	ExpressionWrapper expression;
	SemaDeclaratorArray(const SemaState& state)
		: SemaBase(state)
	{
	}

	void action(cpp::terminal<boost::wave::T_LEFTBRACKET> symbol)
	{
		// we may parse multiple pairs of brackets: omitted constant-expression indicates an array of unknown size
		expression = ExpressionWrapper();
	}
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker)
	{
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		// [temp.dep.constexpr] An identifier is value-dependent if it is:- a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
		addDependent(valueDependent, walker.valueDependent);
		expression = walker.expression;
	}
	void action(cpp::terminal<boost::wave::T_RIGHTBRACKET> symbol)
	{
		rank.push_back(expression);
	}
};

struct SemaDeclarator : public SemaBase
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	UniqueTypeWrapper qualifying;
	ScopePtr paramScope;
	Dependent valueDependent;
	TypeSequence typeSequence;
	CvQualifiers qualifiers;
	Qualifying memberPointer;
	Dependent dependent; // track which template parameters the declarator's type depends on. e.g. 'T::* memberPointer', 'void f(T)'
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	SemaDeclarator(const SemaState& state)
		: SemaBase(state), id(&gAnonymousId), paramScope(0), typeSequence(context), memberPointer(context), conversionType(0, context)
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

	SEMA_POLICY(cpp::ptr_operator, SemaPolicyPush<struct SemaPtrOperator>)
	void action(cpp::ptr_operator* symbol, const SemaPtrOperator& walker)
	{
		qualifiers = walker.qualifiers;
		memberPointer = walker.qualifying;
	}
	template<typename T>
	void walkDeclaratorPtr(T* symbol, SemaDeclarator& walker)
	{
		// if parse fails, state of typeSeqence is not modified.
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
		memberPointer = walker.memberPointer;
		conversionType = walker.conversionType;
	}
	SEMA_POLICY(cpp::declarator_ptr, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::declarator_ptr* symbol, SemaDeclarator& walker)
	{
		return walkDeclaratorPtr(symbol, walker);
	}
	SEMA_POLICY(cpp::abstract_declarator_ptr, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::abstract_declarator_ptr* symbol, SemaDeclarator& walker)
	{
		return walkDeclaratorPtr(symbol, walker);
	}
	SEMA_POLICY(cpp::new_declarator_ptr, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::new_declarator_ptr* symbol, SemaDeclarator& walker)
	{
		return walkDeclaratorPtr(symbol, walker);
	}
	SEMA_POLICY(cpp::conversion_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::conversion_declarator* symbol, SemaDeclarator& walker)
	{
		return walkDeclaratorPtr(symbol, walker);
	}
	SEMA_POLICY(cpp::declarator_id, SemaPolicyPush<struct SemaDeclaratorId>)
	void action(cpp::declarator_id* symbol, SemaDeclaratorId& walker)
	{
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
			conversionType = walker.conversionType;
		}
		addDependent(dependent, conversionType); // TODO: check compliance: conversion-function declarator-id is dependent if it contains a dependent type?
	}
	template<typename T>
	void walkDeclaratorArray(T* symbol, const SemaDeclaratorArray& walker)
	{
		// [temp.dep.type] A type is dependent if it is - an array type [...] whose size is specified by a constant expression that is value-dependent
		addDependent(dependent, walker.valueDependent);
		addDependent(valueDependent, walker.valueDependent);
		typeSequence.push_front(DeclaratorArrayType(walker.rank));
	}
	SEMA_POLICY(cpp::declarator_suffix_array, SemaPolicyPushCached<struct SemaDeclaratorArray>)
	void action(cpp::declarator_suffix_array* symbol, const SemaDeclaratorArray& walker)
	{
		return walkDeclaratorArray(symbol, walker);
	}
	SEMA_POLICY(cpp::declarator_suffix_function, SemaPolicyPushCached<struct SemaDeclaratorFunction>)
	void action(cpp::declarator_suffix_function* symbol, const SemaDeclaratorFunction& walker)
	{
		if(paramScope == 0) // only interested in the innermost parameter-list
		{
			paramScope = walker.paramScope;
		}
		typeSequence.push_front(DeclaratorFunctionType(walker.parameters, walker.qualifiers));
		setDependent(dependent, walker.parameters);
	}
	SEMA_POLICY(cpp::new_declarator_suffix, SemaPolicyPushCached<struct SemaDeclaratorArray>)
	void action(cpp::new_declarator_suffix* symbol, const SemaDeclaratorArray& walker)
	{
		return walkDeclaratorArray(symbol, walker);
	}
	SEMA_POLICY(cpp::expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression* symbol, const SemaExpressionResult& walker) // in direct_new_declarator
	{
	}
	template<typename T>
	void walkDeclarator(T* symbol, const SemaDeclarator& walker)
	{
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
		conversionType = walker.conversionType;
	}
	SEMA_POLICY(cpp::direct_abstract_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::direct_abstract_declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker); // if parse fails, state of typeSeqence is not modified. e.g. type-id: int((int))
	}
	SEMA_POLICY(cpp::direct_abstract_declarator_parenthesis, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::direct_abstract_declarator_parenthesis* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker); // if parse fails, state of typeSeqence is not modified. e.g. function-style-cast type-id followed by parenthesised expression: int(*this)
	}
	SEMA_POLICY(cpp::direct_new_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::direct_new_declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker);
	}
	SEMA_POLICY(cpp::declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker);
	}
	SEMA_POLICY(cpp::abstract_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::abstract_declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker);
	}
	SEMA_POLICY(cpp::new_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::new_declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker);
	}
};

struct SemaBaseSpecifier : public SemaQualified
{
	SEMA_BOILERPLATE;

	Type type;
	SemaBaseSpecifier(const SemaState& state)
		: SemaQualified(state), type(0, context)
	{
	}

	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY_ARGS(cpp::class_name, SemaPolicyPushBool<struct SemaTypeName>, true)
	void action(cpp::class_name* symbol, const SemaTypeName& walker)
	{
		/* [class.derived]
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		makeUniqueTypeSafe(type);
	}
};

struct SemaClassHead : public SemaBase
{
	SEMA_BOILERPLATE;

	DeclarationPtr declaration;
	IdentifierPtr id;
	ScopePtr parent;
	TemplateArguments arguments;
	bool isUnion;
	bool isSpecialization;
	SemaClassHead(const SemaState& state)
		: SemaBase(state), declaration(0), id(0), parent(enclosing), arguments(context), isUnion(false), isSpecialization(false)
	{
	}

	SEMA_POLICY(cpp::class_key, SemaPolicyIdentityCached)
	void action(cpp::class_key* symbol)
	{
		isUnion = symbol->id == cpp::class_key::UNION;
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol) // class_name
	{
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaNestedNameSpecifier& walker)
	{
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
	SEMA_POLICY_ARGS(cpp::simple_template_id, SemaPolicyPushCachedBool<struct SemaTemplateId>, true) // TODO: specifying isTemplate as a temporary workaround: name lookup of qualified class-name currently fails.
	void action(cpp::simple_template_id* symbol, const SemaTemplateId& walker) // class_name
	{
		// TODO: don't declare anything - this is a template (partial) specialization
		id = walker.id;
		arguments = walker.arguments;
		isSpecialization = true;
	}
	void action(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until ':' is discovered
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = declareClass(parent, id, isSpecialization, arguments);
	}
	SEMA_POLICY(cpp::base_specifier, SemaPolicyPush<struct SemaBaseSpecifier>)
	void action(cpp::base_specifier* symbol, const SemaBaseSpecifier& walker) 
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		SEMANTIC_ASSERT(declaration->enclosed != 0);
		SEMANTIC_ASSERT(walker.type.unique != 0);
		addBase(declaration, walker.type);
		setExpressionType(symbol, walker.type.isDependent ? gUniqueTypeNull : UniqueTypeWrapper(walker.type.unique));
	}
};

struct SemaUsingDeclaration : public SemaQualified
{
	SEMA_BOILERPLATE;

	bool isTypename;
	SemaUsingDeclaration(const SemaState& state)
		: SemaQualified(state), isTypename(false)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::unqualified_id, SemaPolicyPushChecked<struct SemaUnqualifiedId>)
	bool action(cpp::unqualified_id* symbol, const SemaUnqualifiedId& walker)
	{
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
		return true;
	}
	void action(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
		isTypename = true;
	}
};

struct SemaUsingDirective : public SemaQualified
{
	SEMA_BOILERPLATE;

	SemaUsingDirective(const SemaState& state)
		: SemaQualified(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	// [basic.lookup.udir]
	// When looking up a namespace-name in a using-directive or namespace-alias-definition, only namespace
	// names are considered.
	SEMA_POLICY(cpp::namespace_name, SemaPolicyPush<struct SemaNamespaceName>)
	void action(cpp::namespace_name* symbol, const SemaNamespaceName& walker)
	{
		if(!findScope(enclosing, walker.declaration->enclosed))
		{
			enclosing->usingDirectives.push_back(walker.declaration->enclosed);
		}
	}
};

struct SemaNamespaceAliasDefinition : public SemaQualified
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	SemaNamespaceAliasDefinition(const SemaState& state)
		: SemaQualified(state), id(0)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityChecked)
	bool action(cpp::identifier* symbol)
	{
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
		return true;
	}
};

struct SemaMemberDeclaration : public SemaBase
{
	SEMA_BOILERPLATE;

	DeclarationPtr declaration;
	SemaMemberDeclaration(const SemaState& state)
		: SemaBase(state), declaration(0)
	{
	}
	SEMA_POLICY(cpp::member_template_declaration, SemaPolicyPush<struct SemaTemplateDeclaration>)
	void action(cpp::member_template_declaration* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::member_declaration_implicit, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::member_declaration_implicit* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::member_declaration_default, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::member_declaration_default* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::member_declaration_nested, SemaPolicyPush<struct SemaQualifiedId>)
	void action(cpp::member_declaration_nested* symbol, const SemaQualifiedId& walker)
	{
	}
	SEMA_POLICY(cpp::using_declaration, SemaPolicyPush<struct SemaUsingDeclaration>)
	void action(cpp::using_declaration* symbol, const SemaUsingDeclaration& walker)
	{
	}
};


struct SemaClassSpecifier : public SemaBase
{
	SEMA_BOILERPLATE;

	DeclarationPtr declaration;
	IdentifierPtr id;
	TemplateArguments arguments;
	DeferredSymbols deferred;
	bool isUnion;
	bool isSpecialization;
	SemaClassSpecifier(const SemaState& state)
		: SemaBase(state), declaration(0), id(0), arguments(context), isUnion(false), isSpecialization(false)
	{
	}

	SEMA_POLICY(cpp::class_head, SemaPolicyPush<struct SemaClassHead>)
	void action(cpp::class_head* symbol, const SemaClassHead& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		isUnion = walker.isUnion;
		isSpecialization = walker.isSpecialization;
		arguments = walker.arguments;
		enclosing = walker.parent;
		templateParams = walker.templateParams; // template-params may have been consumed by qualifying template-name
	}
	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
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

		if(SemaState::enclosingDeferred == 0)
		{
			SemaState::enclosingDeferred = &deferred;
		}
	}
	SEMA_POLICY(cpp::member_declaration, SemaPolicyPush<struct SemaMemberDeclaration>)
	void action(cpp::member_declaration* symbol, const SemaMemberDeclaration& walker)
	{
	}
#if 0 // TODO!
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
#endif
	void action(cpp::terminal<boost::wave::T_RIGHTBRACE> symbol)
	{
		declaration->isComplete = true;
		// If the class definition does not explicitly declare a copy assignment operator, one is declared implicitly.
		// The implicitly-declared copy assignment operator for a class X will have the form
		//   X& X::operator=(const X&)
		// TODO: correct constness of parameter
		// TODO: this must occur at point of instantiation

		parseDeferred(deferred.first, context.parserContext);
		parseDeferred(deferred.second, context.parserContext);
	}
};

struct SemaEnumeratorDefinition : public SemaBase
{
	SEMA_BOILERPLATE;

	DeclarationPtr declaration; // result
	SemaEnumeratorDefinition(const SemaState& state)
		: SemaBase(state), declaration(0)
	{
	}

	SEMA_POLICY(cpp::identifier, SemaPolicyIdentity)
	void action(cpp::identifier* symbol)
	{
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
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker)
	{
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
};

struct SemaEnumSpecifier : public SemaBase
{
	SEMA_BOILERPLATE;

	DeclarationPtr declaration; // result
	IdentifierPtr id; // internal state
	ExpressionWrapper value;
	SemaEnumSpecifier(const SemaState& state)
		: SemaBase(state), declaration(0), id(0)
	{
	}

	SEMA_POLICY(cpp::identifier, SemaPolicyIdentity)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}

	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
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
		if(declaration == 0)
		{
			// unnamed enum
			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, enclosing->getUniqueName(), TYPE_ENUM, 0);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(instance);
#endif
			declaration = instance;
		}
	}

	SEMA_POLICY(cpp::enumerator_definition, SemaPolicyPush<struct SemaEnumeratorDefinition>)
	void action(cpp::enumerator_definition* symbol, const SemaEnumeratorDefinition& walker)
	{
		Declaration& enumerator = *walker.declaration;
		enumerator.type = declaration; // give the enumerator the type of its enumeration
		enumerator.type.qualifiers = CvQualifiers(true, false); // an enumerator may be used in an integral constant expression
		setDependent(enumerator.type); // the enumeration type is dependent if it is a member of a class template
		makeUniqueTypeSafe(enumerator.type);
		if(enumerator.initializer.p != 0)
		{
			SEMANTIC_ASSERT(isDependent(enumerator.valueDependent) || enumerator.initializer.isConstant);
			value = enumerator.initializer;
		}
		else
		{
			enumerator.initializer = value;
		}
		// [dcl.enum] An enumerator-definition without an initializer gives the enumerator the value obtained by increasing the value of the previous enumerator by one.
		ExpressionWrapper one = makeUniqueExpression(IntegralConstantExpression(gSignedInt, IntegralConstant(1)));
		value = makeExpression(BinaryExpression(Name("+"), operator+, 0, value, one), // TODO: type of enumerator
			true, value.isTypeDependent, value.isValueDependent
		);
	}
};

struct SemaElaboratedTypeSpecifier : public SemaQualified
{
	SEMA_BOILERPLATE;

	DeclarationPtr key;
	Type type;
	IdentifierPtr id;
	SemaElaboratedTypeSpecifier(const SemaState& state)
		: SemaQualified(state), key(0), type(0, context), id(0)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::elaborated_type_specifier_default, SemaPolicyPush<struct SemaElaboratedTypeSpecifier>)
	void action(cpp::elaborated_type_specifier_default* symbol, const SemaElaboratedTypeSpecifier& walker)
	{
		type = walker.type;
		id = walker.id;

		if(!isUnqualified(symbol)
			|| !isClassKey(*type.declaration))
		{
			id = 0;
		}
	}
	SEMA_POLICY(cpp::elaborated_type_specifier_template, SemaPolicyPush<struct SemaElaboratedTypeSpecifier>)
	void action(cpp::elaborated_type_specifier_template* symbol, const SemaElaboratedTypeSpecifier& walker)
	{
		type = walker.type;
		id = walker.id;
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker) // elaborated_type_specifier_default | elaborated_type_specifier_template
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::class_key, SemaPolicyIdentityCached)
	void action(cpp::class_key* symbol)
	{
		key = &gClass;
	}
	SEMA_POLICY(cpp::enum_key, SemaPolicyIdentity)
	void action(cpp::enum_key* symbol)
	{
		key = &gEnum;
	}
	SEMA_POLICY(cpp::simple_template_id, SemaPolicyPushCachedChecked<struct SemaTemplateId>)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateId& walker) // elaborated_type_specifier_default | elaborated_type_specifier_template
	{
		SEMANTIC_ASSERT(key == &gClass);
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
		type.templateArguments = walker.arguments;
		type.qualifying.swap(qualifying);
		return true;
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol)
	{
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

struct SemaTypenameSpecifier : public SemaQualified, SemaTypenameSpecifierResult
{
	SEMA_BOILERPLATE;

	SemaTypenameSpecifier(const SemaState& state)
		: SemaQualified(state), SemaTypenameSpecifierResult(context)
	{
	}

	void action(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		// TODO
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY_ARGS(cpp::type_name, SemaPolicyPushCheckedBool<struct SemaTypeName>, true)
	bool action(cpp::type_name* symbol, const SemaTypeName& walker)
	{
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		return true;
	}
};

struct SemaDeclSpecifierSeq : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaDeclSpecifierSeqResult seq;
	unsigned fundamental;
	Once committed;
	SemaDeclSpecifierSeq(const SemaState& state)
		: SemaBase(state), seq(0, context), fundamental(0)
	{
	}

	void commit()
	{
		committed();
		declareEts(seq.type, seq.forward);
	}

	SEMA_POLICY(cpp::simple_type_specifier, SemaPolicyPush<struct SemaTypeSpecifier>)
	void action(cpp::simple_type_specifier* symbol, const SemaTypeSpecifierResult& walker)
	{
		seq.type = walker.type;
		if(seq.type.declaration == 0)
		{
			fundamental = walker.fundamental;
			seq.type = getFundamentalType(fundamental);
		}
	}
	SEMA_POLICY(cpp::simple_type_specifier_builtin, SemaPolicyIdentity)
	void action(cpp::simple_type_specifier_builtin* symbol)
	{
		fundamental = combineFundamental(fundamental, symbol->id);
		seq.type = getFundamentalType(fundamental);
	}
	SEMA_POLICY(cpp::elaborated_type_specifier, SemaPolicyPush<struct SemaElaboratedTypeSpecifier>)
	void action(cpp::elaborated_type_specifier* symbol, const SemaElaboratedTypeSpecifier& walker)
	{
		seq.forward = walker.id;
		seq.type = walker.type;
	}
	SEMA_POLICY(cpp::typename_specifier, SemaPolicyPush<struct SemaTypenameSpecifier>)
	void action(cpp::typename_specifier* symbol, const SemaTypenameSpecifierResult& walker)
	{
		seq.type = walker.type;
	}
	SEMA_POLICY(cpp::class_specifier, SemaPolicyPush<struct SemaClassSpecifier>)
	void action(cpp::class_specifier* symbol, const SemaClassSpecifier& walker)
	{
		seq.type = walker.declaration;
		setDependent(seq.type); // a class-specifier is dependent if it declares a nested class of a template class
		templateParams = walker.templateParams;
		seq.isUnion = walker.isUnion;
	}
	SEMA_POLICY(cpp::enum_specifier, SemaPolicyPush<struct SemaEnumSpecifier>)
	void action(cpp::enum_specifier* symbol, const SemaEnumSpecifier& walker)
	{
		seq.type = walker.declaration;
	}
	SEMA_POLICY(cpp::decl_specifier_default, SemaPolicyIdentity)
	void action(cpp::decl_specifier_default* symbol)
	{
		if(symbol->id == cpp::decl_specifier_default::TYPEDEF)
		{
			seq.specifiers.isTypedef = true;
		}
		else if(symbol->id == cpp::decl_specifier_default::FRIEND)
		{
			seq.specifiers.isFriend = true;
		}
	}
	SEMA_POLICY(cpp::storage_class_specifier, SemaPolicyIdentity)
	void action(cpp::storage_class_specifier* symbol)
	{
		if(symbol->id == cpp::storage_class_specifier::STATIC)
		{
			seq.specifiers.isStatic = true;
		}
		else if(symbol->id == cpp::storage_class_specifier::EXTERN)
		{
			seq.specifiers.isExtern = true;
		}
	}
	SEMA_POLICY(cpp::cv_qualifier, SemaPolicyIdentity)
	void action(cpp::cv_qualifier* symbol)
	{
		if(symbol->id == cpp::cv_qualifier::CONST)
		{
			seq.qualifiers.isConst = true;
		}
		else if(symbol->id == cpp::cv_qualifier::VOLATILE)
		{
			seq.qualifiers.isVolatile = true;
		}
	}
};

struct SemaTryBlock : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaTryBlock(const SemaState& state)
		: SemaBase(state)
	{
	}

	SEMA_POLICY(cpp::compound_statement, SemaPolicyPush<struct SemaCompoundStatement>)
	void action(cpp::compound_statement* symbol, const SemaCompoundStatement& walker)
	{
	}
	SEMA_POLICY(cpp::handler_seq, SemaPolicyPush<struct SemaHandlerSeq>)
	void action(cpp::handler_seq* symbol, const SemaHandlerSeq& walker)
	{
	}
};

struct SemaLabeledStatement : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaLabeledStatement(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentity)
	void action(cpp::identifier* symbol)
	{
		// TODO: goto label
	}
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker)
	{
		SEMANTIC_ASSERT(walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
	}
	SEMA_POLICY(cpp::statement, SemaPolicyPush<struct SemaStatement>)
	void action(cpp::statement* symbol, const SemaStatement& walker)
	{
	}
};

struct SemaStatement : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaStatement(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::simple_declaration, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::simple_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
	}
	SEMA_POLICY(cpp::try_block, SemaPolicyPush<struct SemaTryBlock>)
	void action(cpp::try_block* symbol, const SemaTryBlock& walker)
	{
	}
	SEMA_POLICY(cpp::namespace_alias_definition, SemaPolicyPush<struct SemaNamespaceAliasDefinition>)
	void action(cpp::namespace_alias_definition* symbol, const SemaNamespaceAliasDefinition& walker)
	{
	}
	SEMA_POLICY(cpp::selection_statement, SemaPolicyPush<struct SemaControlStatement>)
	void action(cpp::selection_statement* symbol, const SemaControlStatement& walker)
	{
	}
	SEMA_POLICY(cpp::iteration_statement, SemaPolicyPush<struct SemaControlStatement>)
	void action(cpp::iteration_statement* symbol, const SemaControlStatement& walker)
	{
	}
	SEMA_POLICY(cpp::compound_statement, SemaPolicyPush<struct SemaCompoundStatement>)
	void action(cpp::compound_statement* symbol, const SemaCompoundStatement& walker)
	{
	}
	SEMA_POLICY(cpp::expression_statement, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression_statement* symbol, const SemaExpressionResult& walker)
	{
	}
	SEMA_POLICY(cpp::jump_statement_return, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::jump_statement_return* symbol, const SemaExpressionResult& walker)
	{
	}
	SEMA_POLICY(cpp::jump_statement_goto, SemaPolicyIdentity)
	void action(cpp::jump_statement_goto* symbol)
	{
		// TODO
	}
	SEMA_POLICY(cpp::labeled_statement, SemaPolicyPush<struct SemaLabeledStatement>)
	void action(cpp::labeled_statement* symbol, const SemaLabeledStatement& walker)
	{
	}
	SEMA_POLICY(cpp::using_declaration, SemaPolicyPush<struct SemaUsingDeclaration>)
	void action(cpp::using_declaration* symbol, const SemaUsingDeclaration& walker)
	{
	}
	SEMA_POLICY(cpp::using_directive, SemaPolicyPush<struct SemaUsingDirective>)
	void action(cpp::using_directive* symbol, const SemaUsingDirective& walker)
	{
	}
};

struct SemaControlStatement : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaControlStatement(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	SEMA_POLICY(cpp::condition_init, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::condition_init* symbol, const SemaSimpleDeclarationResult& walker)
	{
	}
	SEMA_POLICY(cpp::simple_declaration, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::simple_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
	}
	SEMA_POLICY(cpp::statement, SemaPolicyPush<struct SemaStatement>)
	void action(cpp::statement* symbol, const SemaStatement& walker)
	{
	}
	SEMA_POLICY(cpp::expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression* symbol, const SemaExpressionResult& walker)
	{
	}
};

struct SemaCompoundStatement : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaCompoundStatement(const SemaState& state)
		: SemaBase(state)
	{
	}

	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	SEMA_POLICY(cpp::statement, SemaPolicyPush<struct SemaStatement>)
	void action(cpp::statement* symbol, const SemaStatement& walker)
	{
	}
};

struct SemaHandler : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaHandler(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_CATCH> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	SEMA_POLICY(cpp::exception_declaration_default, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::exception_declaration_default* symbol, const SemaSimpleDeclarationResult& walker)
	{
	}
	SEMA_POLICY(cpp::compound_statement, SemaPolicyPush<struct SemaCompoundStatement>)
	void action(cpp::compound_statement* symbol, const SemaCompoundStatement& walker)
	{
	}
};

struct SemaHandlerSeq : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaHandlerSeq(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::handler, SemaPolicyPush<struct SemaHandler>)
	void action(cpp::handler* symbol, const SemaHandler& walker)
	{
	}
};

struct SemaQualifiedTypeName : public SemaQualified
{
	SEMA_BOILERPLATE;

	SemaQualifiedTypeName(const SemaState& state)
		: SemaQualified(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::class_name, SemaPolicyPushChecked<struct SemaTypeName>)
	bool action(cpp::class_name* symbol, const SemaTypeName& walker)
	{
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		return true;
	}
};

struct SemaMemInitializer : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaMemInitializer(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::mem_initializer_id_base, SemaPolicyPush<struct SemaQualifiedTypeName>)
	void action(cpp::mem_initializer_id_base* symbol, const SemaQualifiedTypeName& walker)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityChecked)
	bool action(cpp::identifier* symbol)
	{
		SEMANTIC_ASSERT(getQualifyingScope() == 0);
		LookupResultRef declaration = findDeclaration(symbol->value);
		if(declaration == &gUndeclared
			|| !isObject(*declaration))
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "object-name");
		}
		setDecoration(&symbol->value, declaration);
		return true;
	}
	SEMA_POLICY(cpp::expression_list, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression_list* symbol, const SemaExpressionResult& walker)
	{
	}
};

struct SemaMemberDeclaratorBitfield : public SemaBase
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	SemaMemberDeclaratorBitfield(const SemaState& state)
		: SemaBase(state), id(0)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker)
	{
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
	}
};

struct SemaTypeId : public SemaBase, SemaTypeIdResult
{
	SEMA_BOILERPLATE;

	SemaTypeId(const SemaState& state)
		: SemaBase(state), SemaTypeIdResult(context)
	{
	}
	// called when parse of type-id is complete (necessary because trailing abstract-declarator is optional)
	void commit()
	{
		committed();
		makeUniqueTypeSafe(type);
	}
	void action(cpp::terminal<boost::wave::T_OPERATOR> symbol) // conversion_function_id
	{
	}
	SEMA_POLICY(cpp::type_specifier_seq, SemaPolicyPushCommit<struct SemaDeclSpecifierSeq>)
	void action(cpp::type_specifier_seq* symbol, const SemaDeclSpecifierSeq& walker)
	{
		walker.committed.test();
		type = walker.seq.type;
		type.qualifiers = walker.seq.qualifiers;
	}
	SEMA_POLICY(cpp::abstract_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::abstract_declarator* symbol, const SemaDeclarator& walker)
	{
		type.typeSequence = walker.typeSequence;
		// [temp.dep.type] A type is dependent if it is a compound type constructed from any dependent type
		setDependent(type.dependent, walker.dependent);
	}
	SEMA_POLICY(cpp::conversion_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::conversion_declarator* symbol, SemaDeclarator& walker)
	{
		walker.pushPointerType(symbol->op);
		type.typeSequence = walker.typeSequence;
		// [temp.dep.type] A type is dependent if it is a compound type constructed from any dependent type
		setDependent(type.dependent, walker.dependent);
	}
};

struct SemaNewType : public SemaBase, SemaNewTypeResult
{
	SEMA_BOILERPLATE;

	SemaNewType(const SemaState& state)
		: SemaBase(state), SemaNewTypeResult(context)
	{
	}
	void action(cpp::terminal<boost::wave::T_OPERATOR> symbol) 
	{
		// for debugging purposes
	}
	SEMA_POLICY(cpp::type_specifier_seq, SemaPolicyPushCommit<struct SemaDeclSpecifierSeq>)
	void action(cpp::type_specifier_seq* symbol, const SemaDeclSpecifierSeq& walker)
	{
		walker.committed.test();
		type = walker.seq.type;
		type.qualifiers = walker.seq.qualifiers;
	}
	SEMA_POLICY(cpp::new_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::new_declarator* symbol, const SemaDeclarator& walker)
	{
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
	SemaState& context;
	IsTemplateName(SemaState& context) : context(context)
	{
	}
	bool operator()(Identifier& id) const
	{
		LookupResultRef declaration = context.findDeclaration(id);
		return declaration != &gUndeclared && isTemplateName(*declaration);
	}
};

struct SemaInitializer : public SemaBase
{
	SEMA_BOILERPLATE;

	ExpressionWrapper expression;
	Dependent valueDependent;
	SemaInitializer(const SemaState& state) : SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct SemaMemInitializerClause : public SemaBase
{
	SEMA_BOILERPLATE;
	SemaMemInitializerClause(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::mem_initializer, SemaPolicyPush<struct SemaMemInitializer>)
	void action(cpp::mem_initializer* symbol, const SemaMemInitializer& walker)
	{
	}
};

struct SemaStatementSeq : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaStatementSeq(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::statement, SemaPolicyPush<struct SemaStatement>)
	void action(cpp::statement* symbol, const SemaStatement& walker)
	{
	}
};

struct SemaDeclarationSuffix : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaDeclarationArgs args;

	// prefix
	const SemaDeclSpecifierSeqResult& seq;

	// declarator
	TypeId type;
	DeclarationPtr declaration; // the result of the declaration
	IdentifierPtr id;
	ScopePtr parent;
	ScopePtr enclosed;
	Dependent typeDependent;
	Dependent valueDependent;
	cpp::default_argument* defaultArgument; // parsing of this symbol will be deferred if this is a member-declaration
	bool isConversionFunction;
	CvQualifiers conversionFunctionQualifiers;


	SemaDeclarationSuffix(const SemaState& state, SemaDeclarationArgs args, const SemaDeclSpecifierSeqResult& seq) : SemaBase(state),
		args(args),
		seq(seq),
		type(0, context),
		declaration(0),
		id(&gAnonymousId),
		parent(state.enclosing),
		defaultArgument(0),
		enclosed(0),
		isConversionFunction(false)
	{
		*static_cast<Type*>(&type) = seq.type;
		type.qualifiers = seq.qualifiers;
		declareEts(type, seq.forward);
	}

	// commit the declaration to the enclosing scope.
	// invoked when no further ambiguities remain.
	void commit()
	{
		if(id != 0)
		{
			if(id == &gAnonymousId // if the declaration is anonymous
				&& !(args.isParameter || args.templateParameter != INDEX_INVALID)) // and this is not the declaration of a function parameter or non-type template parameter
			{
				id = 0; // do not declare anything
				return;
			}
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
			declaration = declareObject(parent, id, type, enclosed, seq.specifiers, args.templateParameter, valueDependent);

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

	SEMA_POLICY(cpp::declarator, SemaPolicyPushCached<struct SemaDeclarator>)
	void action(cpp::declarator* symbol, const SemaDeclarator& walker)
	{
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
			type = walker.conversionType;
			isConversionFunction = true;
		}
	}
	SEMA_POLICY(cpp::abstract_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::abstract_declarator* symbol, const SemaDeclarator& walker)
	{
		enclosed = walker.paramScope;
		type.typeSequence = walker.typeSequence;
		addDependent(typeDependent, walker.dependent);
	}
	SEMA_POLICY(cpp::member_declarator_bitfield, SemaPolicyPush<struct SemaMemberDeclaratorBitfield>)
	void action(cpp::member_declarator_bitfield* symbol, const SemaMemberDeclaratorBitfield& walker)
	{
		if(walker.id != 0)
		{
			DeclarationPtr tmpDependent = type.dependent;
			setDependent(type.dependent, typeDependent);
			makeUniqueTypeSafe(type);

			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *walker.id, type, 0, seq.specifiers); // 3.3.1.1
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
	void action(cpp::terminal<boost::wave::T_ASSIGN> symbol) // begins initializer_default
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_LEFTPAREN> symbol) // begins initializer_parenthesis
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_TRY> symbol)
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol) // begins function_body
	{
		commit();
		SEMANTIC_ASSERT(declaration != 0);
		declaration->isFunctionDefinition = true;

		// NOTE: we must ensure that symbol-table modifications within the scope of this function are undone on parse fail
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	void action(cpp::terminal<boost::wave::T_COMMA> symbol) // init_declarator_list, member_declarator_list, member_declaration_bitfield
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_SEMICOLON> symbol) // simple_declaration_named, member_declaration_named, type_declaration_suffix, member_declaration_bitfield, member_declaration_nested
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_COLON> symbol) // in member_declarator_bitfield, or ctor_initializer
	{
		commit();
	}

	struct DeferDefaultArgument
	{
		static bool isDeferredParse(SemaInitializer& walker)
		{
			return walker.enclosingDeferred != 0;
		}
		static SkipDefaultArgument<IsTemplateName> getSkipFunc(SemaInitializer& walker)
		{
			return makeSkipDefaultArgument(IsTemplateName(walker));
		}
		static DeferredSymbolsList& getDeferredSymbolsList(SemaInitializer& walker)
		{
			return walker.enclosingDeferred->first;
		}
	};
	typedef SemaPolicyPushDeferred<struct SemaInitializer, DeferDefaultArgument> SemaPolicyDefaultArgument;
	// in case of an inline member-function-definition, default-argument parse will be deferred - otherwise parsed as normal
	SEMA_POLICY(cpp::default_argument, SemaPolicyDefaultArgument)
	void action(cpp::default_argument* symbol, const SemaInitializer& walker) // parameter_declaration (looks like an initializer)
	{
		if(!args.isParameter // if this is not a parameter declaration (parameters cannot be constants)
			&& SemaState::enclosingDeferred == 0) // and parse was not defered
		{
			declaration->initializer = walker.expression;
			addDependent(declaration->valueDependent, walker.valueDependent);
		}
	}
	// handle assignment-expression(s) in initializer
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker) // condition_init
	{
		if(!args.isParameter) // parameters cannot be constants
		{
			declaration->initializer = walker.expression;
			addDependent(declaration->valueDependent, walker.valueDependent);
		}
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	SEMA_POLICY(cpp::initializer_clause, SemaPolicyPush<struct SemaInitializer>)
	void action(cpp::initializer_clause* symbol, const SemaInitializer& walker) // initializer_default
	{
		SEMANTIC_ASSERT(declaration != 0);
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	SEMA_POLICY(cpp::expression_list, SemaPolicyPush<struct SemaInitializer>)
	void action(cpp::expression_list* symbol, const SemaInitializer& walker) // initializer_parenthesis
	{
		SEMANTIC_ASSERT(declaration != 0);
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker) // member_declarator_bitfield
	{
		SEMANTIC_ASSERT(declaration != 0);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}

	typedef void(*SkipFunc)(Parser&);
	template<typename SemaT, SkipFunc skipFunc>
	struct DeferBody
	{
		static bool isDeferredParse(SemaT& walker)
		{
			return walker.enclosingDeferred != 0;
		}
		static SkipFunc getSkipFunc(SemaT& walker)
		{
			return skipFunc;
		}
		static DeferredSymbolsList& getDeferredSymbolsList(SemaT& walker)
		{
			return walker.enclosingDeferred->second;
		}
	};
	typedef DeferBody<SemaMemInitializerClause, skipMemInitializerClause> DeferMemInitializerClause;
	typedef SemaPolicyPushDeferred<struct SemaMemInitializerClause, DeferMemInitializerClause> SemaPolicyDeferMemInitializerClause;
	// in case of an inline constructor-definition, parse will be deferred - otherwise parsed as normal
	SEMA_POLICY(cpp::mem_initializer_clause, SemaPolicyDeferMemInitializerClause)
	void action(cpp::mem_initializer_clause* symbol, const SemaMemInitializerClause& walker) // function_definition_suffix->ctor_initializer
	{
	}
	typedef DeferBody<SemaStatementSeq, skipBraced> DeferFunctionBody;
	typedef SemaPolicyPushDeferred<struct SemaStatementSeq, DeferFunctionBody> SemaPolicyDeferFunctionBody;
	// in case of an inline member-function-definition, parse will be deferred - otherwise parsed as normal
	SEMA_POLICY(cpp::statement_seq_wrapper, SemaPolicyDeferFunctionBody)
	void action(cpp::statement_seq_wrapper* symbol, const SemaStatementSeq& walker) // function_definition_suffix->function_body
	{
	}
	SEMA_POLICY(cpp::handler_seq, SemaPolicyPush<struct SemaHandlerSeq>)
	void action(cpp::handler_seq* symbol, const SemaHandlerSeq& walker) // function_definition_suffix->function_try_block
	{
	}

	SEMA_POLICY(cpp::parameter_declaration_default, SemaPolicyIdentity)
	void action(cpp::parameter_declaration_default* symbol)
	{
		commit();
		defaultArgument = symbol->init;
	}
	SEMA_POLICY(cpp::parameter_declaration_abstract, SemaPolicyIdentity)
	void action(cpp::parameter_declaration_abstract* symbol)
	{
		commit();
		defaultArgument = symbol->init;
	}
};


struct SemaSimpleDeclaration : public SemaBase, SemaSimpleDeclarationResult
{
	SEMA_BOILERPLATE;

	SemaDeclarationArgs args;

	// prefix
	SemaDeclSpecifierSeqResult seq;

	SemaSimpleDeclaration(const SemaState& state, SemaDeclarationArgs args = SemaDeclarationArgs())
		: SemaBase(state), args(args), seq(&gCtor, context)
	{
	}

	SEMA_POLICY(cpp::decl_specifier_seq, SemaPolicyPush<struct SemaDeclSpecifierSeq>)
	void action(cpp::decl_specifier_seq* symbol, const SemaDeclSpecifierSeq& walker)
	{
		seq = walker.seq;
		templateParams = walker.templateParams; // if this is a class template declaration, templateParams will have been cleared
	}
	SEMA_POLICY(cpp::type_specifier_seq, SemaPolicyPush<struct SemaDeclSpecifierSeq>)
	void action(cpp::type_specifier_seq* symbol, const SemaDeclSpecifierSeq& walker)
	{
		seq = walker.seq;
		templateParams = walker.templateParams; // if this is a class template declaration, templateParams will have been cleared
	}
	SEMA_POLICY(cpp::function_specifier, SemaPolicyIdentity)
	void action(cpp::function_specifier* symbol) // in constructor_definition/member_declaration_implicit -> function_specifier_seq
	{
		if(symbol->id == cpp::function_specifier::EXPLICIT)
		{
			seq.specifiers.isExplicit = true;
		}
	}
	typedef Args2<SemaDeclarationArgs, const SemaDeclSpecifierSeqResult&> DeclarationSuffixWalkerArgs;
	typedef SemaPolicyGeneric<SemaPush<SemaDeclarationSuffix, CommitNull, DeclarationSuffixWalkerArgs> > SemaPolicyDeclarationSuffix;
	SEMA_POLICY_ARGS(cpp::simple_declaration_named, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::simple_declaration_named* symbol, const SemaDeclarationSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::member_declaration_named, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::member_declaration_named* symbol, const SemaDeclarationSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::member_declaration_bitfield, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::member_declaration_bitfield* symbol, const SemaDeclarationSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::parameter_declaration_suffix, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::parameter_declaration_suffix* symbol, const SemaDeclarationSuffix& walker) // parameter_declaration
	{
		declaration = walker.declaration;
		defaultArgument = walker.defaultArgument;
	}
	SEMA_POLICY_ARGS(cpp::function_definition, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::function_definition* symbol, const SemaDeclarationSuffix& walker) // constructor_definition
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::condition_declarator, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::condition_declarator* symbol, const SemaDeclarationSuffix& walker) // condition
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::exception_declarator, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::exception_declarator* symbol, SemaDeclarationSuffix& walker) // exception_declaration
	{
		walker.commit();
		declaration = walker.declaration;
	}

	SEMA_POLICY(cpp::type_declaration_suffix, SemaPolicyIdentity)
	void action(cpp::type_declaration_suffix* symbol)
	{
		declaration = seq.type.declaration; // no declarator was specified, this is the declaration of a class/enum
		if(seq.forward != 0) // declare the name found in elaborated-type-specifier parse
		{
			bool isSpecialization = !isClassKey(*seq.type.declaration);
			if(isSpecialization
				&& (seq.specifiers.isFriend
					|| isExplicitInstantiation))
			{
				// friend class C<int>; // friend
				// template class C<int>; // explicit instantiation
			}
			else if(seq.specifiers.isFriend)
			{
				// friend class C;
			}
			else
			{
				// class C;
				// template<class T> class C;
				// template<> class C<int>;
				// template<class T> class C<T*>;
				DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *seq.forward, TYPE_CLASS, 0, DeclSpecifiers(), templateParams != 0, getTemplateParams(), isSpecialization, seq.type.templateArguments);
#ifdef ALLOCATOR_DEBUG
				trackDeclaration(instance);
#endif
				setDecoration(seq.forward, instance);
				declaration = instance;
				if(declaration->templateParamScope == 0)
				{
					declaration->templateParamScope = templateParamScope; // required by findEnclosingType
				}
			}
			seq.type = TypeId(declaration, context); // TODO: is this necessary?
		}
		else if(declaration != 0
			&& declaration->isTemplate
			&& templateParams != 0) // if not an explicit-specialization
		{
			// template<class T> class C;
			SEMANTIC_ASSERT(!declaration->isSpecialization);
			mergeTemplateParamDefaults(*declaration, *templateParams);
		}

		if(seq.isUnion
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


struct SemaTypeParameter : public SemaBase
{
	SEMA_BOILERPLATE;

	TemplateParameter param; // result
	IdentifierPtr id;
	TemplateArgument argument; // the default argument for this param
	TemplateParameters params; // the template parameters for this param (if template-template-param)
	size_t templateParameter;
	Once committed;
	SemaTypeParameter(const SemaState& state, size_t templateParameter)
		: SemaBase(state), param(context), id(&gAnonymousId), argument(context), params(context), templateParameter(templateParameter)
	{
	}
	void commit()
	{
		committed();
		SEMANTIC_ASSERT(param.declaration == 0); // may only be called once, after parse of type-parameter succeeds
		DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_PARAM, 0, DECLSPEC_TYPEDEF, !params.empty(), params, false, TEMPLATEARGUMENTS_NULL, templateParameter);
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(instance);
#endif
		if(id != &gAnonymousId)
		{
			setDecoration(id, instance);
		}
		param = instance;
		setDependent(param);
		makeUniqueTypeSafe(param);
		param.argument.swap(argument);
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentity)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		SEMANTIC_ASSERT(params.empty());
		walker.committed.test();
		argument.type = walker.type;
	}
	SEMA_POLICY(cpp::template_parameter_clause, SemaPolicyPush<struct SemaTemplateParameterClause>)
	void action(cpp::template_parameter_clause* symbol, const SemaTemplateParameterClauseResult& walker)
	{
		params = walker.params;
	}
	SEMA_POLICY(cpp::id_expression, SemaPolicyPushChecked<struct SemaIdExpression>)
	bool action(cpp::id_expression* symbol, const SemaIdExpression& walker) // the default argument for a template-template-parameter
	{
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
		return true;
	}
};

struct SemaTemplateParameterList : public SemaBase
{
	SEMA_BOILERPLATE;

	TemplateParameter param; // internal state
	TemplateParameters params; // result
	size_t count; // internal state
	Once committed;
	SemaTemplateParameterList(const SemaState& state, size_t count)
		: SemaBase(state), param(context), params(context), count(count)
	{
	}
	void commit()
	{
		committed();
		params.push_front(param);
	}
	SEMA_POLICY_ARGS(cpp::type_parameter_default, SemaPolicyPushIndexCommit<struct SemaTypeParameter>, count)
	void action(cpp::type_parameter_default* symbol, const SemaTypeParameter& walker)
	{
		walker.committed.test();
		param = walker.param;
		++count;
	}
	SEMA_POLICY_ARGS(cpp::type_parameter_template, SemaPolicyPushIndexCommit<struct SemaTypeParameter>, count)
	void action(cpp::type_parameter_template* symbol, const SemaTypeParameter& walker)
	{
		walker.committed.test();
		param = walker.param;
		++count;
	}
	SEMA_POLICY_ARGS(cpp::parameter_declaration, SemaPolicyParameterDeclaration<SemaSimpleDeclaration>, SemaDeclarationArgs(false, count))
	void action(cpp::parameter_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
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
	SEMA_POLICY_ARGS(cpp::template_parameter_list, SemaPolicyPushIndexCommit<struct SemaTemplateParameterList>, count)
	void action(cpp::template_parameter_list* symbol, const SemaTemplateParameterList& walker)
	{
		walker.committed.test();
		params = walker.params;
	}
};

struct SemaTemplateParameterClause : public SemaBase, SemaTemplateParameterClauseResult
{
	SEMA_BOILERPLATE;

	SemaTemplateParameterClause(const SemaState& state)
		: SemaBase(state), SemaTemplateParameterClauseResult(context)
	{
		// collect template-params into a new scope
		if(templateParamScope != 0)
		{
			pushScope(templateParamScope); // the existing template parameter scope encloses the new scope
		}
		pushScope(newScope(makeIdentifier("$template"), SCOPETYPE_TEMPLATE));
		clearTemplateParams();
		enclosing->templateDepth = templateDepth;
		enclosingDeferred = 0; // don't defer parse of default-argument for non-type template-parameter
	}
	SEMA_POLICY_ARGS(cpp::template_parameter_list, SemaPolicyPushIndexCommit<struct SemaTemplateParameterList>, 0)
	void action(cpp::template_parameter_list* symbol, const SemaTemplateParameterList& walker)
	{
		walker.committed.test();
		params = walker.params;
	}
};

struct SemaTemplateDeclaration : public SemaBase, SemaDeclarationResult
{
	SEMA_BOILERPLATE;

	TemplateParameters params; // internal state
	SemaTemplateDeclaration(const SemaState& state)
		: SemaBase(state), params(context)
	{
		++templateDepth;
		templateParams = &TEMPLATEPARAMETERS_NULL; // explicit specialization has empty template params: template<> struct S;
	}
	SEMA_POLICY(cpp::template_parameter_clause, SemaPolicyPush<struct SemaTemplateParameterClause>)
	void action(cpp::template_parameter_clause* symbol, const SemaTemplateParameterClause& walker)
	{
		templateParamScope = walker.enclosing;
		enclosing = walker.enclosing->parent;
		params = walker.params;
		templateParams = &params;
	}
	SEMA_POLICY(cpp::declaration, SemaPolicyPushSrc<struct SemaDeclaration>)
	void action(cpp::declaration* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
	SEMA_POLICY(cpp::member_declaration, SemaPolicyPush<struct SemaMemberDeclaration>)
	void action(cpp::member_declaration* symbol, const SemaMemberDeclaration& walker)
	{
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
};

struct SemaExplicitInstantiation : public SemaBase
{
	SEMA_BOILERPLATE;

	DeclarationPtr declaration;
	SemaExplicitInstantiation(const SemaState& state)
		: SemaBase(state), declaration(0)
	{
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isExplicitInstantiation = true;
	}
	SEMA_POLICY(cpp::declaration, SemaPolicyPush<struct SemaDeclaration>)
	void action(cpp::declaration* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
};

struct SemaDeclaration : public SemaBase, SemaDeclarationResult
{
	SEMA_BOILERPLATE;

	SemaDeclaration(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::linkage_specification, SemaPolicyPush<struct SemaNamespace>)
	void action(cpp::linkage_specification* symbol, const SemaNamespaceResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::namespace_definition, SemaPolicyPush<struct SemaNamespace>)
	void action(cpp::namespace_definition* symbol, const SemaNamespaceResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::namespace_alias_definition, SemaPolicyPush<struct SemaNamespaceAliasDefinition>)
	void action(cpp::namespace_alias_definition* symbol, const SemaNamespaceAliasDefinition& walker)
	{
	}
	SEMA_POLICY(cpp::general_declaration, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::general_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	// occurs in for-init-statement
	SEMA_POLICY(cpp::simple_declaration, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::simple_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::constructor_definition, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::constructor_definition* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::template_declaration, SemaPolicyPush<struct SemaTemplateDeclaration>)
	void action(cpp::template_declaration* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::explicit_instantiation, SemaPolicyPush<struct SemaExplicitInstantiation>)
	void action(cpp::explicit_instantiation* symbol, const SemaExplicitInstantiation& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::explicit_specialization, SemaPolicyPush<struct SemaTemplateDeclaration>)
	void action(cpp::explicit_specialization* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::using_declaration, SemaPolicyPush<struct SemaUsingDeclaration>)
	void action(cpp::using_declaration* symbol, const SemaUsingDeclaration& walker)
	{
	}
	SEMA_POLICY(cpp::using_directive, SemaPolicyPush<struct SemaUsingDirective>)
	void action(cpp::using_directive* symbol, const SemaUsingDirective& walker)
	{
	}
};

struct SemaNamespace : public SemaBase, SemaNamespaceResult
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	SemaNamespace(SemaContext& context)
		: SemaBase(context), id(0)
	{
		pushScope(&context.global);
	}

	SemaNamespace(const SemaState& state)
		: SemaBase(state), id(0)
	{
	}

	SEMA_POLICY(cpp::identifier, SemaPolicyIdentity)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
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
	SEMA_POLICY(cpp::declaration, SemaPolicyPushTop<struct SemaDeclaration>)
	void action(cpp::declaration* symbol, const SemaDeclarationResult& walker)
	{
	}
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

	SemaContext& globals = *new SemaContext(context, getAllocator(context));
	SemaNamespace& walker = *new SemaNamespace(globals);
	ParserGeneric<SemaNamespace> parser(context, walker);

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

	SemaContext& globals = *new SemaContext(context, getAllocator(context));
	SemaCompoundStatement& walker = *new SemaCompoundStatement(globals);
	ParserGeneric<SemaCompoundStatement> parser(context, walker);

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


