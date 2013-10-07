
#include "TypeSubstitute.h"
#include "ExpressionEvaluate.h"
#include "TypeUnique.h" // TODO: for makeUniqueTemplateArgument, could be removed
#include "Fundamental.h" // for gVoid

void substitute(UniqueTypeArray& substituted, const UniqueTypeArray& dependent, const InstantiationContext& context)
{
	substituted.reserve(dependent.size());
	for(UniqueTypeArray::const_iterator i = dependent.begin(); i != dependent.end(); ++i)
	{
		UniqueTypeWrapper type = substitute(*i, context);
		substituted.push_back(type);
	}
}

// 'enclosing' is already substituted
inline UniqueTypeWrapper substitute(Declaration* declaration, const SimpleType* enclosing, const TemplateArgumentsInstance& templateArguments, const InstantiationContext& context)
{
	SimpleType result(declaration, enclosing);
	if(declaration->isTemplate)
	{
		result.declaration = result.primary = findPrimaryTemplate(declaration); // TODO: name lookup should always find primary template

		const TemplateParameters& templateParams = result.declaration->templateParams;
		std::size_t count = std::distance(templateParams.begin(), templateParams.end());
		SYMBOLS_ASSERT(count == std::distance(templateParams.defaults.begin(), templateParams.defaults.end()));
		result.templateArguments.reserve(count);

		substitute(result.templateArguments, templateArguments, context);

		TemplateArguments::const_iterator i = templateParams.defaults.begin();
		std::advance(i, result.templateArguments.size());
		for(; i != templateParams.defaults.end(); ++i)
		{
			if((*i).type.declaration == 0)
			{
				throw TooFewTemplateArgumentsError(context.source);
			}
			UniqueTypeWrapper argument = makeUniqueTemplateArgument(*i, setEnclosingTypeSafe(context, &result), isDependent(result)); // evaluate template-parameter defaults in the context of the owning template
			UniqueTypeWrapper substituted = substitute(argument, setEnclosingTypeSafe(context, &result)); // substitute template-parameter defaults in the context of the owning template
			result.templateArguments.push_back(substituted); // handles when template-param-default depends on a template param that was also defaulted
		}

		SYMBOLS_ASSERT(count == result.templateArguments.size());
	}

	static size_t uniqueId = 0;
	result.uniqueId = ++uniqueId;
	return makeUniqueSimpleType(result);
}

inline const SimpleType* substitute(const SimpleType& instance, const InstantiationContext& context)
{
	const SimpleType* enclosing = 0;
	if(instance.enclosing != 0)
	{
		enclosing = substitute(*instance.enclosing, context);
	}
	UniqueTypeWrapper result = substitute(instance.declaration, enclosing, instance.templateArguments, context);
	return &getSimpleType(result.value);
}

