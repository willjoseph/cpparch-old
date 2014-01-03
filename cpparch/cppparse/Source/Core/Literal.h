
#ifndef INCLUDED_CPPPARSE_CORE_LITERAL_H
#define INCLUDED_CPPPARSE_CORE_LITERAL_H

#include "Fundamental.h"
#include "Ast/ExpressionImpl.h"
#include <string>

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
	return IntegralConstantExpression(ExpressionType(getIntegerLiteralSuffixType(suffix), false), result);
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
	return IntegralConstantExpression(ExpressionType(getFloatingLiteralSuffixType(suffix), false), result);
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
	return IntegralConstantExpression(ExpressionType(getCharacterLiteralType(value), false), result);
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
	return IntegralConstantExpression(ExpressionType(gBool, false), IntegralConstant(*value == 't' ? 1 : 0));
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

#endif
