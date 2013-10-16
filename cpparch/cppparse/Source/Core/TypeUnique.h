
#ifndef INCLUDED_CPPPARSE_CORE_TYPEUNIQUE_H
#define INCLUDED_CPPPARSE_CORE_TYPEUNIQUE_H

#include "Ast/Type.h"
#include "TypeSubstitute.h"
#include "ExpressionEvaluate.h"

inline UniqueTypeWrapper makeUniqueQualifying(const Qualifying& qualifying, const InstantiationContext& context, bool allowDependent = false)
{
	return qualifying.empty()
		|| isNamespace(*qualifying.back().declaration)
		? gUniqueTypeNull
		: getUniqueType(qualifying.back(), context, allowDependent);
}

inline const SimpleType* makeUniqueEnclosing(const Qualifying& qualifying, const InstantiationContext& context, bool allowDependent, UniqueTypeWrapper& unique)
{
	if(!qualifying.empty())
	{
		if(isNamespace(*qualifying.back().declaration))
		{
			return 0; // name is qualified by a namespace, therefore cannot be enclosed by a class
		}
		unique = getUniqueType(qualifying.back(), context, allowDependent);
		if(allowDependent && qualifying.back().isDependent)
		{
			return 0;
		}
		const SimpleType& type = getSimpleType(unique.value);
		// [temp.inst] A class template is implicitly instantiated ... if the completeness of the class-type affects the semantics of the program.
		instantiateClass(type, context, allowDependent);
		return &type;
	}
	return context.enclosingType;
}

inline const SimpleType* makeUniqueEnclosing(const Qualifying& qualifying, const InstantiationContext& context, bool allowDependent)
{
	UniqueTypeWrapper tmp;
	return makeUniqueEnclosing(qualifying, context, allowDependent, tmp);
}


inline UniqueTypeWrapper makeUniqueTemplateArgument(const TemplateArgument& argument, const InstantiationContext& context, bool allowDependent)
{
	SYMBOLS_ASSERT(argument.type.declaration != 0);
	extern Declaration gNonType;
	if(argument.type.declaration == &gNonType)
	{
		if(allowDependent && argument.expression.isValueDependent)
		{
			return pushType(gUniqueTypeNull, DependentNonType(argument.expression));
		}
		IntegralConstant value = evaluateExpression(argument.expression, context);
		return pushType(gUniqueTypeNull, NonType(value));
	}

	return getUniqueType(argument.type, context, allowDependent && argument.type.isDependent);
}


inline void makeUniqueTemplateArguments(TemplateArguments& templateArguments, TemplateArgumentsInstance& result, const InstantiationContext& context, bool allowDependent = false)
{
	result.reserve(std::distance(templateArguments.begin(), templateArguments.end()));
	for(TemplateArguments::const_iterator i = templateArguments.begin(); i != templateArguments.end(); ++i)
	{
		UniqueTypeWrapper type = makeUniqueTemplateArgument(*i, context, allowDependent);
		result.push_back(type);
	}
}

inline void makeUniqueTemplateParameters(const TemplateParameters& templateParams, TemplateArgumentsInstance& arguments, const InstantiationContext& context, bool allowDependent)
{
	arguments.reserve(std::distance(templateParams.begin(), templateParams.end()));
	for(Types::const_iterator i = templateParams.begin(); i != templateParams.end(); ++i)
	{
		const Type& argument = (*i);
		UniqueTypeWrapper result;
		extern Declaration gParam;
		if(argument.declaration->type.declaration == &gParam)
		{
			result = getUniqueType(argument, context, allowDependent);
			SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
		}
		else
		{
			UniqueExpression expression = makeUniqueExpression(NonTypeTemplateParameter(argument.declaration, getUniqueType(argument.declaration->type)));
			if(allowDependent)
			{
				result = pushType(gUniqueTypeNull, DependentNonType(expression));
			}
			else
			{
				IntegralConstant value = evaluateExpression(expression, context);
				result = pushType(gUniqueTypeNull, NonType(value));
			}
		}
		arguments.push_back(result);
	}
	SYMBOLS_ASSERT(!arguments.empty());
}

