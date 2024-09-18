
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "env.hpp"
#include "eval.hpp"
#include "lexical.hpp"
#include "lisp.hpp"

// vendor
// #include "cxxopts.hpp"

std::string source_code1 = "(define area (lambda (a) (* a a)))";
std::string source_code2 = "(area (+ 1 1))";

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
            std::cout << *std::get<std::unique_ptr<std::string>>(t.value) << ' ';
        }
        std::cout << '\n';
        break;
    case austlisp::Tokens::K_DEFINE:
        print_info(env->last(), env);
    case austlisp::Tokens::NONE:
        break;
    case austlisp::Tokens::K_LAMBDA:
        std::cout << "lambda.\n";
        break;
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

int main(int argc, const char* argv[]) {

    // cxxopts::Options options("aust lisp");

    // options.add_options()
    //     ("h,help", "print this help message and exit.");

    // auto result = options.parse(argc, argv);

    // if (result.count("help"))
    // {
    //   std::cout << options.help() << std::endl;
    //   exit(0);
    // }
    // std::cout << "lisp>";
    // std::string line;
    // std::getline(std::cin, line);

    auto global_env = std::make_unique<austlisp::Env>();

    austlisp::Eval e(global_env.get());
    size_t t = 0;

    auto tokenize = std::make_unique<austlisp::Tokenize>(source_code1);
    tokenize->debug_tokens();
    auto x = e.parser(tokenize->tokens_list, t);
    e.clear_status();
    auto res = e.eval(x);
    print_info(res, global_env.get());

    tokenize = std::make_unique<austlisp::Tokenize>(source_code2);
    tokenize->debug_tokens();
    t = 0;
    x = e.parser(tokenize->tokens_list, t);
    e.clear_status();
    res = e.eval(x);

    print_info(res, global_env.get());
    return 0;
}