struct SubstituteVisitor : TypeElementVisitor
{
	UniqueTypeWrapper type;
	const InstantiationContext context;
	SubstituteVisitor(UniqueTypeWrapper type, const InstantiationContext& context)
		: type(type), context(context)
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
		const SimpleType* enclosingTemplate = findEnclosingTemplate(context.enclosingType, element.type->scope);
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
				substitute(result.templateArguments, element.templateArguments, context); // TT<T>
				type = gUniqueTypeNull;
				type.push_front(result);
			}
			else
			{
				// template-template-argument
				if(!type.isTemplateTemplateArgument())
				{
					throw ExpectedTemplateTemplateArgumentError(context.source, type);
				}
				const TemplateTemplateArgument& argument = getTemplateTemplateArgument(type.value);
				if(std::distance(argument.declaration->templateParams.begin(), argument.declaration->templateParams.end())
					!= element.templateParameterCount)
				{
					throw MismatchedTemplateTemplateArgumentError(context.source, type);
				}
				if(!element.templateArguments.empty()) // TT<...>
				{ //TODO: TT<>
					type = substitute(argument.declaration, argument.enclosing, element.templateArguments, context);
				}
			}
		}
	}

	virtual void visit(const DependentTypename& element) // substitute T::X, T::template X<...>, x.X, x.template X
	{
		if(isDependent(*context.enclosingType))
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
			UniqueTypeWrapper qualifying = substitute(element.qualifying, context);
			SYMBOLS_ASSERT(qualifying != gUniqueTypeNull);
#if 0
			if(qualifying.isNamespace())
			{
				// look up 'id' in namespace (only declarations visible at point of definition of template)
				// result may be namespace or type
				// if type, substitute within instantiation context
				Scope& scope = *getNamespace(qualifying.value).declaration->enclosed;
				std::size_t visibility = context.enclosingType.instantiation.pointOfInstantiation;
				LookupResultRef result = findNamespaceDeclaration(scope, id, element.isNested ? LookupFilter(IsNestedName(visibility)) : LookupFilter(IsAny(visibility)));
				if(result == 0) // if the name was not found within the qualifying namespace
				{
					throw MemberNotFoundError(context.source, element.name, qualifying);
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
					throw QualifyingIsNotClassError(context.source, qualifying);
				}

				instantiateClass(*enclosing, context);
				std::size_t visibility = enclosing->instantiating ? context.enclosingType->instantiation.pointOfInstantiation : VISIBILITY_ALL;
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
					throw MemberNotFoundError(context.source, element.name, qualifying);
				}

				declaration = result;

				if(!isType(*declaration))
				{
					// [temp.deduct]
					// - Attempting to use a type in the qualifier portion of a qualified name that names a type when [...]
					//   the specified member is not a type where a type is required.
					throw MemberIsNotTypeError(context.source, element.name, qualifying);
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
			type = substitute(declaration, memberEnclosing, element.templateArguments, context);
			return;
		}

		// typedef
		SYMBOLS_ASSERT(declaration->specifiers.isTypedef);
		SYMBOLS_ASSERT(declaration->type.unique != 0);
		type = UniqueTypeWrapper(declaration->type.unique);
		if(declaration->type.isDependent)
		{
			SYMBOLS_ASSERT(memberEnclosing != 0);
			type = substitute(type, setEnclosingTypeSafe(context, memberEnclosing));
		}
	}
	virtual void visit(const DependentNonType& element)
	{
		// TODO: unify DependentNonType and NonType?
		if(isDependent(*context.enclosingType))
		{
			// TODO: occurs when substituting with a dependent template argument list, if a template function is called with an empty (or partial) explicit template argument list.
			type.push_front(element);
			return;
		}

		// TODO: SFINAE for expressions: check that type of template argument matches template parameter
		IntegralConstant value = evaluateExpression(element.expression, context);
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
		const SimpleType* result = substitute(element, context);
		type.push_front(*result);
	}
	virtual void visit(const PointerType& element)
	{
		// [temp.deduct] Attempting to create a pointer to reference type.
		if(type.isReference())
		{
			throw PointerToReferenceError(context.source);
		}
		type.push_front(element);
	}
	virtual void visit(const ReferenceType& element)
	{
		// [temp.deduct] Attempting to create a reference to a reference type or a reference to void
		if(type.isReference()
			|| type == gVoid)
		{
			throw ReferenceToReferenceError(context.source);
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
			throw InvalidArrayError(context.source);
		}
		type.push_front(element); // TODO substitute dependent expressions
	}
	virtual void visit(const MemberPointerType& element)
	{
		UniqueTypeWrapper classType = substitute(element.type, context);
		// [temp.deduct] Attempting to create "pointer to member of T" when T is not a class type.
		if(!classType.isSimple()
			|| !isClass(*getSimpleType(classType.value).declaration))
		{
			throw QualifyingIsNotClassError(context.source, classType);
		}
		type.push_front(MemberPointerType(classType));
	}
	virtual void visit(const FunctionType& element)
	{
		FunctionType result;
		result.isEllipsis = element.isEllipsis;
		substitute(result.parameterTypes, element.parameterTypes, context);
		// [temp.deduct] Attempting to create a function type in which a parameter has a type of void.
		// TODO: Attempting to create a cv-qualified function type.
		if(std::find(result.parameterTypes.begin(), result.parameterTypes.end(), gVoid) != result.parameterTypes.end())
		{
			throw VoidParameterError(context.source);
		}
		type.push_front(result);
	}
};

UniqueTypeWrapper substituteImpl(UniqueTypeWrapper dependent, const InstantiationContext& context)
{
	UniqueTypeWrapper inner = dependent;
	inner.pop_front();
	UniqueTypeWrapper type = inner.empty() ? gUniqueTypeNull : substitute(inner, context);
	SubstituteVisitor visitor(type, context);
	dependent.value->accept(visitor);
	visitor.type.value.addQualifiers(dependent.value.getQualifiers());
	return visitor.type;
}

