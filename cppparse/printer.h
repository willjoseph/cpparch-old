
#ifndef INCLUDED_CPPPARSE_PRINTER_H
#define INCLUDED_CPPPARSE_PRINTER_H

#include <boost/wave/token_ids.hpp>
typedef boost::wave::token_id LexTokenId;

inline bool isSeparated(LexTokenId id)
{
	return IS_CATEGORY(id, boost::wave::KeywordTokenType)
		|| IS_CATEGORY(id, boost::wave::IdentifierTokenType)
		|| IS_CATEGORY(id, boost::wave::LiteralTokenType)
		|| id == boost::wave::T_PP_NUMBER; // TODO: should wave ever emit one of these? should be T_INTLIT?
}

template<typename OutputStreamType>
struct TokenPrinter
{
	OutputStreamType& out;
	size_t braceDepth;
	LexTokenId prev;
	LexTokenId prevprev;
	size_t templateDeclarationDepth;

	TokenPrinter(OutputStreamType& out)
		: out(out),
		braceDepth(0),
		prev(boost::wave::T_UNKNOWN),
		prevprev(boost::wave::T_UNKNOWN),
		templateDeclarationDepth(0)
	{
	}
	void nextLine(LexTokenId id)
	{
		out << std::endl;
		prev = boost::wave::T_NEWLINE;
		templateDeclarationDepth = id == boost::wave::T_TEMPLATE;
	}
	void printToken(LexTokenId id, const char* value)
	{
		if((prev == boost::wave::T_SEMICOLON
				&& id != boost::wave::T_RIGHTBRACE)
			|| (prev == boost::wave::T_LEFTBRACE
				&& id != boost::wave::T_RIGHTBRACE)
			|| (prev == boost::wave::T_RIGHTBRACE
				&& id != boost::wave::T_RIGHTBRACE
				&& id != boost::wave::T_SEMICOLON)
			|| (id == boost::wave::T_MSEXT_ASM))
		{
			nextLine(id);
		}

		if((prevprev == boost::wave::T_PUBLIC
			|| prevprev == boost::wave::T_PRIVATE)
			&& prev == boost::wave::T_COLON)
		{
			nextLine(id);
		}

		if(templateDeclarationDepth != 0)
		{
			if(prev == boost::wave::T_LESS)
			{
				++templateDeclarationDepth;
			}
			else if(prev == boost::wave::T_GREATER)
			{
				--templateDeclarationDepth;
				if(templateDeclarationDepth == 1)
				{
					nextLine(id);
				}
			}
		}

		if(id == boost::wave::T_LEFTBRACE)
		{
			nextLine(id);
			++braceDepth;
		}
		else if(id == boost::wave::T_RIGHTBRACE)
		{
			--braceDepth;
			nextLine(id);
		}

		if(prev == boost::wave::T_NEWLINE
			&& braceDepth != 0)
		{
			size_t i = 0;
			if(id == boost::wave::T_LEFTBRACE
				|| id == boost::wave::T_PUBLIC
				|| id == boost::wave::T_PRIVATE)
			{
				++i;
			}
			for(; i != braceDepth; ++i)
			{
				out << '\t';
			}
		}

		if((isSeparated(prev)
			&& isSeparated(id))
			|| prev == boost::wave::T_COMMA)
		{
			out << " ";
		}

		out << value;

		prevprev = prev;
		prev = id;
	}
};

#endif