inline void makeUniqueTemplateArguments(const TemplateArguments& arguments, TemplateArgumentsInstance& templateArguments, const InstantiationContext& context, bool allowDependent)
{
	templateArguments.reserve(std::distance(arguments.begin(), arguments.end()));
	for(TemplateArguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
	{
		const TemplateArgument& argument = *i;
		SYMBOLS_ASSERT(argument.type.declaration != 0); // TODO: non-fatal error: not enough template arguments!
		UniqueTypeWrapper result = makeUniqueTemplateArgument(argument, context, allowDependent);
		templateArguments.push_back(result);
	}
}


// unqualified object name: int, Object,
// qualified object name: Qualifying::Object
// unqualified typedef: Typedef, TemplateParam
// qualified typedef: Qualifying::Type
// /p type
// /p enclosingType The enclosing template, required when uniquing a template-argument: e.g. Enclosing<int>::Type
//			Note: if 'type' is a class-template template default argument, 'enclosingType' will be the class-template, which does not require instantiation!
inline UniqueTypeWrapper makeUniqueType(const Type& type, const InstantiationContext& context, bool allowDependent)
{
	// the type in which template-arguments are looked up: returns qualifying type if specified, else returns enclosingType
	UniqueTypeWrapper qualifying;
	const SimpleType* enclosing = makeUniqueEnclosing(type.qualifying, context, allowDependent, qualifying);
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
		makeUniqueTemplateArguments(type.templateArguments, templateArguments, context, allowDependent);
		return pushType(gUniqueTypeNull, DependentTypename(type.id->value, qualifying, templateArguments, isNested, declaration->isTemplate));
	}
	size_t index = declaration->templateParameter;
	if(index != INDEX_INVALID)
	{
		SYMBOLS_ASSERT(allowDependent);
		SYMBOLS_ASSERT(type.qualifying.empty());
		// Find the template-specialisation it belongs to:
		const SimpleType* parameterEnclosing = findEnclosingType(enclosing, declaration->scope);
		if(parameterEnclosing != 0
			&& !isDependent(*parameterEnclosing)) // if the enclosing type is not dependent
		{
			SYMBOLS_ASSERT(!parameterEnclosing->declaration->isSpecialization || parameterEnclosing->instantiated); // a specialization must be instantiated (or in the process of instantiating)
			const TemplateArgumentsInstance& arguments = parameterEnclosing->declaration->isSpecialization
				? parameterEnclosing->deducedArguments
				: parameterEnclosing->templateArguments;

			SYMBOLS_ASSERT(index < arguments.size());
			UniqueTypeWrapper result = arguments[index];
			SYMBOLS_ASSERT(result != gUniqueTypeNull); // fails for non-type template-argument
			return result;
		}

		TemplateArgumentsInstance templateArguments; // for template-template-parameter
		makeUniqueTemplateArguments(type.templateArguments, templateArguments, context, allowDependent);
		std::size_t templateParameterCount = declaration->isTemplate ? std::distance(declaration->templateParams.begin(), declaration->templateParams.end()) : 0;
		return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, DependentType(declaration, templateArguments, templateParameterCount)));
	}

	const SimpleType* memberEnclosing = isMember(*declaration) // if the declaration is a class member
		? findEnclosingType(enclosing, declaration->scope) // it must be a member of (a base of) the qualifying class: find which one.
		: 0; // the declaration is not a class member and cannot be found through qualified name lookup

	if(declaration->specifiers.isTypedef)
	{
		UniqueTypeWrapper result = getUniqueType(declaration->type, setEnclosingType(context, memberEnclosing), allowDependent);
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
			makeUniqueTemplateParameters(tmp.declaration->templateParams, tmp.templateArguments, context, dependent);
		}
		else
		{
			tmp.templateArguments.reserve(std::distance(defaults.begin(), defaults.end()));
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
				const SimpleType* enclosing = isTemplateParamDefault ? &tmp : context.enclosingType; // resolve dependent template-parameter-defaults in context of template class
				UniqueTypeWrapper result = makeUniqueTemplateArgument(argument, InstantiationContext(argument.source, enclosing, 0, context.enclosingScope), allowDependent);
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


inline std::size_t evaluateArraySize(const ExpressionWrapper& expression, const InstantiationContext& context)
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
	return evaluate(expression, context).value;
}

struct TypeSequenceMakeUnique : TypeSequenceVisitor
{
	UniqueType& type;
	const InstantiationContext context;
	bool allowDependent;
	TypeSequenceMakeUnique(UniqueType& type, const InstantiationContext& context, bool allowDependent)
		: type(type), context(context), allowDependent(allowDependent)
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
			pushUniqueType(type, ArrayType(evaluateArraySize(*i, context)));
		}
	}
	void visit(const DeclaratorMemberPointerType& element)
	{
		UniqueTypeWrapper tmp = getUniqueType(element.type, context, allowDependent);
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
			result.parameterTypes.push_back(getUniqueType((*i).declaration->type, context, allowDependent));
		}
		pushUniqueType(type, result);
		type.setQualifiers(element.qualifiers);
	}
};

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, const InstantiationContext& context, bool allowDependent)
{
	UniqueTypeWrapper result = makeUniqueType(*static_cast<const Type*>(&type), context, allowDependent);
	result.value.addQualifiers(type.qualifiers);
	TypeSequenceMakeUnique visitor(result.value, context, allowDependent);
	type.typeSequence.accept(visitor);
	return result;
}

#endif
