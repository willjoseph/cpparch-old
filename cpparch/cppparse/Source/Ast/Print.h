
#ifndef INCLUDED_CPPPARSE_AST_PRINT_H
#define INCLUDED_CPPPARSE_AST_PRINT_H

#include "Type.h"
#include "ExpressionImpl.h"

typedef TokenPrinter<std::ostream> FileTokenPrinter;


typedef std::list<UniqueType> TypeElements;

inline void printType(UniqueTypeWrapper type, std::ostream& out = std::cout, bool escape = false);
inline void printType(const SimpleType& type, std::ostream& out = std::cout, bool escape = false);

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
		symbol->accept(walker);
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
	void visit(const struct ObjectExpression& node)
	{
		// TODO
	}
	void visit(const struct DependentObjectExpression& node)
	{
		// TODO
	}
	void visit(const struct ClassMemberAccessExpression& node)
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
	void visit(const struct PostfixOperatorExpression& node)
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



#endif
