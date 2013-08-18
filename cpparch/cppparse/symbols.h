
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
	bool isExplicit;
	DeclSpecifiers()
		: isTypedef(false), isFriend(false), isStatic(false), isExtern(false), isExplicit(false)
	{
	}
	DeclSpecifiers(bool isTypedef, bool isFriend, bool isStatic, bool isExtern)
		: isTypedef(isTypedef), isFriend(isFriend), isStatic(isStatic), isExtern(isExtern), isExplicit(false)
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
	virtual void visit(const struct CastExpression&) = 0;
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
	virtual void visit(const struct ExplicitTypeExpression&) = 0;
	virtual void visit(const struct MemberAccessExpression&) = 0;
	virtual void visit(const struct FunctionCallExpression&) = 0;
	virtual void visit(const struct SubscriptExpression&) = 0;
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
inline UniqueExpression makeUniqueExpression(const T& value)
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

struct SimpleType;

inline IntegralConstant evaluate(ExpressionNode* node, Location source, const SimpleType* enclosing);

struct ExpressionWrapper : ExpressionPtr
{
	bool isConstant;
	bool isTypeDependent;
	bool isValueDependent;
	bool isTemplateArgumentAmbiguity; // [temp.arg] In a template argument, an ambiguity between a typeid and an expression is resolved to a typeid
	bool isNonStaticMemberName;
	bool isQualifiedNonStaticMemberName;
	ExpressionWrapper()
		: ExpressionPtr(0), isConstant(false), isTypeDependent(false), isValueDependent(false), isTemplateArgumentAmbiguity(false), isNonStaticMemberName(false), isQualifiedNonStaticMemberName(false)
	{
	}
	ExpressionWrapper(ExpressionNode* node, bool isConstant = true, bool isTypeDependent = false, bool isValueDependent = false)
		: ExpressionPtr(node), isConstant(isConstant), isTypeDependent(isTypeDependent), isValueDependent(isValueDependent), isTemplateArgumentAmbiguity(false), isNonStaticMemberName(false), isQualifiedNonStaticMemberName(false)
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
	bool isTemplateName; // true if this is a template declaration, or an overload of a template declaration
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
		isTemplateName(isTemplate),
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

const std::size_t VISIBILITY_ALL = UINT_MAX;

// An instance of a declaration - multiple declarations may refer to the same entity.
// e.g. definition, forward declaration, redeclaration
struct DeclarationInstance : DeclarationPtr
{
	Identifier* name; // the identifier used in this declaration.
	const DeclarationInstance* overloaded; // the previously declared overload of this name (which may or may not refer to the same entity.)
	const DeclarationInstance* redeclared; // the previous declaration that refers to the same entity.
	std::size_t visibility; // every declaration declared before this has a lesser value
	DeclarationInstance()
		: DeclarationPtr(0), name(0), overloaded(0), redeclared(0), visibility(VISIBILITY_ALL)
	{
	}
	// used when cloning an existing declaration, in the process of copying declarations from one scope to another.
	explicit DeclarationInstance(Declaration* declaration, std::size_t visibility = VISIBILITY_ALL)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0), visibility(visibility)
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
	size_t templateDepth;

	Scope(const TreeAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(allocator), type(type), bases(allocator), usingDirectives(allocator), declarationList(allocator), templateDepth(0)

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


// global scope
extern Identifier gGlobalId;

inline Scope* getEnclosingNamespace(Scope* scope)
{
	for(; scope != 0; scope = scope->parent)
	{
		if(scope->type == SCOPETYPE_NAMESPACE
			&& scope->name.value != gGlobalId.value)
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
	typedef bool (*Function)(void* context, const DeclarationInstance& declaration);
	Function function;
	void* context;

	bool operator()(const DeclarationInstance& declaration)
	{
		return function(context, declaration);
	}
};

inline bool isAny(const Declaration& declaration)
{
	// always ignore constructors during name-lookup
	return declaration.type.declaration != &gCtor;
}


template<typename T>
struct LookupFilterThunk
{
	static bool apply(void* context, const DeclarationInstance& declaration)
	{
		return (*static_cast<T*>(context))(*declaration);
	}
};

template<typename T>
LookupFilter makeLookupFilter(T& filter)
{
	LookupFilter result = { LookupFilterThunk<T>::apply, &filter };
	return result;
}

template<bool filter(const Declaration& declaration)>
struct LookupFilterDefault : LookupFilter
{
	explicit LookupFilterDefault(std::size_t visibility = VISIBILITY_ALL)
	{
		LookupFilter::context = reinterpret_cast<void*>(visibility);
		LookupFilter::function = apply;
	}
	static bool apply(void* context, const DeclarationInstance& declaration)
	{
		std::size_t visibility = reinterpret_cast<std::size_t>(context);
		return declaration.visibility < visibility
			&& filter(*declaration);
	}
};

typedef LookupFilterDefault<isAny> IsAny;

inline bool isConstructor(const Declaration& declaration)
{
	return declaration.type.declaration == &gCtor;
}

typedef LookupFilterDefault<isConstructor> IsConstructor;

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


// returns true if \p declaration is a template class, function or template-parameter
inline bool isTemplateName(const Declaration& declaration)
{
	return declaration.isTemplateName;
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

inline bool isFunctionName(const Declaration& declaration)
{
	return isFunction(declaration);
}

typedef LookupFilterDefault<isFunctionName> IsFunctionName;


// ----------------------------------------------------------------------------
// unique types
// Representation of a declarator, with type-elements linked in 'normal' order.
// e.g. int(*)[] == pointer to array of == DeclaratorPointerType -> DeclaratorArrayType
// Note that this is the reverse of the order that the declarator is parsed in.
// This means a given unique type sub-sequence need only be stored once.
// This allows fast comparison of types and simplifies printing of declarators.

struct TypeElementVisitor
{
#if 0
	virtual void visit(const struct Namespace&) = 0;
#endif
	virtual void visit(const struct DependentType&) = 0;
	virtual void visit(const struct DependentTypename&) = 0;
	virtual void visit(const struct DependentNonType&) = 0;
	virtual void visit(const struct TemplateTemplateArgument&) = 0;
	virtual void visit(const struct NonType&) = 0;
	virtual void visit(const struct SimpleType&) = 0;
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
		return typeid(*value) == typeid(TypeElementGeneric<SimpleType>);
	}
#if 0
	bool isNamespace() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<struct Namespace>);
	}
#endif
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
	bool isDependentNonType() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DependentNonType>);
	}
	bool isDependentType() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DependentType>);
	}
	bool isDependent() const
	{
		return isDependentType()
			|| typeid(*value) == typeid(TypeElementGeneric<DependentTypename>)
			|| isDependentNonType();
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

template<typename T>
inline UniqueTypeWrapper pushType(UniqueTypeWrapper type, const T& t)
{
	pushUniqueType(type.value, t);
	return type;
}

template<typename T>
inline UniqueTypeWrapper pushBuiltInType(UniqueTypeWrapper type, const T& value)
{
	return UniqueTypeWrapper(pushBuiltInType(type.value, value));
}

inline UniqueTypeWrapper popType(UniqueTypeWrapper type)
{
	type.pop_front();
	return type;
}

inline UniqueTypeWrapper qualifyType(UniqueTypeWrapper type, CvQualifiers qualifiers)
{
	type.value.setQualifiers(qualifiers);
	return type;
}

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

inline bool isGreaterCvQualification(CvQualifiers l, CvQualifiers r)
{
	return l.isConst + l.isVolatile > r.isConst + r.isVolatile;
}

inline bool isGreaterCvQualification(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	return isGreaterCvQualification(to.value.getQualifiers(), from.value.getQualifiers());
}

inline bool isEqualCvQualification(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	return to.value.getQualifiers() == from.value.getQualifiers();
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
	SYMBOLS_ASSERT(typeid(*type) != typeid(TypeElementGeneric<struct SimpleType>));
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
typedef std::vector<const struct SimpleType*> UniqueBases;

struct ChildInstantiation
{
	const struct SimpleType* instance;
	Location source;
	ChildInstantiation(const struct SimpleType* instance, Location source)
		: instance(instance), source(source)
	{
	}
};
typedef std::vector<ChildInstantiation> ChildInstantiations;


typedef std::vector<Location> InstanceLocations; // temporary scaffolding!


struct SimpleType
{
	std::size_t uniqueId;
	DeclarationPtr primary;
	DeclarationPtr declaration; // don't compare declaration pointer - it will change on instantiation if an explicit/partial specialization is chosen
	TemplateArgumentsInstance templateArguments;
	TemplateArgumentsInstance deducedArguments; // the deduced arguments for the partial-specialization's template-parameters
	const SimpleType* enclosing; // the enclosing template
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

	SimpleType(Declaration* declaration, const SimpleType* enclosing)
		: uniqueId(0), primary(declaration), declaration(declaration), enclosing(enclosing), size(0), instantiated(false), instantiating(false), allowLookup(false), visited(false), dumped(false)
	{
		SYMBOLS_ASSERT(enclosing == 0 || isClass(*enclosing->declaration));
	}
};

inline bool operator==(const SimpleType& left, const SimpleType& right)
{
	return left.primary.p == right.primary.p
		&& left.enclosing == right.enclosing
		&& left.templateArguments == right.templateArguments;
}

inline bool operator<(const SimpleType& left, const SimpleType& right)
{
	return left.primary.p != right.primary.p ? left.primary.p < right.primary.p
		: left.enclosing != right.enclosing ? left.enclosing < right.enclosing
		: left.templateArguments != right.templateArguments ? left.templateArguments < right.templateArguments
		: false;
}

inline const SimpleType& getSimpleType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<SimpleType>));
	return static_cast<const TypeElementGeneric<SimpleType>*>(type.getPointer())->value;
}

#if 0
struct Namespace
{
	DeclarationPtr declaration;
	Namespace(DeclarationPtr declaration)
		: declaration(declaration)
	{
	}
};

inline const Namespace& getNamespace(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<Namespace>));
	return static_cast<const TypeElementGeneric<Namespace>*>(type.getPointer())->value;
}


inline bool operator<(const Namespace& left, const Namespace& right)
{
	return left.declaration.p < right.declaration.p;
}
#endif

struct TemplateTemplateArgument
{
	DeclarationPtr declaration; // the primary declaration of the template template argument
	const SimpleType* enclosing;
	TemplateTemplateArgument(Declaration* declaration, const SimpleType* enclosing)
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

inline const DependentType& getDependentType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<DependentType>));
	return static_cast<const TypeElementGeneric<DependentType>*>(type.getPointer())->value;
}


struct DependentTypename
{
	Name name; // the type name
	UniqueTypeWrapper qualifying; // the qualifying type: T::, C<T>::
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
				: left.isTemplate != right.isTemplate
					? left.isTemplate < right.isTemplate
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

inline const SimpleType& getMemberPointerClass(UniqueType type)
{
	return getSimpleType(getMemberPointerType(type).type.value);
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
	DeclaratorFunctionType()
	{
	}
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
inline const DeclaratorFunctionType& getDeclaratorFunctionType(const TypeSequence::Node* node)
{
	SYMBOLS_ASSERT(node != 0);
	SYMBOLS_ASSERT(typeid(*node) == typeid(SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>));
	return static_cast<const SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>*>(node)->value;
}
inline const Parameters& getParameters(const TypeId& type)
{
	const TypeSequence::Node* node = getLastNode(type.typeSequence);
	return getDeclaratorFunctionType(node).parameters;
}

inline const FunctionType& getFunctionType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<FunctionType>));
	return static_cast<const TypeElementGeneric<FunctionType>*>(type.getPointer())->value;
}
inline const ParameterTypes& getParameterTypes(UniqueType type)
{
	return getFunctionType(type).parameterTypes;
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

template<bool builtIn>
struct UniqueTypeGeneric : UniqueTypeWrapper
{
	UniqueTypeGeneric()
	{
	}
	explicit UniqueTypeGeneric(UniqueTypeWrapper value) : UniqueTypeWrapper(value)
	{
	}
};


typedef UniqueTypeGeneric<true> BuiltInType; 

struct BuiltInTypeId : BuiltInType
{
	BuiltInTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
	{
		value = pushBuiltInType(value, SimpleType(declaration, 0));
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
	const SimpleType* enclosing;

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
	LookupResultRef()
	{
	}
	LookupResultRef(const DeclarationInstance& p)
		: DeclarationInstanceRef(p)
	{
	}
	LookupResultRef(const LookupResult& result)
		: DeclarationInstanceRef(*result.filtered)
	{
	}
};


inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = IsAny());

struct RecursionGuard
{
	const SimpleType& instance;
	RecursionGuard(const SimpleType& instance)
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

inline void printType(const SimpleType& type, std::ostream& out = std::cout, bool escape = false);

inline LookupResult findDeclaration(const SimpleType& instance, const Identifier& id, LookupFilter filter)
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

	result.filtered = findDeclaration(instance.declaration->enclosed->declarations, id, filter);
	if(result.filtered)
	{
		result.enclosing = &instance;
		return result;
	}
	for(UniqueBases::const_iterator i = instance.bases.begin(); i != instance.bases.end(); ++i)
	{
		const SimpleType& base = *(*i);
		SYMBOLS_ASSERT(base.declaration->enclosed != 0); // TODO: non-fatal error: incomplete type
		SYMBOLS_ASSERT(base.declaration->enclosed->usingDirectives.empty()); // namespace.udir: A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.

		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->getName().value == id.value
			&& filter(DeclarationInstance(base.declaration)))
		{
			result.filtered = &getDeclaration(base.declaration->getName());
			result.enclosing = base.enclosing;
			return result;
		}

		if(result.append(findDeclaration(base, id, filter)))
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
#if 1
		if(getSimpleType(qualifying.value).instantiating)
		{
			std::cout << "(partially instantiated) ";
		}
#endif
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

struct InstantiationContext
{
	Location source;
	const SimpleType* enclosingType;
	ScopePtr enclosingScope;
	InstantiationContext(Location source, const SimpleType* enclosingType, ScopePtr enclosingScope = 0)
		: source(source), enclosingType(enclosingType), enclosingScope(enclosingScope)
	{
	}
};


struct Argument : ExpressionWrapper
{
	UniqueTypeWrapper type;
	Argument(ExpressionWrapper expression, UniqueTypeWrapper type)
		: ExpressionWrapper(expression), type(type)
	{
	}
};
typedef std::vector<Argument> Arguments;


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


struct CastExpression
{
	UniqueTypeWrapper type;
	ExpressionWrapper operand;
	CastExpression(UniqueTypeWrapper type, ExpressionWrapper operand)
		: type(type), operand(operand)
	{
	}
};

inline bool operator<(const CastExpression& left, const CastExpression& right)
{
	return left.type != right.type
		? left.type < right.type
		: left.operand.p < right.operand.p;
}

inline bool isCastExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<CastExpression>);
}

inline const CastExpression& getCastExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isCastExpression(node));
	return static_cast<const ExpressionNodeGeneric<CastExpression>*>(node)->value;
}


