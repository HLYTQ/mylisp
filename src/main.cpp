#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "env.hpp"
#include "eval.hpp"
#include "lexical.hpp"
#include "lisp.hpp"

// vendor
#include "cxxopts.hpp"

namespace austlisp {

void print_info(const austlisp::Token& res, austlisp::Env* env) {
    switch (res.token_type) {
    case austlisp::Tokens::INTEGER:
        std::cout << std::get<int64_t>(res.value) << '\n';
        break;
    case austlisp::Tokens::DOUBLE:
        std::cout << std::get<double>(res.value) << '\n';
        break;
    case austlisp::Tokens::LIST:
        for (const auto& t : *std::get<std::unique_ptr<austlisp::List>>(res.value)) {
            switch (t.token_type) {
            case austlisp::Tokens::INTEGER:
                std::cout << std::get<int64_t>(t.value) << ' ';
                break;
            case austlisp::Tokens::DOUBLE:
                std::cout << std::get<double>(t.value) << ' ';
                break;
            default:
                std::cout << *std::get<std::unique_ptr<std::string>>(t.value) << ' ';
            }
        }
        std::cout << '\n';
        break;
    case austlisp::Tokens::K_DEFINE:;
    case austlisp::Tokens::NONE:
        break;
    case austlisp::Tokens::K_LAMBDA:
        std::cout << "lambda.\n";
        break;
    case austlisp::Tokens::_BUILDIN_CAR:
    case austlisp::Tokens::_BUILDIN_CDR:
    case austlisp::Tokens::_BUILDIN_EQ:
    case austlisp::Tokens::_BUILDIN_EQUAL:
    case austlisp::Tokens::IDENT_C:
        {
            auto pos = std::get<std::unique_ptr<austlisp::List>>(res.value).get();
            std::cout << *std::get<std::unique_ptr<std::string>>(pos->at(0).value) << '\n';
            break;
        }
    default:
        std::cout << *std::get<std::unique_ptr<std::string>>(res.value) << '\n';
    }
}

void repl(Env* global_env) {
    austlisp::Eval e(global_env);
    size_t t = 0;
    std::cout << "Welcome to austlisp! version 0.1\n";
    // std::string line[] = {"(define b (if (equal \"13\" \"123\") (+ 1 1) (+ 3 4)))", "(+ b 0)"};
    std::string line;
    for (;;) {
        std::cout << ">>> ";
        std::getline(std::cin, line);
        auto tokenize = std::make_unique<austlisp::Tokenize>(line);
        // tokenize->debug_tokens();
        auto ast = e.parser(tokenize->tokens_list, t);
        t        = 0;
        auto res = e.eval(ast);
        e.clear_status();
        print_info(res, global_env);
    }
}

void file_mode(Env* global_env, const cxxopts::ParseResult& result) {
    austlisp::Eval e(global_env);
    std::ifstream file;
    try {
        file.open(result["file"].as<std::string>());
        if (!file.is_open()) {
            std::cout << "no file: " << result["file"].as<std::string>() << '\n';
        }
    } catch (std::exception* e) {
        e->what();
        exit(1);
    }
    std::string line;
    size_t t = 0;
    while (std::getline(file, line)) {
        auto tokenize = std::make_unique<austlisp::Tokenize>(line);
        // tokenize->debug_tokens();
        auto ast = e.parser(tokenize->tokens_list, t);
        t        = 0;
        auto res = e.eval(ast);
        e.clear_status();
        austlisp::print_info(res, global_env);
    }
}


} // namespace austlisp

// static void austlisp_help() {
//
// }

int main(int argc, const char* argv[]) {

    cxxopts::Options options("austlisp", "A simple C++ lisp");

    options.add_options()("f,file", "filename", cxxopts::value<std::string>())("h,help", "Print usage");

    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    auto global_env = std::make_unique<austlisp::Env>();

    if (result.count("file")) {
        austlisp::file_mode(global_env.get(), result);
        return 0;
    }

    repl(global_env.get());

    return 0;
}
