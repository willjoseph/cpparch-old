
#ifndef INCLUDED_CPPPARSE_SYMBOLS_H
#define INCLUDED_CPPPARSE_SYMBOLS_H


#include "cpptree.h"
#include "copied.h"
#include "allocator.h"
#include "list.h"
#include "parser.h"
#include "indirect_set.h"
#include "sequence.h"

#include <list>
#include <map>


#define SYMBOLS_ASSERT ALLOCATOR_ASSERT
typedef AllocatorError SymbolsError;

struct DeclSpecifiers
{
	bool isTypedef;
	bool isFriend;
	bool isStatic;
	bool isExtern;
	DeclSpecifiers()
		: isTypedef(false), isFriend(false), isStatic(false), isExtern(false)
	{
	}
	DeclSpecifiers(bool isTypedef, bool isFriend, bool isStatic, bool isExtern)
		: isTypedef(isTypedef), isFriend(isFriend), isStatic(isStatic), isExtern(isExtern)
	{
	}
};

const DeclSpecifiers DECLSPEC_TYPEDEF = DeclSpecifiers(true, false, false, false);



// ----------------------------------------------------------------------------
// Allocator

#define TREEALLOCATOR_LINEAR

#ifdef TREEALLOCATOR_LINEAR
#define TreeAllocator ParserAllocatorWrapper
#define TREEALLOCATOR_NULL TreeAllocator<int>(NullParserAllocator())
#else
#define TreeAllocator DebugAllocator
#define TREEALLOCATOR_NULL TreeAllocator<int>()
#endif


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

typedef Sequence<TreeAllocator<int>, TypeSequenceVisitor> TypeSequence;

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
// type


typedef ListReference<struct TemplateArgument, TreeAllocator<struct TemplateArgument> > TemplateArguments2;

// wrapper to disable default-constructor
struct TemplateArguments : public TemplateArguments2
{
	TemplateArguments(const TreeAllocator<int>& allocator)
		: TemplateArguments2(allocator)
	{
	}
private:
	TemplateArguments()
	{
	}
};




typedef ListReference<struct Type, TreeAllocator<int> > Types2;

// wrapper to disable default-constructor
struct Types : public Types2
{
	Types(const TreeAllocator<int>& allocator)
		: Types2(allocator)
	{
	}
private:
	Types()
	{
	}
};

typedef ListReference<struct TypeId, TreeAllocator<int> > TypeIds2;

// wrapper to disable default-constructor
struct TypeIds : public TypeIds2
{
	TypeIds(const TreeAllocator<int>& allocator)
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

typedef CopiedReference<const Type, TreeAllocator<int> > TypeRef;

typedef TypeRef Qualifying;


class Declaration;
typedef SafePtr<Declaration> DeclarationPtr;
struct Scope;
typedef SafePtr<Scope> ScopePtr;

const size_t INDEX_INVALID = size_t(-1);

struct Type
{
	IdentifierPtr id;
	DeclarationPtr declaration;
	TemplateArguments templateArguments; // may be non-empty if this is a template
	Qualifying qualifying;
	DeclarationPtr dependent;
	ScopePtr enclosingTemplate;
	UniqueType unique;
	bool isDependent; // true if the type is dependent in the context in which it was parsed
	bool isImplicitTemplateId; // true if this is a template but the template-argument-clause has not been specified
	bool isEnclosingClass; // true if this is the type of an enclosing class
	Type(Declaration* declaration, const TreeAllocator<int>& allocator)
		: id(0), declaration(declaration), templateArguments(allocator), qualifying(allocator), dependent(0), enclosingTemplate(0), unique(0), isDependent(false), isImplicitTemplateId(false), isEnclosingClass(false)
	{
	}
	void swap(Type& other)
	{
		std::swap(id, other.id);
		std::swap(declaration, other.declaration);
		templateArguments.swap(other.templateArguments);
		qualifying.swap(other.qualifying);
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
 
	TypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: Type(declaration, allocator), typeSequence(allocator)
	{
	}
	TypeId& operator=(Declaration* declaration)
	{
		SYMBOLS_ASSERT(typeSequence.empty());
		Type::operator=(declaration);
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

#define TYPE_NULL TypeId(0, TREEALLOCATOR_NULL)

// ----------------------------------------------------------------------------
// dependent-name

// refers to the innermost template scope that a name/type/expression depends on
struct Dependent : DeclarationPtr
{
	Dependent() : DeclarationPtr(0)
	{
	}
};

#if 0
// ----------------------------------------------------------------------------
// deferred lookup of dependent names

typedef ListReference<struct DeferredLookupCallback, TreeAllocator<int> > DeferredLookup2;

struct DeferredLookup : public DeferredLookup2
{
	DeferredLookup(const TreeAllocator<int>& allocator) : DeferredLookup2(allocator)
	{
	}
	void splice(DeferredLookup& other)
	{
		DeferredLookup2::splice(begin(), other);
	}
private:
	DeferredLookup();
};
#endif


// ----------------------------------------------------------------------------
// [expr.const]
struct IntegralConstant
{
	int value;
	IntegralConstant() : value(0)
	{
	}
	explicit IntegralConstant(int value) : value(value)
	{
	}
	explicit IntegralConstant(double value) : value(int(value))
	{
	}
	explicit IntegralConstant(size_t value) : value(int(value))
	{
	}
};

typedef IntegralConstant (*UnaryIceOp)(IntegralConstant);
typedef IntegralConstant (*BinaryIceOp)(IntegralConstant, IntegralConstant);
typedef IntegralConstant (*TernaryIceOp)(IntegralConstant, IntegralConstant, IntegralConstant);

inline IntegralConstant identity(IntegralConstant left)
{
	return left;
}

inline IntegralConstant assign(IntegralConstant left, IntegralConstant right)
{
	return left = right;
}

// [expr.unary.op]
inline IntegralConstant operator++(IntegralConstant left)
{
	return IntegralConstant(++left.value);
}
inline IntegralConstant operator--(IntegralConstant left)
{
	return IntegralConstant(--left.value);
}
inline IntegralConstant operator+(IntegralConstant left)
{
	return IntegralConstant(+left.value);
}
inline IntegralConstant operator-(IntegralConstant left)
{
	return IntegralConstant(-left.value);
}
inline IntegralConstant operator!(IntegralConstant left)
{
	return IntegralConstant(!left.value);
}
inline IntegralConstant operator~(IntegralConstant left)
{
	return IntegralConstant(~left.value);
}
inline IntegralConstant addressOf(IntegralConstant left)
{
	return IntegralConstant(0); // TODO
}
inline IntegralConstant dereference(IntegralConstant left)
{
	return IntegralConstant(0); // TODO
}

inline UnaryIceOp getUnaryIceOp(cpp::unary_expression_op* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::unary_operator::PLUSPLUS: return operator++;
	case cpp::unary_operator::MINUSMINUS: return operator--;
	case cpp::unary_operator::STAR: return dereference;
	case cpp::unary_operator::AND: return addressOf;
	case cpp::unary_operator::PLUS: return operator+;
	case cpp::unary_operator::MINUS: return operator-;
	case cpp::unary_operator::NOT: return operator!;
	case cpp::unary_operator::COMPL: return operator~;
	default: break;
	}
	throw SymbolsError();
}

// [expr.mptr.oper]
inline BinaryIceOp getBinaryIceOp(cpp::pm_expression_default* symbol)
{
	return 0; // N/A
}


// [expr.mul]
inline IntegralConstant operator*(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value * right.value);
}
inline IntegralConstant operator/(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value / right.value);
}
inline IntegralConstant operator%(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value % right.value);
}

inline BinaryIceOp getBinaryIceOp(cpp::multiplicative_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::multiplicative_operator::STAR: return operator*;
	case cpp::multiplicative_operator::DIVIDE: return operator/;
	case cpp::multiplicative_operator::PERCENT: return operator%;
	default: break;
	}
	throw SymbolsError();
}

// [expr.add]
inline IntegralConstant operator+(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value + right.value);
}
inline IntegralConstant operator-(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value - right.value);
}

inline BinaryIceOp getBinaryIceOp(cpp::additive_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::additive_operator::PLUS: return operator+;
	case cpp::additive_operator::MINUS: return operator-;
	default: break;
	}
	throw SymbolsError();
}

// [expr.shift]
inline IntegralConstant operator<<(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value << right.value);
}
inline IntegralConstant operator>>(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value >> right.value);
}

inline BinaryIceOp getBinaryIceOp(cpp::shift_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::shift_operator::SHIFTLEFT: return operator<<;
	case cpp::shift_operator::SHIFTRIGHT: return operator>>;
	default: break;
	}
	throw SymbolsError();
}

// [expr.rel]
inline IntegralConstant operator<(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value < right.value);
}
inline IntegralConstant operator>(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value > right.value);
}
inline IntegralConstant operator<=(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value <= right.value);
}
inline IntegralConstant operator>=(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value >= right.value);
}

inline BinaryIceOp getBinaryIceOp(cpp::relational_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::relational_operator::LESS: return operator<;
	case cpp::relational_operator::GREATER: return operator>;
	case cpp::relational_operator::LESSEQUAL: return operator<=;
	case cpp::relational_operator::GREATEREQUAL: return operator>=;
	default: break;
	}
	throw SymbolsError();
}

// [expr.eq]
inline IntegralConstant operator==(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value == right.value);
}
inline IntegralConstant operator!=(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value != right.value);
}

inline BinaryIceOp getBinaryIceOp(cpp::equality_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::equality_operator::EQUAL: return operator==;
	case cpp::equality_operator::NOTEQUAL: return operator!=;
	default: break;
	}
	throw SymbolsError();
}

// [expr.bit.and]
inline IntegralConstant operator&(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value & right.value);
}
inline BinaryIceOp getBinaryIceOp(cpp::and_expression_default* symbol)
{
	return operator&;
}

// [expr.xor]
inline IntegralConstant operator^(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value ^ right.value);
}
inline BinaryIceOp getBinaryIceOp(cpp::exclusive_or_expression_default* symbol)
{
	return operator^;
}

// [expr.or]
inline IntegralConstant operator|(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value | right.value);
}
inline BinaryIceOp getBinaryIceOp(cpp::inclusive_or_expression_default* symbol)
{
	return operator|;
}

// [expr.log.and]
inline IntegralConstant operator&&(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value && right.value);
}

inline BinaryIceOp getBinaryIceOp(cpp::logical_and_expression_default* symbol)
{
	return operator&&;
}

// [expr.log.or]
inline IntegralConstant operator||(IntegralConstant left, IntegralConstant right)
{
	return IntegralConstant(left.value || right.value);
}

inline BinaryIceOp getBinaryIceOp(cpp::logical_or_expression_default* symbol)
{
	return operator||;
}

// [expr.ass]
inline BinaryIceOp getBinaryIceOp(cpp::assignment_expression_suffix* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::assignment_operator::ASSIGN: return assign;
	case cpp::assignment_operator::STAR: return operator*;
	case cpp::assignment_operator::DIVIDE: return operator/;
	case cpp::assignment_operator::PERCENT: return operator%;
	case cpp::assignment_operator::PLUS: return operator+;
	case cpp::assignment_operator::MINUS: return operator-;
	case cpp::assignment_operator::SHIFTRIGHT: return operator>>;
	case cpp::assignment_operator::SHIFTLEFT: return operator<<;
	case cpp::assignment_operator::AND: return operator&;
	case cpp::assignment_operator::XOR: return operator^;
	case cpp::assignment_operator::OR: return operator|;
	default: break;
	}
	throw SymbolsError();
}

// [expr.cond]
inline IntegralConstant conditional(IntegralConstant first, IntegralConstant second, IntegralConstant third)
{
	return IntegralConstant(first.value ? second.value : third.value);
}


template<boost::wave::token_id id>
inline Name getBinaryOperatorNameImpl(cpp::terminal<id> op)
{
	return op.value;
}

template<typename T>
inline Name getBinaryOperatorNameImpl(T op)
{
	return op->value.value;
}

template<typename T>
inline Name getBinaryOperatorName(T* symbol)
{
	return getBinaryOperatorNameImpl(symbol->op);
}


// [expr.const]
// An integral constant-expression can involve only literals, enumerators, const variables or static
// data members of integral or enumeration types initialized with constant expressions, non-type template
// parameters of integral or enumeration types, and sizeof expressions
struct ExpressionNodeVisitor
{
	virtual void visit(const struct IntegralConstantExpression&) = 0; // literal
	virtual void visit(const struct NonTypeTemplateParameter&) = 0; // non-type template parameter
	virtual void visit(const struct DependentIdExpression&) = 0; // T::name
	virtual void visit(const struct IdExpression&) = 0; // enumerator, const variable, static data member, non-type template parameter
	virtual void visit(const struct SizeofExpression&) = 0;
	virtual void visit(const struct SizeofTypeExpression&) = 0;
	virtual void visit(const struct UnaryExpression&) = 0;
	virtual void visit(const struct BinaryExpression&) = 0;
	virtual void visit(const struct TernaryExpression&) = 0;
	virtual void visit(const struct TypeTraitsUnaryExpression&) = 0;
	virtual void visit(const struct TypeTraitsBinaryExpression&) = 0;
};

struct ExpressionNode
{
	ExpressionNode()
	{
	}
	virtual ~ExpressionNode()
	{
	}
	virtual void accept(ExpressionNodeVisitor& visitor) const = 0;
	virtual bool operator<(const ExpressionNode& other) const = 0;
};

typedef SafePtr<ExpressionNode> ExpressionPtr;

template<typename T>
struct ExpressionNodeGeneric : ExpressionNode
{
	T value;
	ExpressionNodeGeneric(const T& value)
		: value(value)
	{
	}
	void accept(ExpressionNodeVisitor& visitor) const
	{
		visitor.visit(value);
	}
	bool operator<(const ExpressionNodeGeneric& other) const
	{
		return value < other.value;
	}
	bool operator<(const ExpressionNode& other) const
	{
		return abstractLess(*this, other);
	}
};

typedef ExpressionNode* UniqueExpression;

typedef IndirectSet<UniqueExpression> UniqueExpressions;

extern UniqueExpressions gBuiltInExpressions;
extern UniqueExpressions gUniqueExpressions;

template<typename T>
inline UniqueExpression makeBuiltInExpression(const T& value)
{
	ExpressionNodeGeneric<T> node(value);
	return *gBuiltInExpressions.insert(node);
}

template<typename T>
inline UniqueExpression makeExpression(const T& value)
{
	ExpressionNodeGeneric<T> node(value);
	{
		UniqueExpressions::iterator i = gBuiltInExpressions.find(node);
		if(i != gBuiltInExpressions.end())
		{
			return *i;
		}
	}
	return *gUniqueExpressions.insert(node);
}



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

struct TypeInstance;

inline IntegralConstant evaluate(ExpressionNode* node, Location source, const TypeInstance* enclosing);

struct ExpressionWrapper : ExpressionPtr
{
	bool isConstant;
	bool isTypeDependent;
	bool isValueDependent;
	bool isTemplateArgumentAmbiguity; // [temp.arg] In a template argument, an ambiguity between a typeid and an expression is resolved to a typeid
	bool isQualifiedNonStaticMemberName;
	ExpressionWrapper()
		: ExpressionPtr(0), isConstant(false), isTypeDependent(false), isValueDependent(false), isTemplateArgumentAmbiguity(false), isQualifiedNonStaticMemberName(false)
	{
	}
	ExpressionWrapper(ExpressionNode* node, bool isConstant = true, bool isTypeDependent = false, bool isValueDependent = false)
		: ExpressionPtr(node), isConstant(isConstant), isTypeDependent(isTypeDependent), isValueDependent(isValueDependent), isTemplateArgumentAmbiguity(false), isQualifiedNonStaticMemberName(false)
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
	TemplateArgument(const TreeAllocator<int>& allocator)
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

#define TEMPLATEARGUMENT_NULL TemplateArgument(TREEALLOCATOR_NULL)


const TemplateArguments TEMPLATEARGUMENTS_NULL = TemplateArguments(TREEALLOCATOR_NULL);


struct TemplateParameter : Type
{
	TemplateArgument argument;
	TemplateParameter(const TreeAllocator<int>& allocator)
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
	TemplateParameters(const TreeAllocator<int>& allocator)
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

const TemplateParameters TEMPLATEPARAMETERS_NULL = TemplateParameters(TREEALLOCATOR_NULL);


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
	bool isSpecialization;
	bool isFunctionDefinition;
	std::size_t instance;

	Declaration(
		const TreeAllocator<int>& allocator,
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
		isSpecialization(isSpecialization),
		isFunctionDefinition(false),
		instance(INDEX_INVALID)
	{
	}
	Declaration() :
		type(0, TREEALLOCATOR_NULL),
		templateParams(TREEALLOCATOR_NULL),
		templateArguments(TREEALLOCATOR_NULL)
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

// ----------------------------------------------------------------------------
// scope

struct UniqueName : public Identifier
{
	char buffer[10];
	UniqueName(size_t index)
	{
		sprintf(buffer, "$%x", unsigned(index));
		Identifier::value = TokenValue(buffer);
	}
};
typedef std::vector<UniqueName*> UniqueNames;
extern UniqueNames gUniqueNames;

enum ScopeType
{
	SCOPETYPE_UNKNOWN,
	SCOPETYPE_NAMESPACE,
	SCOPETYPE_PROTOTYPE,
	SCOPETYPE_LOCAL,
	SCOPETYPE_CLASS,
	SCOPETYPE_TEMPLATE,
};

extern size_t gScopeCount;

struct ScopeCounter
{
	ScopeCounter()
	{
		++gScopeCount;
	}
	ScopeCounter(const ScopeCounter&)
	{
		++gScopeCount;
	}
	~ScopeCounter()
	{
		--gScopeCount;
	}
};

// An instance of a declaration - multiple declarations may refer to the same entity.
// e.g. definition, forward declaration, redeclaration
struct DeclarationInstance : DeclarationPtr
{
	Identifier* name; // the identifier used in this declaration.
	const DeclarationInstance* overloaded; // the previously declared overload of this name (which may or may not refer to the same entity.)
	const DeclarationInstance* redeclared; // the previous declaration that refers to the same entity.
	std::size_t ordering; // every declaration declared before this has a lesser value
	DeclarationInstance()
		: DeclarationPtr(0), name(0), overloaded(0), redeclared(0), ordering(INDEX_INVALID)
	{
	}
	// used when cloning an existing declaration, in the process of copying declarations from one scope to another.
	explicit DeclarationInstance(Declaration* declaration, std::size_t ordering = INDEX_INVALID)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0), ordering(ordering)
	{
		SYMBOLS_ASSERT(name != 0);
	}
#if 0
	explicit DeclarationInstance(DeclarationPtr declaration)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0)
	{
		SYMBOLS_ASSERT(name != 0);
	}
#endif
};

inline bool operator<(const DeclarationInstance& l, const DeclarationInstance& r)
{
	return l.name < r.name;
}

inline cpp::terminal_identifier& getDeclarationId(const DeclarationInstance* declaration)
{
	return (*declaration)->getName();
}

inline bool isDecorated(const Identifier& id)
{
	return id.dec.p != 0;
}

inline const DeclarationInstance& getDeclaration(const Identifier& id)
{
	SYMBOLS_ASSERT(isDecorated(id));
	return *id.dec.p;
}


struct Scope : public ScopeCounter
{
	ScopePtr parent;
	Identifier name;
	size_t enclosedScopeCount; // number of scopes directly enclosed by this scope
	typedef std::less<TokenValue> IdentifierLess;

	typedef std::multimap<TokenValue, DeclarationInstance, IdentifierLess, TreeAllocator<int> > Declarations2;

	struct Declarations : public Declarations2
	{
		Declarations(const TreeAllocator<int>& allocator)
			: Declarations2(IdentifierLess(), allocator)
		{
		}
		Declarations(const Declarations& other)
			: Declarations2(other)
		{
			SYMBOLS_ASSERT(other.empty());
		}
		~Declarations()
		{
			SYMBOLS_ASSERT(Declarations2::empty());
			// hack: stop declarations being cleared
			new(static_cast<Declarations2*>(this)) Declarations2(IdentifierLess(), TREEALLOCATOR_NULL);
		}

		const DeclarationInstance& insert(const DeclarationInstance& declaration)
		{
			SYMBOLS_ASSERT(declaration.name != 0);
			Declarations2::iterator result = Declarations2::insert(Declarations2::value_type(declaration.name->value, declaration));
			return (*result).second;
		}
	};


	Declarations declarations;
	ScopeType type;
	Types bases;
	typedef List<ScopePtr, TreeAllocator<int> > Scopes;
	Scopes usingDirectives;
	typedef List<DeclarationPtr, TreeAllocator<int> > DeclarationList;
	DeclarationList declarationList;

#if 0
	DeferredLookup deferred;
	size_t deferredCount;
#endif
	size_t templateDepth;

	Scope(const TreeAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(allocator), type(type), bases(allocator), usingDirectives(allocator), declarationList(allocator),
#if 0
		deferred(allocator), deferredCount(0),
#endif
		templateDepth(0)

	{
	}
	~Scope()
	{
	}

	Identifier& getUniqueName()
	{
		if(enclosedScopeCount == gUniqueNames.size())
		{
			gUniqueNames.push_back(new UniqueName(enclosedScopeCount));
		}
		return *gUniqueNames[enclosedScopeCount++];
	}

private:
	//Scope(const Scope&);
	//Scope& operator=(const Scope&);
};

inline Scope::Declarations::iterator findDeclaration(Scope::Declarations& declarations, const DeclarationInstance* declaration)
{
	const Identifier& id = *declaration->name;
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(&(*i).second == declaration)
		{
			return i;
		}
	}

