#ifndef _ENV_HPP_
#define _ENV_HPP_

#include <map>
#include <string>

#include "lexical.hpp"

namespace austlisp {

using std::get;

struct Env {
    Env(Env* outer = nullptr) : _outer(outer) {
        _init_buildin_function();
    }
    ~Env() {
        _outer = nullptr;
    }
    void add(std::string name, Token&& token);
    Token* find(const std::string& name);
    bool update(const std::string& name, Value&& new_value);
    const Token& operator[](const std::string& name);
    const Token& last() noexcept;

    static Token _buildin_func_car(const List& token_list) noexcept;
    static Token _buildin_func_cdr(const List& token_list);
    static Token _buildin_func_eq(const List& token_list);
    static Token _buildin_func_equal(const List& token_list);

protected:
    void _init_buildin_function();

private:
    std::map<std::string, Token> sym_table;
    Env* _outer;
};

struct Lambda {
    Lambda(List&& _p, List&& _b) : params(std::move(_p)), body(std::move(_b)) {}
    List params;
    List body;
};

} // namespace austlisp

#endif
