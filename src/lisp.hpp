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
    NONE, // 起错名字了妈的，应该叫 NIL 的，改了怕😨出事
    LPAREN,
    RPAREN,
    KEYWORDS,
    INTEGER,
    DOUBLE,
    PLUS,
    MINUS,
    STAR,
    DIVISION,
    LOW,
    GREAT,
    IDENT,
    IDENT_C,
    STRING,
    QUOTE,
    LIST,
    TRUE,
    FALSE,
    // keywords
    K_DEFINE,
    K_IF,
    K_LAMBDA,
    _BUILDIN_CAR, // 一开始没考虑好，怎么丑陋起来了，真要这么简单粗暴的实现吗？
    _BUILDIN_CDR,
    _BUILDIN_EQ, // 仅仅比较两个地址
    _BUILDIN_EQUAL, // 比较内容，如果是列表，则逐个比较
    K_SETQ,
    K_WHILE,
    K_QUOTE,
};

static constexpr const char* Tokens_str[] = {
    [int(Tokens::NONE)]           = "",
    [int(Tokens::LPAREN)]         = "T_LPAREN",
    [int(Tokens::RPAREN)]         = "T_RPAREN",
    [int(Tokens::KEYWORDS)]       = "T_KEYWORDS",
    [int(Tokens::INTEGER)]        = "T_INTEGER",
    [int(Tokens::DOUBLE)]         = "T_DOUBLE",
    [int(Tokens::PLUS)]           = "T_PLUS",
    [int(Tokens::MINUS)]          = "T_MINUS",
    [int(Tokens::STAR)]           = "T_STAR",
    [int(Tokens::DIVISION)]       = "T_DIVISION",
    [int(Tokens::LOW)]            = "T_LOW",
    [int(Tokens::GREAT)]          = "T_GREAT",
    [int(Tokens::IDENT)]          = "T_IDENT",
    [int(Tokens::IDENT_C)]        = "T_IDENT_C",
    [int(Tokens::STRING)]         = "T_STRING",
    [int(Tokens::QUOTE)]          = "T_QUOTE",
    [int(Tokens::LIST)]           = "T_LIST",
    [int(Tokens::TRUE)]           = "T_TRUE",
    [int(Tokens::FALSE)]          = "T_FLASE",
    [int(Tokens::K_DEFINE)]       = "K_DEFINE",
    [int(Tokens::K_IF)]           = "K_IF",
    [int(Tokens::K_LAMBDA)]       = "K_LAMBDA",
    [int(Tokens::_BUILDIN_CAR)]   = "_BUILDIN_FUNC_CAR", // 一些常用的lisp内建函数，write by CXX
    [int(Tokens::_BUILDIN_CDR)]   = "_BUILDIN_FUNC_CDR",
    [int(Tokens::_BUILDIN_EQ)]    = "_BUILDIN_FUNC_EQ",
    [int(Tokens::_BUILDIN_EQUAL)] = "_BUILDIN_FUNC_EQUAL",
    [int(Tokens::K_SETQ)]         = "K_SETQ",
    [int(Tokens::K_WHILE)]        = "K_WHILE",
    [int(Tokens::K_QUOTE)]        = "K_QUOTE",
};

struct Token;

using List        = std::vector<Token>;
using _Ptr_List_t = std::unique_ptr<List>;
using _Ptr_Str_t  = std::unique_ptr<std::string>;

struct Lambda;

using Value =
    std::variant<int64_t, double, Token*, std::unique_ptr<List>, std::unique_ptr<Lambda>, std::unique_ptr<std::string>>;

} // namespace austlisp

#endif