	return declarations.end();
}

inline void undeclare(const DeclarationInstance* p, LexerAllocator& allocator)
{
	SYMBOLS_ASSERT(p->name->dec.p == 0 || p->name->dec.p == p);
	p->name->dec.p = 0;

	Declaration* declaration = *p;
	SYMBOLS_ASSERT(!declaration->scope->declarations.empty());
	SYMBOLS_ASSERT(!declaration->scope->declarationList.empty());

	SYMBOLS_ASSERT(declaration == declaration->scope->declarationList.back());
	declaration->scope->declarationList.pop_back(); // TODO: optimise

	Scope::Declarations::iterator i = findDeclaration(declaration->scope->declarations, p);
	SYMBOLS_ASSERT(i != declaration->scope->declarations.end());
	declaration->scope->declarations.erase(i);

}

inline BacktrackCallback makeUndeclareCallback(const DeclarationInstance* p)
{
	BacktrackCallback result = { BacktrackCallbackThunk<const DeclarationInstance, undeclare>::thunk, const_cast<DeclarationInstance*>(p) };
	return result;
}

static void backtrackError(void* data, LexerAllocator& allocator)
{
	throw SymbolsError(); // cannot backtrack before this point!
}

inline BacktrackCallback makeBacktrackErrorCallback()
{
	BacktrackCallback result = { backtrackError, 0 };
	return result;
}

inline const Scope& nullScope()
{
	static Scope null(TREEALLOCATOR_NULL, IDENTIFIER_NULL);
	return null;
}
#define SCOPE_NULL nullScope()

inline bool enclosesEts(ScopeType type)
{
	return type == SCOPETYPE_NAMESPACE
		|| type == SCOPETYPE_LOCAL;
}

inline Scope* getEnclosingClass(Scope* scope)
{
	for(; scope != 0; scope = scope->parent)
	{
		if(scope->type == SCOPETYPE_CLASS
			&& *scope->name.value.c_str() != '$') // ignore anonymous union
		{
			return scope;
		}
	}
	return 0;
}


// ----------------------------------------------------------------------------
// meta types
extern Declaration gArithmetic;
extern Declaration gSpecial;
extern Declaration gClass;
extern Declaration gEnum;

extern Declaration gNamespace;
extern Declaration gCtor;
extern Declaration gEnumerator;
extern Declaration gUnknown;


inline bool isTemplate(const Scope& scope)
{
	return scope.templateDepth != 0;
}


inline bool isType(const Declaration& type)
{
	return type.specifiers.isTypedef
		|| type.type.declaration == &gArithmetic
		|| type.type.declaration == &gSpecial
		|| type.type.declaration == &gEnum
		|| type.type.declaration == &gClass;
}

inline bool isFunction(const Declaration& declaration)
{
	return declaration.enclosed != 0 && declaration.enclosed->type == SCOPETYPE_PROTOTYPE;
}

inline bool isMember(const Declaration& declaration)
{
	return declaration.scope != 0 && declaration.scope->type == SCOPETYPE_CLASS;
}

inline bool isMemberOfTemplate(const Declaration& declaration)
{
	return isMember(declaration) && isTemplate(*declaration.scope);
}

inline bool isNonMember(const Declaration& declaration)
{
	return !isMember(declaration);
}

inline bool isMemberObject(const Declaration& declaration)
{
	return isMember(declaration)
		&& !isFunction(declaration);
}

inline bool isMemberFunction(const Declaration& declaration)
{
	return isMember(declaration)
		&& isFunction(declaration);
}

inline bool isStatic(const Declaration& declaration)
{
	return declaration.specifiers.isStatic;
}

inline bool isStaticMember(const Declaration& declaration)
{
	return isMemberObject(declaration)
		&& isStatic(declaration);
}

inline bool isTypedef(const Declaration& declaration)
{
	return declaration.specifiers.isTypedef;
}

inline bool isClassKey(const Declaration& declaration)
{
	return &declaration == &gClass;
}

inline bool isClass(const Declaration& declaration)
{
	return declaration.type.declaration == &gClass;
}

inline bool isEnum(const Declaration& declaration)
{
	return declaration.type.declaration == &gEnum;
}

inline bool isComplete(const Declaration& declaration)
{
	return declaration.enclosed != 0;
}

inline bool isIncomplete(const Declaration& declaration)
{
	return declaration.enclosed == 0;
}

inline bool isElaboratedType(const Declaration& declaration)
{
	return (isClass(declaration) || isEnum(declaration)) && isIncomplete(declaration);
}

inline bool isNamespace(const Declaration& declaration)
{
	return declaration.type.declaration == &gNamespace;
}

inline bool isObject(const Declaration& declaration)
{
	return !isType(declaration)
		&& !isNamespace(declaration);
}

inline bool isExtern(const Declaration& declaration)
{
	return declaration.specifiers.isExtern;
}

inline bool isSpecialization(const Declaration& declaration)
{
	return declaration.isSpecialization;
}

// ----------------------------------------------------------------------------



struct LookupFilter
{
	typedef bool (*Function)(void* context, const Declaration& declaration);
	Function function;
	void* context;

	bool operator()(const Declaration& declaration)
	{
		return function(context, declaration);
	}
};

inline bool isAny(const Declaration& declaration)
{
	// always ignore constructors during name-lookup
	return declaration.type.declaration != &gCtor;
}

template<bool filter(const Declaration& declaration)>
struct LookupFilterDefault : LookupFilter
{
	LookupFilterDefault()
	{
		LookupFilter::function = apply;
		LookupFilter::context = 0;
	}
	static bool apply(void*, const Declaration& declaration)
	{
		return filter(declaration);
	}
};

typedef LookupFilterDefault<isAny> IsAny;

template<typename T>
struct LookupFilterThunk
{
	static bool apply(void* context, const Declaration& declaration)
	{
		return (*static_cast<T*>(context))(declaration);
	}
};

template<typename T>
LookupFilter makeLookupFilter(T& filter)
{
	LookupFilter result = { LookupFilterThunk<T>::apply, &filter };
	return result;
}


inline bool isTypeName(const Declaration& declaration)
{
	return isType(declaration);
}

typedef LookupFilterDefault<isTypeName> IsTypeName;

inline bool isNamespaceName(const Declaration& declaration)
{
	return isNamespace(declaration);
}

typedef LookupFilterDefault<isNamespaceName> IsNamespaceName;


inline bool isTemplateName(const Declaration& declaration)
{
	// returns true if \p declaration is a template class, function or template-parameter
	return declaration.isTemplate && (isClass(declaration) || isFunction(declaration) || isTypedef(declaration));
}

inline bool isNestedName(const Declaration& declaration)
{
	return isTypeName(declaration)
		|| isNamespaceName(declaration);
}

typedef LookupFilterDefault<isNestedName> IsNestedName;


inline bool isNonMemberName(const Declaration& declaration)
{
	return isNonMember(declaration);
}

typedef LookupFilterDefault<isNonMemberName> IsNonMemberName;


// ----------------------------------------------------------------------------
// unique types
// Representation of a declarator, with type-elements linked in 'normal' order.
// e.g. int(*)[] == pointer to array of == DeclaratorPointerType -> DeclaratorArrayType
// Note that this is the reverse of the order that the declarator is parsed in.
// This means a given unique type sub-sequence need only be stored once.
// This allows fast comparison of types and simplifies printing of declarators.

struct TypeElementVisitor
{
	virtual void visit(const struct DependentType&) = 0;
	virtual void visit(const struct DependentTypename&) = 0;
	virtual void visit(const struct DependentNonType&) = 0;
	virtual void visit(const struct TemplateTemplateArgument&) = 0;
	virtual void visit(const struct NonType&) = 0;
	virtual void visit(const struct ObjectType&) = 0;
	virtual void visit(const struct PointerType&) = 0;
	virtual void visit(const struct ReferenceType&) = 0;
	virtual void visit(const struct ArrayType&) = 0;
	virtual void visit(const struct MemberPointerType&) = 0;
	virtual void visit(const struct FunctionType&) = 0;
};

struct TypeElement
{
	UniqueType next;

	TypeElement()
	{
	}
	virtual ~TypeElement()
	{
	}
	virtual void accept(TypeElementVisitor& visitor) const = 0;
	virtual bool operator<(const TypeElement& other) const = 0;
};

struct TypeElementEmpty : TypeElement
{
	TypeElementEmpty()
	{
		next = 0;
	}
	virtual void accept(TypeElementVisitor& visitor) const
	{
		throw SymbolsError();
	}
	virtual bool operator<(const TypeElement& other) const
	{
		throw SymbolsError();
	}
};

extern const TypeElementEmpty gTypeElementEmpty;


template<typename T>
struct TypeElementGeneric : TypeElement
{
	T value;
	TypeElementGeneric(const T& value)
		: value(value)
	{
	}
	void accept(TypeElementVisitor& visitor) const
	{
		visitor.visit(value);
	}
	bool operator<(const TypeElementGeneric& other) const
	{
		return value < other.value;
	}
	bool operator<(const TypeElement& other) const
	{
		return next != other.next
			? next < other.next
			: abstractLess(*this, other);
	}
};

const UniqueType UNIQUETYPE_NULL = &gTypeElementEmpty;


typedef IndirectSet<UniqueType> UniqueTypes;

extern UniqueTypes gBuiltInTypes;

template<typename T>
inline UniqueType pushBuiltInType(UniqueType type, const T& value)
{
	TypeElementGeneric<T> node(value);
	node.next = type;
	return *gBuiltInTypes.insert(node);
}

template<typename T>
inline UniqueType pushUniqueType(UniqueTypes& types, UniqueType type, const T& value)
{
	TypeElementGeneric<T> node(value);
	node.next = type;
	{
		UniqueTypes::iterator i = gBuiltInTypes.find(node);
		if(i != gBuiltInTypes.end())
		{
			return *i;
		}
	}
	return *types.insert(node);
}

extern UniqueTypes gUniqueTypes;

template<typename T>
inline void pushUniqueType(UniqueType& type, const T& value)
{
	type = pushUniqueType(gUniqueTypes, type, value);
}

inline void popUniqueType(UniqueType& type)
{
	SYMBOLS_ASSERT(type.getBits() != 0);
	type = type->next;
}

struct UniqueTypeWrapper
{
	UniqueType value;

	UniqueTypeWrapper()
		: value(&gTypeElementEmpty)
	{
	}
	explicit UniqueTypeWrapper(UniqueType value)
		: value(value)
	{
	}
	template<typename T>
	void push_front(const T& t)
	{
		pushUniqueType(value, t);
	}
	void pop_front()
	{
		SYMBOLS_ASSERT(value != 0);
		SYMBOLS_ASSERT(value != UNIQUETYPE_NULL);
		popUniqueType(value);
	}
	void swap(UniqueTypeWrapper& other)
	{
		std::swap(value, other.value);
	}
	bool empty() const
	{
		return value == UNIQUETYPE_NULL;
	}
	bool isSimple() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<ObjectType>);
	}
	bool isPointer() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<PointerType>);
	}
	bool isReference() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<ReferenceType>);
	}
	bool isArray() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<ArrayType>);
	}
	bool isMemberPointer() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<MemberPointerType>);
	}
	bool isFunction() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<FunctionType>);
	}
	bool isDependent() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DependentType>)
			|| typeid(*value) == typeid(TypeElementGeneric<DependentTypename>)
			|| typeid(*value) == typeid(TypeElementGeneric<DependentNonType>);
	}
	bool isNonType() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<NonType>);
	}
	bool isTemplateTemplateArgument() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<TemplateTemplateArgument>);
	}
	bool isSimplePointer() const
	{
		return isPointer()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
	bool isSimpleReference() const
	{
		return isReference()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
	bool isSimpleArray() const
	{
		return isArray()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
	bool isFunctionPointer() const
	{
		return isPointer()
			&& UniqueTypeWrapper(value->next).isFunction();
	}
};

inline bool operator==(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value == r.value;
}

inline bool operator!=(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return !operator==(l, r);
}

inline bool operator<(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value < r.value;
}

// ----------------------------------------------------------------------------
struct Parameter
{
	DeclarationPtr declaration;
	cpp::default_argument* argument;
	Parameter(Declaration* declaration, cpp::default_argument* argument)
		: declaration(declaration), argument(argument)
	{
	}
};

struct Parameters : std::vector<Parameter>
{
	bool isEllipsis;
	Parameters() : isEllipsis(false)
	{
	}
};


// ----------------------------------------------------------------------------

typedef UniqueTypeWrapper UniqueTypeId;

const UniqueTypeId gUniqueTypeNull = UniqueTypeId(UNIQUETYPE_NULL);

inline bool isEqual(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return l.value == r.value;
}

inline UniqueType getInner(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) != typeid(TypeElementGeneric<struct ObjectType>));
	return type->next;
}

inline bool isEqualInner(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return getInner(l.value) == getInner(r.value);
}

inline bool isSameType(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return typeid(*l.value) == typeid(*r.value);
}


struct NonType : IntegralConstant
{
	explicit NonType(IntegralConstant value)
		: IntegralConstant(value)
	{
	}
};

inline bool operator<(const NonType& left, const NonType& right)
{
	return left.value < right.value;
}

inline IntegralConstant getNonTypeValue(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<NonType>));
	return static_cast<const TypeElementGeneric<NonType>*>(type.getPointer())->value;
}


typedef std::vector<UniqueTypeWrapper> UniqueTypeArray;
typedef UniqueTypeArray TemplateArgumentsInstance;
typedef UniqueTypeArray InstantiatedTypes;
typedef std::vector<const struct TypeInstance*> UniqueBases;

struct ChildInstantiation
{
	const struct TypeInstance* instance;
	Location source;
	ChildInstantiation(const struct TypeInstance* instance, Location source)
		: instance(instance), source(source)
	{
	}
};
typedef std::vector<ChildInstantiation> ChildInstantiations;


typedef std::vector<Location> InstanceLocations; // temporary scaffolding!


struct TypeInstance
{
	std::size_t uniqueId;
	DeclarationPtr primary;
	DeclarationPtr declaration; // don't compare declaration pointer - it will change on instantiation if an explicit/partial specialization is chosen
	TemplateArgumentsInstance templateArguments;
	TemplateArgumentsInstance deducedArguments; // the deduced arguments for the partial-specialization's template-parameters
	const TypeInstance* enclosing; // the enclosing template
	UniqueBases bases;
	size_t size;
	InstantiatedTypes children; // the dependent types in the specialization
	InstanceLocations childLocations; // temporary scaffolding!
	bool instantiated;
	bool instantiating;
	bool allowLookup;
	mutable bool visited; // used during findDeclaration to prevent infinite recursion
	mutable bool dumped; // used during dumpTemplateInstantiations to prevent duplicates
	Location instantiation;
	ChildInstantiations childInstantiations;

	TypeInstance(Declaration* declaration, const TypeInstance* enclosing)
		: uniqueId(0), primary(declaration), declaration(declaration), enclosing(enclosing), size(0), instantiated(false), instantiating(false), allowLookup(false), visited(false), dumped(false)
	{
		SYMBOLS_ASSERT(enclosing == 0 || isClass(*enclosing->declaration));
	}
};

inline bool operator==(const TypeInstance& left, const TypeInstance& right)
{
	return left.primary.p == right.primary.p
		&& left.enclosing == right.enclosing
		&& left.templateArguments == right.templateArguments;
}

inline bool operator<(const TypeInstance& left, const TypeInstance& right)
{
	return left.primary.p != right.primary.p ? left.primary.p < right.primary.p
		: left.enclosing != right.enclosing ? left.enclosing < right.enclosing
		: left.templateArguments != right.templateArguments ? left.templateArguments < right.templateArguments
		: false;
}

struct ObjectType
{
	TypeInstance type;
	ObjectType(const TypeInstance& type)
		: type(type)
	{
	}
};

inline bool operator<(const ObjectType& left, const ObjectType& right)
{
	return left.type < right.type;
}

inline const TypeInstance& getObjectType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<ObjectType>));
	return static_cast<const TypeElementGeneric<ObjectType>*>(type.getPointer())->value.type;
}


struct TemplateTemplateArgument
{
	DeclarationPtr declaration; // the primary declaration of the template template argument
	const TypeInstance* enclosing;
	TemplateTemplateArgument(Declaration* declaration, const TypeInstance* enclosing)
		: declaration(declaration), enclosing(enclosing)
	{
	}
};

inline bool operator<(const TemplateTemplateArgument& left, const TemplateTemplateArgument& right)
{
	return left.declaration.p < right.declaration.p;
}

inline const TemplateTemplateArgument& getTemplateTemplateArgument(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<TemplateTemplateArgument>));
	return static_cast<const TypeElementGeneric<TemplateTemplateArgument>*>(type.getPointer())->value;
}

// represents a template-parameter (or template-template-parameter)
// T
// TT
// TT<> (TODO)
// TT<T>
struct DependentType
{
	DeclarationPtr type; // the declaration of the template parameter
	TemplateArgumentsInstance templateArguments;
	std::size_t templateParameterCount;
	DependentType(Declaration* type, TemplateArgumentsInstance templateArguments = TemplateArgumentsInstance(), std::size_t templateParameterCount = 0)
		: type(type), templateArguments(templateArguments), templateParameterCount(templateParameterCount)
	{
		SYMBOLS_ASSERT(type->templateParameter != INDEX_INVALID);
	}
};

inline bool operator<(const DependentType& left, const DependentType& right)
{
	return left.templateParameterCount != right.templateParameterCount
		? left.templateParameterCount < right.templateParameterCount
		: left.type->scope->templateDepth != right.type->scope->templateDepth
			? left.type->scope->templateDepth < right.type->scope->templateDepth
			: left.type->templateParameter != right.type->templateParameter
				? left.type->templateParameter < right.type->templateParameter
				: left.templateArguments < right.templateArguments;
}


struct DependentTypename
{
	Name name; // the type name
	UniqueTypeWrapper qualifying; // the qualifying type
	TemplateArgumentsInstance templateArguments;
	bool isNested; // T::dependent::
	bool isTemplate; // T::template dependent<>
	DependentTypename(Name name, UniqueTypeWrapper qualifying, TemplateArgumentsInstance templateArguments, bool isNested, bool isTemplate)
		: name(name), qualifying(qualifying), templateArguments(templateArguments), isNested(isNested), isTemplate(isTemplate)
	{
	}
};

inline bool operator<(const DependentTypename& left, const DependentTypename& right)
{
	return left.name != right.name
		? left.name < right.name
		: left.qualifying != right.qualifying
			? left.qualifying < right.qualifying
			: left.isNested != right.isNested
				? left.isNested < right.isNested
				: left.templateArguments < right.templateArguments;
}


struct DependentNonType
{
	UniqueExpression expression;
	DependentNonType(UniqueExpression expression)
		: expression(expression)
	{
	}
};

inline bool operator<(const DependentNonType& left, const DependentNonType& right)
{
	return left.expression < right.expression;
}


struct DeclaratorPointerType
{
	CvQualifiers qualifiers;
	DeclaratorPointerType()
	{
	}
	explicit DeclaratorPointerType(CvQualifiers qualifiers)
		: qualifiers(qualifiers)
	{
	}
};

struct PointerType
{
	PointerType()
	{
	}
};

inline bool operator<(const PointerType& left, const PointerType& right)
{
	return false;
}

struct DeclaratorReferenceType
{
};

struct ReferenceType
{
};

inline bool operator<(const ReferenceType& left, const ReferenceType& right)
{
	return false;
}

struct DeclaratorMemberPointerType
{
	Type type;
	CvQualifiers qualifiers;
	DeclaratorMemberPointerType(const Type& type, CvQualifiers qualifiers)
		: type(type), qualifiers(qualifiers)
	{
	}
};

struct MemberPointerType
{
	UniqueTypeWrapper type;
	MemberPointerType(UniqueTypeWrapper type)
		: type(type)
	{
	}
};

inline const MemberPointerType& getMemberPointerType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<MemberPointerType>));
	return static_cast<const TypeElementGeneric<MemberPointerType>*>(type.getPointer())->value;
}

inline const TypeInstance& getMemberPointerClass(UniqueType type)
{
	return getObjectType(getMemberPointerType(type).type.value);
}

inline bool operator<(const MemberPointerType& left, const MemberPointerType& right)
{
	return left.type < right.type;
}

typedef std::vector<ExpressionWrapper> ArrayRank;

struct DeclaratorArrayType
{
	ArrayRank rank; // store expressions to be evaluated when template-params are known
	DeclaratorArrayType(const ArrayRank& rank)
		: rank(rank)
	{
	}
};

struct ArrayType
{
	std::size_t size;
	ArrayType(std::size_t size)
		: size(size)
	{
	}
};

inline bool operator<(const ArrayType& left, const ArrayType& right)
{
	return left.size < right.size;
}

inline const ArrayType& getArrayType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<ArrayType>));
	return static_cast<const TypeElementGeneric<ArrayType>*>(type.getPointer())->value;
}

struct DeclaratorFunctionType
{
	Parameters parameters;
	CvQualifiers qualifiers;
	DeclaratorFunctionType(const Parameters& parameters, CvQualifiers qualifiers)
		: parameters(parameters), qualifiers(qualifiers)
	{
	}
};

typedef UniqueTypeArray ParameterTypes;

struct FunctionType
{
	ParameterTypes parameterTypes;
	bool isEllipsis;
	FunctionType()
		: isEllipsis(false)
	{
	}
};

inline bool operator<(const FunctionType& left, const FunctionType& right)
{
	return left.isEllipsis != right.isEllipsis ? left.isEllipsis < right.isEllipsis
		: left.parameterTypes < right.parameterTypes;
}

inline const TypeSequence::Node* getLastNode(const TypeSequence& typeSequence)
{
	const TypeSequence::Node* result = 0;
	for(const TypeSequence::Node* node = typeSequence.get(); node != 0; node = node->get())
	{
		result = node;
	}
	return result;
}
inline const Parameters& getParameters(const TypeId& type)
{
	const TypeSequence::Node* node = getLastNode(type.typeSequence);
	SYMBOLS_ASSERT(node != 0);
	SYMBOLS_ASSERT(typeid(*node) == typeid(SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>));
	return static_cast<const SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>*>(node)->value.parameters;
}

inline const ParameterTypes& getParameterTypes(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<FunctionType>));
	return static_cast<const TypeElementGeneric<FunctionType>*>(type.getPointer())->value.parameterTypes;
}


// ----------------------------------------------------------------------------


inline UniqueTypeWrapper adjustFunctionParameter(UniqueTypeWrapper type)
{
	UniqueTypeWrapper result(type.value.getPointer());  // ignore cv-qualifiers
	if(type.isFunction()) // T() becomes T(*)()
	{
		pushUniqueType(result.value, PointerType());
	}
	else if(type.isArray()) // T[] becomes T*
	{
		popUniqueType(result.value);
		pushUniqueType(result.value, PointerType());
	}
	return result;
}

inline bool isFunctionParameterEquivalent(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return adjustFunctionParameter(left) == adjustFunctionParameter(right);
}

inline bool isEquivalent(const ParameterTypes& left, const ParameterTypes& right)
{
	ParameterTypes::const_iterator l = left.begin();
	ParameterTypes::const_iterator r = right.begin();
	for(;; ++l, ++r)
	{
		if(l == left.end())
		{
			return r == right.end();
		}
		if(r == right.end())
		{
			return false;
		}
		if(!isFunctionParameterEquivalent(*l, *r))
		{
			return false;
		}
	}
	return true;
}


// ----------------------------------------------------------------------------

struct ObjectTypeId : UniqueTypeId
{
	ObjectTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
	{
		value = pushBuiltInType(value, ObjectType(TypeInstance(declaration, 0)));
		declaration->type.unique = value;
		declaration->isComplete = true;
	}
};

// ----------------------------------------------------------------------------

inline Declaration* findPrimaryTemplate(Declaration* declaration)
{
	SYMBOLS_ASSERT(declaration->isTemplate);
	for(;declaration != 0; declaration = declaration->overloaded)
	{
		if(!isSpecialization(*declaration))
		{
			SYMBOLS_ASSERT(declaration->isTemplate);
			return declaration;
		}
	}
	SYMBOLS_ASSERT(false); // primary template not declared!
	return 0;
}

inline const DeclarationInstance& findLastDeclaration(const DeclarationInstance& instance, Declaration* declaration)
{
	for(const DeclarationInstance* p = &instance; p != 0; p = p->overloaded)
	{
		if(*p == declaration)
		{
			return *p;
		}
	}
	throw SymbolsError();
}


inline const DeclarationInstance& findPrimaryTemplateLastDeclaration(const DeclarationInstance& instance)
{
	return findLastDeclaration(instance, findPrimaryTemplate(instance));
}


// ----------------------------------------------------------------------------

struct LookupResult
{
	const DeclarationInstance* filtered; // the declaration found by the name-lookup, using the filter
	const TypeInstance* enclosing;

	LookupResult()
		: filtered(0), enclosing(0)
	{
	}
	operator const DeclarationInstance*() const
	{
		return filtered;
	}

