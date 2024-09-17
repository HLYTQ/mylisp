
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "env.hpp"
#include "eval.hpp"
#include "lisp.hpp"

// vendor
// #include "cxxopts.hpp"

std::string source_code1 = "'(k m n '(h h h)";
std::string source_code2 = "(define a_list '(a b c))";

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