struct DependentIdExpression
{
	Name name;
	UniqueTypeWrapper qualifying;
	TemplateArgumentsInstance templateArguments;
	DependentIdExpression(Name name, UniqueTypeWrapper qualifying, TemplateArgumentsInstance templateArguments)
		: name(name), qualifying(qualifying), templateArguments(templateArguments)
	{
		SYMBOLS_ASSERT(qualifying.value.p != 0);
	}
};

inline bool operator<(const DependentIdExpression& left, const DependentIdExpression& right)
{
	return left.name != right.name
		? left.name < right.name
			: left.qualifying != right.qualifying
			? left.qualifying < right.qualifying
				: left.templateArguments < right.templateArguments;
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
	DeclarationInstanceRef declaration;
	const SimpleType* enclosing;
	TemplateArgumentsInstance templateArguments;
	IdExpression(DeclarationInstanceRef declaration, const SimpleType* enclosing, const TemplateArgumentsInstance& templateArguments)
		: declaration(declaration), enclosing(enclosing), templateArguments(templateArguments)
	{
	}
};

inline bool operator<(const IdExpression& left, const IdExpression& right)
{
#if 1
	// TODO: check compliance: id-expressions cannot be compared for equivalence
	SYMBOLS_ASSERT(false);
#else
	return left.declaration.p != right.declaration.p
		? left.declaration.p < right.declaration.p
		: left.enclosing < right.enclosing;
#endif
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
	ExpressionWrapper operand;
	SizeofExpression(ExpressionWrapper operand)
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
	ExpressionWrapper first;
	UnaryExpression(Name operatorName, UnaryIceOp operation, ExpressionWrapper first)
		: operatorName(operatorName), operation(operation), first(first)
	{
	}
};

inline bool operator<(const UnaryExpression& left, const UnaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
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

typedef UniqueTypeWrapper (*BinaryTypeOp)(Name operatorName, Argument first, Argument second, const InstantiationContext& context);

struct BinaryExpression
{
	Name operatorName;
	BinaryIceOp operation;
	BinaryTypeOp type;
	ExpressionWrapper first;
	ExpressionWrapper second;
	BinaryExpression(Name operatorName, BinaryIceOp operation, BinaryTypeOp type, ExpressionWrapper first, ExpressionWrapper second)
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
	ExpressionWrapper first;
	ExpressionWrapper second;
	ExpressionWrapper third;
	TernaryExpression(TernaryIceOp operation, ExpressionWrapper first, ExpressionWrapper second, ExpressionWrapper third)
		: operation(operation), first(first), second(second), third(third)
	{
	}
};

inline bool operator<(const TernaryExpression& left, const TernaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.first.p != right.first.p
			? left.first.p < right.first.p
			: left.second.p != right.second.p
				? left.second.p < right.second.p
				: left.third.p < right.third.p;
}

typedef bool (*UnaryTypeTraitsOp)(UniqueTypeWrapper);
typedef bool (*BinaryTypeTraitsOp)(UniqueTypeWrapper, UniqueTypeWrapper, Location source, const SimpleType* enclosing);

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



struct ExplicitTypeExpression
{
	UniqueTypeWrapper type;
	ExplicitTypeExpression(UniqueTypeWrapper type)
		: type(type)
	{
	}
};

inline bool operator<(const ExplicitTypeExpression& left, const ExplicitTypeExpression& right)
{
	return left.type < right.type;
}

inline bool isExplicitTypeExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<ExplicitTypeExpression>);
}

inline const ExplicitTypeExpression& getExplicitTypeExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isExplicitTypeExpression(node));
	return static_cast<const ExpressionNodeGeneric<ExplicitTypeExpression>*>(node)->value;
}


struct MemberAccessExpression
{
	ExpressionWrapper left; // extract type, memberClass
	ExpressionWrapper right; // always id-expression
	bool isArrow;
	MemberAccessExpression(ExpressionWrapper left, ExpressionWrapper right, bool isArrow)
		: left(left), right(right), isArrow(isArrow)
	{
	}
};

inline bool operator<(const MemberAccessExpression& left, const MemberAccessExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}

inline bool isMemberAccessExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<MemberAccessExpression>);
}

inline const MemberAccessExpression& getMemberAccessExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isMemberAccessExpression(node));
	return static_cast<const ExpressionNodeGeneric<MemberAccessExpression>*>(node)->value;
}


// id-expression ( expression-list )
// overload resolution is required if the lefthand side is
// - a (parenthesised) id-expression
// - of class type
struct FunctionCallExpression
{
	ExpressionWrapper left; // TODO: extract memberClass, id, idEnclosing, type, templateArguments
	Arguments arguments;
	FunctionCallExpression(ExpressionWrapper left, Arguments arguments)
		: left(left), arguments(arguments)
	{
	}
};

inline bool operator<(const FunctionCallExpression& left, const FunctionCallExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}


struct SubscriptExpression
{
	ExpressionWrapper left;
	ExpressionWrapper right;
	SubscriptExpression(ExpressionWrapper left, ExpressionWrapper right)
		: left(left), right(right)
	{
	}
};

inline bool operator<(const SubscriptExpression& left, const SubscriptExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}



inline UniqueTypeWrapper typeOfExpression(ExpressionNode* node, const InstantiationContext& context);

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
	return UniqueTypeWrapper(node.declaration->type.unique).isFunction();
}

inline bool isDependentPointerToMemberExpression(ExpressionNode* expression)
{
	return isUnaryExpression(expression)
		&& getUnaryExpression(expression).operation == addressOf
		&& isDependentIdExpression(getUnaryExpression(expression).first);
}

extern BuiltInTypeId gUnsignedInt;
extern BuiltInTypeId gBool;
extern const DeclarationInstance gCopyAssignmentOperatorInstance;
extern const DeclarationInstance gDestructorInstance;


// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
inline UniqueTypeWrapper removeReference(UniqueTypeWrapper type)
{
	if(type.isReference())
	{
		type.pop_front();
	}
	return type;
}


inline bool isDependent(UniqueTypeWrapper type);
inline UniqueTypeWrapper typeOfUnaryExpression(Name operatorName, Argument operand, const InstantiationContext& context);
inline UniqueTypeWrapper getConditionalOperatorType(UniqueTypeWrapper leftType, UniqueTypeWrapper rightType);
inline UniqueTypeWrapper typeOfSubscriptExpression(Argument left, Argument right, const InstantiationContext& context);
inline const SimpleType& getMemberOperatorType(UniqueTypeWrapper operand, bool isArrow, Location source, const SimpleType* enclosing);
inline UniqueTypeWrapper makeCopyAssignmentOperatorType(const SimpleType& classType);
inline const SimpleType* findEnclosingType(const SimpleType* enclosing, Scope* scope);
inline UniqueTypeWrapper getUniqueType(const TypeId& type, Location source, const SimpleType* enclosing, bool allowDependent = false);
inline UniqueTypeWrapper typeOfIdExpression(const SimpleType* qualifying, const DeclarationInstance& declaration, Location source, const SimpleType* enclosingType);

struct TypeOfVisitor : ExpressionNodeVisitor
{
	UniqueTypeWrapper result;
	InstantiationContext context;
	explicit TypeOfVisitor(const InstantiationContext& context)
		: context(context)
	{
	}
	void visit(const IntegralConstantExpression& node)
	{
		result = node.type;
	}
	void visit(const CastExpression& node)
	{
		result = node.type;
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const NonTypeTemplateParameter& node)
	{
		// TODO: evaluate non-type template parameter
		SYMBOLS_ASSERT(false);
	}
	void visit(const DependentIdExpression& node)
	{
		// TODO: name lookup
		SYMBOLS_ASSERT(false);
	}
	void visit(const IdExpression& node)
	{
		result = typeOfIdExpression(node.enclosing, node.declaration, context.source, context.enclosingType);
		SYMBOLS_ASSERT(!isDependent(result));
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
		result = typeOfUnaryExpression(node.operatorName,
			Argument(node.first, removeReference(typeOfExpression(node.first, context))),
			context);
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const BinaryExpression& node)
	{
		result = node.type(node.operatorName,
			Argument(node.first, removeReference(typeOfExpression(node.first, context))),
			Argument(node.second, removeReference(typeOfExpression(node.second, context))),
			context);
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const TernaryExpression& node)
	{
		result = getConditionalOperatorType(
			removeReference(typeOfExpression(node.second, context)),
			removeReference(typeOfExpression(node.third, context)));
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const TypeTraitsUnaryExpression& node)
	{
		result = gBool;
	}
	void visit(const TypeTraitsBinaryExpression& node)
	{
		result = gBool;
	}
	void visit(const struct ExplicitTypeExpression& node)
	{
		result = node.type;
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const struct MemberAccessExpression& node)
	{
		UniqueTypeWrapper type = typeOfExpression(node.left, context);
		const SimpleType& classType = getMemberOperatorType(removeReference(type), node.isArrow, context.source, context.enclosingType);
		result = typeOfExpression(node.right, InstantiationContext(context.source, &classType, context.enclosingScope));
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const struct FunctionCallExpression& node)
	{
		// TODO
	}
	void visit(const struct SubscriptExpression& node)
	{
		result = typeOfSubscriptExpression(
			Argument(node.left, removeReference(typeOfExpression(node.left, context))),
			Argument(node.right, removeReference(typeOfExpression(node.right, context))),
			context);
	}
};

inline UniqueTypeWrapper typeOfExpression(ExpressionNode* node, const InstantiationContext& context)
{
	TypeOfVisitor visitor(context);
	node->accept(visitor);
	return visitor.result;
}



inline std::size_t instantiateClass(const SimpleType& instanceConst, Location source, const SimpleType* enclosing, bool allowDependent = false);
inline std::size_t requireCompleteObjectType(UniqueTypeWrapper type, Location source, const SimpleType* enclosing);
inline UniqueTypeWrapper removeReference(UniqueTypeWrapper type);
inline const SimpleType* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const SimpleType* enclosing, bool allowDependent = false);
inline const SimpleType* findEnclosingType(const SimpleType* enclosing, Scope* scope);
inline bool isDependent(const SimpleType& type);
inline UniqueTypeWrapper substitute(UniqueTypeWrapper dependent, Location source, const SimpleType& enclosingType);


inline const SimpleType* findEnclosingTemplate(const SimpleType* enclosing, Declaration* declaration)
{
	Declaration* primary = findPrimaryTemplate(declaration);
	SYMBOLS_ASSERT(primary->isTemplate);
	SYMBOLS_ASSERT(!primary->isSpecialization);
	for(const SimpleType* i = enclosing; i != 0; i = (*i).enclosing)
	{
		SYMBOLS_ASSERT(!(*i).primary->isSpecialization);
		if((*i).primary->isTemplate
			&& (*i).primary == primary)
		{
			return i;
		}
	}
	return 0;
}

inline const SimpleType* findEnclosingTemplate(const SimpleType* enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	SYMBOLS_ASSERT(scope->type == SCOPETYPE_TEMPLATE);
	for(const SimpleType* i = enclosing; i != 0; i = (*i).enclosing)
	{
		if((*i).declaration->templateParamScope != 0
			&& (*i).declaration->templateParamScope->templateDepth == scope->templateDepth)
		{
			return i;
		}
	}
	return 0;
}

inline const SimpleType* getEnclosingType(const SimpleType* enclosing)
{
	for(const SimpleType* i = enclosing; i != 0; i = (*i).enclosing)
	{
		if((*i).declaration->getName().value.c_str()[0] != '$') // ignore anonymous union
		{
			return i;
		}
	}
	return 0;
}


inline IntegralConstant evaluateIdExpression(const Declaration* declaration, Location source, const SimpleType* enclosing)
{
	SYMBOLS_ASSERT(declaration->templateParameter == INDEX_INVALID);

	const SimpleType* memberEnclosing = isMember(*declaration) // if the declaration is a class member
		? findEnclosingType(enclosing, declaration->scope) // it must be a member of (a base of) the qualifying class: find which one.
		: 0; // the declaration is not a class member and cannot be found through qualified name lookup

	return evaluate(declaration->initializer, source, memberEnclosing);
}

inline IntegralConstant evaluateIdExpression(const IdExpression& node, Location source, const SimpleType* enclosing)
{
	if(node.enclosing != 0)
	{
		enclosing = node.enclosing;
	}
	return evaluateIdExpression(node.declaration, source, enclosing);
}

inline void addInstatiation(SimpleType& enclosing, const SimpleType& instance)
{
}

inline IntegralConstant evaluateIdExpression(const DependentIdExpression& node, Location source, const SimpleType* enclosingType)
{
	SYMBOLS_ASSERT(node.qualifying != gUniqueTypeNull);
	SYMBOLS_ASSERT(enclosingType != 0);

	UniqueTypeWrapper substituted = substitute(node.qualifying, source, *enclosingType);
	const SimpleType* qualifyingType = substituted.isSimple() ? &getSimpleType(substituted.value) : 0;

	if(qualifyingType == 0
		|| !isClass(*qualifyingType->declaration))
	{
		throw QualifyingIsNotClassError(source, node.qualifying);
	}

	instantiateClass(*qualifyingType, source, enclosingType);
	Identifier id;
	id.value = node.name;
	std::size_t visibility = qualifyingType->instantiating ? enclosingType->instantiation.pointOfInstantiation : VISIBILITY_ALL;
	LookupResultRef declaration = findDeclaration(*qualifyingType, id, IsAny(visibility));
	if(declaration == 0)
	{
		throw MemberNotFoundError(source, node.name, node.qualifying);
	}

	return evaluateIdExpression(declaration, source, qualifyingType);
}