	// Combines the result of a subsequent lookup, returns true if lookup succeeded
	bool append(const LookupResult& other)
	{
		*this = other;
		return filtered != 0;
	}
};

struct DeclarationInstanceRef
{
	const DeclarationInstance* p;
	DeclarationInstanceRef()
		: p(0)
	{
	}
	DeclarationInstanceRef(const DeclarationInstance& p)
		: p(&p)
	{
		checkAllocated(this->p);
	}
	Declaration& operator*() const
	{
		checkAllocated(p);
		return p->operator*();
	}
	Declaration* operator->() const
	{
		checkAllocated(p);
		return p->operator->();
	}
	operator const DeclarationInstance&() const
	{
		checkAllocated(p);
		return *p;
	}
	operator Declaration*() const
	{
		checkAllocated(p);
		return p == 0 ? 0 : static_cast<Declaration*>(*p);
	}
};

struct LookupResultRef : DeclarationInstanceRef
{
	const TypeInstance* enclosing;
	LookupResultRef()
		: enclosing(0)
	{
	}
	LookupResultRef(const DeclarationInstance& p)
		: DeclarationInstanceRef(p), enclosing(0)
	{
	}
	LookupResultRef(const LookupResult& result)
		: DeclarationInstanceRef(*result.filtered), enclosing(result.enclosing)
	{
	}
};


inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = IsAny(), std::size_t visibility = UINT_MAX);

struct RecursionGuard
{
	const TypeInstance& instance;
	RecursionGuard(const TypeInstance& instance)
		: instance(instance)
	{
		SYMBOLS_ASSERT(!instance.visited);
		instance.visited = true;
	}
	~RecursionGuard()
	{
		instance.visited = false;
	}
};

inline void printType(const TypeInstance& type, std::ostream& out = std::cout, bool escape = false);

inline LookupResult findDeclaration(const TypeInstance& instance, const Identifier& id, LookupFilter filter, std::size_t visibility = UINT_MAX)
{
	SYMBOLS_ASSERT(instance.declaration->enclosed != 0);
	SYMBOLS_ASSERT(instance.instantiated); // the qualifying type should have been instantiated by this point
	SYMBOLS_ASSERT(instance.allowLookup);

	LookupResult result;

	if(instance.visited) // TODO: this occurs when the same type is found twice when searching bases: can be caused by two templates that differ only in non-type template arguments
	{
		std::cout << "duplicate base: ";
		printType(instance);
		std::cout << std::endl;
		return result;
	}
	RecursionGuard guard(instance);

	result.filtered = findDeclaration(instance.declaration->enclosed->declarations, id, filter, visibility);
	if(result.filtered)
	{
		result.enclosing = &instance;
		return result;
	}
	for(UniqueBases::const_iterator i = instance.bases.begin(); i != instance.bases.end(); ++i)
	{
		const TypeInstance& base = *(*i);
		SYMBOLS_ASSERT(base.declaration->enclosed != 0); // TODO: non-fatal error: incomplete type
		SYMBOLS_ASSERT(base.declaration->enclosed->usingDirectives.empty()); // namespace.udir: A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.

		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->getName().value == id.value
			&& filter(*base.declaration))
		{
			result.filtered = &getDeclaration(base.declaration->getName());
			result.enclosing = base.enclosing;
			return result;
		}

		if(result.append(findDeclaration(base, id, filter, visibility)))
		{
			return result;
		}
	}
	return result;
}

// ----------------------------------------------------------------------------
inline void printType(UniqueTypeWrapper type, std::ostream& out = std::cout, bool escape = false);

struct TypeError
{
	virtual void report() = 0;
};


struct TypeErrorBase : TypeError
{
	Location source;
	TypeErrorBase(Location source) : source(source)
	{
	}
	void report()
	{
		printPosition(source);
	}
};

struct MemberNotFoundError : TypeErrorBase
{
	Name name;
	UniqueTypeWrapper qualifying;
	MemberNotFoundError(Location source, Name name, UniqueTypeWrapper qualifying)
		: TypeErrorBase(source), name(name), qualifying(qualifying)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "member '" << name.c_str() << "' not found in ";
		printType(qualifying);
		std::cout << std::endl;
	}
};

struct MemberIsNotTypeError : TypeErrorBase
{
	Name name;
	UniqueTypeWrapper qualifying;
	MemberIsNotTypeError(Location source, Name name, UniqueTypeWrapper qualifying)
		: TypeErrorBase(source), name(name), qualifying(qualifying)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "member '" << name.c_str() << "' is not a type in ";
		printType(qualifying);
		std::cout << std::endl;
	}
};

struct ExpectedTemplateTemplateArgumentError : TypeErrorBase
{
	UniqueTypeWrapper type;
	ExpectedTemplateTemplateArgumentError(Location source, UniqueTypeWrapper type)
		: TypeErrorBase(source), type(type)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "expected template template argument: ";
		printType(type);
		std::cout << std::endl;
	}
};

struct MismatchedTemplateTemplateArgumentError : TypeErrorBase
{
	UniqueTypeWrapper type;
	MismatchedTemplateTemplateArgumentError(Location source, UniqueTypeWrapper type)
		: TypeErrorBase(source), type(type)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "mismatched template template argument: ";
		printType(type);
		std::cout << std::endl;
	}
};

struct QualifyingIsNotClassError : TypeErrorBase
{
	UniqueTypeWrapper qualifying;
	QualifyingIsNotClassError(Location source, UniqueTypeWrapper qualifying)
		: TypeErrorBase(source), qualifying(qualifying)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "qualifying type is not a class: ";
		printType(qualifying);
		std::cout << std::endl;
	}
};

struct PointerToReferenceError : TypeErrorBase
{
	PointerToReferenceError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "cannot create pointer to reference" << std::endl;
	}
};

struct ReferenceToReferenceError : TypeErrorBase
{
	ReferenceToReferenceError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "cannot create reference to reference" << std::endl;
	}
};

struct InvalidArrayError : TypeErrorBase
{
	InvalidArrayError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "cannot create array of type void, function or reference" << std::endl;
	}
};

struct VoidParameterError : TypeErrorBase
{
	VoidParameterError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "cannot create function with void parameter" << std::endl;
	}
};

struct TooFewTemplateArgumentsError : TypeErrorBase
{
	TooFewTemplateArgumentsError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "too few template arguments" << std::endl;
	}
};

// ----------------------------------------------------------------------------
// expression evaluation

typedef UniqueTypeWrapper (*UnaryTypeOp)(UniqueTypeWrapper);
typedef UniqueTypeWrapper (*BinaryTypeOp)(UniqueTypeWrapper, UniqueTypeWrapper);
typedef UniqueTypeWrapper (*TernaryTypeOp)(UniqueTypeWrapper, UniqueTypeWrapper, UniqueTypeWrapper);

struct IntegralConstantExpression
{
	UniqueTypeWrapper type;
	IntegralConstant value;
	IntegralConstantExpression(UniqueTypeWrapper type, IntegralConstant value)
		: type(type), value(value)
	{
	}
};

inline bool operator<(const IntegralConstantExpression& left, const IntegralConstantExpression& right)
{
	return left.type != right.type
		? left.type < right.type
		: left.value.value < right.value.value;
}

struct DependentIdExpression
{
	Name name;
	UniqueTypeWrapper qualifying;
	DependentIdExpression(Name name, UniqueTypeWrapper qualifying)
		: name(name), qualifying(qualifying)
	{
		SYMBOLS_ASSERT(qualifying.value.p != 0);
	}
};

inline bool operator<(const DependentIdExpression& left, const DependentIdExpression& right)
{
	return left.name != right.name
		? left.name < right.name
		: left.qualifying < right.qualifying;
}

inline bool isDependentIdExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<DependentIdExpression>);
}

inline const DependentIdExpression& getDependentIdExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isDependentIdExpression(node));
	return static_cast<const ExpressionNodeGeneric<DependentIdExpression>*>(node)->value;
}


struct IdExpression
{
	DeclarationPtr declaration;
	const TypeInstance* enclosing;
	IdExpression(DeclarationPtr declaration, const TypeInstance* enclosing)
		: declaration(declaration), enclosing(enclosing)
	{
	}
};

inline bool operator<(const IdExpression& left, const IdExpression& right)
{
	return left.declaration.p != right.declaration.p
		? left.declaration.p < right.declaration.p
		: left.enclosing < right.enclosing;
}

inline bool isIdExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<IdExpression>);
}

inline const IdExpression& getIdExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isIdExpression(node));
	return static_cast<const ExpressionNodeGeneric<IdExpression>*>(node)->value;
}


struct NonTypeTemplateParameter
{
	DeclarationPtr declaration;
	NonTypeTemplateParameter(DeclarationPtr declaration)
		: declaration(declaration)
	{
	}
};

inline bool operator<(const NonTypeTemplateParameter& left, const NonTypeTemplateParameter& right)
{
	return left.declaration->scope->templateDepth != right.declaration->scope->templateDepth
		? left.declaration->scope->templateDepth < right.declaration->scope->templateDepth
		: left.declaration->templateParameter < right.declaration->templateParameter;
}

inline bool isNonTypeTemplateParameter(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<NonTypeTemplateParameter>);
}

inline const NonTypeTemplateParameter& getNonTypeTemplateParameter(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isNonTypeTemplateParameter(node));
	return static_cast<const ExpressionNodeGeneric<NonTypeTemplateParameter>*>(node)->value;
}

struct SizeofExpression
{
	// [expr.sizeof] The operand is ... an expression, which is not evaluated
	ExpressionPtr operand;
	SizeofExpression(ExpressionPtr operand)
		: operand(operand)
	{
	}
};

inline bool operator<(const SizeofExpression& left, const SizeofExpression& right)
{
	return left.operand.p < right.operand.p;
}

struct SizeofTypeExpression
{
	// [expr.sizeof] The operand is ... a parenthesized type-id
	UniqueTypeWrapper type;
	SizeofTypeExpression(UniqueTypeWrapper type)
		: type(type)
	{
	}
};

inline bool operator<(const SizeofTypeExpression& left, const SizeofTypeExpression& right)
{
	return left.type < right.type;
}


struct UnaryExpression
{
	Name operatorName;
	UnaryIceOp operation;
	UnaryTypeOp type;
	ExpressionPtr first;
	UnaryExpression(Name operatorName, UnaryIceOp operation, UnaryTypeOp type, ExpressionPtr first)
		: operatorName(operatorName), operation(operation), type(type), first(first)
	{
	}
};

inline bool operator<(const UnaryExpression& left, const UnaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.type != right.type
			? left.type < right.type
			: left.first.p < right.first.p;
}

inline bool isUnaryExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<UnaryExpression>);
}

inline const UnaryExpression& getUnaryExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isUnaryExpression(node));
	return static_cast<const ExpressionNodeGeneric<UnaryExpression>*>(node)->value;
}


struct BinaryExpression
{
	Name operatorName;
	BinaryIceOp operation;
	BinaryTypeOp type;
	ExpressionPtr first;
	ExpressionPtr second;
	BinaryExpression(Name operatorName, BinaryIceOp operation, BinaryTypeOp type, ExpressionPtr first, ExpressionPtr second)
		: operatorName(operatorName), operation(operation), type(type), first(first), second(second)
	{
	}
};

inline bool operator<(const BinaryExpression& left, const BinaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.type != right.type
			? left.type < right.type
			: left.first.p != right.first.p
				? left.first.p < right.first.p
				: left.second.p < right.second.p;
}

struct TernaryExpression
{
	TernaryIceOp operation;
	TernaryTypeOp type;
	ExpressionPtr first;
	ExpressionPtr second;
	ExpressionPtr third;
	TernaryExpression(TernaryIceOp operation, TernaryTypeOp type, ExpressionPtr first, ExpressionPtr second, ExpressionPtr third)
		: operation(operation), type(type), first(first), second(second), third(third)
	{
	}
};

inline bool operator<(const TernaryExpression& left, const TernaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.type != right.type
			? left.type < right.type
			: left.first.p != right.first.p
				? left.first.p < right.first.p
				: left.second.p != right.second.p
					? left.second.p < right.second.p
					: left.third.p < right.third.p;
}

typedef bool (*UnaryTypeTraitsOp)(UniqueTypeWrapper);
typedef bool (*BinaryTypeTraitsOp)(UniqueTypeWrapper, UniqueTypeWrapper, Location source, const TypeInstance* enclosing);

struct TypeTraitsUnaryExpression
{
	Name traitName;
	UnaryTypeTraitsOp operation;
	UniqueTypeWrapper type;
	TypeTraitsUnaryExpression(Name traitName, UnaryTypeTraitsOp operation, UniqueTypeWrapper type)
		: traitName(traitName), operation(operation), type(type)
	{
	}
};

inline bool operator<(const TypeTraitsUnaryExpression& left, const TypeTraitsUnaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
			: left.type < right.type;
}

struct TypeTraitsBinaryExpression
{
	Name traitName;
	BinaryTypeTraitsOp operation;
	UniqueTypeWrapper first;
	UniqueTypeWrapper second;
	TypeTraitsBinaryExpression(Name traitName, BinaryTypeTraitsOp operation, UniqueTypeWrapper first, UniqueTypeWrapper second)
		: traitName(traitName), operation(operation), first(first), second(second)
	{
		SYMBOLS_ASSERT(first != gUniqueTypeNull);
		SYMBOLS_ASSERT(second != gUniqueTypeNull);
	}
};

inline bool operator<(const TypeTraitsBinaryExpression& left, const TypeTraitsBinaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.first != right.first
			? left.first < right.first
			: left.second < right.second;
}

inline UniqueTypeWrapper typeofExpression(ExpressionNode* node, Location source);

inline bool isPointerToMemberExpression(ExpressionNode* expression)
{
	return isUnaryExpression(expression)
		&& getUnaryExpression(expression).operation == addressOf
		&& isIdExpression(getUnaryExpression(expression).first);
}

inline bool isPointerToFunctionExpression(ExpressionNode* expression)
{
	if(isUnaryExpression(expression))
	{
		expression = getUnaryExpression(expression).first;
	}
	if(!isIdExpression(expression))
	{
		return false;
	}
	const IdExpression node = getIdExpression(expression);
	if(node.declaration->type.declaration == &gEnumerator)
	{
		return false;
	}
	return UniqueTypeWrapper(node.declaration->type.unique).isFunction();
}

inline bool isDependentPointerToMemberExpression(ExpressionNode* expression)
{
	return isUnaryExpression(expression)
		&& getUnaryExpression(expression).operation == addressOf
		&& isDependentIdExpression(getUnaryExpression(expression).first);
}

extern ObjectTypeId gUnsignedInt;
extern ObjectTypeId gBool;

struct TypeofVisitor : ExpressionNodeVisitor
{
	UniqueTypeWrapper result;
	Location source;
	explicit TypeofVisitor(Location source)
		: source(source)
	{
	}
	void visit(const IntegralConstantExpression& literal)
	{
		result = literal.type;
	}
	void visit(const NonTypeTemplateParameter& node)
	{
		// TODO: evaluate non-type template parameter
	}
	void visit(const DependentIdExpression& node)
	{
		// TODO: name lookup
	}
	void visit(const IdExpression& node)
	{
		// TODO: evaluate dependent type
		result = UniqueTypeWrapper(node.declaration->type.unique);
	}
	void visit(const SizeofExpression& node)
	{
		result = gUnsignedInt;
	}
	void visit(const SizeofTypeExpression& node)
	{
		result = gUnsignedInt;
	}
	void visit(const UnaryExpression& node)
	{
		result = node.type(typeofExpression(node.first, source));
	}
	void visit(const BinaryExpression& node)
	{
		result = node.type(typeofExpression(node.first, source), typeofExpression(node.second, source));
	}
	void visit(const TernaryExpression& node)
	{
		result = node.type(typeofExpression(node.first, source), typeofExpression(node.second, source), typeofExpression(node.third, source));
	}
	void visit(const TypeTraitsUnaryExpression& node)
	{
		result = gBool;
	}
	void visit(const TypeTraitsBinaryExpression& node)
	{
		result = gBool;
	}
};

inline UniqueTypeWrapper typeofExpression(ExpressionNode* node, Location source)
{
	TypeofVisitor visitor(source);
	node->accept(visitor);
	return visitor.result;
}



inline std::size_t instantiateClass(const TypeInstance& instanceConst, Location source, const TypeInstance* enclosing, bool allowDependent = false);
inline std::size_t requireCompleteObjectType(UniqueTypeWrapper type, Location source, const TypeInstance* enclosing);
inline UniqueTypeWrapper removeReference(UniqueTypeWrapper type);
inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const TypeInstance* enclosing, bool allowDependent = false);
inline const TypeInstance* findEnclosingType(const TypeInstance* enclosing, Scope* scope);
inline bool isDependent(const TypeInstance& type);
inline UniqueTypeWrapper substitute(UniqueTypeWrapper dependent, Location source, const TypeInstance& enclosingType);


inline const TypeInstance* findEnclosingTemplate(const TypeInstance* enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	SYMBOLS_ASSERT(scope->type == SCOPETYPE_TEMPLATE);
	for(const TypeInstance* i = enclosing; i != 0; i = (*i).enclosing)
	{
		if((*i).declaration->templateParamScope != 0
			&& (*i).declaration->templateParamScope->templateDepth == scope->templateDepth)
		{
			return i;
		}
	}
	return 0;
}

inline const TypeInstance* getEnclosingType(const TypeInstance* enclosing)
{
	for(const TypeInstance* i = enclosing; i != 0; i = (*i).enclosing)
	{
		if((*i).declaration->getName().value.c_str()[0] != '$') // ignore anonymous union
		{
			return i;
		}
	}
	return 0;
}


inline IntegralConstant evaluateIdExpression(const Declaration* declaration, Location source, const TypeInstance* enclosing)
{
	SYMBOLS_ASSERT(declaration->templateParameter == INDEX_INVALID);

	const TypeInstance* memberEnclosing = isMember(*declaration) // if the declaration is a class member
		? findEnclosingType(enclosing, declaration->scope) // it must be a member of (a base of) the qualifying class: find which one.
		: 0; // the declaration is not a class member and cannot be found through qualified name lookup

	return evaluate(declaration->initializer, source, memberEnclosing);
}

inline IntegralConstant evaluateIdExpression(const IdExpression& node, Location source, const TypeInstance* enclosing)
{
	if(node.enclosing != 0)
	{
		enclosing = node.enclosing;
	}
	return evaluateIdExpression(node.declaration, source, enclosing);
}

inline void addInstatiation(TypeInstance& enclosing, const TypeInstance& instance)
{
}

inline IntegralConstant evaluateIdExpression(const DependentIdExpression& node, Location source, const TypeInstance* enclosingType)
{
	SYMBOLS_ASSERT(node.qualifying != gUniqueTypeNull);
	SYMBOLS_ASSERT(enclosingType != 0);

	UniqueTypeWrapper substituted = substitute(node.qualifying, source, *enclosingType);
	const TypeInstance* qualifyingType = substituted.isSimple() ? &getObjectType(substituted.value) : 0;

	if(qualifyingType == 0
		|| !isClass(*qualifyingType->declaration))
	{
		throw QualifyingIsNotClassError(source, node.qualifying);
	}

	instantiateClass(*qualifyingType, source, enclosingType);
	Identifier id;
	id.value = node.name;
	std::size_t visibility = qualifyingType->instantiating ? enclosingType->instantiation.pointOfInstantiation : UINT_MAX;
	LookupResultRef declaration = findDeclaration(*qualifyingType, id, IsAny(), visibility);
	if(declaration == 0)
	{
		throw MemberNotFoundError(source, node.name, node.qualifying);
	}

	return evaluateIdExpression(declaration, source, qualifyingType);
}

struct EvaluateVisitor : ExpressionNodeVisitor
{
	IntegralConstant result;
	const TypeInstance* enclosing;
	Location source;
	explicit EvaluateVisitor(Location source, const TypeInstance* enclosing)
		: source(source), enclosing(enclosing)
	{
	}
	void visit(const IntegralConstantExpression& literal)
	{
		result = literal.value;
	}
	void visit(const NonTypeTemplateParameter& node)
	{
		size_t index = node.declaration->templateParameter;
		SYMBOLS_ASSERT(index != INDEX_INVALID);
		const TypeInstance* enclosingType = findEnclosingTemplate(enclosing, node.declaration->scope);
		SYMBOLS_ASSERT(enclosingType != 0);
		SYMBOLS_ASSERT(!isDependent(*enclosingType)); // assert that the enclosing type is not dependent
		SYMBOLS_ASSERT(!enclosingType->declaration->isSpecialization || enclosingType->instantiated); // a specialization must be instantiated (or in the process of instantiating)
		const TemplateArgumentsInstance& templateArguments = enclosingType->declaration->isSpecialization
			? enclosingType->deducedArguments : enclosingType->templateArguments;
		SYMBOLS_ASSERT(index < templateArguments.size());
		UniqueTypeWrapper argument = templateArguments[index];
		SYMBOLS_ASSERT(argument.isNonType());
		result = getNonTypeValue(argument.value);
	}
	void visit(const DependentIdExpression& node)
	{
		result = evaluateIdExpression(node, source, enclosing);
	}
	void visit(const IdExpression& node)
	{
		result = evaluateIdExpression(node, source, enclosing);
	}
	void visit(const SizeofExpression& node)
	{
		if(node.operand == 0)
		{
			std::cout << "sizeof expression with dependent type!" << std::endl;
			return;
		}

		UniqueTypeWrapper type = typeofExpression(node.operand, source);
		// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		// [expr.sizeof] The sizeof operator shall not be applied to an expression that has function or incomplete type.
		result = IntegralConstant(requireCompleteObjectType(removeReference(type), source, enclosing));
	}
	void visit(const SizeofTypeExpression& node)
	{
		// TODO: type-substitution for dependent node.type
		// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		// [expr.sizeof] The sizeof operator shall not be applied to an expression that has function or incomplete type... or to the parenthesized name of such types.
		result = IntegralConstant(requireCompleteObjectType(removeReference(node.type), source, enclosing));
	}
	void visit(const UnaryExpression& node)
	{
		result = node.operation(
			evaluate(node.first, source, enclosing)
		);
	}
	void visit(const BinaryExpression& node)
	{
		result = node.operation(
			evaluate(node.first, source, enclosing),
			evaluate(node.second, source, enclosing)
		);
	}
	void visit(const TernaryExpression& node)
	{
		result = node.operation(
			evaluate(node.first, source, enclosing),
			evaluate(node.second, source, enclosing),
			evaluate(node.third, source, enclosing)
		);
	}
	void visit(const TypeTraitsUnaryExpression& node)
	{
		result = IntegralConstant(node.operation(
			substitute(node.type, source, *enclosing)
		));
	}
	void visit(const TypeTraitsBinaryExpression& node)
	{
		result = IntegralConstant(node.operation(
			substitute(node.first, source, *enclosing),
			substitute(node.second, source, *enclosing),
			source,
			enclosing
		));
	}
};

inline IntegralConstant evaluate(ExpressionNode* node, Location source, const TypeInstance* enclosing)
{
	EvaluateVisitor visitor(source, enclosing);
	node->accept(visitor);
	return visitor.result;
}

inline IntegralConstant evaluateExpression(ExpressionNode* node, Location source, const TypeInstance* enclosing)
{
	if(isDependentPointerToMemberExpression(node))
	{
		// TODO: check this names a valid non-static member
		return IntegralConstant(0); // TODO: unique value for address of member
	}
	return evaluate(node, source, enclosing);
}

