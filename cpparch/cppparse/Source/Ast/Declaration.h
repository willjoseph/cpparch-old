
#ifndef INCLUDED_CPPPARSE_AST_DECLARATION_H
#define INCLUDED_CPPPARSE_AST_DECLARATION_H


#include "Parse/Grammar.h"
#include "AstAllocator.h"
#include "Expression.h"
#include "Common/Sequence.h"
#include "Common/Copied.h"

struct DeclSpecifiers
{
	bool isTypedef;
	bool isFriend;
	bool isStatic;
	bool isExtern;
	bool isExplicit;
	bool isMutable;
	DeclSpecifiers()
		: isTypedef(false), isFriend(false), isStatic(false), isExtern(false), isExplicit(false), isMutable(false)
	{
	}
	DeclSpecifiers(bool isTypedef, bool isFriend, bool isStatic, bool isExtern)
		: isTypedef(isTypedef), isFriend(isFriend), isStatic(isStatic), isExtern(isExtern), isExplicit(false), isMutable(false)
	{
	}
};

const DeclSpecifiers DECLSPEC_TYPEDEF = DeclSpecifiers(true, false, false, false);


// ----------------------------------------------------------------------------
// identifier

typedef cpp::terminal_identifier Identifier;

inline Identifier makeIdentifier(const char* value)
{
	Identifier result = { TokenValue(value) };
	return result;
}

const Identifier IDENTIFIER_NULL = Identifier();

inline const char* getValue(const Identifier& id)
{
	return id.value.empty() ? "$unnamed" : id.value.c_str();
}

typedef SafePtr<Identifier> IdentifierPtr;



// ----------------------------------------------------------------------------
// type sequence


struct TypeSequenceVisitor
{
	virtual void visit(const struct DeclaratorPointerType&) = 0;
	virtual void visit(const struct DeclaratorReferenceType&) = 0;
	virtual void visit(const struct DeclaratorArrayType&) = 0;
	virtual void visit(const struct DeclaratorMemberPointerType&) = 0;
	virtual void visit(const struct DeclaratorFunctionType&) = 0;
};

typedef Sequence<AstAllocator<int>, TypeSequenceVisitor> TypeSequence;

// ----------------------------------------------------------------------------
// type


typedef ListReference<struct TemplateArgument, AstAllocator<struct TemplateArgument> > TemplateArguments2;

// wrapper to disable default-constructor
struct TemplateArguments : public TemplateArguments2
{
	TemplateArguments(const AstAllocator<int>& allocator)
		: TemplateArguments2(allocator)
	{
	}
private:
	TemplateArguments()
	{
	}
};




typedef ListReference<struct Type, AstAllocator<int> > Types2;

// wrapper to disable default-constructor
struct Types : public Types2
{
	Types(const AstAllocator<int>& allocator)
		: Types2(allocator)
	{
	}
private:
	Types()
	{
	}
};

typedef ListReference<struct TypeId, AstAllocator<int> > TypeIds2;

// wrapper to disable default-constructor
struct TypeIds : public TypeIds2
{
	TypeIds(const AstAllocator<int>& allocator)
		: TypeIds2(allocator)
	{
	}
private:
	TypeIds()
	{
	}
};


struct Type;

typedef Reference<const Type> TypePtr;

typedef CopiedReference<const Type, AstAllocator<int> > TypeRef;

typedef TypeRef Qualifying;


class Declaration;
typedef SafePtr<Declaration> DeclarationPtr;
struct Scope;
typedef SafePtr<Scope> ScopePtr;

const size_t INDEX_INVALID = size_t(-1);


// refers to the innermost template scope that a name/type/expression depends on
struct Dependent : DeclarationPtr
{
	Dependent()
		: DeclarationPtr(0)
	{
	}
	explicit Dependent(Declaration* p)
		: DeclarationPtr(p)
	{
	}
};

