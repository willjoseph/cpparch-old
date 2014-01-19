
#ifndef INCLUDED_CPPPARSE_SEMA_SEMASPECIFIER_H
#define INCLUDED_CPPPARSE_SEMA_SEMASPECIFIER_H

#include "SemaCommon.h"
#include "SemaClass.h"

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
	void action(cpp::elaborated_type_specifier* symbol, const SemaElaboratedTypeSpecifierResult& walker)
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
	void action(cpp::enum_specifier* symbol, const SemaEnumSpecifierResult& walker)
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
		else if(symbol->id == cpp::storage_class_specifier::MUTABLE)
		{
			seq.specifiers.isMutable = true;
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

#endif
