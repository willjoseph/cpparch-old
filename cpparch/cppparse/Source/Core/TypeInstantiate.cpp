
#if BUILD_STANDALONE

#include "TypeInstantiate.h"
#include "Common/Util.h"
#include "TemplateDeduce.h"
#include "TypeUnique.h"


inline bool deduceAndSubstitute(const UniqueTypeArray& parameters, const UniqueTypeArray& arguments, const InstantiationContext& context, SimpleType& enclosing, TemplateArgumentsInstance& substituted)
{
	// deduce the partial-specialization's template arguments from the original argument list
	TemplateArgumentsInstance& deduced = enclosing.deducedArguments;
	if(!deducePairs(parameters, arguments, deduced)
		|| !isValid(deduced))
	{
		return false; // cannot deduce
	}
	try
	{
		// substitute the template-parameters in the partial-specialization's signature with the deduced template-arguments
		substitute(substituted, parameters, setEnclosingTypeSafe(context, &enclosing));
	}
	catch(TypeError&)
	{
		SYMBOLS_ASSERT(isValid(substituted));
		return false; // cannot substitute: SFINAE
	}

	return true;
}


inline bool matchTemplatePartialSpecialization(Declaration* declaration, TemplateArgumentsInstance& deducedArguments, const TemplateArgumentsInstance& specializationArguments, const TemplateArgumentsInstance& arguments, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(!declaration->templateParams.empty());
	TemplateArgumentsInstance deduced(std::distance(declaration->templateParams.begin(), declaration->templateParams.end()), gUniqueTypeNull);
	TemplateArgumentsInstance substituted;
	SimpleType enclosing(declaration, 0);
	enclosing.deducedArguments.swap(deduced);
	enclosing.instantiated = true;
	if(!deduceAndSubstitute(specializationArguments, arguments, context, enclosing, substituted))
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

inline bool matchTemplatePartialSpecialization(Declaration* declaration, const TemplateArgumentsInstance& specializationArguments, const TemplateArgumentsInstance& arguments, const InstantiationContext& context)
{
	TemplateArgumentsInstance deducedArguments;
	return matchTemplatePartialSpecialization(declaration, deducedArguments, specializationArguments, arguments, context);
}

inline Declaration* findTemplateSpecialization(Declaration* declaration, TemplateArgumentsInstance& deducedArguments, const TemplateArgumentsInstance& arguments, const InstantiationContext& context, bool allowDependent)
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
		makeUniqueTemplateArguments(declaration->templateArguments, specializationArguments, context, true);

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
		if(matchTemplatePartialSpecialization(declaration, deduced, specializationArguments, arguments, context)) // if this partial-specialization can be deduced for the specified types
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
				bool atLeastAsSpecializedCurrent = matchTemplatePartialSpecialization(best, bestArguments, specializationArguments, context); // deduce current against best
				bool atLeastAsSpecializedBest = matchTemplatePartialSpecialization(declaration, specializationArguments, bestArguments, context);

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


#include <fstream>
#include "Common/Util.h"


struct InstantiationSuffix
{
	char value[12];
	InstantiationSuffix(const void* p)
	{
		sprintf(value, "_%08x", p);
	}
	const char* c_str() const
	{
		return value;
	}
};

struct InstantiationName : public Concatenate
{
	InstantiationName(const SimpleType& instance)
		: Concatenate(
		makeRange(getValue(instance.declaration->getName())),
		makeRange(InstantiationSuffix(&instance).c_str()))
	{
	}
};

struct InstantiationPath : public Concatenate
{
	InstantiationPath(const SimpleType& instance)
		: Concatenate(
		makeRange(InstantiationName(instance).c_str()),
		makeRange(".html"))
	{
	}
};

inline void printPosition(const Source& source, std::ostream& out)
{
	out << source.absolute.c_str() << "(" << source.line << ", " << source.column << "): ";
}

void printTypeReadable(const SimpleType& type, std::ostream& out, bool escape = true)
{
	if(type.enclosing != 0)
	{
		printTypeReadable(*type.enclosing, out, escape);
		out << ".";
	}
	else
	{
		printName(type.declaration->scope, out);
	}
	out << getValue(type.declaration->getName());
	if(type.declaration->isTemplate)
	{
		out << (escape ? "&lt;" : "<");
		bool separator = false;
		for(TemplateArgumentsInstance::const_iterator i = type.templateArguments.begin(); i != type.templateArguments.end(); ++i)
		{
			if(separator)
			{
				out << ",";
			}
			out << std::endl << "\t";
			printType(*i, out, escape);
			separator = true;
		}
		if(!type.templateArguments.empty())
		{
			out << std::endl;
		}
		out << (escape ? "&gt;" : ">");
	}
}

inline void dumpTemplateInstantiations(const SimpleType& instance, bool root = false)
{
	if(instance.dumped)
	{
		return;
	}
	instance.dumped = true;
	SYMBOLS_ASSERT(!instance.visited);
	instance.visited = true;
	std::ofstream out(Concatenate(makeRange(root ? "debug/!" : "debug/"), makeRange(InstantiationPath(instance).c_str())).c_str());
	SYMBOLS_ASSERT(out.is_open());

	out << "<html>\n"
		"<head>\n"
		"</head>\n"
		"<body>\n"
		"<pre style='color:#000000;background:#ffffff;'>\n";
	printPosition(instance.declaration->getName().source, out);
	out << std::endl;
	printTypeReadable(instance, out, true);
	out << std::endl << std::endl;

	typedef std::map<const SimpleType*, Location> InstanceMap;
	InstanceMap instanceMap;
	for(ChildInstantiations::const_iterator i = instance.childInstantiations.begin(); i != instance.childInstantiations.end(); ++i)
	{
		instanceMap.insert(InstanceMap::value_type((*i).instance, (*i).source));
	}
	for(InstanceMap::const_iterator i = instanceMap.begin(); i != instanceMap.end(); ++i)
	{
		printPosition((*i).second, out);
		out << std::endl;
		out << "<a href='" << InstantiationPath(*(*i).first).c_str() << "'>";
		printTypeReadable(*(*i).first, out, true);
		out << "</a>";
		out << std::endl;
		dumpTemplateInstantiations(*(*i).first);
	}
	out << "</pre>\n"
		"</body>\n"
		"</html>\n";
	instance.visited = false;
}

inline std::size_t addBase(SimpleType& instance, UniqueTypeWrapper base, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(!isDependent(base));
	SYMBOLS_ASSERT(base.isSimple());
	const SimpleType& objectType = getSimpleType(base.value);
	std::size_t size = instantiateClass(objectType, setEnclosingTypeSafe(context, &instance));
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

// If the class definition does not explicitly declare a copy assignment operator, one is declared implicitly.
// The implicitly-declared copy assignment operator for a class X will have the form
//   X& X::operator=(const X&)
// TODO: correct constness of parameter
inline bool hasCopyAssignmentOperator(const SimpleType& classType, const InstantiationContext& context)
{
	Identifier id;
	id.value = gOperatorAssignId;
	const DeclarationInstance* result = ::findDeclaration(classType.declaration->enclosed->declarations, id);
	if(result == 0)
	{
		return false;
	}
	InstantiationContext memberContext = setEnclosingTypeSafe(context, &classType);
	for(const Declaration* p = findOverloaded(*result); p != 0; p = p->overloaded)
	{
		if(p->isTemplate)
		{
			continue; // TODO: check compliance: copy-assignment-operator cannot be a template?
		}

		UniqueTypeWrapper type = getUniqueType(p->type, memberContext);
		SYMBOLS_ASSERT(type.isFunction());
		const ParameterTypes& parameters = getParameterTypes(type.value);
		SYMBOLS_ASSERT(parameters.size() == 1);
		UniqueTypeWrapper parameterType = removeReference(parameters[0]);
		if(parameterType.isSimple()
			&& &getSimpleType(parameterType.value) == &classType)
		{
			return true;
		}
	}
	return false;
}


std::size_t instantiateClass(const SimpleType& instanceConst, const InstantiationContext& context, bool allowDependent)
{
	SimpleType& instance = const_cast<SimpleType&>(instanceConst);
	SYMBOLS_ASSERT(isClass(*instance.declaration));

	if(context.enclosingType != 0)
	{
		ChildInstantiations& instantiations = const_cast<SimpleType*>(context.enclosingType)->childInstantiations;
		instantiations.push_back(ChildInstantiation(&instance, context.source));
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
		instance.instantiation = context.source;

		static std::size_t uniqueId = 0;
		instance.uniqueId = ++uniqueId;

		if(!allowDependent
			&& instance.declaration->isTemplate)
		{
			// find the most recently declared specialization
			// TODO: optimise
			const DeclarationInstance* declaration = findDeclaration(instance.declaration->scope->declarations, instance.declaration->getName());
			SYMBOLS_ASSERT(declaration != 0);
			Declaration* specialization = findTemplateSpecialization(
				findOverloaded(*declaration), instance.deducedArguments, instance.templateArguments,
				InstantiationContext(context.source, instance.enclosing, 0, context.enclosingScope), false);
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
			InstantiationContext baseContext = InstantiationContext(original.instantiation, &instance, 0, context.enclosingScope);
			UniqueTypeId base = getUniqueType(*i, baseContext, allowDependent);
			SYMBOLS_ASSERT((*i).unique != 0);
			SYMBOLS_ASSERT((*i).isDependent || base.value == (*i).unique);
			if(allowDependent && (*i).isDependent)
			{
				// this occurs during 'instantiation' of a template class definition, in which case we postpone instantiation of this dependent base
				continue;
			}
			instance.size += addBase(instance, base, baseContext);
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
					InstantiationContext childContext(*l, &instance, 0, context.enclosingScope);
					UniqueTypeWrapper substituted = substitute(*i, childContext);
					SYMBOLS_ASSERT(!isDependent(substituted));
					instance.children.push_back(substituted);
				}
			}

			instance.hasCopyAssignmentOperator = hasCopyAssignmentOperator(instance, context);
		}
		instance.instantiating = false;
	}
	catch(TypeError&)
	{
		printPosition(context.source);
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

		if(context.enclosingType == 0
			|| !isTemplate(*context.enclosingType))
		{
			dumpTemplateInstantiations(instance, true);
		}
		throw;
	}
	return instance.size;
}

#endif