struct Type
{
	IdentifierPtr id;
	DeclarationPtr declaration;
	TemplateArguments templateArguments; // may be non-empty if this is a template
	Qualifying qualifying;
	ExpressionWrapper expression; // for decltype(expression)
	Dependent dependent;
	ScopePtr enclosingTemplate;
	UniqueType unique;
	bool isDependent; // true if the type is dependent in the context in which it was parsed
	bool isImplicitTemplateId; // true if this is a template but the template-argument-clause has not been specified
	bool isEnclosingClass; // true if this is the type of an enclosing class
	Type(Declaration* declaration, const AstAllocator<int>& allocator)
		: id(0), declaration(declaration), templateArguments(allocator), qualifying(allocator), enclosingTemplate(0), unique(0), isDependent(false), isImplicitTemplateId(false), isEnclosingClass(false)
	{
	}
	void swap(Type& other)
	{
		std::swap(id, other.id);
		std::swap(declaration, other.declaration);
		templateArguments.swap(other.templateArguments);
		qualifying.swap(other.qualifying);
		std::swap(expression, other.expression);
		std::swap(dependent, other.dependent);
		std::swap(enclosingTemplate, other.enclosingTemplate);
		std::swap(unique, other.unique);
		std::swap(isDependent, other.isDependent);
		std::swap(isImplicitTemplateId, other.isImplicitTemplateId);
		std::swap(isEnclosingClass, other.isEnclosingClass);
	}
	Type& operator=(Declaration* declaration)
	{
		SYMBOLS_ASSERT(templateArguments.empty());
		SYMBOLS_ASSERT(qualifying.empty());
		this->declaration = declaration;
		return *this;
	}
private:
	Type();
};

struct TypeId : Type
{
	TypeSequence typeSequence;
	CvQualifiers qualifiers;

	TypeId(Declaration* declaration, const AstAllocator<int>& allocator)
		: Type(declaration, allocator), typeSequence(allocator)
	{
	}
	TypeId& operator=(Declaration* declaration)
	{
		SYMBOLS_ASSERT(typeSequence.empty());
		Type::operator=(declaration);
		return *this;
	}
	TypeId& operator=(const Type& type)
	{
		SYMBOLS_ASSERT(typeSequence.empty());
		Type::operator=(type);
		return *this;
	}
	void swap(TypeId& other)
	{
		Type::swap(other);
		typeSequence.swap(other.typeSequence);
		std::swap(qualifiers, other.qualifiers);
	}
	void swap(Type& other)
	{
		SYMBOLS_ASSERT(typeSequence.empty());
		Type::swap(other);
	}
};

#define TYPE_NULL TypeId(0, AST_ALLOCATOR_NULL)


// ----------------------------------------------------------------------------


struct Location : Source
{
	std::size_t pointOfInstantiation;
	Location()
	{
	}
	Location(Source source, std::size_t pointOfInstantiation)
		: Source(source), pointOfInstantiation(pointOfInstantiation)
	{
	}
};



// ----------------------------------------------------------------------------
// template-argument



struct TemplateArgument
{
	TypeId type;
	Dependent valueDependent;
	ExpressionWrapper expression;
	Location source;
#if 0
	TemplateArgument(const TypeId& type) : type(type)
	{
	}
#endif
	TemplateArgument(const AstAllocator<int>& allocator)
		: type(0, allocator)
	{
	}
	void swap(TemplateArgument& other)
	{
		type.swap(other.type);
		std::swap(valueDependent, other.valueDependent);
		std::swap(expression, other.expression);
	}
};

#define TEMPLATEARGUMENT_NULL TemplateArgument(AST_ALLOCATOR_NULL)


const TemplateArguments TEMPLATEARGUMENTS_NULL = TemplateArguments(AST_ALLOCATOR_NULL);


struct TemplateParameter : Type
{
	TemplateArgument argument;
	TemplateParameter(const AstAllocator<int>& allocator)
		: Type(0, allocator), argument(allocator)
	{
	}
	void swap(TemplateParameter& other)
	{
		Type::swap(other);
		argument.swap(other.argument);
	}
	Type& operator=(Declaration* declaration)
	{
		return Type::operator=(declaration);
	}
};

