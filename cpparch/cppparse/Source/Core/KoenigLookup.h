
#ifndef INCLUDED_CPPPARSE_CORE_KOENIGLOOKUP_H
#define INCLUDED_CPPPARSE_CORE_KOENIGLOOKUP_H

#include "Ast/Type.h"
#include "Special.h"

//-----------------------------------------------------------------------------
// Argument dependent lookup

struct KoenigAssociated
{
	typedef std::vector<Scope*> Namespaces;
	Namespaces namespaces;
	typedef std::vector<const SimpleType*> Classes;
	Classes classes;
};

inline void addAssociatedNamespace(KoenigAssociated& associated, Scope& scope)
{
	SYMBOLS_ASSERT(scope.type == SCOPETYPE_NAMESPACE);
	if(std::find(associated.namespaces.begin(), associated.namespaces.end(), &scope) == associated.namespaces.end())
	{
		associated.namespaces.push_back(&scope);
	}
}

inline void addAssociatedClass(KoenigAssociated& associated, const SimpleType& type)
{
	SYMBOLS_ASSERT(isClass(*type.declaration));
	if(std::find(associated.classes.begin(), associated.classes.end(), &type) == associated.classes.end())
	{
		associated.classes.push_back(&type);
	}
}


inline Scope* getEnclosingNamespace(Scope* scope)
{
	// global scope
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

inline void addAssociatedEnclosingNamespace(KoenigAssociated& associated, const SimpleType& type)
{
	Scope* scope = getEnclosingNamespace(type.declaration->scope);
	if(scope != 0)
	{
		addAssociatedNamespace(associated, *scope);
	}
}

inline void addAssociatedClassAndNamespace(KoenigAssociated& associated, const SimpleType& classType)
{
	SYMBOLS_ASSERT(isClass(*classType.declaration));
	addAssociatedClass(associated, classType);
	addAssociatedEnclosingNamespace(associated, classType);
}

inline void addAssociatedClassRecursive(KoenigAssociated& associated, const SimpleType& classType)
{
	SYMBOLS_ASSERT(isClass(*classType.declaration));
	addAssociatedClassAndNamespace(associated, classType);
	for(UniqueBases::const_iterator i = classType.bases.begin(); i != classType.bases.end(); ++i)
	{
		const SimpleType* base = *i;
		addAssociatedClassRecursive(associated, *base); // TODO: check for cyclic base-class, prevent infinite recursion
	}
}

inline void addKoenigAssociated(KoenigAssociated& associated, const SimpleType& classType)
{
	if(classType.enclosing != 0)
	{
		addAssociatedClassAndNamespace(associated, *classType.enclosing);
	}
	addAssociatedClassRecursive(associated, classType);
}

inline void addKoenigAssociated(KoenigAssociated& associated, UniqueTypeWrapper type);

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


inline void addKoenigAssociated(KoenigAssociated& associated, UniqueTypeWrapper type)
{
	for(; type != gUniqueTypeNull; type.pop_front())
	{
		if(type == gOverloaded)
		{
			// [basic.lookup.argdep]
			// TODO In addition, if the argument is the name or address of a set of overloaded functions
			// and/or function templates, its associated classes and namespaces are the union of those associated with
			// each of the members of the set, i.e., the classes and namespaces associated with its parameter types and
			// return type. Additionally, if the aforementioned set of overloaded functions is named with a template-id,
			// its associated classes and namespaces also include those of its type template-arguments and its template
			// template-arguments.
			return; // TODO
		}
		KoenigVisitor visitor(associated);
		type.value->accept(visitor);
	}
}

inline const SimpleType* findKoenigAssociatedClass(const KoenigAssociated& associated, const Declaration& declaration)
{
	SYMBOLS_ASSERT(isClass(declaration));
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

#endif
