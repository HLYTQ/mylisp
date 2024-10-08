#pragma once
#include <cctype>
#include <cstdint>
#ifndef _LEXICAL_HPP_
#define _LEXICAL_HPP_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "lisp.hpp"

namespace austlisp {

struct Token {
    Token() : token_type(Tokens::NONE), value(0){};
    template <typename T>
    Token(Tokens t, std::enable_if_t<std::is_fundamental_v<T>, T> v) : token_type(t), value(v){};
    Token(Token&& other) noexcept {
        token_type = other.token_type;
        value      = std::move(other.value);
    }
    Token(const Token& other) {
        auto tt    = other.copy();
        token_type = tt.token_type;
        value      = std::move(tt.value);
    }
    const Token& operator=(Token&& other) noexcept {
        if (this != &other) {
            token_type = other.token_type;
            value      = std::move(other.value);
        }
        return *this;
    }
    const Token& operator=(const Token& other) {
        if (this != &other) {
            auto tt    = other.copy();
            token_type = tt.token_type;
            value      = std::move(tt.value);
        }
        return *this;
    }
    Token(Tokens t, Value&& v) {
        token_type = t;
        value      = std::move(v);
    }
    constexpr bool operator==(const Token& other) {
        if (this->token_type == other.token_type && this->value == other.value) {
            return true;
        } else {
            return false;
        }
    }
    constexpr bool operator!=(const Token& other) {
        return !(*this == other);
    }

    /**
     * @brief 
     *  return true if is LIST or STRING.
     * @return bool
     */
    constexpr bool is_complex_type() const noexcept {
        return (token_type == Tokens::LIST) || (token_type == Tokens::STRING);
    }
    // Add this for 'eq'
    Token reference() noexcept {
        return Token{this->token_type, (int64_t)(this) };
    }
    // is indent is lambda, we can't just do a fucking copy, a lambda object maybe very big
    Token copy() const {
        switch (token_type) {
        case Tokens::DOUBLE:
            {
                Token tt;
                tt.token_type = Tokens::DOUBLE;
                tt.value      = std::get<double>(value);
                return tt;
            }
        case Tokens::INTEGER:
            {
                Token tt;
                tt.token_type = Tokens::INTEGER;
                tt.value      = std::get<int64_t>(value);
                return tt;
            }
        case Tokens::LIST:
            {
                Token tt;
                tt.token_type = Tokens::LIST;
                tt.value      = std::make_unique<List>(*std::get<std::unique_ptr<List>>(value));
                return tt;
            }
        case Tokens::K_LAMBDA:
            {
                std::cerr << "不可复制类型Lambda.\n";
                return Token{};
            }
        default:
            {
                Token tt;
                tt.token_type = token_type;
                tt.value      = std::make_unique<std::string>(*std::get<std::unique_ptr<std::string>>(value));
                return tt;
            }
        }
    }

