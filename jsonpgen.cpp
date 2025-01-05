// jsonpgen.cpp
// An attempt at generating a lexer for JSON.
// This is a work in progress.
// The lexer is generated using the McKeeman form of the grammar.
// The grammar is then converted to a DFA using the DFA construction algorithm.
// The DFA is then used to generate the lexer.
// The lexer is then used to tokenize the input.
// dbien
// 04JAN2025
// The grammer is recursive at its very core so this doesn't work well - need the parser generator combined with the lexer generator to handle this.

#include "lexang.h"

__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE

void
GenerateJsonLex(EGeneratorFamilyDisposition _egfdFamilyDisp)
{
    typedef _l_transport_fixedmem<char8_t> _TyTransport;
    typedef _TyDefaultAllocator _TyAllocator;
    typedef char8_t _TyCTok;
    typedef _regexp_final<_TyCTok, _TyAllocator> _TyFinal;
    typedef _regexp_trigger<_TyCTok, _TyAllocator> _TyTrigger;

    // Whitespace
    _TyFinal ws = --( l(0x20) | l(0x0A) | l(0x0D) | l(0x09) );

    // String handling
    _TyFinal hex = lr(u8'0',u8'9') | lr(u8'A',u8'F') | lr(u8'a',u8'f');
    _TyFinal escape = l(u8'"') | l(u8'\\') | l(u8'/') | l(u8'b') | 
                     l(u8'f') | l(u8'n') | l(u8'r') | l(u8't') |
                     (l(u8'u') * hex * hex * hex * hex);
    _TyFinal character = (lr(0x20, 0x21) | lr(0x23, 0x5B) | lr(0x5D, 0xFF)) |
                        (l(u8'\\') * escape);
    _TyFinal characters = --character;
    _TyFinal string = l(u8'"') * 
                     t(TyGetTriggerStringBegin<_TyLexT>()) * 
                     characters * 
                     t(TyGetTriggerStringEnd<_TyLexT>()) * 
                     l(u8'"');

    // Number handling
    _TyFinal onenine = lr(u8'1',u8'9');
    _TyFinal digit = l(u8'0') | onenine;
    _TyFinal digits = ++digit;
    _TyFinal integer = digit | (onenine * digits) | 
                      (l(u8'-') * digit) | (l(u8'-') * onenine * digits);
    _TyFinal fraction = --(l(u8'.') * digits);
    _TyFinal exponent = --((l(u8'E') | l(u8'e')) * --(l(u8'+') | l(u8'-')) * digits);
    _TyFinal number = t(TyGetTriggerNumberBegin<_TyLexT>()) * 
                     integer * fraction * exponent *
                     t(TyGetTriggerNumberEnd<_TyLexT>());

    // Keywords (only need end triggers)
    _TyFinal true_val = ls(u8"true") * t(TyGetTriggerTrueEnd<_TyLexT>());
    _TyFinal false_val = ls(u8"false") * t(TyGetTriggerFalseEnd<_TyLexT>());
    _TyFinal null_val = ls(u8"null") * t(TyGetTriggerNullEnd<_TyLexT>());

    // Value can be string, number, true, false, or null
    _TyFinal value = string | number | true_val | false_val | null_val;

    // Object productions following McKeeman form
    _TyFinal member = ws * string * ws * l(u8':') * ws * 
                     t(TyGetTriggerMemberValueBegin<_TyLexT>()) *
                     value * 
                     t(TyGetTriggerMemberValueEnd<_TyLexT>());

    _TyFinal members = member * --( l(u8',') * member );

    _TyFinal object = l(u8'{') * ( ws * l(u8'}') | 
                                  ws * members * ws * l(u8'}') );

    // Array productions split for recursion
    _TyFinal array_prefix = l(u8'[') * ws;
    _TyFinal array_separator = l(u8',') * ws;
    _TyFinal array_suffix = l(u8']');

    // Set actions for the top-level tokens only
    object.SetAction(TyGetTokenObject<_TyLexT>());
    array_prefix.SetAction(TyGetTokenArrayPrefix<_TyLexT>());
    array_separator.SetAction(TyGetTokenArraySeparator<_TyLexT>());
    array_suffix.SetAction(TyGetTokenArraySuffix<_TyLexT>());

    // Only include the top-level productions that the parser needs to handle
    _TyFinal AllJsonTokens(object);
    AllJsonTokens.AddRule(array_prefix);
    AllJsonTokens.AddRule(array_separator);
    AllJsonTokens.AddRule(array_suffix);

    // Generate DFA
    typedef _dfa<_TyCTok, _TyAllocator> _TyDfa;
    typedef _TyDfa::_TyDfaCtxt _TyDfaCtxt;

    _TyDfa dfaJson;
    _TyDfaCtxt dctxtJson(dfaJson);
    gen_dfa(AllJsonTokens, dfaJson, dctxtJson);

    // Generate the lexer
    _l_generator<_TyDfa, char> gen(
        _egfdFamilyDisp,
        "_jsonlex.h",
        "JSONLEX",
        true,
        "ns_jsonlex",
        "stateJSON",
        "char8_t",
        "u8'",
        "'"
    );

    gen.add_dfa(dfaJson, dctxtJson, "startJSON");
    gen.generate();
}