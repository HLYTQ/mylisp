#include "env.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "lisp.hpp"

namespace austlisp {

void Env::add(std::string name, Token&& token) {
    if (sym_table.find(name) == sym_table.end()) {
        sym_table[name] = std::move(token);
    } else {
        std::cerr << "error!: this: " << name << ", have been used.\n";
    }
}

Token* Env::find(const std::string& name) {
    Token* token = nullptr;
    if (sym_table.find(name) != sym_table.end()) {
        token = &sym_table[name];
    }
    if (_outer != nullptr && !token && _outer->sym_table.find(name) != _outer->sym_table.end()) {
        token = &_outer->sym_table[name];
    }
    return token;
}

bool Env::update(const std::string& name, Tokens token_type, Value&& new_value) {
    Token* t = find(name);
    if (t == nullptr) {
        std::cerr << "can't find symbol: " << name << ", " << "updata failure.\n";
        return false;
    }
    t->token_type = token_type;
    t->value      = std::move(new_value);
    return true;
}

const Token& Env::operator[](const std::string& name) {
    return sym_table[name];
}
const Token& Env::last() noexcept {
    return (--sym_table.end())->second;
}

Token Env::_buildin_func_car(const List& token_list) noexcept {
    if (token_list.size() != 2 || token_list[1].token_type != Tokens::LIST) {
        std::cerr << "error!: car接受一个列表.\n";
        return Token{};
    }
    return (*std::get<_Ptr_List_t>(token_list[1].value))[1].copy();
}

Token Env::_buildin_func_cdr(const List& token_list) {
    if (token_list.size() != 2 || token_list[1].token_type != Tokens::LIST) {
        std::cerr << "error!: car接受一个列表.\n";
        return Token{};
    }
    const auto& _list = *std::get<_Ptr_List_t>(token_list[1].value);
    List ret;
    ret.emplace_back(_list[0]);
    for (int i = 2; i < _list.size(); ++i) {
        ret.emplace_back(_list[i].copy());
    }
    return Token{Tokens::LIST, std::make_unique<List>(ret)};
}

Token Env::_buildin_func_eq(const List& token_list) {
    if (token_list[1].is_complex_type() && token_list[2].is_complex_type()) {
        if (token_list[1].token_type == Tokens::STRING && token_list[2].token_type == Tokens::STRING
            && std::get<_Ptr_Str_t>(token_list[1].value) == std::get<_Ptr_Str_t>(token_list[2].value)) {
            return Token{Tokens::TRUE, 1};
        } else {
            return Token{Tokens::FALSE, 0};
        }
        if (token_list[1].token_type == Tokens::LIST && token_list[2].token_type == Tokens::LIST
            && std::get<_Ptr_List_t>(token_list[1].value) == std::get<_Ptr_List_t>(token_list[2].value)) {
            return Token{Tokens::TRUE, 1};
        } else {
            return Token{Tokens::FALSE, 0};
        }
    }
    auto _token_get_number = [](Token tt) -> double {
        if (tt.token_type == Tokens::INTEGER) {
            return std::get<int64_t>(tt.value);
        } else {
            return std::get<double>(tt.value);
        }
    };
    if (token_list[1].token_type == Tokens::INTEGER && token_list[2].token_type == Tokens::INTEGER) {
        if (std::get<int64_t>(token_list[1].value) == std::get<int64_t>(token_list[2].value)) {
            return Token{Tokens::TRUE, 1};
        } else {
            return Token{Tokens::FALSE, 0};
        }
    }
    if (token_list[1].token_type == Tokens::DOUBLE || token_list[2].token_type == Tokens::DOUBLE) {
        if (_token_get_number(token_list[1]) == _token_get_number(token_list[2])) {
            return Token{Tokens::TRUE, 1};
        } else {
            return Token{Tokens::FALSE, 0};
        }
    }
    if (token_list[1].token_type != token_list[2].token_type) {
        throw new std::logic_error("无法比较.");
    }
    return Token{Tokens::FALSE, 0};
}

Token Env::_buildin_func_equal(const List& token_list) {
    if (token_list[1].is_complex_type() && token_list[2].is_complex_type()) {
        if (token_list[1].token_type == Tokens::LIST && token_list[2].token_type == Tokens::LIST) {
            auto& lhs = *std::get<_Ptr_List_t>(token_list[1].value);
            auto& rhs = *std::get<_Ptr_List_t>(token_list[2].value);
            if (lhs.size() != rhs.size()) {
                return Token{Tokens::FALSE, 0};
            }
            for (int i = 0; i < lhs.size(); ++i) {
                if (lhs[i] != rhs[i]) {
                    return Token{Tokens::FALSE, 0};
                }
            }
        } else if (token_list[1].token_type == Tokens::STRING && token_list[2].token_type == Tokens::STRING) {
            auto& lhs = *std::get<_Ptr_Str_t>(token_list[1].value);
            auto& rhs = *std::get<_Ptr_Str_t>(token_list[2].value);
            if (lhs.size() != rhs.size()) {
                return Token{Tokens::FALSE, 0};
            }
            for (int i = 0; i < lhs.size(); ++i) {
                if (lhs[i] != rhs[i]) {
                    return Token{Tokens::FALSE, 0};
                }
            }
        }
        return Token{Tokens::TRUE, 1};
    }
    return _buildin_func_eq(token_list);
}

void Env::_init_buildin_function() {
    this->add("car", Token{Tokens::_BUILDIN_CAR, 0});
    this->add("cdr", Token{Tokens::_BUILDIN_CDR, 0});
    this->add("eq", Token{Tokens::_BUILDIN_EQ, 0});
    this->add("equal", Token{Tokens::_BUILDIN_EQUAL, 0});
}

} // namespace austlisp
