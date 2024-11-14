#ifndef SCANNER_HPP
#define SCANNER_HPP

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "../build/parser.tab.hh"
#include "../build/location.hh"

namespace Bird
{
    class Scanner : public yyFlexLexer
    {
    public:
        Scanner(std::istream *in) : yyFlexLexer(in) {}

        virtual ~Scanner() {}

        using FlexLexer::yylex;
        virtual int yylex(Bird::Parser::semantic_type *const lval, Bird::Parser::location_type *loc);

    private:
        Bird::Parser::semantic_type *yylval = nullptr;
    };
}

#endif