inline IntegralConstant evaluateExpression(const ExpressionWrapper& expression, Location source, const TypeInstance* enclosing)
{
	if(isPointerToMemberExpression(expression))
	{
		return IntegralConstant(0); // TODO: unique value for address of member
	}
	if(isPointerToFunctionExpression(expression))
	{
		return IntegralConstant(0); // TODO: unique value for address of function
	}
	SYMBOLS_ASSERT(expression.isConstant);
	return evaluateExpression(expression.p, source, enclosing);
}


// ----------------------------------------------------------------------------
// template instantiation


inline bool isDependent(UniqueTypeWrapper type);

inline bool isDependent(const UniqueTypeArray& types)
{
	for(UniqueTypeArray::const_iterator i = types.begin(); i != types.end(); ++i)
	{
		if(isDependent(*i))
		{
			return true;
		}
	}
	return false;
}


inline bool isDependent(const TypeInstance& type)
{
	if(type.enclosing
		&& isDependent(*type.enclosing))
	{
		return true;
	}
	if(isDependent(type.templateArguments))
	{
		return true;
	}
	return false;
}


struct IsDependentVisitor : TypeElementVisitor
{
	bool result;
	IsDependentVisitor()
		: result(false)
	{
	}
	virtual void visit(const DependentType&)
	{
		result = true;
	}
	virtual void visit(const DependentTypename&)
	{
		result = true;
	}
	virtual void visit(const DependentNonType&)
	{
		result = true;
	}
	virtual void visit(const TemplateTemplateArgument&)
	{
	}
	virtual void visit(const NonType&)
	{
	}
	virtual void visit(const ObjectType& element)
	{
		if(isDependent(element.type))
		{
			result = true;
		}
	}
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
		if(isDependent(element.type))
		{
			result = true;
		}
	}
	virtual void visit(const FunctionType& element)
	{
		if(isDependent(element.parameterTypes))
		{
			result = true;
		}
	}
};


inline bool isDependent(UniqueTypeWrapper type)
{
	for(UniqueTypeWrapper i = type; !i.empty(); i.pop_front())
	{
		IsDependentVisitor visitor;
		i.value->accept(visitor);
		if(visitor.result)
		{
			return true;
		}
	}
	return false;
}


// ----------------------------------------------------------------------------
// template argument deduction

inline bool deduce(UniqueTypeWrapper parameter, UniqueTypeWrapper argument, TemplateArgumentsInstance& result);

inline bool deduce(const UniqueTypeArray& parameters, const UniqueTypeArray& arguments, TemplateArgumentsInstance& result)
{
	UniqueTypeArray::const_iterator p = parameters.begin();
	for(UniqueTypeArray::const_iterator a = arguments.begin();
		a != arguments.end() && p != parameters.end(); // for each pair P, A
		// fewer arguments than parameters: occurs when some parameters are defaulted
		// more arguments than parameters: occurs when matching a specialization such as 'struct S<>'
		++a, ++p)
	{
		if(!deduce(*p, *a, result))
		{
			return false;
		}
	}
	return true;
}

// [temp.deduct.type] The nondeduced contexts are:
// - The nestedname-specifier of a type that was specified using a qualifiedid.
// - A type that is a template-id in which one or more of the template-arguments is an expression that references
//	 a templateparameter.
// When a type name is specified in a way that includes a nondeduced context, all of the types that comprise
// that type name are also nondeduced.
inline bool isNonDeduced(const TypeInstance& type)
{
	return false; // TODO
}

struct DeduceVisitor : TypeElementVisitor
{
	UniqueTypeWrapper argument;
	TemplateArgumentsInstance& templateArguments;
	bool result;
	DeduceVisitor(UniqueTypeWrapper argument, TemplateArgumentsInstance& templateArguments)
		: argument(argument), templateArguments(templateArguments), result(true)
	{
	}
	void commit(std::size_t index)
	{
		SYMBOLS_ASSERT(index != INDEX_INVALID);
		SYMBOLS_ASSERT(index < templateArguments.size());
		if(templateArguments[index] == gUniqueTypeNull) // if this argument was not already deduced (or explicitly specified)
		{
			templateArguments[index] = argument; // use the deduced argument
		}
		else if(templateArguments[index] != argument) // if the deduced (or explicitly specified) argument is different
		{
			result = false;
		}
	}
	virtual void visit(const DependentType& element) // deduce from T, TT, TT<...>
	{
		if(element.templateParameterCount != 0) // TT or TT<..>
		{
			if(element.templateArguments.empty()) // TT
			{
				if(!argument.isTemplateTemplateArgument())
				{
					result = false;
					return;
				}
			}
			else  // TT<..>
			{
				if(!argument.isSimple())
				{
					result = false;
					return;
				}
				const TypeInstance& type = getObjectType(argument.value);
				if(!type.declaration->isTemplate
					|| !deduce(element.templateArguments, type.templateArguments, templateArguments)) // template-template-parameter may have template-arguments that refer to a template parameter
				{
					result = false;
					return;
				}
				argument = gUniqueTypeNull;
				argument.push_front(TemplateTemplateArgument(type.declaration, type.enclosing));
			}
		}
		commit(element.type->templateParameter);
	}
	virtual void visit(const DependentTypename&)
	{
		// cannot deduce from T::
	}
	virtual void visit(const DependentNonType& element)
	{
		// if this expression is of the form 'i' where 'i' is a non-type template parameter
		if(isNonTypeTemplateParameter(element.expression))
		{
			// deduce the argument from the name of the non-type template parameter
			commit(getNonTypeTemplateParameter(element.expression).declaration->templateParameter);
		}
	}
	virtual void visit(const TemplateTemplateArgument& element)
	{
		// cannot deduce from name of primary template
	}
	virtual void visit(const NonType&)
	{
		// cannot deduce from integral constant expression
	}
	virtual void visit(const ObjectType& element)
	{
		SYMBOLS_ASSERT(argument.isSimple());
		const TypeInstance& type = getObjectType(argument.value);
		if(type.primary != element.type.primary) // if the class type does not match
		{
			result = false; // deduction fails
			return;
		}
		// [temp.deduct.type] The nondeduced contexts are:- The nested-name-specifier of a type that was specified using a qualified-id.
		// not attempting to deduce from enclosing type
		if(!isNonDeduced(element.type))
		{
			result = deduce(element.type.templateArguments, type.templateArguments, templateArguments);
		}
	}
	virtual void visit(const PointerType&)
	{
		// cannot deduce from pointer
	}
	virtual void visit(const ReferenceType&)
	{
		// cannot deduce from reference
	}
	virtual void visit(const ArrayType&)
	{
		// TODO: deduce type-name[i]
	}
	virtual void visit(const MemberPointerType& element)
	{
		SYMBOLS_ASSERT(argument.isMemberPointer());
		result = deduce(element.type, getMemberPointerType(argument.value).type, templateArguments);
	}
	virtual void visit(const FunctionType& element)
	{
		SYMBOLS_ASSERT(argument.isFunction());
		result = deduce(element.parameterTypes, getParameterTypes(argument.value), templateArguments);
	}
};

inline bool deduce(UniqueTypeWrapper parameter, UniqueTypeWrapper argument, TemplateArgumentsInstance& result)
{
	for(; !parameter.empty() && !argument.empty(); parameter.pop_front(), argument.pop_front())
	{
		if(!parameter.isDependent()
			&& (!isSameType(parameter, argument)
				|| argument.value.getQualifiers() != parameter.value.getQualifiers()))
		{
			return false;
		}
		if(parameter.isDependent()) // TODO only relevant if this is a DependentType?
		{
			CvQualifiers qualifiers = argument.value.getQualifiers();
			if(parameter.value.getQualifiers().isConst
				&& !qualifiers.isConst)
			{
				return false;
			}
			if(parameter.value.getQualifiers().isVolatile
				&& !qualifiers.isVolatile)
			{
				return false;
			}
			// if both are const, remove const
			qualifiers.isConst ^= parameter.value.getQualifiers().isConst;
			qualifiers.isVolatile ^= parameter.value.getQualifiers().isVolatile;
			argument.value.setQualifiers(qualifiers);
		}
		DeduceVisitor visitor(argument, result);
		parameter.value->accept(visitor);
		if(!visitor.result)
		{
			return false;
		}
	}
	return true;
}


extern ObjectTypeId gVoid;


inline void substitute(UniqueTypeArray& substituted, const UniqueTypeArray& dependent, Location source, const TypeInstance& enclosingType)
{
	for(UniqueTypeArray::const_iterator i = dependent.begin(); i != dependent.end(); ++i)
	{
		UniqueTypeWrapper type = substitute(*i, source, enclosingType);
		substituted.push_back(type);
	}
}

inline UniqueTypeWrapper makeUniqueObjectType(const TypeInstance& type);
inline Declaration* findTemplateSpecialization(Declaration* declaration, TemplateArgumentsInstance& deducedArguments, const TemplateArgumentsInstance& arguments, Location source, const TypeInstance* enclosing, bool allowDependent);

// 'enclosing' is already substituted
inline UniqueTypeWrapper substitute(Declaration* declaration, const TypeInstance* enclosing, const TemplateArgumentsInstance& templateArguments, Location source, const TypeInstance& enclosingType)
{
	TypeInstance result(declaration, enclosing);
	if(declaration->isTemplate)
	{
		result.declaration = result.primary = findPrimaryTemplate(declaration); // TODO: name lookup should always find primary template

		substitute(result.templateArguments, templateArguments, source, enclosingType);

		TemplateArguments::const_iterator i = result.declaration->templateParams.defaults.begin();
		std::advance(i, templateArguments.size());
		for(; i != result.declaration->templateParams.defaults.end(); ++i)
		{
			if((*i).type.declaration == 0)
			{
				throw TooFewTemplateArgumentsError(source);
			}
			extern Declaration gNonType;
			SYMBOLS_ASSERT((*i).type.declaration == &gNonType || (*i).type.unique != 0);
			UniqueTypeWrapper argument = UniqueTypeWrapper((*i).type.declaration == &gNonType
				? pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, NonType(evaluateExpression((*i).expression, source, &result))) // evaluate template-parameter defaults in the context of the owning template
				: (*i).type.unique);
			UniqueTypeWrapper substituted = substitute(argument, source, result); // substitute template-parameter defaults in the context of the owning template
			result.templateArguments.push_back(substituted); // handles when template-param-default depends on a template param that was also defaulted
		}

		SYMBOLS_ASSERT(std::distance(result.declaration->templateParams.begin(), result.declaration->templateParams.end()) == result.templateArguments.size());
	}

	static size_t uniqueId = 0;
	result.uniqueId = ++uniqueId;
	return makeUniqueObjectType(result);
}

inline const TypeInstance* substitute(const TypeInstance& instance, Location source, const TypeInstance& enclosingType)
{
	const TypeInstance* enclosing = 0;
	if(instance.enclosing != 0)
	{
		enclosing = substitute(*instance.enclosing, source, enclosingType);
	}
	UniqueTypeWrapper result = substitute(instance.declaration, enclosing, instance.templateArguments, source, enclosingType);
	return &getObjectType(result.value);
}


struct SubstituteVisitor : TypeElementVisitor
{
	UniqueTypeWrapper type;
	Location source;
	const TypeInstance& enclosingType;
	SubstituteVisitor(UniqueTypeWrapper type, Location source, const TypeInstance& enclosingType)
		: type(type), source(source), enclosingType(enclosingType)
	{
	}
	virtual void visit(const DependentType& element) // substitute T, TT, TT<...>
	{
		std::size_t index = element.type->templateParameter;
		SYMBOLS_ASSERT(index != INDEX_INVALID);
		const TypeInstance* enclosingTemplate = findEnclosingTemplate(&enclosingType, element.type->scope);
		SYMBOLS_ASSERT(enclosingTemplate != 0);
		SYMBOLS_ASSERT(!enclosingTemplate->declaration->isSpecialization || enclosingTemplate->instantiated); // a specialization must be instantiated (or in the process of instantiating)
		const TemplateArgumentsInstance& templateArguments = enclosingTemplate->declaration->isSpecialization
			? enclosingTemplate->deducedArguments : enclosingTemplate->templateArguments;
		SYMBOLS_ASSERT(index < templateArguments.size());
		SYMBOLS_ASSERT(type == gUniqueTypeNull);
		type = templateArguments[index];
		if(element.templateParameterCount != 0) // TT or TT<...>
		{
			// template-template-argument
			if(!type.isTemplateTemplateArgument())
			{
				throw ExpectedTemplateTemplateArgumentError(source, type);
			}
			const TemplateTemplateArgument& argument = getTemplateTemplateArgument(type.value);
			if(std::distance(argument.declaration->templateParams.begin(), argument.declaration->templateParams.end())
				!= element.templateParameterCount)
			{
				throw MismatchedTemplateTemplateArgumentError(source, type);
			}
			if(!element.templateArguments.empty()) // TT<...>
			{ //TODO: TT<>
				type = substitute(argument.declaration, argument.enclosing, element.templateArguments, source, enclosingType);
			}
		}
	}

	virtual void visit(const DependentTypename& element) // substitute T::X, T::template X<...>
	{
		UniqueTypeWrapper qualifying = substitute(element.qualifying, source, enclosingType);
		const TypeInstance* enclosing = qualifying == gUniqueTypeNull || !qualifying.isSimple() ? 0 : &getObjectType(qualifying.value);
		// [temp.deduct] Attempting to use a type that is not a class type in a qualified name
		if(enclosing == 0
			|| !isClass(*enclosing->declaration))
		{
			throw QualifyingIsNotClassError(source, qualifying);
		}

		instantiateClass(*enclosing, source, &enclosingType);
		Identifier id;
		id.value = element.name;
		std::size_t visibility = enclosing->instantiating ? enclosingType.instantiation.pointOfInstantiation : UINT_MAX;
		LookupResultRef declaration = findDeclaration(*enclosing, id, element.isNested ? LookupFilter(IsNestedName()) : LookupFilter(IsAny()), visibility);
		// [temp.deduct]
		// - Attempting to use a type in the qualifier portion of a qualified name that names a type when that
		//   type does not contain the specified member, or if the specified member is not a type where a type is
		//   required.
		if(declaration == 0)
		{
			throw MemberNotFoundError(source, element.name, qualifying);
		}
		if(!isType(*declaration))
		{
			throw MemberIsNotTypeError(source, element.name, qualifying);
		}

		SYMBOLS_ASSERT(isMember(*declaration));

		const TypeInstance* memberEnclosing = findEnclosingType(enclosing, declaration->scope); // the declaration must be a member of (a base of) the qualifying class: find which one.
		SYMBOLS_ASSERT(memberEnclosing != 0);

		if(isClass(*declaration)
			|| isEnum(*declaration))
		{
			type = substitute(declaration, memberEnclosing, element.templateArguments, source, enclosingType);
		}
		else
		{
			SYMBOLS_ASSERT(declaration->type.unique != 0);
			type = UniqueTypeWrapper(declaration->type.unique);
			if(declaration->type.isDependent)
			{
				type = substitute(type, source, *memberEnclosing);
			}
		}
	}
	virtual void visit(const DependentNonType& element)
	{
		// TODO: SFINAE for expressions: check that type of template argument matches template parameter
		IntegralConstant value = evaluateExpression(element.expression, source, &enclosingType);
		type.push_front(NonType(value));
	}
	virtual void visit(const TemplateTemplateArgument& element)
	{
		type.push_front(element);
	}
	virtual void visit(const NonType& element)
	{
		// TODO: SFINAE for expressions: check that type of template argument matches template parameter
		type.push_front(element);
	}
	virtual void visit(const ObjectType& element)
	{
		const TypeInstance* result = substitute(element.type, source, enclosingType);
		type.push_front(ObjectType(*result));
	}
	virtual void visit(const PointerType& element)
	{
		// [temp.deduct] Attempting to create a pointer to reference type.
		if(type.isReference())
		{
			throw PointerToReferenceError(source);
		}
		type.push_front(element);
	}
	virtual void visit(const ReferenceType& element)
	{
		// [temp.deduct] Attempting to create a reference to a reference type or a reference to void
		if(type.isReference()
			|| type == gVoid)
		{
			throw ReferenceToReferenceError(source);
		}
		type.push_front(element);
	}
	virtual void visit(const ArrayType& element)
	{
		// [temp.deduct] Attempting to create an array with an element type that is void, a function type, or a reference type,
		//	or attempting to create an array with a size that is zero or negative.
		if(type.isFunction()
			|| type.isReference()
			|| type == gVoid)
		{
			throw InvalidArrayError(source);
		}
		type.push_front(element); // TODO substitute dependent expressions
	}
	virtual void visit(const MemberPointerType& element)
	{
		UniqueTypeWrapper classType = substitute(element.type, source, enclosingType);
		// [temp.deduct] Attempting to create "pointer to member of T" when T is not a class type.
		if(!classType.isSimple()
			|| !isClass(*getObjectType(classType.value).declaration))
		{
			throw QualifyingIsNotClassError(source, classType);
		}
		type.push_front(MemberPointerType(classType));
	}
	virtual void visit(const FunctionType& element)
	{
		FunctionType result;
		result.isEllipsis = element.isEllipsis;
		substitute(result.parameterTypes, element.parameterTypes, source, enclosingType);
		// [temp.deduct] Attempting to create a function type in which a parameter has a type of void.
		// TODO: Attempting to create a cv-qualified function type.
		if(std::find(result.parameterTypes.begin(), result.parameterTypes.end(), gVoid) != result.parameterTypes.end())
		{
			throw VoidParameterError(source);
		}
		type.push_front(result);
	}
};

inline UniqueTypeWrapper substitute(UniqueTypeWrapper dependent, Location source, const TypeInstance& enclosingType)
{
	UniqueTypeWrapper inner = dependent;
	inner.pop_front();
	UniqueTypeWrapper type = inner.empty() ? gUniqueTypeNull : substitute(inner, source, enclosingType);
	SubstituteVisitor visitor(type, source, enclosingType);
	dependent.value->accept(visitor);
	visitor.type.value.addQualifiers(dependent.value.getQualifiers());
	return visitor.type;
}


// ----------------------------------------------------------------------------
inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source);
inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth);
inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth);

inline void checkUniqueType(UniqueTypeWrapper result, const Type& type, Location source, const TypeInstance* enclosing, bool allowDependent)
{
	SYMBOLS_ASSERT(type.unique == 0 || type.isDependent || isDependent(result) || result.value == type.unique);
	if(type.isDependent
		&& !allowDependent)
	{
		SYMBOLS_ASSERT(enclosing != 0);
		UniqueTypeWrapper substituted = substitute(UniqueTypeWrapper(type.unique), source, *enclosing);
		if(substituted != result)
		{
			std::cout << "uniqued: ";
			printType(result);
			std::cout << std::endl;
			std::cout << "substituted: ";
			printType(substituted);
			std::cout << std::endl;
			throw SymbolsError();
		}
	}
}

inline void reportTypeInfo(const Type& type, const TypeInstance* enclosing)
{
	printPosition(type.id->source);
	std::cout << std::endl;
	std::cout << "while uniquing: ";
	SYMBOLS_ASSERT(type.unique != 0);
	printType(UniqueTypeWrapper(type.unique));
	std::cout << std::endl;
	std::cout << "in context: ";
	SYMBOLS_ASSERT(enclosing != 0);
	printType(*enclosing);
	std::cout << std::endl;
}

extern ObjectTypeId gSignedInt;

template<typename T>
inline UniqueTypeWrapper getUniqueTypeImpl(const T& type, Location source, const TypeInstance* enclosing, bool allowDependent)
{
	if(type.declaration == &gEnumerator)
	{
		return gSignedInt;
	}
	SYMBOLS_ASSERT(type.unique != 0);
	UniqueTypeWrapper result = UniqueTypeWrapper(type.unique);
	if(type.isDependent
		&& !allowDependent)
	{
		UniqueTypeWrapper substituted = substitute(result, source, *enclosing);
		SYMBOLS_ASSERT(!isDependent(substituted));
		return substituted;
	}
	return result;
}

inline UniqueTypeWrapper getUniqueType(const TypeId& type, Location source, const TypeInstance* enclosing, bool allowDependent = false)
{
	return getUniqueTypeImpl(type, source, enclosing, allowDependent);
}

inline UniqueTypeWrapper getUniqueType(const Type& type, Location source, const TypeInstance* enclosing, bool allowDependent = false)
{
	return getUniqueTypeImpl(type, source, enclosing, allowDependent);
}

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source, const TypeInstance* enclosing, bool allowDependent = false)
{
	try
	{
		UniqueTypeWrapper result = makeUniqueType(type, source, enclosing, allowDependent, 0);
		checkUniqueType(result, type, source, enclosing, allowDependent);
		return result;
	}
	catch(TypeError&)
	{
		reportTypeInfo(type, enclosing);
		throw;
	}
}

inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source, const TypeInstance* enclosing, bool allowDependent = false)
{
	try
	{
		UniqueTypeWrapper result = makeUniqueType(type, source, enclosing, allowDependent, 0);
		checkUniqueType(result, type, source, enclosing, allowDependent);
		return result;
	}
	catch(TypeError&)
	{
		reportTypeInfo(type, enclosing);
		throw;
	}
}

inline Scope* getEnclosingTemplate(Scope* enclosing)
{
	for(Scope* scope = enclosing; scope != 0; scope = scope->parent)
	{
		if(isTemplate(*scope))
		{
			return scope;
		}
	}
	return 0;
}


inline bool findScope(Scope* scope, Scope* other)
{
	if(scope == 0)
	{
		return false;
	}
	if(scope == other)
	{
		return true;
	}
	return findScope(scope->parent, other);
}

inline Declaration* getClassDeclaration(Scope* scope)
{
	SYMBOLS_ASSERT(scope);
	SYMBOLS_ASSERT(scope->type == SCOPETYPE_CLASS);
	return getDeclaration(scope->name);
}

inline Declaration* findEnclosingClassTemplate(Declaration* dependent)
{
	if(dependent != 0
		&& (isClass(*dependent)
			|| isEnum(*dependent)) // type of enum within class template is dependent
		&& isMember(*dependent))
	{
		Scope* scope = getEnclosingClass(dependent->scope);
		if(scope == 0)
		{
			// enclosing class was anonymous and at namespace scope.
			return 0;
		}
		Declaration* declaration = getClassDeclaration(scope);
		return declaration->isTemplate
			? declaration
			: findEnclosingClassTemplate(declaration);
	}
	return 0;
}

inline bool isDependent(Declaration* dependent, Scope* enclosing, Scope* templateParamScope)
{
	return dependent != 0
		&& (findScope(enclosing, dependent->scope) != 0
		|| findScope(templateParamScope, dependent->scope) != 0); // if we are within the candidate template-parameter's template-definition
}

// returns the most recent declaration that is not a redeclaration
inline Declaration* findOverloaded(const DeclarationInstance& instance)
{
	for(const DeclarationInstance* p = &instance; p != 0; p = p->overloaded)
	{
		if((*p).redeclared == 0)
		{
			return *p;
		}
	}
	return 0;
}

