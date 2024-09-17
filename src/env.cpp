#include "env.hpp"

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

bool Env::update(const std::string& name, Value&& new_value) {
    Token* t = find(name);
    if (t == nullptr) {
        std::cerr << "can't find symbol: " << name << ", " << "updata failure.\n";
        return false;
    }
    t->value = std::move(new_value);
    return true;
}

const Token& Env::operator[](const std::string& name) {
    return sym_table[name];
}
const Token& Env::last() noexcept {
    return (--sym_table.end())->second;
}


} // namespace austlisp
