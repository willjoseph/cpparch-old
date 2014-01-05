
#ifndef INCLUDED_CPPPARSE_SEMA_SEMATYPEID_H
#define INCLUDED_CPPPARSE_SEMA_SEMATYPEID_H

#include "SemaCommon.h"
#include "SemaSpecifier.h"
#include "SemaDeclarator.h"

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
		addDependent(type.dependent, walker.dependent);
	}
	SEMA_POLICY(cpp::conversion_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::conversion_declarator* symbol, SemaDeclarator& walker)
	{
		walker.pushPointerType(symbol->op);
		type.typeSequence = walker.typeSequence;
		// [temp.dep.type] A type is dependent if it is a compound type constructed from any dependent type
		addDependent(type.dependent, walker.dependent);
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
		addDependent(type.dependent, walker.dependent);
		// new T
		// new T*
		// new T[variable]
		// new T[variable][constant]
		// new T*[variable]
		// new T C::*
	}
};

#endif