inline std::size_t addBase(TypeInstance& instance, UniqueTypeWrapper base, Location source)
{
	SYMBOLS_ASSERT(!isDependent(base));
	SYMBOLS_ASSERT(base.isSimple());
	const TypeInstance& objectType = getObjectType(base.value);
	std::size_t size = instantiateClass(objectType, source, &instance);
	SYMBOLS_ASSERT(isClass(*objectType.declaration));
	SYMBOLS_ASSERT(objectType.declaration->enclosed != 0); // this can occur when the primary template is incomplete, and a specialization was not chosen
	instance.bases.push_back(&objectType);
	return size;
}

inline bool isTemplate(const TypeInstance& instance)
{
	if(instance.declaration->isTemplate)
	{
		return true;
	}
	return instance.enclosing != 0
		&& isTemplate(*instance.enclosing);
}

void dumpTemplateInstantiations(const TypeInstance& instance, bool root = false);

inline std::size_t instantiateClass(const TypeInstance& instanceConst, Location source, const TypeInstance* enclosing, bool allowDependent)
{
	TypeInstance& instance = const_cast<TypeInstance&>(instanceConst);
	SYMBOLS_ASSERT(isClass(*instance.declaration));

	if(enclosing != 0)
	{
		ChildInstantiations& instantiations = const_cast<TypeInstance*>(enclosing)->childInstantiations;
		instantiations.push_back(ChildInstantiation(&instance, source));
	}

	if(instance.instantiated)
	{
		return instance.size;
	}
	try
	{
		instance.instantiated = true; // prevent recursion
		SYMBOLS_ASSERT(!instance.instantiating);
		instance.instantiating = true;
		instance.instantiation = source;

		static std::size_t uniqueId = 0;
		instance.uniqueId = ++uniqueId;

		if(!allowDependent
			&& instance.declaration->isTemplate)
		{
			// find the most recently declared specialization
			const DeclarationInstance* declaration = findDeclaration(instance.declaration->scope->declarations, instance.declaration->getName());
			SYMBOLS_ASSERT(declaration != 0);
			Declaration* specialization = findTemplateSpecialization(findOverloaded(*declaration), instance.deducedArguments, instance.templateArguments, source, instance.enclosing, false);
			if(specialization != 0)
			{
				instance.declaration = specialization;
			}
		}

		if(instance.declaration->enclosed == 0)
		{
			std::cout << "instantiateClass failed: ";
			printType(instance);
			std::cout << std::endl;
			return 0; // TODO: this can occur when the primary template is incomplete, and a specialization was not chosen
		}

		SYMBOLS_ASSERT(instance.declaration->type.unique != 0);
		// the is the (possibly dependent) unique type of the unspecialized (template) class on which this specialization is based
		const TypeInstance& original = getObjectType(instance.declaration->type.unique);

		instance.size = 4; // TODO: get size of built-in types
		SYMBOLS_ASSERT(instance.declaration->enclosed != 0);
		Types& bases = instance.declaration->enclosed->bases;
		instance.bases.reserve(std::distance(bases.begin(), bases.end()));
		for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
		{
			// TODO: check compliance: the point of instantiation of a base is the point of declaration of the enclosing (template) class
			// .. along with the point of instantiation of types required when naming the base type. e.g. struct C : A<T>::B {}; struct C : B<A<T>::value> {};
			UniqueTypeId base = getUniqueType(*i, original.instantiation, &instance, allowDependent);
			SYMBOLS_ASSERT((*i).unique != 0);
			SYMBOLS_ASSERT((*i).isDependent || base.value == (*i).unique);
			if(allowDependent && (*i).isDependent)
			{
				// this occurs during 'instantiation' of a template class definition, in which case we postpone instantiation of this dependent base
				continue;
			}
			instance.size += addBase(instance, base, original.instantiation);
		}
		instance.allowLookup = true; // prevent searching bases during lookup within incomplete instantiation
		if(!allowDependent)
		{
			if(!original.children.empty()
				&& &instance != &original) // TODO: this will be an assert when instantiateClass is no longer called at the beginning of a template-definition
			{
				SYMBOLS_ASSERT(instance.declaration->isComplete);
				instance.children.reserve(original.children.size());
				InstanceLocations::const_iterator l = original.childLocations.begin();
				for(InstantiatedTypes::const_iterator i = original.children.begin(); i != original.children.end(); ++i, ++l)
				{
					UniqueTypeWrapper substituted = substitute(*i, *l, instance);
					SYMBOLS_ASSERT(!isDependent(substituted));
					instance.children.push_back(substituted);
				}
			}
		}
		instance.instantiating = false;
	}
	catch(TypeError&)
	{
		printPosition(source);
		std::cout << "while instantiating ";
		printType(instance);
		std::cout << std::endl;
		if(instance.declaration->isTemplate)
		{
			const TemplateArgumentsInstance& templateArguments = instance.declaration->isSpecialization ? instance.deducedArguments : instance.templateArguments;
			TemplateArgumentsInstance::const_iterator a = templateArguments.begin();
			for(TemplateParameters::const_iterator i = instance.declaration->templateParams.begin(); i != instance.declaration->templateParams.end(); ++i)
			{
				SYMBOLS_ASSERT(a != templateArguments.end());
				std::cout << getValue((*i).declaration->getName()) << ": ";
				printType(*a++);
				std::cout << std::endl;
			}
		}

		if(enclosing == 0
			|| !isTemplate(*enclosing))
		{
			dumpTemplateInstantiations(instance, true);
		}
		throw;
	}
	return instance.size;
}

inline std::size_t requireCompleteObjectType(UniqueTypeWrapper type, Location source, const TypeInstance* enclosing)
{
	std::size_t count = 1;
	while(type.isArray()
		&& getArrayType(type.value).size != 0)
	{
		count *= getArrayType(type.value).size;
		type.pop_front(); // arrays of known size are complete object types
	}
	if(type.isSimple())
	{
		const TypeInstance& objectType = getObjectType(type.value);
		if(isClass(*objectType.declaration))
		{
			return instantiateClass(objectType, source, enclosing) * count;
		}
	}
	return 4; // TODO: size of non-object types
}

// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
inline UniqueTypeWrapper removeReference(UniqueTypeWrapper type)
{
	if(type.isReference())
	{
		type.pop_front();
	}
	return type;
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth, UniqueTypeWrapper& unique)
{
	if(!qualifying.empty())
	{
		if(isNamespace(*qualifying.back().declaration))
		{
			return 0; // name is qualified by a namespace, therefore cannot be enclosed by a class
		}
		unique = getUniqueType(qualifying.back(), source, enclosing, allowDependent);
		if(allowDependent && unique.isDependent())
		{
			return 0;
		}
		const TypeInstance& type = getObjectType(unique.value);
		// [temp.inst] A class template is implicitly instantiated ... if the completeness of the class-type affects the semantics of the program.
		instantiateClass(type, source, enclosing, allowDependent);
		return &type;
	}
	return enclosing;
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth)
{
	UniqueTypeWrapper tmp;
	return makeUniqueEnclosing(qualifying, source, enclosing, allowDependent, depth, tmp);
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const TypeInstance* enclosing, bool allowDependent)
{
	try
	{
		return makeUniqueEnclosing(qualifying, source, enclosing, allowDependent, 0);
	}
	catch(TypeError&)
	{
		if(!qualifying.empty())
		{
			const Type& type = qualifying.back();
			std::cout << "while uniquing: ";
			SYMBOLS_ASSERT(type.unique != 0);
			printType(UniqueTypeWrapper(type.unique));
			std::cout << std::endl;
		}
		throw;
	}
}


inline bool deduceAndSubstitute(const UniqueTypeArray& parameters, const UniqueTypeArray& arguments, Location source, TypeInstance& enclosing, TemplateArgumentsInstance& substituted)
{
	// deduce the partial-specialization's template arguments from the original argument list
	if(!deduce(parameters, arguments, enclosing.deducedArguments)
		|| std::find(enclosing.deducedArguments.begin(), enclosing.deducedArguments.end(), gUniqueTypeNull) != enclosing.deducedArguments.end())
	{
		return false; // cannot deduce
	}
	try
	{
		// substitute the template-parameters in the partial-specialization's signature with the deduced template-arguments
		substitute(substituted, parameters, source, enclosing);
	}
	catch(TypeError&)
	{
		SYMBOLS_ASSERT(std::find(substituted.begin(), substituted.end(), gUniqueTypeNull) == substituted.end());
		return false; // cannot substitute: SFINAE
	}

	return true;
}


inline bool matchTemplatePartialSpecialization(Declaration* declaration, TemplateArgumentsInstance& deducedArguments, const TemplateArgumentsInstance& specializationArguments, const TemplateArgumentsInstance& arguments, Location source)
{
	SYMBOLS_ASSERT(!declaration->templateParams.empty());
	TemplateArgumentsInstance deduced(std::distance(declaration->templateParams.begin(), declaration->templateParams.end()), gUniqueTypeNull);
	TemplateArgumentsInstance substituted;
	TypeInstance enclosing(declaration, 0);
	enclosing.deducedArguments.swap(deduced);
	enclosing.instantiated = true;
	if(!deduceAndSubstitute(specializationArguments, arguments, source, enclosing, substituted))
	{
		return false; // partial-specialization only matches if template-argument-deduction succeeds
	}
	if(std::equal(substituted.begin(), substituted.end(), arguments.begin()))
	{
		deducedArguments.swap(enclosing.deducedArguments);
		return true;
	}
	return false;
}

inline bool matchTemplatePartialSpecialization(Declaration* declaration, const TemplateArgumentsInstance& specializationArguments, const TemplateArgumentsInstance& arguments, Location source)
{
	TemplateArgumentsInstance deducedArguments;
	return matchTemplatePartialSpecialization(declaration, deducedArguments, specializationArguments, arguments, source);
}

inline void makeSpecializationArguments(Declaration* declaration, TemplateArgumentsInstance& specializationArguments, Location source, const TypeInstance* enclosing, bool allowDependent)
{
	SYMBOLS_ASSERT(isSpecialization(*declaration));
	for(TemplateArguments::const_iterator i = declaration->templateArguments.begin(); i != declaration->templateArguments.end(); ++i)
	{
		UniqueTypeWrapper type;
		SYMBOLS_ASSERT((*i).type.declaration != 0);
		extern Declaration gNonType;
		if((*i).type.declaration == &gNonType)
		{
			allowDependent || (*i).expression.isValueDependent
				? pushUniqueType(type.value, DependentNonType((*i).expression))
				: pushUniqueType(type.value, NonType(evaluateExpression((*i).expression, (*i).source, enclosing)));
		}
		else
		{
			SYMBOLS_ASSERT((*i).type.unique != 0);
			type = getUniqueType((*i).type, (*i).source, enclosing, allowDependent || (*i).type.isDependent); // a partial-specialization may have dependent template-arguments: template<class T> class C<T*>
			SYMBOLS_ASSERT((*i).type.unique == type.value);
		}
		specializationArguments.push_back(type);
	}
}

inline void makePrimaryArguments(Declaration* declaration, TemplateArgumentsInstance& arguments, Location source, const TypeInstance* enclosing, bool allowDependent)
{
	for(Types::const_iterator i = declaration->templateParams.begin(); i != declaration->templateParams.end(); ++i)
	{
		const Type& argument = (*i);
		UniqueTypeWrapper result;
		extern Declaration gParam;
		if(argument.declaration->type.declaration == &gParam)
		{
			result = getUniqueType(argument, source, enclosing, allowDependent);
			SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
		}
		else
		{
			UniqueExpression expression = makeExpression(NonTypeTemplateParameter(argument.declaration));
			allowDependent
				? pushUniqueType(result.value, DependentNonType(expression))
				: pushUniqueType(result.value, NonType(evaluateExpression(expression, source, enclosing)));
		}
		arguments.push_back(result);
	}
	SYMBOLS_ASSERT(!arguments.empty());
}

#if 0
inline bool matchEnclosingTemplate(Declaration* declaration, const TemplateArgumentsInstance& arguments, Location source, const TypeInstance* enclosing, bool allowDependent)
{
	TemplateArgumentsInstance expected;
	if(isSpecialization(*declaration))
	{
		makeSpecializationArguments(declaration, expected, source, enclosing, allowDependent);
	}
	else
	{
		makePrimaryArguments(declaration, expected, source, enclosing, allowDependent);
	}
	return expected.size() == arguments.size()
		&& std::equal(expected.begin(), expected.end(), arguments.begin());
}
#endif

inline Declaration* findTemplateSpecialization(Declaration* declaration, TemplateArgumentsInstance& deducedArguments, const TemplateArgumentsInstance& arguments, Location source, const TypeInstance* enclosing, bool allowDependent)
{
	Declaration* best = 0;
	TemplateArgumentsInstance bestArguments;
	for(; declaration != 0; declaration = declaration->overloaded)
	{
		if(!isSpecialization(*declaration))
		{
			continue;
		}

		// TODO: check that all non-defaulted arguments are specified!
		TemplateArgumentsInstance specializationArguments;
		makeSpecializationArguments(declaration, specializationArguments, source, enclosing, allowDependent);

		SYMBOLS_ASSERT(specializationArguments.size() <= arguments.size()); // a template-specialization must have no more arguments than the template parameters
		
		if(declaration->templateParams.empty()) // if this is an explicit specialization
		{
			if(std::equal(specializationArguments.begin(), specializationArguments.end(), arguments.begin()))
			{
				deducedArguments.clear();
				return declaration; // exact match
			}
			continue;
		}

		TemplateArgumentsInstance deduced;
		if(matchTemplatePartialSpecialization(declaration, deduced, specializationArguments, arguments, source)) // if this partial-specialization can be deduced for the specified types
		{
			// consider two specializations: A<T**> and A<T*>
			// when deducing with int**, we deduce against A<T*> and find a match, recording it as 'best'
			// we then try to deduce against A<T**> and also find a match.
			// to break the tie, we attempt to deduce the current P** against the best <T*>: success (T = P*)
			// T** is at least as specialized as T*, because deduction succeeds for P** against T*
			// then we attempt to deduce the best P* against the current <T**>: fail
			// T* is not at least as specialized as T**, because deduction fails for P* against T**
			// therefore T** is more specialized than T*
			// we replace the best <T*> with the current <T**>.
			if(best != 0)
			{
				bool atLeastAsSpecializedCurrent = matchTemplatePartialSpecialization(best, bestArguments, specializationArguments, source); // deduce current against best
				bool atLeastAsSpecializedBest = matchTemplatePartialSpecialization(declaration, specializationArguments, bestArguments, source);
				
				if(atLeastAsSpecializedCurrent
					&& atLeastAsSpecializedBest)
				{
					// TODO: this may occur if the specializations differ only in non-type arguments
					//SYMBOLS_ASSERT(isNonType(arguments));
					struct AmbiguousSpecialization : TypeError
					{
						void report()
						{
							std::cout << "ambiguous specialization!";
							std::cout << std::endl;
						}
					};
					throw AmbiguousSpecialization();
				}
				
				if(atLeastAsSpecializedBest)
				{
					continue; // the best specialization is at least as specialized as the current
				}
			}
			best = declaration;
			bestArguments.swap(specializationArguments);
			deducedArguments.swap(deduced);
		}
	}
	return best;
}


inline void makeUniqueTemplateArguments(const TemplateArguments& arguments, TemplateArgumentsInstance& templateArguments, Location source, const TypeInstance* enclosingType, bool allowDependent)
{
	for(TemplateArguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
	{
		const TemplateArgument& argument = *i;
		SYMBOLS_ASSERT(argument.type.declaration != 0); // TODO: non-fatal error: not enough template arguments!
		UniqueTypeWrapper result;
		extern Declaration gNonType;
		if(argument.type.declaration == &gNonType)
		{
			allowDependent && argument.expression.isValueDependent // for now, do not evaluate dependent expressions!
				? pushUniqueType(result.value, DependentNonType(argument.expression))
				: pushUniqueType(result.value, NonType(evaluateExpression(argument.expression, argument.source, enclosingType)));
		}
		else
		{
			result = getUniqueType(argument.type, argument.source, enclosingType, allowDependent);
			SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
		}
		templateArguments.push_back(result);
	}
}


inline const TypeInstance* findEnclosingType(const TypeInstance& enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	if(scope->type == SCOPETYPE_TEMPLATE)
	{
		return enclosing.declaration->templateParamScope == scope
			? &enclosing
			: 0; // don't search base classes for template-parameter
	}

	if(enclosing.declaration->enclosed == scope)
	{
		return &enclosing;
	}

	if(enclosing.declaration->enclosed != 0) // TODO: 'enclosing' may be incomplete if we're finding the enclosing type for a template default argument. 
	{
		SYMBOLS_ASSERT(enclosing.instantiated); // the enclosing type should have been instantiated by this point
	}

	for(UniqueBases::const_iterator i = enclosing.bases.begin(); i != enclosing.bases.end(); ++i)
	{
		const TypeInstance* result = findEnclosingType(*(*i), scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

inline const TypeInstance* findEnclosingType(const TypeInstance* enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	for(const TypeInstance* i = enclosing; i != 0; i = (*i).enclosing)
	{
		const TypeInstance* result = findEnclosingType(*i, scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}


inline UniqueTypeWrapper makeUniqueObjectType(const TypeInstance& type)
{
	SYMBOLS_ASSERT(!(type.primary->isTemplate && isSpecialization(*type.primary))); // primary declaration must not be a specialization!
	return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, ObjectType(type)));
}

struct LookupFailed : TypeError
{
	const TypeInstance* enclosing;
	const Identifier* id;
	LookupFailed(const TypeInstance* enclosing, const Identifier* id)
		: enclosing(enclosing), id(id)
	{
	}
	void report()
	{
		std::cout << "lookup failed: ";
		printType(*enclosing);
		std::cout << "::" << id->value.c_str() << std::endl;
	}
};

// unqualified object name: int, Object,
// qualified object name: Qualifying::Object
// unqualified typedef: Typedef, TemplateParam
// qualified typedef: Qualifying::Type
// /p type
// /p enclosingType The enclosing template, required when uniquing a template-argument: e.g. Enclosing<int>::Type
//			Note: if 'type' is a class-template template default argument, 'enclosingType' will be the class-template, which does not require instantiation!
inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source, const TypeInstance* enclosingType, bool allowDependent, std::size_t depth)
{
	if(depth++ == 256)
	{
		struct MaximumRecursionDepth : TypeError
		{
			void report()
			{
				std::cout << "makeUniqueType reached maximum recursion depth!" << std::endl;
			}
		};
		throw MaximumRecursionDepth();
	}

	// the type in which template-arguments are looked up: returns qualifying type if specified, else returns enclosingType
	UniqueTypeWrapper qualifying;
	const TypeInstance* enclosing = makeUniqueEnclosing(type.qualifying, source, enclosingType, allowDependent, depth, qualifying);
	Declaration* declaration = type.declaration;
	extern Declaration gDependentType;
	extern Declaration gDependentTemplate;
	extern Declaration gDependentNested;
	extern Declaration gDependentNestedTemplate;
	if(declaration == &gDependentType
		|| declaration == &gDependentTemplate
		|| declaration == &gDependentNested
		|| declaration == &gDependentNestedTemplate) // this is a type-name with a dependent nested-name-specifier
	{
		bool isNested = declaration == &gDependentNested || declaration == &gDependentNestedTemplate;
		SYMBOLS_ASSERT(!type.qualifying.empty()); // the type-name must be qualified
		SYMBOLS_ASSERT(type.id != IdentifierPtr(0));
		const DeclarationInstance* instance = 0;
		if(!allowDependent // the qualifying/enclosing type is not dependent
			&& enclosing->declaration->enclosed != 0) // the qualifying/enclosing type is complete
		{
			if(enclosing->instantiating) // TODO: this occurs when choosing the wrong specialization between two that differ only in non-type template arguments 
			{
				std::cout << "not finished instantiating: ";
				printType(*enclosing);
				std::cout << std::endl;
			}
			else
			{
				instance = findDeclaration(*enclosing, *type.id, isNested ? LookupFilter(IsNestedName()) : LookupFilter(IsAny()));
			}
		}
		// SYMBOLS_ASSERT(declaration != 0); // TODO: assert
		if(instance == 0)
		{
			if(allowDependent)
			{
				TemplateArgumentsInstance templateArguments;
				makeUniqueTemplateArguments(type.templateArguments, templateArguments, source, enclosingType, allowDependent);
				return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, DependentTypename(type.id->value, qualifying, templateArguments, isNested, declaration->isTemplate)));
			}

			throw LookupFailed(enclosing, type.id);
		}
		declaration = *instance;
	}
	size_t index = declaration->templateParameter; // TODO: template-template-parameter
	if(index != INDEX_INVALID)
	{
		SYMBOLS_ASSERT(type.qualifying.empty());
		// Find the template-specialisation it belongs to:
		const TypeInstance* enclosingType = findEnclosingType(enclosing, declaration->scope);
		if(enclosingType != 0
			&& (!allowDependent // if we expect the type not to be dependent
				|| !isDependent(*enclosingType))) // or the enclosing type is not dependent
		{
			SYMBOLS_ASSERT(allowDependent || !isDependent(*enclosingType)); // if we expect the type not to be dependent, assert that the enclosing type is not dependent
			SYMBOLS_ASSERT(!enclosingType->declaration->isSpecialization || enclosingType->instantiated); // a specialization must be instantiated (or in the process of instantiating)
			const TemplateArgumentsInstance& arguments = enclosingType->declaration->isSpecialization
				? enclosingType->deducedArguments
				: enclosingType->templateArguments;

			SYMBOLS_ASSERT(index < arguments.size());
			UniqueTypeWrapper result = arguments[index];
			// TODO: SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL); // fails for non-type template-argument
			return result;
		}

		if(allowDependent
			/*&& enclosing == 0*/)
		{
			TemplateArgumentsInstance templateArguments;
			makeUniqueTemplateArguments(type.templateArguments, templateArguments, source, enclosingType, allowDependent);
			std::size_t templateParameterCount = declaration->isTemplate ? std::distance(declaration->templateParams.begin(), declaration->templateParams.end()) : 0;
			return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, DependentType(declaration, templateArguments, templateParameterCount)));
		}

		throw SymbolsError();
		return gUniqueTypeNull; // error: can't find template specialisation for this template parameter
	}

	const TypeInstance* memberEnclosing = isMember(*declaration) // if the declaration is a class member
		? findEnclosingType(enclosing, declaration->scope) // it must be a member of (a base of) the qualifying class: find which one.
		: 0; // the declaration is not a class member and cannot be found through qualified name lookup

	if(declaration->specifiers.isTypedef)
	{
		UniqueTypeWrapper result = getUniqueType(declaration->type, source, memberEnclosing, allowDependent);
		if(memberEnclosing != 0 // if the typedef is a member
			&& !allowDependent // and we expect the enclosing class to have been instantiated (qualified access, e.g. C<T>::m)
			&& declaration->instance != INDEX_INVALID) // and its type was dependent when parsed
		{
			SYMBOLS_ASSERT(memberEnclosing->instantiated); // assert that the enclosing class was instantiated
			SYMBOLS_ASSERT(declaration->instance < memberEnclosing->children.size());
			SYMBOLS_ASSERT(memberEnclosing->children[declaration->instance] == result);
		}
		return result;
	}

	if(declaration->isTemplate
		&& type.isImplicitTemplateId // if no template argument list was specified
		&& !type.isEnclosingClass) // and the type is not the name of an enclosing class
	{
		// this is a template-name
		return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, TemplateTemplateArgument(declaration, memberEnclosing)));
	}

	TypeInstance tmp(declaration, memberEnclosing);
	SYMBOLS_ASSERT(declaration->type.declaration != &gArithmetic || tmp.enclosing == 0); // arithmetic types should not have an enclosing template!
	if(declaration->isTemplate)
	{
		tmp.declaration = tmp.primary = findPrimaryTemplate(declaration);

		bool isEnclosingSpecialization = type.isEnclosingClass && isSpecialization(*type.declaration);

		// [temp.local]: when the name of a template is used without arguments, substitute the parameters (in case of an enclosing explicit/partial-specialization, substitute the arguments
		const TemplateArguments& defaults = tmp.declaration->templateParams.defaults;
		SYMBOLS_ASSERT(!defaults.empty());
		if(type.isImplicitTemplateId // if no template argument list was specified
			&& !isEnclosingSpecialization) // and the type is not the name of an enclosing class-template explicit/partial-specialization
		{
			// when the type refers to a template-name outside an enclosing class, it is a template-template-parameter:
			// we substitute the primary template's (possibly dependent) template parameters.
			bool dependent = allowDependent || !type.isEnclosingClass;
			makePrimaryArguments(tmp.declaration, tmp.templateArguments, source, enclosingType, dependent);
		}
		else
		{
			const TemplateArguments& arguments = isEnclosingSpecialization
				? type.declaration->templateArguments
				: type.templateArguments;
			TemplateArguments::const_iterator a = arguments.begin();
			for(TemplateArguments::const_iterator i = defaults.begin(); i != defaults.end(); ++i)
			{
				bool isTemplateParamDefault = a == arguments.end();
				if(allowDependent && isTemplateParamDefault) // for dependent types, don't substitute default for unspecified arguments
				{
					break;
				}
				const TemplateArgument& argument = isTemplateParamDefault ? (*i) : (*a++);
				SYMBOLS_ASSERT(argument.type.declaration != 0); // TODO: non-fatal error: not enough template arguments!
				UniqueTypeWrapper result;
				extern Declaration gNonType;
				if(argument.type.declaration == &gNonType)
				{
					allowDependent && argument.expression.isValueDependent // for now, do not evaluate dependent expressions!
						? pushUniqueType(result.value, DependentNonType(argument.expression))
						: pushUniqueType(result.value, NonType(evaluateExpression(argument.expression, argument.source, enclosingType)));
				}
				else
				{
					const TypeInstance* enclosing = isTemplateParamDefault ? &tmp : enclosingType; // resolve dependent template-parameter-defaults in context of template class
					result = getUniqueType(argument.type, argument.source, enclosing, allowDependent);
					SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
				}
				tmp.templateArguments.push_back(result);
			}
			SYMBOLS_ASSERT(allowDependent || !tmp.templateArguments.empty()); // dependent types may have no arguments
		}

#if 0
		if(type.isEnclosingClass
			&& matchEnclosingTemplate(declaration, tmp.templateArguments, source, enclosing, allowDependent))
		{
			tmp.declaration = type.declaration;
		}
		else
		{
			Declaration* specialization = findTemplateSpecialization(declaration, tmp.deducedArguments, tmp.templateArguments, source, enclosing, allowDependent);
			if(specialization != 0)
			{
				tmp.declaration = specialization;
			}
		}
#endif
	}
	SYMBOLS_ASSERT(tmp.bases.empty());
	SYMBOLS_ASSERT(tmp.children.empty());
	static size_t uniqueId = 0;
	tmp.uniqueId = ++uniqueId;
	return makeUniqueObjectType(tmp);
}

inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source)
{
	return makeUniqueType(type, source, 0);
}

inline std::size_t evaluateArraySize(const ExpressionWrapper& expression, Location source, const TypeInstance* enclosing)
{
	if(expression == 0) // []
	{
		return 0;
	}
	if(expression.isValueDependent) // TODO
	{
		return -1;
	}
	SYMBOLS_ASSERT(expression.isConstant);
	return evaluate(expression, source, enclosing).value;
}

struct TypeSequenceMakeUnique : TypeSequenceVisitor
{
	UniqueType& type;
	Location source;
	const TypeInstance* enclosing;
	bool allowDependent;
	TypeSequenceMakeUnique(UniqueType& type, Location source, const TypeInstance* enclosing, bool allowDependent)
		: type(type), source(source), enclosing(enclosing), allowDependent(allowDependent)
	{
	}
	void visit(const DeclaratorPointerType& element)
	{
		pushUniqueType(type, PointerType());
		type.setQualifiers(element.qualifiers);
	}
	void visit(const DeclaratorReferenceType& element)
	{
		pushUniqueType(type, ReferenceType());
	}
	void visit(const DeclaratorArrayType& element)
	{
		for(ArrayRank::const_reverse_iterator i = element.rank.rbegin(); i != element.rank.rend(); ++i)
		{
			pushUniqueType(type, ArrayType(evaluateArraySize(*i, source, enclosing)));
		}
	}
	void visit(const DeclaratorMemberPointerType& element)
	{
		UniqueTypeWrapper tmp = getUniqueType(element.type, source, enclosing, allowDependent);
		SYMBOLS_ASSERT(allowDependent || !tmp.isDependent());
		pushUniqueType(type, MemberPointerType(tmp));
		type.setQualifiers(element.qualifiers);
	}
	void visit(const DeclaratorFunctionType& element)
	{
		FunctionType result;
		result.isEllipsis = element.parameters.isEllipsis;
		result.parameterTypes.reserve(element.parameters.size());
		for(Parameters::const_iterator i = element.parameters.begin(); i != element.parameters.end(); ++i)
		{
			result.parameterTypes.push_back(getUniqueType((*i).declaration->type, source, enclosing, allowDependent));
		}
		pushUniqueType(type, result);
		type.setQualifiers(element.qualifiers);
	}
};

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth)
{
	UniqueTypeWrapper result = makeUniqueType(*static_cast<const Type*>(&type), source, enclosing, allowDependent, depth);
	result.value.addQualifiers(type.qualifiers);
	TypeSequenceMakeUnique visitor(result.value, source, enclosing, allowDependent);
	type.typeSequence.accept(visitor);
	return result;
}

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source)
{
	return makeUniqueType(type, source, 0);
}


// ----------------------------------------------------------------------------
// expression helper

template<typename T, bool isExpression = IsConvertible<T, cpp::expression>::RESULT>
struct ExpressionType;

template<typename T>
struct ExpressionType<T, false>
{
	static UniqueTypeId get(T* symbol)
	{
		return gUniqueTypeNull;
	}
	static void set(T* symbol, UniqueTypeId declaration)
	{
	}
};

inline UniqueTypeId getExpressionType(cpp::expression* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionType(cpp::expression* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionType(cpp::postfix_expression_suffix* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionType(cpp::postfix_expression_suffix* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionType(cpp::type_id* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionType(cpp::type_id* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionType(cpp::base_specifier* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionType(cpp::base_specifier* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

template<typename T>
struct ExpressionType<T, true>
{
	static UniqueTypeId get(T* symbol)
	{
		return getExpressionType(symbol);
	}
	static void set(T* symbol, UniqueTypeId declaration)
	{
		setExpressionType(symbol, declaration);
	}
};


// ----------------------------------------------------------------------------
// built-in symbols

// special-case
extern Declaration gUndeclared;
extern const DeclarationInstance gUndeclaredInstance;

#define TYPE_ARITHMETIC TypeId(&gArithmetic, TREEALLOCATOR_NULL)
#define TYPE_SPECIAL TypeId(&gSpecial, TREEALLOCATOR_NULL)
#define TYPE_CLASS TypeId(&gClass, TREEALLOCATOR_NULL)
#define TYPE_ENUM TypeId(&gEnum, TREEALLOCATOR_NULL)

// types
struct BuiltInTypeDeclaration : Declaration
{
	BuiltInTypeDeclaration(Identifier& name, const TypeId& type = TYPE_ARITHMETIC)
		: Declaration(TREEALLOCATOR_NULL, 0, name, type, 0)
	{
	}
};


#define TYPE_NAMESPACE TypeId(&gNamespace, TREEALLOCATOR_NULL)
#define TYPE_CTOR TypeId(&gCtor, TREEALLOCATOR_NULL)
#define TYPE_ENUMERATOR TypeId(&gEnumerator, TREEALLOCATOR_NULL)
#define TYPE_UNKNOWN TypeId(&gUnknown, TREEALLOCATOR_NULL)


// fundamental types
extern BuiltInTypeDeclaration gCharDeclaration;
extern BuiltInTypeDeclaration gSignedCharDeclaration;
extern BuiltInTypeDeclaration gUnsignedCharDeclaration;
extern BuiltInTypeDeclaration gSignedShortIntDeclaration;
extern BuiltInTypeDeclaration gUnsignedShortIntDeclaration;
extern BuiltInTypeDeclaration gSignedIntDeclaration;
extern BuiltInTypeDeclaration gUnsignedIntDeclaration;
extern BuiltInTypeDeclaration gSignedLongIntDeclaration;
extern BuiltInTypeDeclaration gUnsignedLongIntDeclaration;
extern BuiltInTypeDeclaration gSignedLongLongIntDeclaration;
extern BuiltInTypeDeclaration gUnsignedLongLongIntDeclaration;
extern BuiltInTypeDeclaration gWCharTDeclaration;
extern BuiltInTypeDeclaration gBoolDeclaration;
extern BuiltInTypeDeclaration gFloatDeclaration;
extern BuiltInTypeDeclaration gDoubleDeclaration;
extern BuiltInTypeDeclaration gLongDoubleDeclaration;
extern BuiltInTypeDeclaration gVoidDeclaration;
extern ObjectTypeId gChar;
extern ObjectTypeId gSignedChar;
extern ObjectTypeId gUnsignedChar;
extern ObjectTypeId gSignedShortInt;
extern ObjectTypeId gUnsignedShortInt;
extern ObjectTypeId gSignedInt;
extern ObjectTypeId gUnsignedInt;
extern ObjectTypeId gSignedLongInt;
extern ObjectTypeId gUnsignedLongInt;
extern ObjectTypeId gSignedLongLongInt;
extern ObjectTypeId gUnsignedLongLongInt;
extern ObjectTypeId gWCharT;
extern ObjectTypeId gBool;
extern ObjectTypeId gFloat;
extern ObjectTypeId gDouble;
extern ObjectTypeId gLongDouble;
extern ObjectTypeId gVoid;

inline bool isVoidParameter(const TypeId& type)
{
	return type.declaration == &gVoidDeclaration
		&& type.typeSequence.empty();
}


struct StringLiteralTypeId : ObjectTypeId
{
	StringLiteralTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: ObjectTypeId(declaration, allocator)
	{
		value = pushBuiltInType(value, ArrayType(0));
	}
};

extern StringLiteralTypeId gStringLiteral;
extern StringLiteralTypeId gWideStringLiteral;

inline unsigned combineFundamental(unsigned fundamental, unsigned token)
{
	unsigned mask = 1 << token;
	if((fundamental & mask) != 0)
	{
		mask <<= 16;
	}
	return fundamental | mask;
}

#define MAKE_FUNDAMENTAL(token) (1 << cpp::simple_type_specifier_builtin::token)
#define MAKE_FUNDAMENTAL2(token) (MAKE_FUNDAMENTAL(token) << 16)

inline Declaration* getFundamentalType(unsigned fundamental)
{
	switch(fundamental)
	{
	case MAKE_FUNDAMENTAL(CHAR): return &gCharDeclaration;
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(CHAR): return &gSignedCharDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(CHAR): return &gUnsignedCharDeclaration;
	case MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT): return &gSignedShortIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT): return &gUnsignedShortIntDeclaration;
	case MAKE_FUNDAMENTAL(INT):
	case MAKE_FUNDAMENTAL(SIGNED):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(INT): return &gSignedIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(INT): return &gUnsignedIntDeclaration;
	case MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT): return &gSignedLongIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT): return &gUnsignedLongIntDeclaration;
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG) | MAKE_FUNDAMENTAL(INT): return &gSignedLongLongIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG) | MAKE_FUNDAMENTAL(INT): return &gUnsignedLongLongIntDeclaration;
	case MAKE_FUNDAMENTAL(WCHAR_T): return &gWCharTDeclaration;
	case MAKE_FUNDAMENTAL(BOOL): return &gBoolDeclaration;
	case MAKE_FUNDAMENTAL(FLOAT): return &gFloatDeclaration;
	case MAKE_FUNDAMENTAL(DOUBLE): return &gDoubleDeclaration;
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(DOUBLE): return &gLongDoubleDeclaration;
	case MAKE_FUNDAMENTAL(VOID): return &gVoidDeclaration;
	case MAKE_FUNDAMENTAL(INT64):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(INT64): return &gSignedLongLongIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(INT64): return &gUnsignedLongLongIntDeclaration;
	}
	SYMBOLS_ASSERT(false);
	return 0;
}

#define MAKE_INTEGERLITERALSUFFIX(token) (1 << cpp::simple_type_specifier_builtin::token)

inline bool isHexadecimalLiteral(const char* value)
{
	return *value++ == '0'
		&& (*value == 'x' || *value == 'X');
}

inline bool isFloatingLiteral(const char* value)
{
	if(!isHexadecimalLiteral(value))
	{
		const char* p = value;
		for(; *p != '\0'; ++p)
		{
			if(std::strchr(".eE", *p) != 0)
			{
				return true;
			}
		}
	}
	return false;
}


inline const char* getIntegerLiteralSuffix(const char* value)
{
	const char* p = value;
	for(; *p != '\0'; ++p)
	{
		if(std::strchr("ulUL", *p) != 0)
		{
			break;
		}
	}
	return p;
}

inline const UniqueTypeId& getIntegerLiteralSuffixType(const char* suffix)
{
	if(*suffix == '\0')
	{
		return gSignedInt; // TODO: return long on overflow
	}
	if(*(suffix + 1) == '\0') // u U l L
	{
		return *suffix == 'u' || *suffix == 'U' ? gUnsignedInt : gSignedLongInt; // TODO: return long/unsigned on overflow
	}
	if(*(suffix + 2) == '\0') // ul lu uL Lu Ul lU UL LU
	{
		return gUnsignedLongInt; // TODO: long long
	}
	throw SymbolsError();
}

inline const UniqueTypeId& getIntegerLiteralType(const char* value)
{
	return getIntegerLiteralSuffixType(getIntegerLiteralSuffix(value));
}

inline IntegralConstantExpression parseIntegerLiteral(const char* value)
{
	char* suffix;
	IntegralConstant result(strtol(value, &suffix, 0)); // TODO: handle overflow
	return IntegralConstantExpression(getIntegerLiteralSuffixType(suffix), result);
}

inline const char* getFloatingLiteralSuffix(const char* value)
{
	const char* p = value;
	for(; *p != '\0'; ++p)
	{
		if(std::strchr("flFL", *p) != 0)
		{
			break;
		}
	}
	return p;
}

inline const UniqueTypeId& getFloatingLiteralSuffixType(const char* suffix)
{
	if(*suffix == '\0')
	{
		return gDouble;
	}
	if(*(suffix + 1) == '\0') // f F l L
	{
		return *suffix == 'f' || *suffix == 'F' ? gFloat : gLongDouble;
	}
	throw SymbolsError();
}

inline const UniqueTypeId& getFloatingLiteralType(const char* value)
{
	return getFloatingLiteralSuffixType(getFloatingLiteralSuffix(value));
}

inline IntegralConstantExpression parseFloatingLiteral(const char* value)
{
	char* suffix;
	IntegralConstant result(strtod(value, &suffix)); // TODO: handle overflow
	return IntegralConstantExpression(getFloatingLiteralSuffixType(suffix), result);
}

inline const UniqueTypeId& getCharacterLiteralType(const char* value)
{
	return *value == 'L' ? gWCharT : gSignedChar; // TODO: multicharacter literal
}

inline IntegralConstantExpression parseCharacterLiteral(const char* value)
{
	IntegralConstant result;
	// TODO: parse character value
	return IntegralConstantExpression(getCharacterLiteralType(value), result);
}

inline const UniqueTypeId& getNumericLiteralType(cpp::numeric_literal* symbol)
{
	const char* value = symbol->value.value.c_str();
	switch(symbol->id)
	{
	case cpp::numeric_literal::INTEGER: return getIntegerLiteralType(value);
	case cpp::numeric_literal::CHARACTER: return getCharacterLiteralType(value);
	case cpp::numeric_literal::FLOATING: return getFloatingLiteralType(value);
	case cpp::numeric_literal::BOOLEAN: return gBool;
	default: break;
	}
	throw SymbolsError();
}

inline IntegralConstantExpression parseBooleanLiteral(const char* value)
{
	return IntegralConstantExpression(gBool, IntegralConstant(*value == 't' ? 1 : 0));
}

inline IntegralConstantExpression parseNumericLiteral(cpp::numeric_literal* symbol)
{
	const char* value = symbol->value.value.c_str();
	switch(symbol->id)
	{
	case cpp::numeric_literal::INTEGER: return parseIntegerLiteral(value);
	case cpp::numeric_literal::CHARACTER: return parseCharacterLiteral(value);
	case cpp::numeric_literal::FLOATING: return parseFloatingLiteral(value);
	case cpp::numeric_literal::BOOLEAN: return parseBooleanLiteral(value);
	default: break;
	}
	throw SymbolsError();
}

inline const UniqueTypeId& getStringLiteralType(cpp::string_literal* symbol)
{
	const char* value = symbol->value.value.c_str();
	return *value == 'L' ? gWideStringLiteral : gStringLiteral;
}



extern Declaration gDependentType;
extern const DeclarationInstance gDependentTypeInstance;
extern Declaration gDependentObject;
extern const DeclarationInstance gDependentObjectInstance;
extern Declaration gDependentTemplate;
extern const DeclarationInstance gDependentTemplateInstance;
extern Declaration gDependentNested;
extern const DeclarationInstance gDependentNestedInstance;
extern Declaration gDependentNestedTemplate;
extern const DeclarationInstance gDependentNestedTemplateInstance;

extern Declaration gParam;
extern Declaration gNonType;


#define TYPE_PARAM TypeId(&gParam, TREEALLOCATOR_NULL)


// objects
extern Name gOperatorNewId;
extern Name gOperatorDeleteId;
extern Name gOperatorNewArrayId;
extern Name gOperatorDeleteArrayId;
extern Name gOperatorPlusId;
extern Name gOperatorMinusId;
extern Name gOperatorStarId;
extern Name gOperatorDivideId;
extern Name gOperatorPercentId;
extern Name gOperatorXorId;
extern Name gOperatorAndId;
extern Name gOperatorOrId;
extern Name gOperatorComplId;
extern Name gOperatorNotId;
extern Name gOperatorAssignId;
extern Name gOperatorLessId;
extern Name gOperatorGreaterId;
extern Name gOperatorPlusAssignId;
extern Name gOperatorMinusAssignId;
extern Name gOperatorStarAssignId;
extern Name gOperatorDivideAssignId;
extern Name gOperatorPercentAssignId;
extern Name gOperatorXorAssignId;
extern Name gOperatorAndAssignId;
extern Name gOperatorOrAssignId;
extern Name gOperatorShiftLeftId;
extern Name gOperatorShiftRightId;
extern Name gOperatorShiftRightAssignId;
extern Name gOperatorShiftLeftAssignId;
extern Name gOperatorEqualId;
extern Name gOperatorNotEqualId;
extern Name gOperatorLessEqualId;
extern Name gOperatorGreaterEqualId;
extern Name gOperatorAndAndId;
extern Name gOperatorOrOrId;
extern Name gOperatorPlusPlusId;
extern Name gOperatorMinusMinusId;
extern Name gOperatorCommaId;
extern Name gOperatorArrowStarId;
extern Name gOperatorArrowId;
extern Name gOperatorFunctionId;
extern Name gOperatorArrayId;

inline Name getOverloadableOperatorId(cpp::overloadable_operator_default* symbol)
{
	switch(symbol->id)
	{
	case cpp::overloadable_operator_default::ASSIGN: return gOperatorAssignId;
	case cpp::overloadable_operator_default::STARASSIGN: return gOperatorStarAssignId;
	case cpp::overloadable_operator_default::DIVIDEASSIGN: return gOperatorDivideAssignId;
	case cpp::overloadable_operator_default::PERCENTASSIGN: return gOperatorPercentAssignId;
	case cpp::overloadable_operator_default::PLUSASSIGN: return gOperatorPlusAssignId;
	case cpp::overloadable_operator_default::MINUSASSIGN: return gOperatorMinusAssignId;
	case cpp::overloadable_operator_default::SHIFTRIGHTASSIGN: return gOperatorShiftRightAssignId;
	case cpp::overloadable_operator_default::SHIFTLEFTASSIGN: return gOperatorShiftLeftAssignId;
	case cpp::overloadable_operator_default::ANDASSIGN: return gOperatorAndAssignId;
	case cpp::overloadable_operator_default::XORASSIGN: return gOperatorXorAssignId;
	case cpp::overloadable_operator_default::ORASSIGN: return gOperatorOrAssignId;
	case cpp::overloadable_operator_default::EQUAL: return gOperatorEqualId;
	case cpp::overloadable_operator_default::NOTEQUAL: return gOperatorNotEqualId;
	case cpp::overloadable_operator_default::LESS: return gOperatorLessId;
	case cpp::overloadable_operator_default::GREATER: return gOperatorGreaterId;
	case cpp::overloadable_operator_default::LESSEQUAL: return gOperatorLessEqualId;
	case cpp::overloadable_operator_default::GREATEREQUAL: return gOperatorGreaterEqualId;
	case cpp::overloadable_operator_default::ANDAND: return gOperatorAndAndId;
	case cpp::overloadable_operator_default::OROR: return gOperatorOrOrId;
	case cpp::overloadable_operator_default::PLUSPLUS: return gOperatorPlusPlusId;
	case cpp::overloadable_operator_default::MINUSMINUS: return gOperatorMinusMinusId;
	case cpp::overloadable_operator_default::STAR: return gOperatorStarId;
	case cpp::overloadable_operator_default::DIVIDE: return gOperatorDivideId;
	case cpp::overloadable_operator_default::PERCENT: return gOperatorPercentId;
	case cpp::overloadable_operator_default::PLUS: return gOperatorPlusId;
	case cpp::overloadable_operator_default::MINUS: return gOperatorMinusId;
	case cpp::overloadable_operator_default::SHIFTLEFT: return gOperatorShiftLeftId;
	case cpp::overloadable_operator_default::SHIFTRIGHT: return gOperatorShiftRightId;
	case cpp::overloadable_operator_default::AND: return gOperatorAndId;
	case cpp::overloadable_operator_default::OR: return gOperatorOrId;
	case cpp::overloadable_operator_default::XOR: return gOperatorXorId;
	case cpp::overloadable_operator_default::NOT: return gOperatorNotId;
	case cpp::overloadable_operator_default::COMPL: return gOperatorComplId;
	case cpp::overloadable_operator_default::ARROW: return gOperatorArrowId;
	case cpp::overloadable_operator_default::ARROWSTAR: return gOperatorArrowStarId;
	case cpp::overloadable_operator_default::COMMA: return gOperatorCommaId;
	default: break;
	}
	throw SymbolsError();
}

inline Name getOverloadableOperatorId(cpp::new_operator* symbol)
{
	if(symbol->array.p != 0)
	{
		return gOperatorNewArrayId;
	}
	return gOperatorNewId;
}

inline Name getOverloadableOperatorId(cpp::delete_operator* symbol)
{
	if(symbol->array.p != 0)
	{
		return gOperatorDeleteArrayId;
	}
	return gOperatorDeleteId;
}

inline Name getOverloadableOperatorId(cpp::function_operator* symbol)
{
	return gOperatorFunctionId;
}

inline Name getOverloadableOperatorId(cpp::array_operator* symbol)
{
	return gOperatorArrayId;
}

inline Name getUnaryOperatorName(cpp::unary_operator* symbol)
{
	switch(symbol->id)
	{
	case cpp::unary_operator::PLUSPLUS: return gOperatorPlusPlusId;
	case cpp::unary_operator::MINUSMINUS: return gOperatorMinusMinusId;
	case cpp::unary_operator::STAR: return gOperatorStarId;
	case cpp::unary_operator::AND: return gOperatorAndId;
	case cpp::unary_operator::PLUS: return gOperatorPlusId;
	case cpp::unary_operator::MINUS: return gOperatorMinusId;
	case cpp::unary_operator::NOT: return gOperatorNotId;
	case cpp::unary_operator::COMPL: return gOperatorComplId;
	default: break;
	}
	throw SymbolsError();
}



extern Identifier gConversionFunctionId;
extern Identifier gOperatorFunctionTemplateId;
// TODO: don't declare if id is anonymous?
extern Identifier gAnonymousId;


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

inline UniqueTypeWrapper binaryOperatorNull(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return gUniqueTypeNull;
}

inline UniqueTypeWrapper ternaryOperatorNull(UniqueTypeWrapper first, UniqueTypeWrapper second, UniqueTypeWrapper third)
{
	return gUniqueTypeNull;
}



inline bool isClass(UniqueTypeWrapper type)
{
	return type.isSimple() && getObjectType(type.value).declaration->type.declaration == &gClass;
}

inline bool isEnum(UniqueTypeWrapper type)
{
	return type.isSimple() && getObjectType(type.value).declaration->type.declaration == &gEnum;
}

inline bool isComplete(const UniqueTypeId& type)
{
	return type.isSimple() && isComplete(*getObjectType(type.value).declaration);
}

inline bool isArithmetic(const UniqueTypeId& type)
{
	return type.isSimple() && getObjectType(type.value).declaration->type.declaration == &gArithmetic;
}

inline bool isFloating(const UniqueTypeId& type)
{
	return isEqual(type, gFloat)
		|| isEqual(type, gDouble)
		|| isEqual(type, gLongDouble);
}

inline bool isIntegral(const UniqueTypeId& type)
{
	return isArithmetic(type) && !isFloating(type);
}

inline bool isEnumerator(const UniqueTypeId& type)
{
	return type.isSimple() && getObjectType(type.value).declaration == &gEnumerator;
}

inline bool isEnumeration(const UniqueTypeId& type)
{
	return (type.isSimple() && getObjectType(type.value).declaration->type.declaration == &gEnum)
		|| isEnumerator(type); // TODO: remove when enumerators are correctly typed
}


// int i; // type -> int
// typedef int I; // type -> int
// I i; // type -> I -> int
// typedef I J; // type -> I -> int
// J j; // type -> J -> I -> int
// struct S; // type -> struct
// typedef struct S S; // type -> S -> struct
// typedef struct S {} S; // type -> S -> struct
// typedef enum E {} E; // type -> E -> enum

// returns the type of a declaration
// int i; -> built-in
// class A a; -> A
// enum E e; -> E
// typedef int T; T t; -> built-in
inline const Declaration* getType(const Declaration& declaration)
{
	if(declaration.specifiers.isTypedef)
	{
		return getType(*declaration.type.declaration);
	}
	return declaration.type.declaration;
}

inline const Type& getUnderlyingType(const Type& type)
{
	if(type.declaration->specifiers.isTypedef
		&& type.declaration->templateParameter == INDEX_INVALID)
	{
		return getUnderlyingType(type.declaration->type);
	}
	return type;
}


inline bool isEqual(const TypeId& l, const TypeId& r)
{
	SYMBOLS_ASSERT(l.unique != 0);
	SYMBOLS_ASSERT(r.unique != 0);
	return l.unique == r.unique;
}

inline bool isEqual(const TemplateArgument& l, const TemplateArgument& r)
{
	if((l.type.declaration == &gNonType)
		!= (r.type.declaration == &gNonType))
	{
		return false;
	}
	return l.type.declaration == &gNonType
		? l.expression.p == r.expression.p
		: isEqual(l.type, r.type);
}

inline bool matchTemplateSpecialization(const Declaration& declaration, const TemplateArguments& arguments)
{
	// TODO: check that all arguments are specified!
	TemplateArguments::const_iterator a = arguments.begin();
	for(TemplateArguments::const_iterator i = declaration.templateArguments.begin(); i != declaration.templateArguments.end(); ++i)
	{
		SYMBOLS_ASSERT(a != arguments.end()); // a template-specialization must have no more arguments than the template parameters

		if(!isEqual(*i, *a))
		{
			return false;
		}
		++a;
	}
	SYMBOLS_ASSERT(a == arguments.end());
	return true;
}

inline Declaration* findTemplateSpecialization(Declaration* declaration, const TemplateArguments& arguments)
{
	for(; declaration != 0; declaration = declaration->overloaded)
	{
		if(!isSpecialization(*declaration))
		{
			continue;
		}

		if(matchTemplateSpecialization(*declaration, arguments))
		{
			return declaration;
		}
	}
	return 0;
}

// ----------------------------------------------------------------------------
// implicit conversion sequence

enum IcsRank
{
	ICSRANK_STANDARDEXACT,
	ICSRANK_STANDARDPROMOTION,
	ICSRANK_STANDARDCONVERSION,
	ICSRANK_USERDEFINED,
	ICSRANK_ELLIPSIS,
	ICSRANK_INVALID,
};

// [over.ics.scs]
enum ScsRank
{
	SCSRANK_IDENTITY, // no conversion
	SCSRANK_EXACT,
	SCSRANK_PROMOTION,
	SCSRANK_CONVERSION,
	SCSRANK_INVALID,
};

struct StandardConversionSequence
{
	ScsRank rank;
	CvQualifiers adjustment; // TODO: cv-qualification signature for multi-level pointer type
	StandardConversionSequence(ScsRank rank, CvQualifiers adjustment)
		: rank(rank), adjustment(adjustment)
	{
	}
};

const StandardConversionSequence STANDARDCONVERSIONSEQUENCE_INVALID = StandardConversionSequence(SCSRANK_INVALID, CvQualifiers());

inline bool findBase(const TypeInstance& other, const TypeInstance& type)
{
	SYMBOLS_ASSERT(other.declaration != &gParam); // TODO: when type-evaluation fails, sometimes template-params are uniqued
	SYMBOLS_ASSERT(other.declaration->enclosed != 0);
	SYMBOLS_ASSERT(isClass(*type.declaration));
	for(UniqueBases::const_iterator i = other.bases.begin(); i != other.bases.end(); ++i)
	{
		const TypeInstance& base = *(*i);
		SYMBOLS_ASSERT(isClass(*base.declaration));
		if(&base == &type)
		{
			return true;
		}
		if(findBase(base, type))
		{
			return true;
		}
	}
	return false;
}

// Returns true if 'type' is a base of 'other'
inline bool isBaseOf(const TypeInstance& type, const TypeInstance& other, Location source, const TypeInstance* enclosing)
{
	if(!isClass(*type.declaration)
		|| !isClass(*other.declaration))
	{
		return false;
	}
	if(isIncomplete(*type.declaration)
		|| isIncomplete(*other.declaration))
	{
		return false;
	}
	instantiateClass(other, source, enclosing);
	return findBase(other, type);
}

// 4.5 Integral Promotions
// TODO: handle bitfield types?
inline const UniqueTypeId& promoteToIntegralType(const UniqueTypeId& type)
{
	if(isEqual(type, gChar)
		|| isEqual(type, gSignedChar)
		|| isEqual(type, gUnsignedChar)
		|| isEqual(type, gSignedShortInt)
		|| isEqual(type, gUnsignedShortInt))
	{
		return gSignedInt;
	}
	if(isEqual(type, gWCharT)
		|| isEnumeration(type))
	{
		return gSignedInt;
	}
	if(isEqual(type, gBool))
	{
		return gSignedInt;
	}
	return type;
}

inline UniqueTypeWrapper applyArrayToPointerConversion(UniqueTypeWrapper type)
{
	SYMBOLS_ASSERT(type.isArray());
	return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, getInner(type.value), PointerType())); // T[] -> T*
}