    Tokens token_type;
    Value value;
};

struct Tokenize {
public:
    Tokenize(std::string_view source) {
        size_t i = 0;
        while (std::isspace(source[i]) || !std::isprint(source[i])) {
            i++;
        }
        while (i < source.size() && source[i] != '\n') {

            if (std::isspace(source[i])) {
                i++;
                continue;
            }

            std::string str = "";
            Tokens t        = Tokens::NONE;
            Value value{};

            switch (source[i]) {
            case '(':
                str = source[i++];
                t   = Tokens::LPAREN;
                break;
            case ')':
                str = source[i++];
                t   = Tokens::RPAREN;
                break;
            case '+':
                if (!std::isdigit(source[i + 1])) {
                    str = source[i++];
                    t   = Tokens::PLUS;
                    break;
                } else {
                    goto default_handle;
                }
            case '-':
                if (!std::isdigit(source[i + 1])) {
                    str = source[i++];
                    t   = Tokens::MINUS;
                    break;
                } else {
                    goto default_handle;
                }
            case '*':
                str = source[i++];
                t   = Tokens::STAR;
                break;
            case '/':
                str = source[i++];
                t   = Tokens::DIVISION;
                break;
            case '\'':
                str = source[i++];
                t   = Tokens::QUOTE; // such as '(1 2 3) or (quote (1 2 3))
                break;
            case '"':
                i++;
                while (source[i] != '"') {
                    str += source[i++];
                }
                i++;
                t = Tokens::STRING;
                break;
            default:
            default_handle:
                if (std::isalpha(source[i]) || source[i] == '_') {
                    while (std::isalnum(source[i]) || source[i] == '_') {
                        str += source[i++];
                    }
                    if (Tokens _k_xxx = _is_keywords(str); _k_xxx != Tokens::NONE) {
                        t = _k_xxx;
                    } else {
                        t = Tokens::IDENT;
                    }
                }
                // number
                else if (std::isdigit(source[i])
                         || ((source[i] == '+' || source[i] == '-') && std::isdigit(source[i + 1]))) {
                    if (source[i] == '+' || source[i] == '-') {
                        str += source[i++];
                    }
                    int base = 10;
                    if (source[i] == '0' && std::islower(source[i + 1]) == 'x' && std::isxdigit(source[i + 2])) {
                        str += "0x";
                        i += 2;
                        base = 16;
                    }
                    while (std::isdigit(source[i]) || source[i] == '.' || (base == 16 && std::isxdigit(source[i]))) {
                        if (source[i] == '.') {
                            if (base == 16) { // then we have an error!
                                std::cerr << "number read error.\n";
                                break;
                            }
                            t = Tokens::DOUBLE;
                        }
                        str += source[i++];
                    }
                    if (t != Tokens::DOUBLE) {
                        t = Tokens::INTEGER;
                    }
                }
            }
            if (str == "") {
                std::cerr << "unknown character '" << source[i++] << "' ignored." << std::endl;
            } else {
                // NOTE: 这里疑似有一个GCC的bug，在std::variant添加一个函数指针后，怎么t.value成空值了？
                // 或者其他我自身的原因，总之更改了实现
                value = _str_convert2value(str, t);
                tokens_list.emplace_back(Token{t, std::move(value)});
            }
        }
    }
    void debug_tokens() const {
        for (const auto& i : tokens_list) {
            if (i.token_type == Tokens::DOUBLE) {
                std::cout << "[ " << Tokens_str[int(i.token_type)] << ": " << get<double>(i.value) << " ], ";
            } else if (i.token_type == Tokens::INTEGER) {
                std::cout << "[ " << Tokens_str[int(i.token_type)] << ": " << get<int64_t>(i.value) << " ], ";
            } else if (i.token_type == Tokens::STRING) {
                std::cout << "[ " << Tokens_str[int(i.token_type)] << ": \""
                          << *get<std::unique_ptr<std::string>>(i.value) << "\" ], ";
            } else {
                std::cout << "[ " << Tokens_str[int(i.token_type)] << ": "
                          << *get<std::unique_ptr<std::string>>(i.value) << " ], ";
            }
        }
        endl(std::cout);
    }

protected:
    Value _str_convert2value(const std::string& str, Tokens t) {
        Value v{};
        switch (t) {
        case Tokens::DOUBLE:
            v = atof(str.c_str());
            break;
        case Tokens::INTEGER:
            v = atol(str.c_str());
            break;
        default:
            v = std::make_unique<std::string>(str);
            break;
        }
        return v;
    }

    Tokens _is_keywords(std::string_view str) {
        switch (str[0]) {
        case 'd':
            return Tokens::K_DEFINE;
        case 's':
            if (str == "setq") {
                return Tokens::K_SETQ;
            } else {
                return Tokens::NONE;
            }
        case 'l':
            if (str == "lambda") {
                return Tokens::K_LAMBDA;
            } else {
                return Tokens::NONE;
            }
        case 'i':
            if (str == "if") {
                return Tokens::K_IF;
            } else {
                return Tokens::NONE;
            }
        case 'q':
            if (str == "lambda") {
                return Tokens::K_QUOTE;
            } else {
                return Tokens::NONE;
            }
        case 'n':
            if (str == "nil") {
                return Tokens::NONE;
            }
            return Tokens::NONE; // "nil" 作为一个关键字，代表null类型
        case 't':
            if (str == "true")
                return Tokens::TRUE;
            return Tokens::NONE;
        case 'f':
            if (str == "false")
                return Tokens::FALSE;
            return Tokens::NONE;
        case 'w':
            if (str == "while")
                return Tokens::K_WHILE;
            return Tokens::NONE;
        default:
            return Tokens::NONE;
        }
    }

private:
    static constexpr const char* keywords[] = {
        "define",
        "setq",
        "lambda",
        "if",
        "quote",
        "while",
        "nil",
    };

public:
    std::vector<Token> tokens_list;
};

} // namespace austlisp

#endif