struct TemplateParameters : Types
{
	TemplateArguments defaults;
	TemplateParameters(const AstAllocator<int>& allocator)
		: Types(allocator), defaults(allocator)
	{
	}
	void swap(TemplateParameters& other)
	{
		Types::swap(other);
		defaults.swap(other.defaults);
	}
	void push_front(const TemplateParameter& other)
	{
		Types::push_front(other);
		defaults.push_front(other.argument);
	}
};

const TemplateParameters TEMPLATEPARAMETERS_NULL = TemplateParameters(AST_ALLOCATOR_NULL);


// ----------------------------------------------------------------------------
// declaration


class Declaration
{
	Identifier* name;

#if 0
	Declaration(const Declaration&);
	Declaration& operator=(const Declaration&);
#endif
public:
	std::size_t uniqueId;
	Scope* scope;
	TypeId type;
	Scope* enclosed;
	Scope* templateParamScope;
	Declaration* overloaded;
	Dependent valueDependent; // the dependent-types/names that are referred to in the declarator-suffix (array size)
	ExpressionWrapper initializer; // if this is a constant (enumerator or const integral), the initializer constant-expression
	DeclSpecifiers specifiers;
	std::size_t templateParameter;
	TemplateParameters templateParams;
	TemplateArguments templateArguments; // non-empty if this is an explicit (or partial) specialization
	bool isComplete; // for class declarations, set to true when the closing brace is parsed.
	bool isTemplate;
	bool isTemplateName; // true if this is a template declaration, or an overload of a template declaration
	bool isSpecialization;
	bool isFunctionDefinition;
	std::size_t instance;

	Declaration(
		const AstAllocator<int>& allocator,
		Scope* scope,
		Identifier& name,
		const TypeId& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateParameters& templateParams = TEMPLATEPARAMETERS_NULL,
		bool isSpecialization = false,
		const TemplateArguments& templateArguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = Dependent()
		) : name(&name),
		uniqueId(0),
		scope(scope),
		type(type),
		enclosed(enclosed),
		templateParamScope(0),
		overloaded(0),
		valueDependent(valueDependent),
		specifiers(specifiers),
		templateParameter(templateParameter),
		templateParams(templateParams),
		templateArguments(templateArguments),
		isComplete(false),
		isTemplate(isTemplate),
		isTemplateName(isTemplate),
		isSpecialization(isSpecialization),
		isFunctionDefinition(false),
		instance(INDEX_INVALID)
	{
	}
	Declaration() :
	type(0, AST_ALLOCATOR_NULL),
		templateParams(AST_ALLOCATOR_NULL),
		templateArguments(AST_ALLOCATOR_NULL)
	{
	}
	void swap(Declaration& other)
	{
		std::swap(name, other.name);
		std::swap(uniqueId, other.uniqueId);
		std::swap(scope, other.scope);
		type.swap(other.type);
		std::swap(enclosed, other.enclosed);
		std::swap(templateParamScope, other.templateParamScope);
		std::swap(overloaded, other.overloaded);
		std::swap(valueDependent, other.valueDependent);
		std::swap(initializer, other.initializer);
		std::swap(specifiers, other.specifiers);
		std::swap(templateParameter, other.templateParameter);
		templateParams.swap(other.templateParams);
		templateArguments.swap(other.templateArguments);
		std::swap(isComplete, other.isComplete);
		std::swap(isTemplate, other.isTemplate);
		std::swap(isTemplateName, other.isTemplateName);
		std::swap(isSpecialization, other.isSpecialization);
		std::swap(isFunctionDefinition, other.isFunctionDefinition);
		std::swap(instance, other.instance);
	}


	Identifier& getName()
	{
		return *name;
	}
	const Identifier& getName() const
	{
		return *name;
	}
	void setName(Identifier& other)
	{
		name = &other;
	}
};

typedef SafePtr<Declaration> DeclarationPtr;



#endif