inline UniqueTypeWrapper applyFunctionToPointerConversion(UniqueTypeWrapper type)
{
	SYMBOLS_ASSERT(type.isFunction());
	return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, type.value, PointerType())); // T() -> T(*)()
}

inline UniqueTypeWrapper applyLvalueTransformation(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	if(to.isPointer()
		&& from.isArray())
	{
		return applyArrayToPointerConversion(from);
	}
	if(to.isFunctionPointer()
		&& from.isFunction())
	{
		return applyFunctionToPointerConversion(from);
	}
	return from;
}

inline CvQualifiers makeQualificationAdjustment(UniqueTypeId to, UniqueTypeId from)
{
	return CvQualifiers(to.value.getQualifiers().isConst > from.value.getQualifiers().isConst,
		to.value.getQualifiers().isVolatile > from.value.getQualifiers().isVolatile);
}

inline bool isGreaterCvQualification(UniqueTypeId to, UniqueTypeId from)
{
	return to.value.getQualifiers().isConst > from.value.getQualifiers().isConst
		|| to.value.getQualifiers().isVolatile > from.value.getQualifiers().isVolatile;
}

inline bool isEqualCvQualification(UniqueTypeId to, UniqueTypeId from)
{
	return to.value.getQualifiers() == from.value.getQualifiers();
}



inline StandardConversionSequence makeScsPromotion(UniqueTypeId to, UniqueTypeId from)
{
	if(isArithmetic(from) && from.isSimple()
		&& isArithmetic(to) && to.isSimple()
		&& (isEqual(from, gFloat) && isEqual(to, gDouble))
			|| (isEqual(promoteToIntegralType(from), to)))
	{
		return StandardConversionSequence(SCSRANK_PROMOTION, CvQualifiers());
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

inline StandardConversionSequence makeScsConversion(Location source, const TypeInstance* enclosing, UniqueTypeId to, UniqueTypeId from, bool isNullPointerConstant = false) // TODO: detect null pointer constant
{
	SYMBOLS_ASSERT(to.value.getQualifiers() == CvQualifiers());
	SYMBOLS_ASSERT(from.value.getQualifiers() == CvQualifiers());
	if((isArithmetic(from) || isEnumeration(from)) && from.isSimple()
		&& isArithmetic(to) && to.isSimple())
	{
		// can convert from enumeration to integer/floating/bool, but not in reverse
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers());
	}
	if((to.isPointer() || to.isMemberPointer())
		&& isIntegral(from)
		&& isNullPointerConstant)
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // 0 -> T*
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& getInner(to.value) == gVoid.value)
	{
		to = UniqueTypeWrapper(getInner(to.value));
		from = UniqueTypeWrapper(getInner(from.value));
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from))
			: STANDARDCONVERSIONSEQUENCE_INVALID; // T* -> void*
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& isBaseOf(getObjectType(getInner(to.value)), getObjectType(getInner(from.value)), source, enclosing))
	{
		to = UniqueTypeWrapper(getInner(to.value));
		from = UniqueTypeWrapper(getInner(from.value));
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from))
			: STANDARDCONVERSIONSEQUENCE_INVALID; // D* -> B*
	}
	if(to.isMemberPointer()
		&& from.isMemberPointer()
		&& isBaseOf(getMemberPointerClass(to.value), getMemberPointerClass(from.value), source, enclosing))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // D::* -> B::*
	}
	if(isEqual(to, gBool)
		&& (from.isPointer() || from.isMemberPointer()))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // T* -> bool, T::* -> bool
	}
	if(to.isSimple()
		&& from.isSimple()
		&& isBaseOf(getObjectType(to.value), getObjectType(from.value), source, enclosing))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // D -> B
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

// exact
// T& <- T
// T& <- T&
// const T& <- T
// const T& <- T&
// derived to base conversion
// B& <- D
// B& <- D&
// const B& <- D
// const B& <- D&
// invalid
// T& <- const T
// T& <- const T&
// B& <- const D
// B& <- const D&

// [conv]
// exact
// T <- T
// T* <- T[]
// T(*)() <- T()
// const T* <- T*
// T C::const*  <- T C::*
// multi-level pointer / member-pointer
// derived to base conversion
// B <- D

inline StandardConversionSequence makeScsExactMatch(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	for(;;)
	{
		if(to.value.getPointer() == from.value.getPointer())
		{
			return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
				? StandardConversionSequence(SCSRANK_EXACT, makeQualificationAdjustment(to, from))
				: STANDARDCONVERSIONSEQUENCE_INVALID;
		}
		if(to.isPointer()
			&& from.isPointer())
		{
		}
		else if(to.isMemberPointer()
			&& from.isMemberPointer()
			&& &getMemberPointerClass(to.value) == &getMemberPointerClass(from.value))
		{
		}
		else
		{
			break;
		}
		to = UniqueTypeWrapper(getInner(to.value));
		from = UniqueTypeWrapper(getInner(from.value));
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

// 13.3.3.1 [over.best.ics]
inline StandardConversionSequence makeStandardConversionSequence(UniqueTypeWrapper to, UniqueTypeWrapper from, Location source, const TypeInstance* enclosing, bool isNullPointerConstant = false, bool isLvalue = false)
{
	// TODO: user-defined conversion
	if(from.value == UNIQUETYPE_NULL)
	{
		return STANDARDCONVERSIONSEQUENCE_INVALID; // TODO: assert
	}
	// 13.3.3.1.4 [over.ics.ref]: reference binding
	if(to.isReference()) 
	{
		to = UniqueTypeWrapper(getInner(to.value));
		if(from.isReference())
		{
			isLvalue = true;
			from = UniqueTypeWrapper(getInner(from.value)); // TODO: removal of reference won't be detected later
		}
		// 8.5.3 [dcl.init.ref]
		// does it directly bind?
		if(isLvalue
			&& (isEqualCvQualification(to, from)
				|| isGreaterCvQualification(to, from))) // TODO: track 'added qualification' if qualification is greater
		{
			if(to.value.getPointer() == from.value.getPointer())
			{
				return StandardConversionSequence(SCSRANK_EXACT, makeQualificationAdjustment(to, from));
			}
			if(to.isSimple()
				&& from.isSimple()
				&& isBaseOf(getObjectType(to.value), getObjectType(from.value), source, enclosing))
			{
				return StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from));
			}
		}
		// if not bound directly, a standard conversion is required (which produces an rvalue)
		if(!to.value.getQualifiers().isConst
			|| to.value.getQualifiers().isVolatile) // 8.5.3-5: otherwise, the reference shall be to a non-volatile const type
		{
			// can't bind rvalue to a non-const reference
			return STANDARDCONVERSIONSEQUENCE_INVALID;
		}
	}

	if(!to.isReference()
		&& from.isReference())
	{
		from = UniqueTypeWrapper(getInner(from.value)); // T& -> T
	}

	// ignore top level cv-qualifiers
	to.value.setQualifiers(CvQualifiers());
	from.value.setQualifiers(CvQualifiers());

	UniqueTypeWrapper tmp = from;
	from = applyLvalueTransformation(to, from);

	if(tmp == from // no l-value transformation
		&& to == from) // no other conversions required
	{
		return StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers());
	}

	{
		StandardConversionSequence result = makeScsExactMatch(to, from);
		if(result.rank != SCSRANK_INVALID)
		{
			return result;
		}
	}
	{
		StandardConversionSequence result = makeScsPromotion(to, from);
		if(result.rank != SCSRANK_INVALID)
		{
			return result;
		}
	}
	{
		StandardConversionSequence result = makeScsConversion(source, enclosing, to, from, isNullPointerConstant); // TODO: ordering of conversions by inheritance distance
		if(result.rank != SCSRANK_INVALID)
		{
			return result;
		}
	}

	// TODO: user-defined
	// TODO: ellipsis
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

inline IcsRank getIcsRank(UniqueTypeWrapper to, UniqueTypeWrapper from, Location source, const TypeInstance* enclosing, bool isNullPointerConstant = false, bool isLvalue = false)
{
	StandardConversionSequence sequence = makeStandardConversionSequence(to, from, source, enclosing, isNullPointerConstant, isLvalue);
	switch(sequence.rank)
	{
	case SCSRANK_IDENTITY:
	case SCSRANK_EXACT: return ICSRANK_STANDARDEXACT;
	case SCSRANK_PROMOTION: return ICSRANK_STANDARDPROMOTION;
	case SCSRANK_CONVERSION: return ICSRANK_STANDARDCONVERSION;
	}
	return ICSRANK_INVALID;
}

inline bool isProperSubsequence(CvQualifiers l, CvQualifiers r)
{
	return (!l.isConst && r.isConst)
		|| (!l.isVolatile && r.isVolatile);
}

inline bool isProperSubsequence(const StandardConversionSequence& l, const StandardConversionSequence& r)
{
	return (l.rank == SCSRANK_IDENTITY && r.rank != SCSRANK_IDENTITY)
		|| isProperSubsequence(l.adjustment, r.adjustment);
}

// [over.ics.rank]
inline bool isBetter(const StandardConversionSequence& l, const StandardConversionSequence& r)
{
	// TODO: assert rank not INVALID
	if(isProperSubsequence(l, r))
	{
		return true;
	}
	if(l.rank < r.rank) // TODO: ranking derived->base conversions by inheritance distance
	{
		return true;
	}
	// TODO: both sequences are similar references, but differ only in cv-qualification
	// TODO: user-defined conversion sequence ranking
	return false;
}

enum IcsType
{
	ICSTYPE_STANDARD,
	ICSTYPE_USERDEFINED,
	ICSTYPE_ELLIPSIS,
};

struct ImplicitConversion
{
	StandardConversionSequence sequence;
	IcsType type;
	ImplicitConversion(StandardConversionSequence sequence, IcsType type = ICSTYPE_STANDARD)
		: sequence(sequence), type(type)
	{
	}
};

const ImplicitConversion IMPLICITCONVERSION_USERDEFINED = ImplicitConversion(StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers()), ICSTYPE_USERDEFINED); // TODO

// [over.ics.rank]
inline bool isBetter(const ImplicitConversion& l, const ImplicitConversion& r)
{
	if(l.type < r.type)
	{
		return true;
	}
	return isBetter(l.sequence, r.sequence);
}

inline bool isValid(const ImplicitConversion& conversion)
{
	return conversion.sequence.rank != SCSRANK_INVALID;
}


typedef std::vector<ImplicitConversion> ArgumentConversions;

struct FunctionOverload
{
	Declaration* declaration;
	UniqueTypeWrapper type;
	FunctionOverload(Declaration* declaration, UniqueTypeWrapper type)
		: declaration(declaration), type(type)
	{
	}
};

struct CandidateFunction : FunctionOverload
{
	UniqueTypeWrapper type;
	ArgumentConversions conversions;
	bool isTemplate;
	CandidateFunction()
		: FunctionOverload(0, gUniqueTypeNull)
	{
	}
	CandidateFunction(FunctionOverload overload)
		: FunctionOverload(overload), isTemplate(false)
	{
	}
};

inline bool isBetter(const CandidateFunction& l, const CandidateFunction& r)
{
	SYMBOLS_ASSERT(l.conversions.size() == r.conversions.size());
	for(size_t i = 0; i != l.conversions.size(); ++i)
	{
		if(isBetter(r.conversions[i], l.conversions[i]))
		{
			return false; // at least one argument is not a better conversion sequence
		}
	}
	for(size_t i = 0; i != l.conversions.size(); ++i)
	{
		if(isBetter(l.conversions[i], r.conversions[i]))
		{
			return true; // at least one argument is a better conversion sequence
		}
	}
	if(!l.isTemplate && r.isTemplate)
	{
		return true; // non-template better than template
	}
	// TODO: ordering of template specialisations
	// TODO: in context of initialisation by user defined conversion, consider return type
	return false;
}


inline const char* getDeclarationType(const Declaration& declaration)
{
	if(isNamespace(declaration))
	{
		return "namespace";
	}
	if(isType(declaration))
	{
		return declaration.isTemplate ? "templateName" : "type";
	}
	return "object";
}

inline bool isAnonymous(const Declaration& declaration)
{
	return *declaration.getName().value.c_str() == '$';
}

struct DeclarationError
{
	const char* description;
	DeclarationError(const char* description) : description(description)
	{
	}
};

inline const Declaration& getPrimaryDeclaration(const Declaration& first, const Declaration& second)
{
	if(isNamespace(first))
	{
		if(!isNamespace(second))
		{
			throw DeclarationError("non-namespace already declared as namespace");
		}
		return first; // namespace continuation
	}
	if(isType(first))
	{
		if(!isType(second))
		{
			throw DeclarationError("non-type already declared as type");
		}
		if(getType(first) != getType(second))
		{
			throw DeclarationError("type already declared as different type");
		}
		if(isTypedef(first))
		{
			return second; // redeclaration of typedef, or definition of type previously used in typedef
		}
		if(isTypedef(second))
		{
			return second; // typedef of type previously declared: typedef struct S {} S;
		}
		if(isClass(first))
		{
			if(isSpecialization(second))
			{
				return second; // TODO: class template partial/explicit-specialization
			}
			if(isSpecialization(first))
			{
				return second; // TODO: class template partial/explicit-specialization
			}
			if(isIncomplete(second))
			{
				return second; // redeclaration of previously-declared class
			}
			if(isIncomplete(first))
			{
				return second; // definition of forward-declared class
			}
			throw DeclarationError("class-definition already defined");
		}
		if(isEnum(first))
		{
			throw DeclarationError("enum-definition already defined");
		}
		throw SymbolsError(); // should not be reachable
	}
	if(isType(second))
	{
		throw DeclarationError("type already declared as non-type");
	}
	if(isFunction(first)
		|| isFunction(second))// TODO: function overloading
	{
		return second; // multiple declarations allowed
	}
#if 0 // fails when comparing types if type is template-param dependent
	if(getBaseType(first) != getBaseType(second))
	{
		throw DeclarationError("variable already declared with different type");
	}
#endif
	if(isStaticMember(first))
	{
		// TODO: disallow inline definition of static member: class C { static int i; int i; };
		if(!isMemberObject(second))
		{
			throw DeclarationError("non-member-object already declared as static member-object");
		}
		return second; // multiple declarations allowed
	}
	if(isExtern(first)
		|| isExtern(second))
	{
		return second; // multiple declarations allowed
	}
	// HACK: ignore multiple declarations for members of template - e.g. const char Tmpl<char>::VALUE; const int Tmpl<int>::VALUE;
	if(!first.templateParams.defaults.empty())
	{
		// if enclosing is a template
		return first;
	}
	throw DeclarationError("symbol already defined");
}

inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter, std::size_t visibility)
{
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		SYMBOLS_ASSERT((*i).second.ordering != UINT_MAX);
		if((*i).second.ordering < visibility // if this declaration was visible at the specified point
			&& filter(*(*i).second)) // and this declaration is not filtered
		{
			return &(*i).second;
		}
	}

	return 0;
}