struct EvaluateVisitor : ExpressionNodeVisitor
{
	IntegralConstant result;
	const SimpleType* enclosing;
	Location source;
	explicit EvaluateVisitor(Location source, const SimpleType* enclosing)
		: source(source), enclosing(enclosing)
	{
	}
	void visit(const IntegralConstantExpression& node)
	{
		result = node.value;
	}
	void visit(const CastExpression& node)
	{
		result = evaluate(node.operand, source, enclosing);
	}
	void visit(const NonTypeTemplateParameter& node)
	{
		size_t index = node.declaration->templateParameter;
		SYMBOLS_ASSERT(index != INDEX_INVALID);
		const SimpleType* enclosingType = findEnclosingTemplate(enclosing, node.declaration->scope);
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

		if(isPointerToMemberExpression(node.operand))
		{
			return; // TODO
		}
		if(isPointerToFunctionExpression(node.operand))
		{
			return; // TODO
		}

		UniqueTypeWrapper type = typeOfExpression(node.operand, InstantiationContext(source, enclosing));
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
	void visit(const struct ExplicitTypeExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct MemberAccessExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct FunctionCallExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct SubscriptExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
};

inline IntegralConstant evaluate(ExpressionNode* node, Location source, const SimpleType* enclosing)
{
	EvaluateVisitor visitor(source, enclosing);
	node->accept(visitor);
	return visitor.result;
}

inline IntegralConstant evaluateExpression(ExpressionNode* node, Location source, const SimpleType* enclosing)
{
	if(isDependentPointerToMemberExpression(node))
	{
		// TODO: check this names a valid non-static member
		return IntegralConstant(0); // TODO: unique value for address of member
	}
	return evaluate(node, source, enclosing);
}

inline IntegralConstant evaluateExpression(const ExpressionWrapper& expression, Location source, const SimpleType* enclosing)
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


inline bool isDependent(const SimpleType& type)
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
#if 0
	virtual void visit(const Namespace& element)
	{
	}
#endif
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
	virtual void visit(const SimpleType& element)
	{
		if(isDependent(element))
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

inline bool deduce(UniqueTypeWrapper parameter, UniqueTypeWrapper argument, TemplateArgumentsInstance& result, bool allowGreaterCvQualification = false);

inline bool deducePairs(const UniqueTypeArray& parameters, const UniqueTypeArray& arguments, TemplateArgumentsInstance& result)
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
inline bool isNonDeduced(const SimpleType& type)
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
#if 0
	virtual void visit(const Namespace& element)
	{
		SYMBOLS_ASSERT(false);
	}
#endif
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
				const SimpleType& type = getSimpleType(argument.value);
				if(!type.declaration->isTemplate
					|| !deducePairs(element.templateArguments, type.templateArguments, templateArguments)) // template-template-parameter may have template-arguments that refer to a template parameter
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
	virtual void visit(const SimpleType& element)
	{
		SYMBOLS_ASSERT(argument.isSimple());
		const SimpleType& type = getSimpleType(argument.value);
		if(type.primary != element.primary) // if the class type does not match
		{
			result = false; // deduction fails
			return;
		}
		// [temp.deduct.type] The nondeduced contexts are:- The nested-name-specifier of a type that was specified using a qualified-id.
		// not attempting to deduce from enclosing type
		if(!isNonDeduced(element))
		{
			result = deducePairs(element.templateArguments, type.templateArguments, templateArguments);
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
		result = deducePairs(element.parameterTypes, getParameterTypes(argument.value), templateArguments);
	}
};

inline UniqueTypeWrapper applyArrayToPointerConversion(UniqueTypeWrapper type);
inline UniqueTypeWrapper applyFunctionToPointerConversion(UniqueTypeWrapper type);
inline UniqueTypeWrapper makeUniqueSimpleType(const SimpleType& type);

struct DeductionFailure
{
};

inline const SimpleType* findUniqueBase(const SimpleType& derived, const Declaration& type, const SimpleType* result = 0)
{
	SYMBOLS_ASSERT(derived.instantiated);
	SYMBOLS_ASSERT(derived.declaration->enclosed != 0);
	SYMBOLS_ASSERT(isClass(type));
	for(UniqueBases::const_iterator i = derived.bases.begin(); i != derived.bases.end(); ++i)
	{
		const SimpleType& base = *(*i);
		SYMBOLS_ASSERT(isClass(*base.declaration));
		if(base.primary == &type)
		{
			if(result != 0)
			{
				throw DeductionFailure();
			}
			result = &base;
		}
		result = findUniqueBase(base, type, result);
	}
	return result;
}

inline UniqueTypeWrapper removePointer(UniqueTypeWrapper type)
{
	if(type.isPointer())
	{
		type.pop_front();
	}
	return type;
}

inline const SimpleType* getClassType(UniqueTypeWrapper type)
{
	if(!type.isSimple())
	{
		return 0;
	}
	const SimpleType* result = &getSimpleType(type.value);
	if(!isClass(*result->declaration))
	{
		return 0;
	}
	return result;
}

inline void adjustFunctionCallDeductionPair(UniqueTypeWrapper& parameter, UniqueTypeWrapper& argument, Location source, const SimpleType* enclosing)
{
	argument = removeReference(argument);

	// [temp.deduct.call]
	// If P is a cv-qualified type, the top level cv-qualifiers of P’s type are ignored for type deduction.
	parameter.value.setQualifiers(CvQualifiers());
	// If P is a reference type, the type referred to by P is used for type deduction.
	if(parameter.isReference())
	{
		parameter = removeReference(parameter);
	}
	// If P is not a reference type:
	else
	{
		// - If A is an array type, the pointer type produced by the array-to-pointer standard conversion (4.2) is used
		// in place of A for type deduction; otherwise,
		if(argument.isArray())
		{
			argument = applyArrayToPointerConversion(argument);
		}
		// - If A is a function type, the pointer type produced by the function-to-pointer
		// standard conversion (4.3) is used in place of A for type deduction; otherwise,
		else if(argument.isFunction())
		{
			argument = applyFunctionToPointerConversion(argument);
		}
		// - If A is a cv-qualified type, the top level cv-qualifiers
		// of A’s type are ignored for type deduction.
		else
		{
			argument.value.setQualifiers(CvQualifiers());
		}
	}

	// [temp.deduct.call]
	// In general, the deduction process attempts to find template argument values that will make the deduced A identical to A
	// However, there are three cases that allow a difference:
	// — If the original P is a reference type, the deduced A (i.e., the type referred to by the reference) can be
	// more cv-qualified than A.
	// 	— A can be another pointer or pointer to member type that can be converted to the deduced A via a qualification
	// 	conversion (4.4).
	// 	— If P is a class, and P has the form template-id, then A can be a derived class of the deduced A. Likewise,
	// 	if P is a pointer to a class of the form template-id, A can be a pointer to a derived class pointed to
	// 		by the deduced A.
	// These alternatives are considered only if type deduction would otherwise fail. If they yield more than one
	// possible deduced A, the type deduction fails.

	const SimpleType* parameterType = getClassType(removePointer(parameter));
	const SimpleType* argumentType = getClassType(removePointer(argument));
	if(parameterType != 0 && parameterType->declaration->isTemplate // if P is a class-template
		&& argumentType != 0 && isComplete(*argumentType->declaration) // and A is a complete class
		&& parameter.isPointer() == argument.isPointer() // and neither (or both) are pointers
		&& argumentType->primary != parameterType->primary) // and deduction would fail
	{
		instantiateClass(*argumentType, source, enclosing); // A must be instantiated before searching its bases
		const SimpleType* base = findUniqueBase(*argumentType, *parameterType->primary);
		if(base != 0) // if P is an unambiguous base-class of A
		{
			// A can be a derived class of the deduced A
			bool isPointer = argument.isPointer();
			CvQualifiers qualifiers = removePointer(argument).value.getQualifiers();
			argument = makeUniqueSimpleType(*base); // use the base-class in place of A for deduction
			argument.value.setQualifiers(qualifiers); // preserve the cv-qualification of the original A
			if(isPointer)
			{
				argument.push_front(PointerType());
			}
		}
	}
}

inline bool deduce(UniqueTypeWrapper parameter, UniqueTypeWrapper argument, TemplateArgumentsInstance& result, bool allowGreaterCvQualification)
{
	// [temp.deduct.type]
	// Template arguments can be deduced in several different contexts, but in each case a type that is specified in
	// terms of template parameters (call it P) is compared with an actual type (call it A), and an attempt is made
	// to find template argument values (a type for a type parameter, a value for a non-type parameter, or a template
	// for a template parameter) that will make P, after substitution of the deduced values (call it the deduced
	// A), compatible with A.
	if(!isDependent(parameter))
	{
		// P is not specified in terms of template parameters. Deduction succeeds, but does not deduce anything,
		// unless the argument is specified in terms of template parameters.
		return !isDependent(argument);
	}

	std::size_t depth = 0;
	// compare P and A, to find a deduced A that matches P.
	// 'parameter' becomes the deduced A, while 'argument' is the original A.
	for(; !parameter.empty() && !argument.empty(); parameter.pop_front(), argument.pop_front(), ++depth)
	{
		if(allowGreaterCvQualification) // if this is a function-call and we are comparing either the outermost elements, or the outer elements form a const pointer/member-pointer sequence
		{
			// greater cv-qualification of the inner elements is allowed only if the outer elements form a const pointer/member-pointer sequence
			allowGreaterCvQualification = (parameter.isPointer() || parameter.isMemberPointer())
				&& (depth == 0 || parameter.value.getQualifiers().isConst);

			if(isGreaterCvQualification(parameter, argument)) // and the deduced A is more qualified than A (i.e. deduction would fail)
			{
				parameter.value.setQualifiers(argument.value.getQualifiers()); // use cv-qualification of A to ensure deduction succeeds
			}
		}

		if(!parameter.isDependent())
		{
			if(!isSameType(parameter, argument))
			{
				return false; // the deduced A must be identical to A: e.g. T* <- int*, S<T> <- S<int>
			}

			if(!isEqualCvQualification(parameter, argument))
			{
				return false; // the deduced A can not be differently cv-qualified than A
			}
		}

		if(parameter.isDependent()) // if template-parameter 'T' is found
		{
			// if only P is qualified, fail!: e.g. const T <- int
			if(isGreaterCvQualification(parameter, argument))
			{
				return false;
			}
			// if both are qualified, remove qualification: e.g. const T <- const int = int
			// if only A is qualified, add qualification: e.g. T <- const int = const int
			CvQualifiers qualifiers = argument.value.getQualifiers();
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

// deduce the function's template arguments by comparing the original argument list with the substituted parameters
inline bool deduceFunctionCall(const ParameterTypes& parameters, const UniqueTypeArray& arguments, TemplateArgumentsInstance& result, Location source, const SimpleType* enclosing)
{
	try
	{
		UniqueTypeArray::const_iterator p = parameters.begin();
		for(UniqueTypeArray::const_iterator a = arguments.begin();
			a != arguments.end() && p != parameters.end(); // for each pair P, A
			// fewer arguments than parameters: occurs when some parameters are defaulted
			// TODO: more arguments than parameters: occurs when ???
			++a, ++p)
		{
			UniqueTypeWrapper parameter = *p;
			UniqueTypeWrapper argument = *a;
			adjustFunctionCallDeductionPair(parameter, argument, source, enclosing);
			if(!deduce(parameter, argument, result, true))
			{
				throw DeductionFailure();
			}
		}
		if(std::find(result.begin(), result.end(), gUniqueTypeNull) != result.end())
		{
			throw DeductionFailure();
		}
	}
	catch(DeductionFailure)
	{
		return false;
	}
	return true;
}


extern BuiltInTypeId gVoid;


inline void substitute(UniqueTypeArray& substituted, const UniqueTypeArray& dependent, Location source, const SimpleType& enclosingType)
{
	for(UniqueTypeArray::const_iterator i = dependent.begin(); i != dependent.end(); ++i)
	{
		UniqueTypeWrapper type = substitute(*i, source, enclosingType);
		substituted.push_back(type);
	}
}

inline Declaration* findTemplateSpecialization(Declaration* declaration, TemplateArgumentsInstance& deducedArguments, const TemplateArgumentsInstance& arguments, Location source, const SimpleType* enclosing, bool allowDependent);

// 'enclosing' is already substituted
inline UniqueTypeWrapper substitute(Declaration* declaration, const SimpleType* enclosing, const TemplateArgumentsInstance& templateArguments, Location source, const SimpleType& enclosingType)
{
	SimpleType result(declaration, enclosing);
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
	return makeUniqueSimpleType(result);
}

inline const SimpleType* substitute(const SimpleType& instance, Location source, const SimpleType& enclosingType)
{
	const SimpleType* enclosing = 0;
	if(instance.enclosing != 0)
	{
		enclosing = substitute(*instance.enclosing, source, enclosingType);
	}
	UniqueTypeWrapper result = substitute(instance.declaration, enclosing, instance.templateArguments, source, enclosingType);
	return &getSimpleType(result.value);
}

inline LookupResult findNamespaceDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny());

struct SubstituteVisitor : TypeElementVisitor
{
	UniqueTypeWrapper type;
	Location source;
	const SimpleType& enclosingType;
	SubstituteVisitor(UniqueTypeWrapper type, Location source, const SimpleType& enclosingType)
		: type(type), source(source), enclosingType(enclosingType)
	{
	}
#if 0
	virtual void visit(const Namespace& element)
	{
		SYMBOLS_ASSERT(false);
	}
#endif
	virtual void visit(const DependentType& element) // substitute T, TT, TT<...>
	{
		std::size_t index = element.type->templateParameter;
		SYMBOLS_ASSERT(index != INDEX_INVALID);
		const SimpleType* enclosingTemplate = findEnclosingTemplate(&enclosingType, element.type->scope);
		SYMBOLS_ASSERT(enclosingTemplate != 0);
		SYMBOLS_ASSERT(!enclosingTemplate->declaration->isSpecialization || enclosingTemplate->instantiated); // a specialization must be instantiated (or in the process of instantiating)
		const TemplateArgumentsInstance& templateArguments = enclosingTemplate->declaration->isSpecialization
			? enclosingTemplate->deducedArguments : enclosingTemplate->templateArguments;
		SYMBOLS_ASSERT(index < templateArguments.size());
		SYMBOLS_ASSERT(type == gUniqueTypeNull);
		type = templateArguments[index];
		if(element.templateParameterCount != 0) // TT or TT<...>
		{
			if(type.isDependentType()) // occurs when substituting a template-template-parameter with a template-template-parameter
			{
				DependentType result = getDependentType(type.value);
				substitute(result.templateArguments, element.templateArguments, source, enclosingType); // TT<T>
				type = gUniqueTypeNull;
				type.push_front(result);
			}
			else
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
	}

	virtual void visit(const DependentTypename& element) // substitute T::X, T::template X<...>, x.X, x.template X
	{
		if(isDependent(enclosingType))
		{
			// TODO: occurs when substituting with a dependent template argument list, if a template function is called with an empty (or partial) explicit template argument list.
			type.push_front(element);
			return;
		}

		Identifier id;
		id.value = element.name;

		Declaration* declaration = 0;
		const SimpleType* memberEnclosing = 0;
#if 0
		if(element.qualifying == gUniqueTypeNull) // class member access: x.Dependent::
		{
			// If the id-expression in a class member access is a qualified-id of the form
			//   class-name-or-namespace-name::...
			// the class-name-or-namespace-name following the . or -> operator is looked up both in the context of the
			// entire postfix-expression and in the scope of the class of the object expression. If the name is found only in
			// the scope of the class of the object expression, the name shall refer to a class-name. If the name is found
			// only in the context of the entire postfix-expression, the name shall refer to a class-name or namespace-name.
			// If the name is found in both contexts, the class-name-or-namespace-name shall refer to the same entity.

			// look up id both within type of object expression and in enclosing scope
			// result may be namespace or class (template)
			SYMBOLS_ASSERT(false); // TODO
			// if result is class member, report error if it is not a type
			// set memberEnclosing to the member's enclosing class
		}
		else // T::Dependent
#endif
		{
			UniqueTypeWrapper qualifying = substitute(element.qualifying, source, enclosingType);
			SYMBOLS_ASSERT(qualifying != gUniqueTypeNull);
#if 0
			if(qualifying.isNamespace())
			{
				// look up 'id' in namespace (only declarations visible at point of definition of template)
				// result may be namespace or type
				// if type, substitute within instantiation context
				Scope& scope = *getNamespace(qualifying.value).declaration->enclosed;
				std::size_t visibility = enclosingType.instantiation.pointOfInstantiation;
				LookupResultRef result = findNamespaceDeclaration(scope, id, element.isNested ? LookupFilter(IsNestedName(visibility)) : LookupFilter(IsAny(visibility)));
				if(result == 0) // if the name was not found within the qualifying namespace
				{
					throw MemberNotFoundError(source, element.name, qualifying);
				}
				declaration = result;
			}
			else
#endif
			{
				const SimpleType* enclosing = qualifying.isSimple() ? &getSimpleType(qualifying.value) : 0;
				if(enclosing == 0
					|| !isClass(*enclosing->declaration))
				{
					// [temp.deduct] Attempting to use a type that is not a class type in a qualified name
					throw QualifyingIsNotClassError(source, qualifying);
				}

				instantiateClass(*enclosing, source, &enclosingType);
				std::size_t visibility = enclosing->instantiating ? enclosingType.instantiation.pointOfInstantiation : VISIBILITY_ALL;
				LookupResultRef result = findDeclaration(*enclosing, id, element.isNested ? LookupFilter(IsNestedName(visibility)) : LookupFilter(IsAny(visibility)));

				if(result == 0) // if the name was not found within the qualifying class
				{
#if 1
					LookupResultRef result = findDeclaration(*enclosing, id, element.isNested ? LookupFilter(IsNestedName(VISIBILITY_ALL)) : LookupFilter(IsAny(VISIBILITY_ALL)));
					if(result != 0)
					{
						std::cout << "visibility: " << visibility << std::endl;
						std::cout << "found with VISIBILITY_ALL: " << result.p->visibility << std::endl;
					}
#endif
					// [temp.deduct]
					// - Attempting to use a type in the qualifier portion of a qualified name that names a type when that
					//   type does not contain the specified member
					throw MemberNotFoundError(source, element.name, qualifying);
				}

				declaration = result;

				if(!isType(*declaration))
				{
					// [temp.deduct]
					// - Attempting to use a type in the qualifier portion of a qualified name that names a type when [...]
					//   the specified member is not a type where a type is required.
					throw MemberIsNotTypeError(source, element.name, qualifying);
				}

				SYMBOLS_ASSERT(isMember(*declaration));
				memberEnclosing = findEnclosingType(enclosing, declaration->scope); // the declaration must be a member of (a base of) the qualifying class: find which one.
			}
		}

#if 0
		if(isNamespace(*declaration))
		{
			type = pushType(gUniqueTypeNull, Namespace(declaration));
			return;
		}
#endif

		if(isClass(*declaration)
			|| isEnum(*declaration))
		{
			type = substitute(declaration, memberEnclosing, element.templateArguments, source, enclosingType);
			return;
		}
	
		// typedef
		SYMBOLS_ASSERT(declaration->specifiers.isTypedef);
		SYMBOLS_ASSERT(declaration->type.unique != 0);
		type = UniqueTypeWrapper(declaration->type.unique);
		if(declaration->type.isDependent)
		{
			SYMBOLS_ASSERT(memberEnclosing != 0);
			type = substitute(type, source, *memberEnclosing);
		}
	}
	virtual void visit(const DependentNonType& element)
	{
		// TODO: unify DependentNonType and NonType?
		if(isDependent(enclosingType))
		{
			// TODO: occurs when substituting with a dependent template argument list, if a template function is called with an empty (or partial) explicit template argument list.
			type.push_front(element);
			return;
		}
	
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
	virtual void visit(const SimpleType& element)
	{
		const SimpleType* result = substitute(element, source, enclosingType);
		type.push_front(*result);
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
			|| !isClass(*getSimpleType(classType.value).declaration))
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

inline UniqueTypeWrapper substitute(UniqueTypeWrapper dependent, Location source, const SimpleType& enclosingType)
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
inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source, const SimpleType* enclosing, bool allowDependent);
inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source, const SimpleType* enclosing, bool allowDependent);

inline void reportTypeInfo(const Type& type, const SimpleType* enclosing)
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

extern BuiltInTypeId gSignedInt;

template<typename T>
inline UniqueTypeWrapper getUniqueTypeImpl(const T& type, Location source, const SimpleType* enclosing, bool allowDependent)
{
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

inline UniqueTypeWrapper getUniqueType(const TypeId& type, Location source, const SimpleType* enclosing, bool allowDependent)
{
	return getUniqueTypeImpl(type, source, enclosing, allowDependent);
}

inline UniqueTypeWrapper getUniqueType(const Type& type, Location source, const SimpleType* enclosing, bool allowDependent = false)
{
	return getUniqueTypeImpl(type, source, enclosing, allowDependent);
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

inline bool isDependentImpl(Declaration* dependent, Scope* enclosing, Scope* templateParamScope)
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

inline std::size_t addBase(SimpleType& instance, UniqueTypeWrapper base, Location source)
{
	SYMBOLS_ASSERT(!isDependent(base));
	SYMBOLS_ASSERT(base.isSimple());
	const SimpleType& objectType = getSimpleType(base.value);
	std::size_t size = instantiateClass(objectType, source, &instance);
	SYMBOLS_ASSERT(isClass(*objectType.declaration));
	SYMBOLS_ASSERT(objectType.declaration->enclosed != 0); // this can occur when the primary template is incomplete, and a specialization was not chosen
	instance.bases.push_back(&objectType);
	return size;
}

inline bool isTemplate(const SimpleType& instance)
{
	if(instance.declaration->isTemplate)
	{
		return true;
	}
	return instance.enclosing != 0
		&& isTemplate(*instance.enclosing);
}

void dumpTemplateInstantiations(const SimpleType& instance, bool root = false);

inline std::size_t instantiateClass(const SimpleType& instanceConst, Location source, const SimpleType* enclosing, bool allowDependent)
{
	SimpleType& instance = const_cast<SimpleType&>(instanceConst);
	SYMBOLS_ASSERT(isClass(*instance.declaration));

	if(enclosing != 0)
	{
		ChildInstantiations& instantiations = const_cast<SimpleType*>(enclosing)->childInstantiations;
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
		const SimpleType& original = getSimpleType(instance.declaration->type.unique);

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

inline std::size_t requireCompleteObjectType(UniqueTypeWrapper type, Location source, const SimpleType* enclosing)
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
		const SimpleType& objectType = getSimpleType(type.value);
		if(isClass(*objectType.declaration))
		{
			return instantiateClass(objectType, source, enclosing) * count;
		}
	}
	return 4; // TODO: size of non-object types
}

inline UniqueTypeWrapper makeUniqueQualifying(const Qualifying& qualifying, Location source, const SimpleType* enclosingType, bool allowDependent = false)
{
	return qualifying.empty()
		|| isNamespace(*qualifying.back().declaration)
		? gUniqueTypeNull
		: getUniqueType(qualifying.back(), source, enclosingType, allowDependent);
}

inline const SimpleType* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const SimpleType* enclosing, bool allowDependent, UniqueTypeWrapper& unique)
{
	if(!qualifying.empty())
	{
		if(isNamespace(*qualifying.back().declaration))
		{
			return 0; // name is qualified by a namespace, therefore cannot be enclosed by a class
		}
		unique = getUniqueType(qualifying.back(), source, enclosing, allowDependent);
		if(allowDependent && qualifying.back().isDependent)
		{
			return 0;
		}
		const SimpleType& type = getSimpleType(unique.value);
		// [temp.inst] A class template is implicitly instantiated ... if the completeness of the class-type affects the semantics of the program.
		instantiateClass(type, source, enclosing, allowDependent);
		return &type;
	}
	return enclosing;
}

inline const SimpleType* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const SimpleType* enclosing, bool allowDependent)
{
	UniqueTypeWrapper tmp;
	return makeUniqueEnclosing(qualifying, source, enclosing, allowDependent, tmp);
}

inline bool deduceAndSubstitute(const UniqueTypeArray& parameters, const UniqueTypeArray& arguments, Location source, SimpleType& enclosing, TemplateArgumentsInstance& substituted)
{
	// deduce the partial-specialization's template arguments from the original argument list
	if(!deducePairs(parameters, arguments, enclosing.deducedArguments)
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
	SimpleType enclosing(declaration, 0);
	enclosing.deducedArguments.swap(deduced);
	enclosing.instantiated = true;
	if(!deduceAndSubstitute(specializationArguments, arguments, source, enclosing, substituted))
	{
		return false; // partial-specialization only matches if template-argument-deduction succeeds
	}
	// TODO: same as comparing deduced arguments with original template parameters?
	// TODO: not necessary unless testing partial ordering?
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

inline void makeUniqueTemplateArguments(TemplateArguments& templateArguments, TemplateArgumentsInstance& result, Location source, const SimpleType* enclosing, bool allowDependent = false)
{
	for(TemplateArguments::const_iterator i = templateArguments.begin(); i != templateArguments.end(); ++i)
	{
		UniqueTypeWrapper type;
		SYMBOLS_ASSERT((*i).type.declaration != 0);
		extern Declaration gNonType;
		if((*i).type.declaration == &gNonType)
		{
			allowDependent && (*i).expression.isValueDependent
				? pushUniqueType(type.value, DependentNonType((*i).expression))
				: pushUniqueType(type.value, NonType(evaluateExpression((*i).expression, (*i).source, enclosing)));
		}
		else
		{
			SYMBOLS_ASSERT((*i).type.unique != 0);
			type = getUniqueType((*i).type, (*i).source, enclosing, allowDependent && (*i).type.isDependent);
			SYMBOLS_ASSERT((*i).type.unique == type.value);
		}
		result.push_back(type);
	}
}

inline void makeUniqueTemplateParameters(const TemplateParameters& templateParams, TemplateArgumentsInstance& arguments, Location source, const SimpleType* enclosing, bool allowDependent)
{
	for(Types::const_iterator i = templateParams.begin(); i != templateParams.end(); ++i)
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
			UniqueExpression expression = makeUniqueExpression(NonTypeTemplateParameter(argument.declaration));
			allowDependent
				? pushUniqueType(result.value, DependentNonType(expression))
				: pushUniqueType(result.value, NonType(evaluateExpression(expression, source, enclosing)));
		}
		arguments.push_back(result);
	}
	SYMBOLS_ASSERT(!arguments.empty());
}

inline Declaration* findTemplateSpecialization(Declaration* declaration, TemplateArgumentsInstance& deducedArguments, const TemplateArgumentsInstance& arguments, Location source, const SimpleType* enclosing, bool allowDependent)
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
		// a partial-specialization may have dependent template-arguments: template<class T> class C<T*>
		makeUniqueTemplateArguments(declaration->templateArguments, specializationArguments, source, enclosing, true);

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


inline void makeUniqueTemplateArguments(const TemplateArguments& arguments, TemplateArgumentsInstance& templateArguments, Location source, const SimpleType* enclosingType, bool allowDependent)
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


inline const SimpleType* findEnclosingType(const SimpleType& enclosing, Scope* scope)
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
		const SimpleType* result = findEnclosingType(*(*i), scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

inline const SimpleType* findEnclosingType(const SimpleType* enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	for(const SimpleType* i = enclosing; i != 0; i = (*i).enclosing)
	{
		const SimpleType* result = findEnclosingType(*i, scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}


inline UniqueTypeWrapper makeUniqueSimpleType(const SimpleType& type)
{
	SYMBOLS_ASSERT(!(type.primary->isTemplate && isSpecialization(*type.primary))); // primary declaration must not be a specialization!
	return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, type));
}

struct LookupFailed : TypeError
{
	const SimpleType* enclosing;
	const Identifier* id;
	LookupFailed(const SimpleType* enclosing, const Identifier* id)
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
inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source, const SimpleType* enclosingType, bool allowDependent)
{
	// the type in which template-arguments are looked up: returns qualifying type if specified, else returns enclosingType
	UniqueTypeWrapper qualifying;
	const SimpleType* enclosing = makeUniqueEnclosing(type.qualifying, source, enclosingType, allowDependent, qualifying);
	Declaration* declaration = type.declaration;
	extern Declaration gDependentType;
	extern Declaration gDependentTemplate;
	extern Declaration gDependentNested;
	extern Declaration gDependentNestedTemplate;
	if(declaration == &gDependentType // T::Type
		|| declaration == &gDependentTemplate // T::Type<>
		|| declaration == &gDependentNested // T::Type::
		|| declaration == &gDependentNestedTemplate) // T::Type<>::
	{
		// this is a type-name (or template-id) preceded by a dependent nested-name-specifier
		SYMBOLS_ASSERT(allowDependent);
		bool isNested = declaration == &gDependentNested || declaration == &gDependentNestedTemplate;
		SYMBOLS_ASSERT(type.id != IdentifierPtr(0));
		TemplateArgumentsInstance templateArguments;
		makeUniqueTemplateArguments(type.templateArguments, templateArguments, source, enclosingType, allowDependent);
		return pushType(gUniqueTypeNull, DependentTypename(type.id->value, qualifying, templateArguments, isNested, declaration->isTemplate));
	}
	size_t index = declaration->templateParameter;
	if(index != INDEX_INVALID)
	{
		SYMBOLS_ASSERT(allowDependent);
		SYMBOLS_ASSERT(type.qualifying.empty());
		// Find the template-specialisation it belongs to:
		const SimpleType* enclosingType = findEnclosingType(enclosing, declaration->scope);
		if(enclosingType != 0
			&& !isDependent(*enclosingType)) // if the enclosing type is not dependent
		{
			SYMBOLS_ASSERT(!enclosingType->declaration->isSpecialization || enclosingType->instantiated); // a specialization must be instantiated (or in the process of instantiating)
			const TemplateArgumentsInstance& arguments = enclosingType->declaration->isSpecialization
				? enclosingType->deducedArguments
				: enclosingType->templateArguments;

			SYMBOLS_ASSERT(index < arguments.size());
			UniqueTypeWrapper result = arguments[index];
			SYMBOLS_ASSERT(result != gUniqueTypeNull); // fails for non-type template-argument
			return result;
		}

		TemplateArgumentsInstance templateArguments; // for template-template-parameter
		makeUniqueTemplateArguments(type.templateArguments, templateArguments, source, enclosingType, allowDependent);
		std::size_t templateParameterCount = declaration->isTemplate ? std::distance(declaration->templateParams.begin(), declaration->templateParams.end()) : 0;
		return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, DependentType(declaration, templateArguments, templateParameterCount)));
	}

	const SimpleType* memberEnclosing = isMember(*declaration) // if the declaration is a class member
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

	SimpleType tmp(declaration, memberEnclosing);
	SYMBOLS_ASSERT(declaration->type.declaration != &gArithmetic || tmp.enclosing == 0); // arithmetic types should not have an enclosing template!
	if(declaration->isTemplate)
	{
		tmp.declaration = tmp.primary = findPrimaryTemplate(declaration);

		bool isEnclosingSpecialization = type.isEnclosingClass && isSpecialization(*type.declaration);

		// [temp.local]: when the name of a class template is used without arguments, substitute the parameters (in case of an enclosing explicit/partial-specialization, substitute the arguments
		const TemplateArguments& defaults = tmp.declaration->templateParams.defaults;
		SYMBOLS_ASSERT(!defaults.empty());
		if(type.isImplicitTemplateId // if no template argument list was specified
			&& !isEnclosingSpecialization) // and the type is not the name of an enclosing class-template explicit/partial-specialization
		{
			// when the type refers to a template-name outside an enclosing class, it is a template-template-parameter:
			// we substitute the primary template's (possibly dependent) template parameters.
			bool dependent = allowDependent || !type.isEnclosingClass;
			makeUniqueTemplateParameters(tmp.declaration->templateParams, tmp.templateArguments, source, enclosingType, dependent);
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
					const SimpleType* enclosing = isTemplateParamDefault ? &tmp : enclosingType; // resolve dependent template-parameter-defaults in context of template class
					result = getUniqueType(argument.type, argument.source, enclosing, allowDependent);
					SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
				}
				tmp.templateArguments.push_back(result);
			}
			SYMBOLS_ASSERT(allowDependent || !tmp.templateArguments.empty()); // dependent types may have no arguments
		}
	}
	SYMBOLS_ASSERT(tmp.bases.empty());
	SYMBOLS_ASSERT(tmp.children.empty());
	static size_t uniqueId = 0;
	tmp.uniqueId = ++uniqueId;
	return makeUniqueSimpleType(tmp);
}


inline std::size_t evaluateArraySize(const ExpressionWrapper& expression, Location source, const SimpleType* enclosing)
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
	const SimpleType* enclosing;
	bool allowDependent;
	TypeSequenceMakeUnique(UniqueType& type, Location source, const SimpleType* enclosing, bool allowDependent)
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

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source, const SimpleType* enclosing, bool allowDependent)
{
	UniqueTypeWrapper result = makeUniqueType(*static_cast<const Type*>(&type), source, enclosing, allowDependent);
	result.value.addQualifiers(type.qualifiers);
	TypeSequenceMakeUnique visitor(result.value, source, enclosing, allowDependent);
	type.typeSequence.accept(visitor);
	return result;
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

extern Declaration gEnumerator;

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
extern BuiltInTypeId gChar;
extern BuiltInTypeId gSignedChar;
extern BuiltInTypeId gUnsignedChar;
extern BuiltInTypeId gSignedShortInt;
extern BuiltInTypeId gUnsignedShortInt;
extern BuiltInTypeId gSignedInt;
extern BuiltInTypeId gUnsignedInt;
extern BuiltInTypeId gSignedLongInt;
extern BuiltInTypeId gUnsignedLongInt;
extern BuiltInTypeId gSignedLongLongInt;
extern BuiltInTypeId gUnsignedLongLongInt;
extern BuiltInTypeId gWCharT;
extern BuiltInTypeId gBool;
extern BuiltInTypeId gFloat;
extern BuiltInTypeId gDouble;
extern BuiltInTypeId gLongDouble;
extern BuiltInTypeId gVoid;

typedef ArrayRange<BuiltInType> BuiltInTypeArrayRange;


typedef UniqueTypeGeneric<false> UserType;

template<bool builtIn, int N>
struct TypeTuple;

template<bool builtIn>
struct TypeTuple<builtIn, 1> : UniqueTypeGeneric<builtIn>
{
	TypeTuple(UniqueTypeGeneric<builtIn> type)
		: UniqueTypeGeneric<builtIn>(type)
	{
	}
};

template<bool builtIn>
struct TypeTuple<builtIn, 2> : std::pair<UniqueTypeGeneric<builtIn>, UniqueTypeGeneric<builtIn> >
{
	typedef UniqueTypeGeneric<builtIn> Value;
	TypeTuple(Value first, Value second)
		: std::pair<Value, Value>(first, second)
	{
	}
};

template<int N>
struct BuiltInGenericType : BuiltInType
{
	typedef UserType (*Substitute)(TypeTuple<false, N> args);
	Substitute substitute;
	BuiltInGenericType(BuiltInType type, Substitute substitute) : BuiltInType(type), substitute(substitute)
	{
	}
};

typedef BuiltInGenericType<1> BuiltInGenericType1;
typedef BuiltInGenericType<2> BuiltInGenericType2;

typedef ArrayRange<BuiltInGenericType1> BuiltInGenericType1ArrayRange;
typedef ArrayRange<BuiltInGenericType2> BuiltInGenericType2ArrayRange;


inline bool isVoid(UniqueTypeWrapper type)
{
	return type.value.getPointer() == gVoid.value.getPointer();
}

inline bool isVoidPointer(UniqueTypeWrapper type)
{
	if(!type.isPointer())
	{
		return false;
	}
	type.pop_front();
	return isVoid(type);
}

inline bool isVoidParameter(const TypeId& type)
{
	return type.declaration == &gVoidDeclaration
		&& type.typeSequence.empty();
}


struct StringLiteralTypeId : BuiltInTypeId
{
	StringLiteralTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: BuiltInTypeId(declaration, allocator)
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
	// [lex.ccon]
	// An ordinary character literal that contains a single c-char has type char.
	// A wide-character literal has type wchar_t.
	return *value == 'L' ? gWCharT : gChar; // TODO: multicharacter literal
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
extern Name gOperatorSubscriptId;

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
	return gOperatorSubscriptId;
}

inline Name getOverloadedOperatorId(cpp::unary_operator* symbol)
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

inline Name getOverloadedOperatorId(cpp::pm_expression_default* symbol)
{
	if(symbol->op->id == cpp::pm_operator::ARROWSTAR)
	{
		return gOperatorArrowStarId;
	}
	return Name();
}
inline Name getOverloadedOperatorId(cpp::multiplicative_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::multiplicative_operator::STAR: return gOperatorStarId;
	case cpp::multiplicative_operator::DIVIDE: return gOperatorDivideId;
	case cpp::multiplicative_operator::PERCENT: return gOperatorPercentId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::additive_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::additive_operator::PLUS: return gOperatorPlusId;
	case cpp::additive_operator::MINUS: return gOperatorMinusId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::shift_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::shift_operator::SHIFTLEFT: return gOperatorShiftLeftId;
	case cpp::shift_operator::SHIFTRIGHT: return gOperatorShiftRightId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::relational_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::relational_operator::LESS: return gOperatorLessId;
	case cpp::relational_operator::GREATER: return gOperatorGreaterId;
	case cpp::relational_operator::LESSEQUAL: return gOperatorLessEqualId;
	case cpp::relational_operator::GREATEREQUAL: return gOperatorGreaterEqualId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::equality_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::equality_operator::EQUAL: return gOperatorEqualId;
	case cpp::equality_operator::NOTEQUAL: return gOperatorNotEqualId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::and_expression_default* symbol)
{
	return gOperatorAndId;
}
inline Name getOverloadedOperatorId(cpp::exclusive_or_expression_default* symbol)
{
	return gOperatorXorId;
}

inline Name getOverloadedOperatorId(cpp::inclusive_or_expression_default* symbol)
{
	return gOperatorOrId;
}

inline Name getOverloadedOperatorId(cpp::logical_and_expression_default* symbol)
{
	return gOperatorAndAndId;
}

inline Name getOverloadedOperatorId(cpp::logical_or_expression_default* symbol)
{
	return gOperatorOrOrId;
}

inline Name getOverloadedOperatorId(cpp::assignment_expression_suffix* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::assignment_operator::ASSIGN: return gOperatorAssignId;
	case cpp::assignment_operator::STAR: return gOperatorStarAssignId;
	case cpp::assignment_operator::DIVIDE: return gOperatorDivideAssignId;
	case cpp::assignment_operator::PERCENT: return gOperatorPercentAssignId;
	case cpp::assignment_operator::PLUS: return gOperatorPlusAssignId;
	case cpp::assignment_operator::MINUS: return gOperatorMinusAssignId;
	case cpp::assignment_operator::SHIFTRIGHT: return gOperatorShiftRightAssignId;
	case cpp::assignment_operator::SHIFTLEFT: return gOperatorShiftLeftAssignId;
	case cpp::assignment_operator::AND: return gOperatorAndAssignId;
	case cpp::assignment_operator::XOR: return gOperatorXorAssignId;
	case cpp::assignment_operator::OR: return gOperatorOrAssignId;
	default: break;
	}
	throw SymbolsError();
}

extern Identifier gConversionFunctionId;
extern Identifier gOperatorFunctionTemplateId;
// TODO: don't declare if id is anonymous?
extern Identifier gAnonymousId;
extern Identifier gDestructorId;


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

inline UniqueTypeWrapper ternaryOperatorNull(UniqueTypeWrapper first, UniqueTypeWrapper second, UniqueTypeWrapper third)
{
	return gUniqueTypeNull;
}



inline bool isClass(UniqueTypeWrapper type)
{
	return type.isSimple() && getSimpleType(type.value).declaration->type.declaration == &gClass;
}

inline bool isEnum(UniqueTypeWrapper type)
{
	return type.isSimple() && getSimpleType(type.value).declaration->type.declaration == &gEnum;
}

inline bool isObject(UniqueTypeWrapper type)
{
	return type != gVoid && !type.isReference() && !type.isFunction();
}

inline bool isComplete(const UniqueTypeId& type)
{
	return type.isSimple() && isComplete(*getSimpleType(type.value).declaration);
}

inline bool isArithmetic(const UniqueTypeId& type)
{
	return type.isSimple() && getSimpleType(type.value).declaration->type.declaration == &gArithmetic;
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

inline bool isPromotedIntegral(UniqueTypeWrapper type)
{
	return type == gSignedInt
		|| type == gUnsignedInt
		|| type == gSignedLongInt
		|| type == gUnsignedLongInt
		|| type == gSignedLongLongInt
		|| type == gUnsignedLongLongInt;
}

inline bool isPromotedArithmetic(UniqueTypeWrapper type)
{
	return isPromotedIntegral(type) || isFloating(type);
}


inline bool isEnumeration(const UniqueTypeId& type)
{
	return isEnum(type);
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

inline bool isEqual(const Type& left, const Type& right)
{
	SYMBOLS_ASSERT(left.unique != 0);
	SYMBOLS_ASSERT(right.unique != 0);
	return left.unique == right.unique;
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
	bool isReference;
	UniqueTypeWrapper matched;
	StandardConversionSequence(ScsRank rank, CvQualifiers adjustment, UniqueTypeWrapper matched = gUniqueTypeNull)
		: rank(rank), adjustment(adjustment), isReference(false), matched(matched)
	{
	}
};

const StandardConversionSequence STANDARDCONVERSIONSEQUENCE_INVALID = StandardConversionSequence(SCSRANK_INVALID, CvQualifiers());

inline bool findBase(const SimpleType& other, const SimpleType& type)
{
	SYMBOLS_ASSERT(other.declaration != &gParam); // TODO: when type-evaluation fails, sometimes template-params are uniqued
	SYMBOLS_ASSERT(other.declaration->enclosed != 0);
	SYMBOLS_ASSERT(isClass(*type.declaration));
	for(UniqueBases::const_iterator i = other.bases.begin(); i != other.bases.end(); ++i)
	{
		const SimpleType& base = *(*i);
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
inline bool isBaseOf(const SimpleType& type, const SimpleType& other, Location source, const SimpleType* enclosing)
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


// 5 Expressions
// paragraph 9: usual arithmetic conversions
// [expr]
// Many binary operators that expect operands of arithmetic or enumeration type cause conversions and yield
// result types in a similar way. The purpose is to yield a common type, which is also the type of the result.
// This pattern is called the usual arithmetic conversions
inline BuiltInType usualArithmeticConversions(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	SYMBOLS_ASSERT(left != gUniqueTypeNull);
	SYMBOLS_ASSERT(right != gUniqueTypeNull);

	SYMBOLS_ASSERT(isArithmetic(left) || isEnumeration(left));
	SYMBOLS_ASSERT(isArithmetic(right) || isEnumeration(right));

	if(isEqual(left, gLongDouble)
		|| isEqual(right, gLongDouble))
	{
		return gLongDouble;
	}
	if(isEqual(left, gDouble)
		|| isEqual(right, gDouble))
	{
		return gDouble;
	}
	if(isEqual(left, gFloat)
		|| isEqual(right, gFloat))
	{
		return gFloat;
	}
	left = promoteToIntegralType(left);
	right = promoteToIntegralType(right);
	if(isEqual(left, gUnsignedLongInt)
		|| isEqual(right, gUnsignedLongInt))
	{
		return gUnsignedLongInt;
	}
	if((isEqual(left, gSignedLongInt)
		&& isEqual(right, gUnsignedInt))
		|| (isEqual(left, gUnsignedInt)
		&& isEqual(right, gSignedLongInt)))
	{
		return gUnsignedLongInt;
	}
	if(isEqual(left, gSignedLongInt)
		|| isEqual(right, gSignedLongInt))
	{
		return gSignedLongInt;
	}
	if(isEqual(left, gUnsignedInt)
		|| isEqual(right, gUnsignedInt))
	{
		return gUnsignedInt;
	}
	return gSignedInt;
}

// T[] -> T*
inline UniqueTypeWrapper applyArrayToPointerConversion(UniqueTypeWrapper type)
{
	SYMBOLS_ASSERT(type.isArray());
	type.pop_front();
	type.push_front(PointerType());
	return type;
}

// T() -> T(*)()
inline UniqueTypeWrapper applyFunctionToPointerConversion(UniqueTypeWrapper type)
{
	SYMBOLS_ASSERT(type.isFunction());
	type.push_front(PointerType());
	return type;
}

inline UniqueTypeWrapper applyLvalueToRvalueConversion(UniqueTypeWrapper type)
{
	if(type.isArray())
	{
		return applyArrayToPointerConversion(type);
	}
	if(type.isFunction())
	{
		return applyFunctionToPointerConversion(type);
	}
	return type;
}

inline CvQualifiers makeQualificationAdjustment(UniqueTypeId to, UniqueTypeId from)
{
	return CvQualifiers(to.value.getQualifiers().isConst > from.value.getQualifiers().isConst,
		to.value.getQualifiers().isVolatile > from.value.getQualifiers().isVolatile);
}


struct TargetType : UniqueTypeWrapper
{
	TargetType(UniqueTypeWrapper type)
		: UniqueTypeWrapper(type)
	{
	}
};

extern BuiltInTypeId gArithmeticPlaceholder;
extern BuiltInTypeId gIntegralPlaceholder;
extern BuiltInTypeId gPromotedIntegralPlaceholder;
extern BuiltInTypeId gPromotedArithmeticPlaceholder;
extern BuiltInTypeId gEnumerationPlaceholder;
extern BuiltInTypeId gPointerToAnyPlaceholder;
extern BuiltInTypeId gPointerToObjectPlaceholder;
extern BuiltInTypeId gPointerToClassPlaceholder;
extern BuiltInTypeId gPointerToFunctionPlaceholder;
extern BuiltInTypeId gPointerToMemberPlaceholder;

inline UniqueTypeWrapper getExactMatch(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	return isEqual(to, from) ? from : gUniqueTypeNull;
}

inline UniqueTypeWrapper getExactMatch(TargetType to, UniqueTypeWrapper from)
{
	if(to == gPromotedArithmeticPlaceholder
		&& isPromotedArithmetic(from))
	{
		return from;
	}
	if(to == gArithmeticPlaceholder
		&& isArithmetic(from))
	{
		return from;
	}
	if(to == gIntegralPlaceholder
		&& isIntegral(from))
	{
		return from;
	}
	if(to == gPromotedIntegralPlaceholder
		&& isPromotedIntegral(from))
	{
		return from;
	}
	if(to == gEnumerationPlaceholder
		&& isEnumeration(from))
	{
		return from;
	}
	if(from.isPointer())
	{
		if(to == gPointerToAnyPlaceholder)
		{
			return from;
		}
		if(to == gPointerToObjectPlaceholder
			&& isObject(popType(from)))
		{
			return from;
		}
		if(to == gPointerToClassPlaceholder
			&& isClass(popType(from)))
		{
			return from;
		}
		if(to == gPointerToFunctionPlaceholder
			&& popType(from).isFunction())
		{
			return from;
		}
	}
	if(to == gPointerToMemberPlaceholder
		&& from.isMemberPointer())
	{
		return from;
	}
	return getExactMatch(UniqueTypeWrapper(to), from);
}

template<typename To>
inline UniqueTypeWrapper getExactMatchNoQualifiers(To to, UniqueTypeWrapper from)
{
	to.value.setQualifiers(CvQualifiers());
	from.value.setQualifiers(CvQualifiers());
	return getExactMatch(to, from);
}

inline bool isArithmetic(TargetType to)
{
	if(to == gArithmeticPlaceholder
		|| to == gPromotedArithmeticPlaceholder
		|| to == gIntegralPlaceholder
		|| to == gPromotedIntegralPlaceholder)
	{
		return true;
	}
	return isArithmetic(UniqueTypeWrapper(to));
}

template<typename To>
inline StandardConversionSequence makeScsPromotion(To to, UniqueTypeWrapper from)
{
	if(isArithmetic(from)
		&& isArithmetic(to))
	{
		if(from == gFloat)
		{
			UniqueTypeWrapper matched = getExactMatch(to, gDouble);
			if(matched != gUniqueTypeNull)
			{
				return StandardConversionSequence(SCSRANK_PROMOTION, CvQualifiers(), matched);
			}
		}
		UniqueTypeWrapper matched = getExactMatch(to, promoteToIntegralType(from));
		if(matched != gUniqueTypeNull)
		{
			return StandardConversionSequence(SCSRANK_PROMOTION, CvQualifiers(), matched);
		}	
	}
	if(isEnum(from)
		&& getExactMatch(to, gSignedInt) != gUniqueTypeNull) // TODO: correct type of integral promotion for enum
	{
		return StandardConversionSequence(SCSRANK_PROMOTION, CvQualifiers(), gSignedInt);
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

inline bool isGeneralPointer(UniqueTypeWrapper type)
{
	return type.isPointer() || type.isMemberPointer();
}

inline bool isPointerPlaceholder(UniqueTypeWrapper type)
{
	return type == gPointerToAnyPlaceholder
		|| type == gPointerToObjectPlaceholder
		|| type == gPointerToClassPlaceholder
		|| type == gPointerToFunctionPlaceholder;
}

inline bool isGeneralPointer(TargetType type)
{
	return isPointerPlaceholder(type)
		|| type == gPointerToMemberPlaceholder
		|| isGeneralPointer(UniqueTypeWrapper(type));
}

template<typename To>
inline StandardConversionSequence makeScsConversion(Location source, const SimpleType* enclosing, To to, UniqueTypeWrapper from, bool isNullPointerConstant = false)
{
	SYMBOLS_ASSERT(to.value.getQualifiers() == CvQualifiers());
	SYMBOLS_ASSERT(from.value.getQualifiers() == CvQualifiers());
	if((isArithmetic(from) || isEnumeration(from))
		&& isArithmetic(to))
	{
		// can convert from enumeration to integer/floating/bool, but not in reverse
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers(), gSignedInt); // TODO: correct type of integral conversion
	}
	if(isGeneralPointer(to)
		&& isIntegral(from)
		&& isNullPointerConstant)
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // 0 -> T*, 0 -> T C::*
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& getInner(to.value).getPointer() == gVoid.value.getPointer()) // ignore cv-qualifiers here!
	{
		to.pop_front();
		from.pop_front();
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from)) // T* -> void*
			: STANDARDCONVERSIONSEQUENCE_INVALID;
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& isBaseOf(getSimpleType(getInner(to.value)), getSimpleType(getInner(from.value)), source, enclosing))
	{
		to.pop_front();
		from.pop_front();
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from)) // D* -> B*
			: STANDARDCONVERSIONSEQUENCE_INVALID;
	}
	if(to.isMemberPointer()
		&& from.isMemberPointer()
		&& isBaseOf(getMemberPointerClass(to.value), getMemberPointerClass(from.value), source, enclosing))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // T D::* -> T B::*
	}
	if(to == gBool
		&& (from.isPointer() || from.isMemberPointer()))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // T* -> bool, T C::* -> bool
	}
	if(to.isSimple()
		&& from.isSimple()
		&& isBaseOf(getSimpleType(to.value), getSimpleType(from.value), source, enclosing))
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

template<typename To>
inline StandardConversionSequence makeScsExactMatch(To target, UniqueTypeWrapper from)
{
	UniqueTypeWrapper matched = getExactMatchNoQualifiers(target, from);
	UniqueTypeWrapper to = target;
	for(;;)
	{
		if(matched != gUniqueTypeNull)
		{
			return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
				? StandardConversionSequence(SCSRANK_EXACT, makeQualificationAdjustment(to, from), matched)
				: STANDARDCONVERSIONSEQUENCE_INVALID;
		}
		if(to.isPointer()
			&& from.isPointer())
		{
		}
		else if(to.isMemberPointer()
			&& from.isMemberPointer()
			&& getMemberPointerType(to.value).type == getMemberPointerType(from.value).type)
		{
		}
		else
		{
			break;
		}
		to.pop_front();
		from.pop_front();
		matched = getExactMatchNoQualifiers(to, from);
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

// 13.3.3.1 [over.best.ics]
template<typename To>
inline StandardConversionSequence makeStandardConversionSequence(To to, UniqueTypeWrapper from, Location source, const SimpleType* enclosing, bool isNullPointerConstant = false, bool isLvalue = false)
{
	SYMBOLS_ASSERT(to != gUniqueTypeNull);
	SYMBOLS_ASSERT(from != gUniqueTypeNull);

	// ignore top level cv-qualifiers
	to.value.setQualifiers(CvQualifiers());
	from.value.setQualifiers(CvQualifiers());

	UniqueTypeWrapper tmp = from;
	from = applyLvalueToRvalueConversion(from); // TODO: lvalue-ness

	if(tmp == from // no l-value to rvalue conversion
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

	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

inline bool isProperSubsequence(CvQualifiers l, CvQualifiers r)
{
	return !l.isConst && !l.isVolatile
		&& (r.isConst || r.isVolatile); // true if r has any cv-qualification while l does not
}

inline bool isProperSubsequence(const StandardConversionSequence& l, const StandardConversionSequence& r)
{
	// TODO: consider lvalue-transformation!
	return (l.rank == SCSRANK_IDENTITY && r.rank != SCSRANK_IDENTITY)
		|| (l.rank == r.rank
			&& !l.isReference && !r.isReference && isProperSubsequence(l.adjustment, r.adjustment)); // TODO: compare cv-qualification signature of multi-level pointer
}

// [over.ics.rank]
// Standard conversion sequence S1 is a better conversion sequence than standard conversion sequence
// S2 if
// - S1 is a proper subsequence of S2 (comparing the conversion sequences in the canonical form
//   defined by 13.3.3.1.1, excluding any Lvalue Transformation; the identity conversion sequence is
//   considered to be a subsequence of any non-identity conversion sequence) or, if not that,
// - the rank of S1 is better than the rank of S2, or S1 and S2 have the same rank and are distinguishable
//   by the rules in the paragraph below, or, if not that,
// - S1 and S2 differ only in their qualification conversion and yield similar types T1 and T2 (4.4),
//   respectively, and the cv-qualification signature of type T1 is a proper subset of the cv-qualification
//   signature of type T2.
// - S1 and S2 are reference bindings (8.5.3), and the types to which the references refer are the same
//   type except for top-level cv-qualifiers, and the type to which the reference initialized by S2 refers
//   is more cv-qualified than the type to which the reference initialized by S1 refers.
// - User-defined conversion sequence U1 is a better conversion sequence than another user-defined conversion
//   sequence U2 if they contain the same user-defined conversion function or constructor and if the
//   second standard conversion sequence of U1 is better than the second standard conversion sequence of
//   U2.
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
	if(l.isReference && r.isReference
		&& isGreaterCvQualification(r.adjustment, l.adjustment))
	{
		return true; // both sequences are similar references, and the second is more cv-qualified than the first.
	}
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
	StandardConversionSequence sequence; // if user-defined, this is the second standard conversion
	IcsType type;
	Declaration* conversion; // if user-defined, this is the declaration of the conversion function
	explicit ImplicitConversion(StandardConversionSequence sequence, IcsType type = ICSTYPE_STANDARD, Declaration* conversion = 0)
		: sequence(sequence), type(type), conversion(conversion)
	{
	}
};

const ImplicitConversion IMPLICITCONVERSION_ELLIPSIS = ImplicitConversion(StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers()), ICSTYPE_ELLIPSIS);


struct FunctionOverload
{
	Declaration* declaration;
	UniqueTypeWrapper type;
	FunctionOverload(Declaration* declaration, UniqueTypeWrapper type)
		: declaration(declaration), type(type)
	{
	}
};

template<typename To>
FunctionOverload findBestConversionFunction(To to, UniqueTypeWrapper from, Location source, const SimpleType* enclosing, bool isNullPointerConstant = false, bool isLvalue = false);

template<typename To>
ImplicitConversion makeImplicitConversionSequence(To to, UniqueTypeWrapper from, Location source, const SimpleType* enclosing, bool isNullPointerConstant = false, bool isLvalue = false, bool isUserDefinedConversion = false)
{
	SYMBOLS_ASSERT(to != gUniqueTypeNull);
	SYMBOLS_ASSERT(from != gUniqueTypeNull);

	bool isReference = false;
	if(from.isReference())
	{
		isLvalue = true;
		isReference = true;
		from.pop_front(); // TODO: removal of reference won't be detected later
	}

	// 13.3.3.1.4 [over.ics.ref]: reference binding
	if(to.isReference()) 
	{
		to.pop_front();
		// [dcl.init.ref]
		// Given types "cv1 T1" and "cv2 T2," "cv1 T1" is reference-related to "cv2 T2" if T1 is the same type as
		// T2, or T1 is a base class of T2. "cv1 T1" is reference-compatible with "cv2 T2" if T1 is reference-related
		// to T2 and cv1 is the same cv-qualification as, or greater cv-qualification than, cv2. For purposes of overload
		// resolution, cases for which cv1 is greater cv-qualification than cv2 are identified as reference-compatible
		// with added qualification (see 13.3.3.2).
		// 
		// A reference to type "cv1 T1" is initialized by an expression of type "cv2 T2" as follows:
		// If the initializer expression
		// - is an lvalue (but is not a bit-field), and "cv1 T1" is reference-compatible with "cv2 T2," or
		// - has a class type (i.e., T2 is a class type) and can be implicitly converted to an lvalue of type
		//   "cv3 T3," where "cv1 T1" is reference-compatible with "cv3 T3"
		// then the reference is bound directly to the initializer expression lvalue in the first case, and the reference
		// is bound to the lvalue result of the conversion in the second case. In these cases the reference is said to
		// bind directly to the initializer expression.
		if(isLvalue
			&& (isEqualCvQualification(to, from)
			|| isGreaterCvQualification(to, from)))
		{
			// [over.ics.ref]
			// When a parameter of reference type binds directly (8.5.3) to an argument expression, the implicit conversion
			// sequence is the identity conversion, unless the argument expression has a type that is a derived class of
			// the parameter type, in which case the implicit conversion sequence is a derived-to-base Conversion
			UniqueTypeWrapper matched = getExactMatchNoQualifiers(to, from);
			if(matched != gUniqueTypeNull)
			{
				StandardConversionSequence sequence(SCSRANK_IDENTITY, to.value.getQualifiers(), matched);
				sequence.isReference = true;
				return ImplicitConversion(sequence);
			}
			if(to.isSimple()
				&& from.isSimple()
				&& isBaseOf(getSimpleType(to.value), getSimpleType(from.value), source, enclosing))
			{
				StandardConversionSequence sequence(SCSRANK_CONVERSION, to.value.getQualifiers());
				sequence.isReference = true;
				return ImplicitConversion(sequence);
			}
			// drop through...
		}
		if(isClass(from))
		{
			To tmp = to;
			tmp.push_front(ReferenceType());
			FunctionOverload overload = findBestConversionFunction(tmp, from, source, enclosing);
			if(overload.declaration != 0)
			{
				// If the parameter binds directly to the result of applying a conversion function to the argument
				// expression, the implicit conversion sequence is a user-defined conversion sequence (13.3.3.1.2), with
				// the second standard conversion sequence either an identity conversion or, if the conversion function returns
				// an entity of a type that is a derived class of the parameter type, a derived-to-base Conversion.
				UniqueTypeWrapper type = overload.type;
				SYMBOLS_ASSERT(type.isReference());
				type.pop_front();
				bool isIdentity = type.value.getPointer() == to.value.getPointer();
				StandardConversionSequence second(isIdentity ? SCSRANK_IDENTITY : SCSRANK_CONVERSION, to.value.getQualifiers());
				second.isReference = true;
				return ImplicitConversion(second, ICSTYPE_USERDEFINED, overload.declaration);
			}
			// drop through...
		}

		// if not bound directly, a standard conversion is required (which produces an rvalue)
		if(!to.value.getQualifiers().isConst
			|| to.value.getQualifiers().isVolatile) // 8.5.3-5: otherwise, the reference shall be to a non-volatile const type
		{
			// can't bind rvalue to a non-const reference
			return ImplicitConversion(STANDARDCONVERSIONSEQUENCE_INVALID);
		}
		// drop through...
	}


	// [over.best.ics]
	// When the parameter type is not a reference, the implicit conversion sequence models a copy-initialization
	// of the parameter from the argument expression. The implicit conversion sequence is the one required to
	// convert the argument expression to an rvalue of the type of the parameter. [Note: when the parameter has a
	// class type, this is a conceptual conversion defined for the purposes of clause 13; the actual initialization is
	// defined in terms of constructors and is not a conversion. ] Any difference in top-level cv-qualification is
	// subsumed by the initialization itself and does not constitute a conversion. [Example: a parameter of type A
	// can be initialized from an argument of type const A. The implicit conversion sequence for that case is
	// the identity sequence; it contains no "conversion" from const A to A. ] When the parameter has a class
	// type and the argument expression has the same type, the implicit conversion sequence is an identity conversion.
	// When the parameter has a class type and the argument expression has a derived class type, the
	// implicit conversion sequence is a derived-to-base Conversion from the derived class to the base class.
	// [Note: there is no such standard conversion; this derived-to-base Conversion exists only in the description
	// of implicit conversion sequences. ] A derived-to-base Conversion has Conversion rank

	if(isClass(to))
	{
		// [dcl.init] If the destination type is a (possibly cv-qualified) class type: [..] where the cv-unqualified version
		// of the source type is the same class as, or a derived class of, the class of the destination, constructors
		// are considered.
		// TODO: perform overload resolution to choose a constructor
		// TODO: add implicit copy constructor if not already declared
		// for now, always allow conversion if 'from' is same or derived
		UniqueTypeWrapper matched = getExactMatchNoQualifiers(to, from);
		if(matched != gUniqueTypeNull)
		{
			return ImplicitConversion(StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers(), matched));
		}
		if(to.isSimple()
			&& from.isSimple()
			&& isBaseOf(getSimpleType(to.value), getSimpleType(from.value), source, enclosing))
		{
			return ImplicitConversion(StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()));
		}
	}

	if(!isUserDefinedConversion
		&& (isClass(to)
			|| isClass(from)))
	{

		// [over.ics.user]
		// A user-defined conversion sequence consists of an initial standard conversion sequence followed by a
		// user-defined conversion (12.3) followed by a second standard conversion sequence. If the user-defined
		// conversion is specified by a constructor (12.3.1), the initial standard conversion sequence converts the
		// source type to the type required by the argument of the constructor. If the user-defined conversion is specified
		// by a conversion function (12.3.2), the initial standard conversion sequence converts the source type to
		// the implicit object parameter of the conversion function.

		// [dcl.init]
		// Otherwise (i.e., for the remaining copy-initialization cases), user-defined conversion sequences that
		// can convert from the source type to the destination type or (when a conversion function is used) to a
		// derived class thereof are enumerated as described in 13.3.1.4, and the best one is chosen through
		// overload resolution (13.3). If the conversion cannot be done or is ambiguous, the initialization is
		// ill-formed.

		FunctionOverload overload = findBestConversionFunction(to, from, source, enclosing, isNullPointerConstant, isLvalue);

		if(overload.declaration == 0)
		{
			return ImplicitConversion(STANDARDCONVERSIONSEQUENCE_INVALID);
		}

		// The second standard conversion sequence converts the result of the user-defined conversion to the target
		// type for the sequence.
		bool isLvalue = overload.type.isReference(); // TODO: proper lvalueness
		StandardConversionSequence second = makeStandardConversionSequence(to, removeReference(overload.type), source, enclosing, false, isLvalue);
		second.isReference = isReference;
		return ImplicitConversion(second, ICSTYPE_USERDEFINED, overload.declaration);
	}

	// standard conversion
	StandardConversionSequence sequence = makeStandardConversionSequence(to, from, source, enclosing, isNullPointerConstant, isLvalue);
	sequence.isReference = isReference;
	return ImplicitConversion(sequence);
}


inline IcsRank getIcsRank(ScsRank rank)
{
	switch(rank)
	{
	case SCSRANK_IDENTITY:
	case SCSRANK_EXACT: return ICSRANK_STANDARDEXACT;
	case SCSRANK_PROMOTION: return ICSRANK_STANDARDPROMOTION;
	case SCSRANK_CONVERSION: return ICSRANK_STANDARDCONVERSION;
	}
	return ICSRANK_INVALID;
}

inline IcsRank getIcsRank(UniqueTypeWrapper to, UniqueTypeWrapper from, Location source, const SimpleType* enclosing, bool isNullPointerConstant = false, bool isLvalue = false)
{
	ImplicitConversion conversion = makeImplicitConversionSequence(to, from, source, enclosing, isNullPointerConstant, isLvalue);
	return getIcsRank(conversion.sequence.rank);
}

// [over.ics.rank]
inline bool isBetter(const ImplicitConversion& l, const ImplicitConversion& r)
{
	if(l.type != r.type)
	{
		return l.type < r.type;
	}
	// TODO: User-defined conversion sequence U1 is a better conversion sequence than another user-defined conversion
	// sequence U2 if they contain the same user-defined conversion function or constructor and if the
	// second standard conversion sequence of U1 is better than the second standard conversion sequence of
	// U2.
	return isBetter(l.sequence, r.sequence);
}

inline bool isValid(const ImplicitConversion& conversion)
{
	return conversion.sequence.rank != SCSRANK_INVALID;
}


typedef std::vector<ImplicitConversion> ArgumentConversions;

struct FunctionTemplate
{
	ParameterTypes parameters;
	UniqueTypeArray templateParameters;
	FunctionTemplate()
	{
	}
	FunctionTemplate(const ParameterTypes& parameters, const UniqueTypeArray& templateParameters)
		: parameters(parameters), templateParameters(templateParameters)
	{
	}
};

struct CandidateFunction : FunctionOverload, FunctionTemplate
{
	ArgumentConversions conversions;
	bool isTemplate;
	CandidateFunction()
		: FunctionOverload(0, gUniqueTypeNull)
	{
	}
	CandidateFunction(FunctionOverload overload, const FunctionTemplate& functionTemplate = FunctionTemplate())
		: FunctionOverload(overload), FunctionTemplate(functionTemplate), isTemplate(overload.declaration->isTemplate)
	{
	}
};

extern BuiltInTypeId gImplicitObjectParameter;

inline bool isMoreSpecialized(const FunctionTemplate& left, const FunctionTemplate& right)
{
	bool isMoreCvQualified = false;
	UniqueTypeArray leftDeduced(left.templateParameters.size(), gUniqueTypeNull);
	UniqueTypeArray rightDeduced(right.templateParameters.size(), gUniqueTypeNull);
	SYMBOLS_ASSERT(left.parameters.size() == right.parameters.size());
	UniqueTypeArray::const_iterator l = left.parameters.begin();
	UniqueTypeArray::const_iterator r = right.parameters.begin();
	if(*l == gImplicitObjectParameter // if the left template is a static member
		|| *r == gImplicitObjectParameter) // or the right template is a static member
	{
		// ignore the first parameter
		++l;
		++r;
	}
	for(; l != left.parameters.end(); ++l, ++r)
	{
		UniqueTypeWrapper leftType = *l;
		UniqueTypeWrapper rightType = *r;
		// [temp.deduct.partial] (C++11 n3242)
		// Before the partial ordering is done, certain transformations are performed on the types used for partial
		// ordering:
		// - If P is a reference type, P is replaced by the type referred to.
		// - If A is a reference type, A is replaced by the type referred to.
		leftType = removeReference(leftType);
		rightType = removeReference(rightType);
		bool leftMoreCvQualified = isGreaterCvQualification(leftType, rightType);
		bool rightMoreCvQualified = isGreaterCvQualification(rightType, leftType);
		// Remove any top-level cv-qualifiers:
		// - If P is a cv-qualified type, P is replaced by the cv-unqualified version of P.
		// - If A is a cv-qualified type, A is replaced by the cv-unqualified version of A.
		leftType.value.setQualifiers(CvQualifiers());
		rightType.value.setQualifiers(CvQualifiers());

		// Using the resulting types P and A the deduction is then done [..]. If deduction succeeds
		// for a given type, the type from the argument template is considered to be at least as specialized as the type
		// from the parameter template.
		// If for each type being considered a given template is at least as specialized for all types and more specialized
		// for some set of types and the other template is not more specialized for any types or is not at least as
		// specialized for any types, then the given template is more specialized than the other template. Otherwise,
		// neither template is more specialized than the other.
		if(!deduce(rightType, leftType, rightDeduced)) // if left is not at least as specialized as right
		{
			return false;
		}
		if(!deduce(leftType, rightType, leftDeduced)) // if right is not at least as specialized as left
		{
			return true;
		}
		// If, for a given type, deduction succeeds in both directions (i.e., the types are identical after the transformations
		// above) and both P and A were reference types (before being replaced with the type referred to above):
		if((*l).isReference()
			&& (*r).isReference())
		{
			// - if the type from the argument template was an lvalue reference and the type from the parameter
			// template was not, the argument type is considered to be more specialized than the other; otherwise,
			// - if the type from the argument template is more cv-qualified than the type from the parameter template
			// (as described above), the argument type is considered to be more specialized than the other; otherwise,
			// - neither type is more specialized than the other.
			// TODO: rvalue reference?
			isMoreCvQualified |= leftMoreCvQualified;
			if(rightMoreCvQualified)
			{
				return false; // the template is only more specialized if the other template is not more specialized for any types.
			}
		}
	}
	return isMoreCvQualified; // after transformation, all parameters are identical; the template is more specialized if it is more specialized for any type.
}

// [over.match.best]
// a viable function F1 is defined to be a better function than another viable function
// F2 if for all arguments i, ICSi(F1) is not a worse conversion sequence than ICSi(F2), and then
// - for some argument j, ICSj(F1) is a better conversion sequence than ICSj(F2), or, if not that,
// - F1 is a non-template function and F2 is a function template specialization, or, if not that,
// - F1 and F2 are function template specializations, and the function template for F1 is more specialized
// - than the template for F2 according to the partial ordering rules
inline bool isBetter(const CandidateFunction& l, const CandidateFunction& r)
{
	SYMBOLS_ASSERT(l.conversions.size() == r.conversions.size());
	std::size_t first = 0;
	if((!l.parameters.empty() && l.parameters.front() == gImplicitObjectParameter) // if the left function is a static member
		|| (!r.parameters.empty() && r.parameters.front() == gImplicitObjectParameter)) // or the right function is a static member
	{
		// [over.match.best]
		// If a function is a static member function [...] the first argument, the implied object parameter, has no effect
		// in the determination of whether the function is better or worse than any other function
		++first; // ignore the implicit object parameter when ranking
	}
	for(std::size_t i = first; i != l.conversions.size(); ++i)
	{
		if(isBetter(r.conversions[i], l.conversions[i]))
		{
			return false; // at least one argument is not a better conversion sequence
		}
	}
	for(std::size_t i = first; i != l.conversions.size(); ++i)
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
	if(l.isTemplate && r.isTemplate
		&& isMoreSpecialized(l, r))
	{
		return true;
	}
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

inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter)
{
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(filter((*i).second)) // if the filter passes this declaration
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
		const SimpleType& base = getSimpleType((*i).unique);

		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->getName().value == id.value
			&& filter(DeclarationInstance(base.declaration)))
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

