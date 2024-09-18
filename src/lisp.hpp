#pragma once

#ifndef LISP_HPP
#define LISP_HPP


#include <cctype>
#include <memory>
#include <variant>
#include <vector>

#define KEYWORDS_NUM 5

namespace austlisp {

enum class Tokens {
    NONE,
    LPAREN,
    RPAREN,
    KEYWORDS,
    INTEGER,
    DOUBLE,
    PLUS,
    MINUS,
    STAR,
    DIVISION,
    IDENT,
    IDENT_C,
    STRING,
    QUOTE,
    LIST,
    NIL,
    // keywords
    K_DEFINE,
    K_IF,
    K_LAMBDA,
    K_SETQ,
    K_QUOTE,
};

static constexpr const char* Tokens_str[] = {
    [int(Tokens::NONE)]     = "",
    [int(Tokens::LPAREN)]   = "T_LPAREN",
    [int(Tokens::RPAREN)]   = "T_RPAREN",
    [int(Tokens::KEYWORDS)] = "T_KEYWORDS",
    [int(Tokens::INTEGER)]  = "T_INTEGER",
    [int(Tokens::DOUBLE)]   = "T_DOUBLE",
    [int(Tokens::PLUS)]     = "T_PLUS",
    [int(Tokens::MINUS)]    = "T_MINUS",
    [int(Tokens::STAR)]     = "T_STAR",
    [int(Tokens::DIVISION)] = "T_DIVISION",
    [int(Tokens::IDENT)]    = "T_IDENT",
    [int(Tokens::IDENT_C)]  = "T_IDENT_C",
    [int(Tokens::STRING)]   = "T_STRING",
    [int(Tokens::QUOTE)]    = "T_QUOTE",
    [int(Tokens::LIST)]     = "T_LIST",
    [int(Tokens::NIL)]      = "T_NIL",
    [int(Tokens::K_DEFINE)] = "K_DEFINE",
    [int(Tokens::K_IF)]     = "K_IF",
    [int(Tokens::K_LAMBDA)] = "K_LAMBDA",
    [int(Tokens::K_SETQ)]   = "K_SETQ",
    [int(Tokens::K_QUOTE)]  = "K_QUOTE",
};

struct Token;

using List = std::vector<Token>;
struct Lambda;

// NOTE: Always use std:move, there is only buildin type and
// std::unique_ptr<std::string>
using Value =
    std::variant<int64_t, double, std::unique_ptr<List>, std::unique_ptr<Lambda>, std::unique_ptr<std::string>>;

} // namespace austlisp

#endif