// find a declaration within a scope.
inline LookupResult findDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	result.filtered = findDeclaration(scope.declarations, id, filter);
	return result;
}

inline LookupResult findNamespaceDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny());


// find a declaration within the set of using-directives present in a namespace
inline LookupResult findDeclaration(Scope::Scopes& scopes, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	for(Scope::Scopes::iterator i = scopes.begin(); i != scopes.end(); ++i)
	{
		Scope& scope = *(*i);

#ifdef LOOKUP_DEBUG
		std::cout << "searching '";
		printName(scope);
		std::cout << "'" << std::endl;
#endif

		if(result.append(findNamespaceDeclaration(scope, id, filter)))
		{
			return result;
		}
	}
	return result;
}

// find a declaration within a namespace scope. Does not search enclosing scopes.
inline LookupResult findNamespaceDeclaration(Scope& scope, const Identifier& id, LookupFilter filter)
{
	SYMBOLS_ASSERT(scope.type == SCOPETYPE_NAMESPACE);
	LookupResult result;
	if(result.append(::findDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(result.append(::findDeclaration(scope.usingDirectives, id, filter)))
	{
		return result;
	}
	return result;
}

inline LookupResult findMemberDeclaration(Scope& scope, const Identifier& id, LookupFilter filter)
{
	LookupResult result;
	if(result.append(findDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(scope.type != SCOPETYPE_CLASS)
	{
		return result;
	}
	for(Types::iterator i = scope.bases.begin(); i != scope.bases.end(); ++i)
	{
		SYMBOLS_ASSERT((*i).unique != 0);
		if((*i).isDependent) // if base class is dependent
		{
			continue;
		}
		const TypeInstance& base = getObjectType((*i).unique);

		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->getName().value == id.value
			&& filter(*base.declaration))
		{
			result.filtered = base.declaration->getName().dec.p;
			result.enclosing = base.enclosing;
			return result;
		}
		Scope* scope = base.declaration->enclosed;
		if(scope != 0)
		{
			// [namespace.udir] A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.
			SYMBOLS_ASSERT(scope->usingDirectives.empty());
			if(result.append(findMemberDeclaration(*scope, id, filter)))
			{
				return result;
			}
		}
	}
	return result;
}

// find a declaration within a class or namespace
inline LookupResult findClassOrNamespaceMemberDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny())
{
#ifdef LOOKUP_DEBUG
	std::cout << "searching '";
	printName(scope);
	std::cout << "'" << std::endl;
#endif

	LookupResult result;
	if(result.append(findMemberDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(scope.parent != 0)
	{
		if(result.append(findClassOrNamespaceMemberDeclaration(*scope.parent, id, filter)))
		{
			return result;
		}
	}
	/* basic.lookup.unqual
	The declarations from the namespace nominated by a using-directive become visible in a namespace enclosing
	the using-directive; see 7.3.4. For the purpose of the unqualified name lookup rules described in 3.4.1, the
	declarations from the namespace nominated by the using-directive are considered members of that enclosing
	namespace.
	*/
	result.append(findDeclaration(scope.usingDirectives, id, filter));
	return result;
}

inline bool hasTemplateParamDefaults(const TemplateParameters& params)
{
	for(TemplateArguments::const_iterator i = params.defaults.begin(); i != params.defaults.end(); ++i)
	{
		if((*i).type.declaration != 0)
		{
			return true;
		}
	}
	return false;
}

// substitute references to template-parameters of 'otherParams' for template-parameters of 'params'
inline void fixTemplateParamDefault(TemplateArgument& argument, const TemplateParameters& params, const TemplateParameters& otherParams)
{
	if(argument.type.declaration == 0)
	{
		return;
	}
	std::size_t index = argument.type.declaration->templateParameter;
	if(index != INDEX_INVALID)
	{
		Types::const_iterator i = params.begin();
		std::advance(i, index);
		Types::const_iterator j = otherParams.begin();
		std::advance(j, index);
		if(argument.type.declaration->scope == (*j).declaration->scope)
		{
			argument.type.declaration = (*i).declaration;
		}
	}
	for(TemplateArguments::iterator i = argument.type.templateArguments.begin(); i != argument.type.templateArguments.end(); ++i)
	{
		fixTemplateParamDefault(*i, params, otherParams);
	}
}

inline void copyTemplateParamDefault(TemplateArgument& argument, const TemplateArgument& otherArgument, const TemplateParameters& params, const TemplateParameters& otherParams)
{
	argument = otherArgument;
	fixTemplateParamDefault(argument, params, otherParams);
}

inline void copyTemplateParamDefaults(TemplateParameters& params, const TemplateParameters& otherParams)
{
	SYMBOLS_ASSERT(params.defaults.empty());
	for(TemplateArguments::const_iterator i = otherParams.defaults.begin(); i != otherParams.defaults.end(); ++i)
	{
		params.defaults.push_back(TEMPLATEARGUMENT_NULL);
		copyTemplateParamDefault(params.defaults.back(), *i, params, otherParams);
	}
}

/// 14.1-10: the set of template param defaults is obtained by merging those from the definition and all declarations currently in scope (excluding explicit-specializations)
inline void mergeTemplateParamDefaults(TemplateParameters& params, const TemplateParameters& otherParams)
{
	if(params.defaults.empty())
	{
		copyTemplateParamDefaults(params, otherParams);
		return;
	}
	if(!hasTemplateParamDefaults(otherParams)) // ignore declarations with no default-arguments, e.g. explicit/partial-specializations
	{
		return;
	}
	SYMBOLS_ASSERT(!otherParams.defaults.empty());
	TemplateArguments::iterator d = params.defaults.begin();
	for(TemplateArguments::const_iterator i = otherParams.defaults.begin(); i != otherParams.defaults.end(); ++i)
	{
		SYMBOLS_ASSERT(d != params.defaults.end());
		SYMBOLS_ASSERT((*d).type.declaration == 0 || (*i).type.declaration == 0); // TODO: non-fatal error: default param defined more than once
		if((*d).type.declaration == 0)
		{
			copyTemplateParamDefault(*d, *i, params, otherParams);
		}
		++d;
	}
	SYMBOLS_ASSERT(d == params.defaults.end());
}

inline void mergeTemplateParamDefaults(Declaration& declaration, const TemplateParameters& templateParams)
{
	SYMBOLS_ASSERT(declaration.isTemplate);
	SYMBOLS_ASSERT(isClass(declaration));
	SYMBOLS_ASSERT(!isSpecialization(declaration)); // explicit/partial-specializations cannot have default-arguments
	mergeTemplateParamDefaults(declaration.templateParams, templateParams);
	SYMBOLS_ASSERT(!declaration.templateParams.defaults.empty());
}


typedef TokenPrinter<std::ostream> FileTokenPrinter;


typedef std::list<UniqueType> TypeElements;


struct SymbolPrinter : TypeElementVisitor, ExpressionNodeVisitor
{
	FileTokenPrinter& printer;
	bool escape;
	SymbolPrinter(FileTokenPrinter& printer, bool escape = false)
		: printer(printer), escape(escape)
	{
		typeStack.push_back(false);
	}
#if 0
	template<typename T>
	void printSymbol(T* symbol)
	{
		SourcePrinter walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
#endif
	void printName(const Scope* scope)
	{
		if(scope != 0
			&& scope->parent != 0)
		{
			printName(scope->parent);
			if(scope->type != SCOPETYPE_TEMPLATE)
			{
				printer.out << getValue(scope->name) << ".";
			}
		}
	}

	void visit(const IntegralConstantExpression& literal)
	{
		printer.out << literal.value.value;
	}
	void visit(const NonTypeTemplateParameter& node)
	{
		printer.out << "$i" << node.declaration->scope->templateDepth << "_" << node.declaration->templateParameter;
	}
	void visit(const DependentIdExpression& node)
	{
		printType(node.qualifying);
		printer.out << ".";
		printer.out << node.name.c_str();
		// TODO: template arguments
	}
	void visit(const IdExpression& node)
	{
		if(node.enclosing != 0)
		{
			printType(*node.enclosing);
			printer.out << ".";
		}
		printer.out << getValue(node.declaration->getName());
		// TODO: template arguments
	}
	void visit(const SizeofExpression& node)
	{
		printer.out << "sizeof(";
		printExpression(node.operand);
		printer.out << ")";
	}
	void visit(const SizeofTypeExpression& node)
	{
		printer.out << "sizeof(";
		printType(node.type);
		printer.out << ")";
	}
	void visit(const UnaryExpression& node)
	{
		printer.out << "(";
		printer.out << " " << node.operatorName.c_str() << " ";
		printExpression(node.first);
		printer.out << ")";
	}
	void visit(const BinaryExpression& node)
	{
		printer.out << "(";
		printExpression(node.first);
		printer.out << " " << node.operatorName.c_str() << " ";
		printExpression(node.second);
		printer.out << ")";
	}
	void visit(const TernaryExpression& node)
	{
		printer.out << "(";
		printExpression(node.first);
		printer.out << " " << "?" << " ";
		printExpression(node.second);
		printer.out << " " << ":" << " ";
		printExpression(node.third);
		printer.out << ")";
	}
	void visit(const TypeTraitsUnaryExpression& node)
	{
		printer.out << node.traitName.c_str();
		printer.out << "(";
		printType(node.type);
		printer.out << ")";
	}
	void visit(const TypeTraitsBinaryExpression& node)
	{
		printer.out << node.traitName.c_str();
		printer.out << "(";
		printType(node.first);
		printer.out << ", ";
		printType(node.second);
		printer.out << ")";
	}

	void printExpression(ExpressionNode* node)
	{
		node->accept(*this);
	}

	std::vector<bool> typeStack;

	void pushType(bool isPointer)
	{
		bool wasPointer = typeStack.back();
		bool parenthesise = typeStack.size() != 1 && !wasPointer && isPointer;
		if(parenthesise)
		{
			printer.out << "(";
		}
		typeStack.back() = parenthesise;
		typeStack.push_back(isPointer);
	}
	void popType()
	{
		typeStack.pop_back();
		if(typeStack.back())
		{
			printer.out << ")";
		}
	}

	std::vector<CvQualifiers> qualifierStack;

	void visit(const DependentType& element)
	{
		if(qualifierStack.back().isConst)
		{
			printer.out << "const ";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << "volatile ";
		}
		printer.out << "$T" << element.type->scope->templateDepth << "_" << element.type->templateParameter;
		visitTypeElement();
	}
	void visit(const DependentTypename& element)
	{
		printType(element.qualifying);
		printer.out << "." << element.name.c_str();
		if(element.isTemplate)
		{
			printTemplateArguments(element.templateArguments);
		}
		visitTypeElement();
	}
	void visit(const DependentNonType& element)
	{
		printExpression(element.expression);
		visitTypeElement();
	}
	void visit(const TemplateTemplateArgument& element)
	{
		printName(element.declaration);
		visitTypeElement();
	}
	void visit(const NonType& element)
	{
		printer.out << element.value;
		visitTypeElement();
	}
	void visit(const ObjectType& object)
	{
		if(qualifierStack.back().isConst)
		{
			printer.out << "const ";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << "volatile ";
		}
		printType(object.type);
		visitTypeElement();
	}
	void visit(const ReferenceType& pointer)
	{
		pushType(true);
		printer.out << "&";
		visitTypeElement();
		popType();
	}
	void visit(const PointerType& pointer)
	{
		pushType(true);
		printer.out << "*";
		if(qualifierStack.back().isConst)
		{
			printer.out << " const";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << " volatile";
		}
		visitTypeElement();
		popType();
	}
	void visit(const ArrayType& array)
	{
		pushType(false);
		visitTypeElement();
		printer.out << "[";
		if(array.size != 0)
		{
			printer.out << array.size;
		}
		printer.out << "]";
		popType();
	}
	void visit(const MemberPointerType& pointer)
	{
		pushType(true);
		{
			SymbolPrinter walker(printer, escape);
			walker.printType(pointer.type);
		}
		printer.out << "::*";
		if(qualifierStack.back().isConst)
		{
			printer.out << " const";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << " volatile";
		}
		visitTypeElement();
		popType();
	}
	void visit(const FunctionType& function)
	{
		pushType(false);
		visitTypeElement();
		printParameters(function.parameterTypes);
		if(function.isEllipsis)
		{
			printer.out << "...";
		}
		if(qualifierStack.back().isConst)
		{
			printer.out << " const";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << " volatile";
		}
		popType();
	}

	TypeElements typeElements;

	void visitTypeElement()
	{
		if(!typeElements.empty())
		{
			UniqueType element = typeElements.front();
			typeElements.pop_front();
			qualifierStack.push_back(element);
			element->accept(*this);
			qualifierStack.pop_back();
		}
	}


	void printType(const Type& type)
	{
		SYMBOLS_ASSERT(type.unique != 0);
		printType(UniqueTypeWrapper(type.unique));
	}

	void printType(const Declaration& declaration)
	{
		SYMBOLS_ASSERT(declaration.type.unique != 0);
		printType(UniqueTypeWrapper(declaration.type.unique));
	}

	void printType(const TypeInstance& type)
	{
		if(type.enclosing != 0)
		{
			printType(*type.enclosing);
			printer.out << ".";
		}
		else
		{
			printName(type.declaration->scope);
		}
		printer.out << getValue(type.declaration->getName());
		if(type.declaration->isTemplate)
		{
			printTemplateArguments(type.templateArguments);
		}
	}

	void printType(const UniqueTypeId& type)
	{
		for(UniqueType i = type.value; i != UNIQUETYPE_NULL; i = i->next)
		{
			typeElements.push_front(i);
		}
		visitTypeElement();
	}

	void printParameters(const Scope::DeclarationList& parameters)
	{
		printer.out << "(";
		bool separator = false;
		for(Scope::DeclarationList::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			const Declaration* declaration = *i;
			if(declaration->templateParameter == INDEX_INVALID)
			{
				if(separator)
				{
					printer.out << ",";
				}
				SymbolPrinter walker(printer, escape);
				walker.printType(*declaration);
				separator = true;
			}
		}
		printer.out << ")";
	}
	void printParameters(const ParameterTypes& parameters)
	{
		printer.out << "(";
		bool separator = false;
		for(ParameterTypes::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			if(separator)
			{
				printer.out << ",";
			}
			SymbolPrinter walker(printer, escape);
			walker.printType(*i);
			separator = true;
		}
		printer.out << ")";
	}

	void printTemplateArguments(const TemplateArgumentsInstance& templateArguments)
	{
		printer.out << (escape ? "&lt;" : "<");
		bool separator = false;
		for(TemplateArgumentsInstance::const_iterator i = templateArguments.begin(); i != templateArguments.end(); ++i)
		{
			if(separator)
			{
				printer.out << ",";
			}
			SymbolPrinter walker(printer, escape);
			walker.printType(*i);
			separator = true;
		}
		printer.out << (escape ? "&gt;" : ">");
	}

	void printName(const Declaration* name)
	{
		if(name == 0)
		{
			printer.out << "<unknown>";
		}
		else
		{
			printName(name->scope);
			printer.out << getValue(name->getName());
			SYMBOLS_ASSERT(!isFunction(*name) || name->type.unique != 0);
			if(isFunction(*name)
				&& UniqueTypeWrapper(name->type.unique).isFunction())
			{
				printParameters(getParameterTypes(name->type.unique));
			}
		}
	}

	void printName(const char* caption, Declaration* type, Declaration* name)
	{
		printer.out << "/* ";
		printer.out << caption;
		printName(type);
		printer.out << ": ";
		printName(name);
		printer.out << " */";
	}

};

inline void printName(const Declaration* name, std::ostream& out = std::cout)
{
	FileTokenPrinter tokenPrinter(out);
	SymbolPrinter printer(tokenPrinter);
	printer.printName(name);
}

inline void printName(const Scope* scope, std::ostream& out = std::cout)
{
	FileTokenPrinter tokenPrinter(out);
	SymbolPrinter printer(tokenPrinter);
	printer.printName(scope);
}

inline void printType(const TypeInstance& type, std::ostream& out, bool escape)
{
	FileTokenPrinter tokenPrinter(out);
	SymbolPrinter printer(tokenPrinter, escape);
	printer.printName(type.declaration);
	if(type.declaration->isTemplate)
	{
		printer.printTemplateArguments(type.templateArguments);
	}
}

inline void printType(UniqueTypeWrapper type, std::ostream& out, bool escape)
{
	FileTokenPrinter tokenPrinter(out);
	SymbolPrinter printer(tokenPrinter, escape);
	printer.printType(type);
}

inline bool deduceFunctionCall(UniqueTypeWrapper parameter, UniqueTypeWrapper argument, TemplateArgumentsInstance& result)
{
	// TODO: ignore top-level cv-qualifiers of P
	if(parameter.isReference())
	{
		parameter = removeReference(parameter);
	}
	else
	{
		if(argument.isArray())
		{
			argument = applyArrayToPointerConversion(argument);
		}
		else if(argument.isFunction())
		{
			argument = applyFunctionToPointerConversion(argument);
		}
		// TODO: ignore top-level cv-qualifiers of A
	}

	return deduce(parameter, argument, result);
}

typedef ListReference<UniqueTypeId, TreeAllocator<int> > UniqueTypeIds2;

// wrapper to disable default-constructor
struct UniqueTypeIds : public UniqueTypeIds2
{
	UniqueTypeIds(const TreeAllocator<int>& allocator)
		: UniqueTypeIds2(allocator)
	{
	}
private:
	UniqueTypeIds()
	{
	}
};

struct OverloadResolver
{
	const UniqueTypeIds& arguments;
	Location source;
	const TypeInstance* enclosing;
	CandidateFunction best;
	Declaration* ambiguous;

	OverloadResolver(const UniqueTypeIds& arguments, Location source, const TypeInstance* enclosing)
		: arguments(arguments), source(source), enclosing(enclosing), ambiguous(0)
	{
		size_t count = std::distance(arguments.begin(), arguments.end());
		best.conversions.resize(count, ImplicitConversion(STANDARDCONVERSIONSEQUENCE_INVALID));
	}
	FunctionOverload get() const
	{
		return ambiguous != 0 ? FunctionOverload(0, gUniqueTypeNull) : best;
	}
	bool isViable(const CandidateFunction& candidate)
	{
		if(candidate.conversions.size() != best.conversions.size())
		{
			return false; // TODO: early-out for functions with not enough params
		}

		for(ArgumentConversions::const_iterator i = candidate.conversions.begin(); i != candidate.conversions.end();  ++i)
		{
			if(!isValid(*i))
			{
				return false;
			}
		}

		return true;
	}
	void add(const CandidateFunction& candidate)
	{
		if(best.declaration != 0
			&& candidate.declaration->enclosed == best.declaration->enclosed)
		{
			return; // TODO: don't add multiple declarations of same signature
		}

		if(!isViable(candidate))
		{
			return;
		}

		if(best.declaration == 0
			|| isBetter(candidate, best))
		{
			best = candidate;
			ambiguous = 0;
		}
		else if(!isBetter(best, candidate)) // the best candidate is an equally good match
		{
			ambiguous = candidate.declaration;
		}
	}
	void add(const FunctionOverload& overload)
	{
		CandidateFunction candidate(overload);
		candidate.conversions.reserve(best.conversions.size());

		if(!overload.type.isFunction())
		{
			return; // TODO: invoke operator() on object of class-type
		}
		const ParameterTypes& parameters = getParameterTypes(overload.type.value);
		UniqueTypeIds::const_iterator a = arguments.begin();
		const Parameters& defaults = getParameters(overload.declaration->type);
		Parameters::const_iterator p = defaults.begin();
		// TODO: ellipsis
		for(ParameterTypes::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			UniqueTypeId to = (*i);
			UniqueTypeId from;
			if(a != arguments.end())
			{
				candidate.conversions.push_back(makeStandardConversionSequence(to, *a, source, enclosing)); // TODO: null-pointer-constant, l-value
				++a;
			}
			else if((*p).argument == 0) // TODO: catch this earlier
			{
				return; // [over.match.viable] no default-argument available, this candidate is not viable
			}
			else
			{
				SYMBOLS_ASSERT((*p).argument->expr != 0); // TODO: non-fatal error: trying to use a default-argument before it has been declared. 
			}
			++p;
		}

		add(candidate);
	}
};




template<typename T>
inline Name getTypeTraitName(T* symbol)
{
	return symbol->trait->value.value;
}

inline bool hasNothrowConstructor(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasNothrowCopy(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasTrivialAssign(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasTrivialConstructor(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasTrivialCopy(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasTrivialDestructor(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasVirtualDestructor(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isAbstract(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isEmpty(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isPod(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isPolymorphic(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isUnion(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isBaseOf(UniqueTypeWrapper base, UniqueTypeWrapper derived, Location source, const TypeInstance* enclosing)
{
	if(!base.isSimple()
		|| !derived.isSimple())
	{
		return false;
	}
	const TypeInstance& baseType = getObjectType(base.value);
	const TypeInstance& derivedType = getObjectType(derived.value);
	if(&baseType == &derivedType)
	{
		return true;
	}
	SYMBOLS_ASSERT(!isIncomplete(*derivedType.declaration)); // TODO: does SFINAE apply?
	return isBaseOf(baseType, derivedType, source, enclosing);
}

inline bool isConvertibleTo(UniqueTypeWrapper from, UniqueTypeWrapper to, Location source, const TypeInstance* enclosing)
{
	return false; // TODO
}

inline UnaryTypeTraitsOp getUnaryTypeTraitsOp(cpp::typetraits_unary* symbol)
{
	switch(symbol->id)
	{
	case cpp::typetraits_unary::HAS_NOTHROW_CONSTRUCTOR: return hasNothrowConstructor;
	case cpp::typetraits_unary::HAS_NOTHROW_COPY: return hasNothrowCopy;
	case cpp::typetraits_unary::HAS_TRIVIAL_ASSIGN: return hasTrivialAssign;
	case cpp::typetraits_unary::HAS_TRIVIAL_CONSTRUCTOR: return hasTrivialConstructor;
	case cpp::typetraits_unary::HAS_TRIVIAL_COPY: return hasTrivialCopy;
	case cpp::typetraits_unary::HAS_TRIVIAL_DESTRUCTOR: return hasTrivialDestructor;
	case cpp::typetraits_unary::HAS_VIRTUAL_DESTRUCTOR: return hasVirtualDestructor;
	case cpp::typetraits_unary::IS_ABSTRACT: return isAbstract;
	case cpp::typetraits_unary::IS_CLASS: return isClass;
	case cpp::typetraits_unary::IS_EMPTY: return isEnum;
	case cpp::typetraits_unary::IS_ENUM: return isEmpty;
	case cpp::typetraits_unary::IS_POD: return isPod;
	case cpp::typetraits_unary::IS_POLYMORPHIC: return isPolymorphic;
	case cpp::typetraits_unary::IS_UNION: return isUnion;
	default: break;
	}
	throw SymbolsError();
}

inline BinaryTypeTraitsOp getBinaryTypeTraitsOp(cpp::typetraits_binary* symbol)
{
	switch(symbol->id)
	{
	case cpp::typetraits_binary::IS_BASE_OF: return isBaseOf;
	case cpp::typetraits_binary::IS_CONVERTIBLE_TO: return isConvertibleTo;
	default: break;
	}
	throw SymbolsError();
}

// ----------------------------------------------------------------------------

#endif