	void visit(const IntegralConstantExpression& node)
	{
		printer.out << node.value.value;
	}
	void visit(const CastExpression& node)
	{
		// TODO
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
	void visit(const struct ExplicitTypeExpression& node)
	{
		// TODO
	}
	void visit(const struct MemberAccessExpression& node)
	{
		// TODO
	}
	void visit(const struct FunctionCallExpression& node)
	{
		// TODO
	}
	void visit(const struct SubscriptExpression& node)
	{
		// TODO
	}

	void printExpression(ExpressionNode* node)
	{
		// TODO: assert
		if(node == 0)
		{
			printer.out << "[unknown]";
			return;
		}
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

#if 0
	void visit(const Namespace& element)
	{
		printer.out << getValue(element.declaration->getName()) << ".";
		visitTypeElement();
	}
#endif
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
	void visit(const SimpleType& element)
	{
		if(qualifierStack.back().isConst)
		{
			printer.out << "const ";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << "volatile ";
		}
		printType(element);
		visitTypeElement();
	}
	void visit(const ReferenceType& element)
	{
		pushType(true);
		printer.out << "&";
		visitTypeElement();
		popType();
	}
	void visit(const PointerType& element)
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
	void visit(const ArrayType& element)
	{
		pushType(false);
		visitTypeElement();
		printer.out << "[";
		if(element.size != 0)
		{
			printer.out << element.size;
		}
		printer.out << "]";
		popType();
	}
	void visit(const MemberPointerType& element)
	{
		pushType(true);
		{
			SymbolPrinter walker(printer, escape);
			walker.printType(element.type);
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

	void printType(const SimpleType& type)
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

inline void printType(const SimpleType& type, std::ostream& out, bool escape)
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



const CandidateFunction gOverloadNull;

struct OverloadResolver
{
	const Arguments& arguments;
	const TemplateArgumentsInstance* templateArguments;
	Location source;
	const SimpleType* enclosing;
	CandidateFunction best;
	Declaration* ambiguous;
	bool isUserDefinedConversion;

	OverloadResolver(const Arguments& arguments, const TemplateArgumentsInstance* templateArguments, Location source, const SimpleType* enclosing, bool isUserDefinedConversion = false)
		: arguments(arguments), templateArguments(templateArguments), source(source), enclosing(enclosing), ambiguous(0), isUserDefinedConversion(isUserDefinedConversion)
	{
		best.conversions.resize(arguments.size(), ImplicitConversion(STANDARDCONVERSIONSEQUENCE_INVALID));
	}
	const CandidateFunction& get() const
	{
		return ambiguous != 0 ? gOverloadNull : best;
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
	template<typename To>
	ImplicitConversion makeConversion(To to, const Argument& from)
	{
		// DR 903: a value-dependent expression may or may not be a null pointer constant, but the behaviour is unspecified.
		// simple fix: don't allow a value-dependent expression to be a null pointer constant.
		bool isNullPointerConstant = !from.isValueDependent && from.isConstant && evaluateExpression(from, source, enclosing).value == 0;
		return makeImplicitConversionSequence(to, from.type, source, enclosing, isNullPointerConstant, true, isUserDefinedConversion); // TODO: l-value
	}
	void add(const FunctionOverload& overload, const ParameterTypes& parameters, bool isEllipsis, const SimpleType* memberEnclosing, FunctionTemplate& functionTemplate = FunctionTemplate())
	{
		CandidateFunction candidate(overload, functionTemplate);
		candidate.conversions.reserve(best.conversions.size());

		ParameterTypes::const_iterator p = parameters.begin();
		Arguments::const_iterator a = arguments.begin();

		if(isMember(*overload.declaration)
			&& overload.declaration->type.declaration != &gCtor)
		{
			SYMBOLS_ASSERT(p != parameters.end());
			UniqueTypeWrapper implicitObjectParameter = *p++;
			SYMBOLS_ASSERT(a != arguments.end());
			const Argument& impliedObjectArgument = *a++;
			// [over.match.funcs]
			// a member function is considered to have an extra parameter, called the implicit object parameter, which
			// represents the object for which the member function has been called. For the purposes of overload resolution,
			// both static and non-static member functions have an implicit object parameter, but constructors do not.
			SYMBOLS_ASSERT(isClass(*memberEnclosing->declaration));
			if(!isStatic(*overload.declaration))
			{
				candidate.conversions.push_back(makeImplicitConversionSequence(implicitObjectParameter, impliedObjectArgument.type, source, enclosing, false, true)); // TODO: l-value
			}
			else
			{
				// TODO: [over.match.funcs] static members:
				// - no temporary object can be introduced to hold the argument for the implicit object parameter;
				// - no user-defined conversions can be applied to achieve a type match with it; and
				// - even if the implicit object parameter is not const-qualified, an rvalue temporary can be bound to the
				//   parameter as long as in all other respects the temporary can be converted to the type of the implicit
				//   object parameter.
				candidate.conversions.push_back(ImplicitConversion(StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers())));
			}
		}

		if(arguments.size() < parameters.size())
		{
			if(overload.declaration == &gUnknown)
			{
				return; // TODO: don't include built-in operator candidates with wrong number of arguments
			}
			std::size_t argumentCount = arguments.end() - a;
			const Parameters& defaults = getParameters(overload.declaration->type);
			Parameters::const_iterator d = defaults.begin();
			std::advance(d, argumentCount);
			for(ParameterTypes::const_iterator i = p + argumentCount; i != parameters.end(); ++i)
			{
				SYMBOLS_ASSERT(d != defaults.end());
				if((*d).argument == 0) // TODO: catch this earlier
				{
					return; // [over.match.viable] no default-argument available, this candidate is not viable
				}
				else
				{
					SYMBOLS_ASSERT((*d).argument->expr != 0); // TODO: non-fatal error: trying to use a default-argument before it has been declared. 
				}
				++d;
			}
		}

		for(; a != arguments.end() && p != parameters.end(); ++a, ++p)
		{
			candidate.conversions.push_back(makeConversion(*p, *a)); // TODO: l-value
		}

		// [over.match.viable]
		// A candidate function having fewer than m parameters is viable only if it has an ellipsis in its parameter
		// list. For the purposes of overload resolution, any argument for which there is no corresponding
		// parameter is considered to "match the ellipsis"
		if(!isEllipsis
			&& a != arguments.end())
		{
			return;
		}
		for(; a != arguments.end(); ++a)
		{
			candidate.conversions.push_back(IMPLICITCONVERSION_ELLIPSIS);
		}

		add(candidate);
	}
};



inline ParameterTypes addOverload(OverloadResolver& resolver, const Declaration& declaration, Location source, const SimpleType* enclosing = 0)
{
	UniqueTypeWrapper type = getUniqueType(declaration.type, source, enclosing, declaration.isTemplate);
	SYMBOLS_ASSERT(type.isFunction());

	ParameterTypes parameters;
	if(isMember(declaration)
		&& declaration.type.declaration != &gCtor)
	{
		// [over.match.funcs]
		// a member function is considered to have an extra parameter, called the implicit object parameter, which
		// represents the object for which the member function has been called. For the purposes of overload resolution,
		// both static and non-static member functions have an implicit object parameter, but constructors do not.
		SYMBOLS_ASSERT(isClass(*enclosing->declaration));
		// For static member functions, the implicit object parameter is considered to match any object
		UniqueTypeWrapper implicitObjectParameter = gImplicitObjectParameter;
		if(!isStatic(declaration))
		{
			// For non-static member functions, the type of the implicit object parameter is "reference to cv X" where X is
			// the class of which the function is a member and cv is the cv-qualification on the member function declaration.
			// TODO: conversion-functions, non-conversions introduced by using-declaration
			implicitObjectParameter = makeUniqueSimpleType(*enclosing);
			implicitObjectParameter.value.setQualifiers(type.value.getQualifiers());
			implicitObjectParameter.push_front(ReferenceType());
		}
		parameters.push_back(implicitObjectParameter);
	}
	bool isEllipsis = getFunctionType(type.value).isEllipsis;
	parameters.insert(parameters.end(), getParameterTypes(type.value).begin(), getParameterTypes(type.value).end());
	type.pop_front();

	if(!declaration.isTemplate)
	{
		// [temp.arg.explicit] An empty template argument list can be used to indicate that a given use refers to a
		// specialization of a function template even when a normal (i.e., non-template) function is visible that would
		// otherwise be used.
		if(resolver.templateArguments != 0)
		{
			return ParameterTypes();
		}
		resolver.add(FunctionOverload(const_cast<Declaration*>(&declaration), type), parameters, isEllipsis, enclosing);
		return parameters;
	}

	if(declaration.isSpecialization) // function template specializations don't take part in overload resolution?
	{
		return ParameterTypes();
	}

	FunctionTemplate functionTemplate;
	functionTemplate.parameters.swap(parameters);
	makeUniqueTemplateParameters(declaration.templateParams, functionTemplate.templateParameters, source, enclosing, true);

	if(resolver.arguments.size() > functionTemplate.parameters.size())
	{
		return ParameterTypes(); // more arguments than parameters. TODO: same for non-template?
	}

	try
	{
		// [temp.deduct] When an explicit template argument list is specified, the template arguments must be compatible with the template parameter list
		if(resolver.templateArguments != 0
			&& resolver.templateArguments->size() > functionTemplate.templateParameters.size())
		{
			throw TypeErrorBase(source); // too many explicitly specified template arguments
		}
		SimpleType specialization(const_cast<Declaration*>(&declaration), enclosing);
		specialization.instantiated = true;
		{
			UniqueTypeArray::const_iterator p = functionTemplate.templateParameters.begin();
			if(resolver.templateArguments != 0)
			{
				for(TemplateArgumentsInstance::const_iterator a = resolver.templateArguments->begin(); a != resolver.templateArguments->end(); ++a, ++p)
				{
					SYMBOLS_ASSERT(p != functionTemplate.templateParameters.end());
					if((*a).isNonType() != (*p).isDependentNonType())
					{
						throw TypeErrorBase(source); // incompatible explicitly specified arguments
					}
					specialization.templateArguments.push_back(*a);
				}
			}
			for(; p != functionTemplate.templateParameters.end(); ++p)
			{
				specialization.templateArguments.push_back(*p);
			}
		}

		// substitute the template-parameters in the function's parameter list with the explicitly specified template-arguments
		ParameterTypes substituted1;
		substitute(substituted1, functionTemplate.parameters, source, specialization);
		// TODO: [temp.deduct]
		// After this substitution is performed, the function parameter type adjustments described in 8.3.5 are performed.

		UniqueTypeArray arguments;
		arguments.reserve(resolver.arguments.size());
		for(Arguments::const_iterator a = resolver.arguments.begin(); a != resolver.arguments.end(); ++a)
		{
			arguments.push_back((*a).type);
		}

		specialization.templateArguments.resize(resolver.templateArguments == 0 ? 0 : resolver.templateArguments->size()); // preserve the explicitly specified arguments
		specialization.templateArguments.resize(functionTemplate.templateParameters.size(), gUniqueTypeNull);
		// NOTE: in rare circumstances, deduction may cause implicit instantiations, which occur at the point of overload resolution 
		if(!deduceFunctionCall(substituted1, arguments, specialization.templateArguments, resolver.source, resolver.enclosing))
		{
			throw TypeErrorBase(source); // deduction failed
		}


		// substitute the template-parameters in the function's parameter list with the deduced template-arguments
		ParameterTypes substituted2;
		substitute(substituted2, substituted1, source, specialization);
		type = substitute(type, source, specialization); // substitute the return type. TODO: should wait until overload is chosen?

		resolver.add(FunctionOverload(const_cast<Declaration*>(&declaration), type), substituted2, isEllipsis, enclosing, functionTemplate);
		return substituted2;
	}
	catch(TypeError&)
	{
		// deduction and checking failed
	}

	return ParameterTypes();
}


inline bool isBaseOf(UniqueTypeWrapper base, UniqueTypeWrapper derived, Location source, const SimpleType* enclosing);

template<typename To>
inline void addConversionFunctionOverloads(OverloadResolver& resolver, const SimpleType& classType, To to, Location source, const SimpleType* enclosing)
{
	instantiateClass(classType, source, enclosing); // searching for overloads requires a complete type
	LookupResultRef declaration = ::findDeclaration(classType, gConversionFunctionId, IsAny());
	// TODO: conversion functions in base classes should not be hidden by those in derived
	if(declaration != 0)
	{
		const SimpleType* memberEnclosing = findEnclosingType(&classType, declaration->scope); // find the base class which contains the member-declaration
		SYMBOLS_ASSERT(memberEnclosing != 0);

		for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
		{
			SYMBOLS_ASSERT(p->enclosed != 0);

			SYMBOLS_ASSERT(!p->isTemplate); // TODO: template-argument-deduction for conversion function
			// 'template<typename T> operator T()' can be explicitly invoked with template argument list: e.g. 'x.operator int()'
			// [temp.deduct.conv] Template argument deduction is done by comparing the return type of the template conversion function
			// (call it P) with the type that is required as the result of the conversion (call it A)

			UniqueTypeWrapper yielded = getUniqueType(p->type, source, memberEnclosing);
			yielded.pop_front();

			if(isClass(to)) // [over.match.copy]
			{
				// When the type of the initializer expression is a class type "cv S", the conversion functions of S and its
				// base classes are considered. Those that are not hidden within S and yield a type whose cv-unqualified
				// version is the same type as T or is a derived class thereof are candidate functions. Conversion functions
				// that return "reference to X" return lvalues of type X and are therefore considered to yield X for this process
				// of selecting candidate functions.
				if(!(getExactMatch(to, removeReference(yielded)) != gUniqueTypeNull
					|| isBaseOf(to, removeReference(yielded), source, enclosing)))
				{
					continue;
				}
			}
			else if(to.isReference()) // [over.match.ref]
			{
				// Assuming that "cv1 T" is the underlying type of the reference being initialized,
				// and "cv S" is the type of the initializer expression, with S a class type, the candidate functions are
				// selected as follows:
				UniqueTypeWrapper tmpTo = to;
				tmpTo.pop_front(); // remove reference
				// The conversion functions of S and its base classes are considered. Those that are not hidden within S
				// and yield type "reference to cv2 T2", where "cv1 T" is reference-compatible with "cv2 T2", are
				// candidate functions.
				if(!yielded.isReference())
				{
					continue;
				}
				yielded.pop_front(); // remove reference
				if(!((getExactMatch(tmpTo, yielded) != gUniqueTypeNull
						|| isBaseOf(tmpTo, yielded, source, enclosing))
					&& (isEqualCvQualification(tmpTo, yielded)
						|| isGreaterCvQualification(tmpTo, yielded))))
				{
					continue;
				}
			}
			else // [over.match.conv]
			{
				// The conversion functions of S and its base classes are considered. Those that are not hidden within S
				// and yield type T or a type that can be converted to type T via a standard conversion sequence
				// are candidate functions. Conversion functions that return a cv-qualified type are considered
				// to yield the cv-unqualified version of that type for this process of selecting candidate functions. Conversion
				// functions that return "reference to cv2 X" return lvalues of type "cv2 X" and are therefore considered
				// to yield X for this process of selecting candidate functions.
				bool isLvalue = yielded.isReference(); // TODO: lvalueness!
				yielded = removeReference(yielded);
				yielded.value.setQualifiers(CvQualifiers());
				if(makeStandardConversionSequence(to, yielded, source, enclosing, false, isLvalue).rank == SCSRANK_INVALID)
				{
					continue;
				}
			}

			addOverload(resolver, *p, source, memberEnclosing);
		}
	}
}

template<typename To>
FunctionOverload findBestConversionFunction(To to, UniqueTypeWrapper from, Location source, const SimpleType* enclosing, bool isNullPointerConstant, bool isLvalue)
{
	UniqueExpression nullPointerConstantExpression = makeUniqueExpression(IntegralConstantExpression(gSignedInt, IntegralConstant(0)));
	ExpressionWrapper expression(isNullPointerConstant ? nullPointerConstantExpression : 0, isNullPointerConstant);
	Arguments arguments;
	arguments.push_back(Argument(expression, from));

	// [over.best.ics]
	// However, when considering the argument of a user-defined conversion function that is a candidate by
	// 13.3.1.3 when invoked for the copying of the temporary in the second step of a class copy-initialization, or
	// by 13.3.1.4, 13.3.1.5, or 13.3.1.6 in all cases, only standard conversion sequences and ellipsis conversion
	// sequences are allowed.
	OverloadResolver resolver(arguments, 0, source, enclosing, true); // disallow user-defined conversion when considering argument to conversion function

	// [dcl.init]\14
	if(isClass(to)
		&& isComplete(to)) // can only convert to a class that is complete
	{
		// add converting constructors of 'to'
		const SimpleType& classType = getSimpleType(to.value);
		instantiateClass(classType, source, enclosing); // searching for overloads requires a complete type
		Identifier tmp;
		tmp.value = classType.declaration->getName().value;
		tmp.source = source;
		LookupResultRef declaration = ::findDeclaration(classType, tmp, IsConstructor());

		if(declaration != 0) // TODO: add implicit copy constructor!
		{
			const SimpleType* memberEnclosing = findEnclosingType(&classType, declaration->scope); // find the base class which contains the member-declaration
			SYMBOLS_ASSERT(memberEnclosing != 0);

			for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
			{
				SYMBOLS_ASSERT(p->enclosed != 0);

				// [class.conv.ctor]
				// A constructor declared without the function-specifier explicit that can be called with a single parameter
				// specifies a conversion from the type of its first parameter to the type of its class. Such a constructor is
				// called a converting constructor.
				if(declaration->specifiers.isExplicit)
				{
					continue;
				}

				addOverload(resolver, *p, source, memberEnclosing); // will reject constructors that cannot be called with a single argument, because they are not viable.
			}
		}
	}

	if(isClass(from)
		&& isComplete(from)) // can only convert from a class that is complete
	{
		addConversionFunctionOverloads(resolver, getSimpleType(from.value), to, source, enclosing);
	}

	// TODO: return-type of constructor should be 'to'
	FunctionOverload result = resolver.get();
	if(result.declaration != 0
		&& result.type.isSimple()
		&& getSimpleType(result.type.value).declaration == &gCtor)
	{
		result.type = to;
		result.type.value.setQualifiers(CvQualifiers());
	}
	return result;
}



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

inline bool isBaseOf(UniqueTypeWrapper base, UniqueTypeWrapper derived, Location source, const SimpleType* enclosing)
{
	if(!base.isSimple()
		|| !derived.isSimple())
	{
		return false;
	}
	const SimpleType& baseType = getSimpleType(base.value);
	const SimpleType& derivedType = getSimpleType(derived.value);
	if(&baseType == &derivedType)
	{
		return true;
	}
	SYMBOLS_ASSERT(!isClass(*derivedType.declaration) || !isIncomplete(*derivedType.declaration)); // TODO: does SFINAE apply?
	return isBaseOf(baseType, derivedType, source, enclosing);
}

inline bool isConvertibleTo(UniqueTypeWrapper type, UniqueTypeWrapper other, Location source, const SimpleType* enclosing)
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

extern const SimpleType gDependentSimpleType;

#endif


