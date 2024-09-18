#pragma once

#ifndef _EVAL_HPP_
#define _EVAL_HPP_

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "env.hpp"
#include "lexical.hpp"
#include "lisp.hpp"

#define NO_MATCHING_RPAREN std::cerr << "no matching ')'.\n"
#define UNEXCEPTED_RPAREN  std::cerr << "unexcepted ')'.\n"

#define paren_handler()                                 \
    do {                                                \
        if (match_rparen(token_list[++t])) {            \
            paren_stack--;                              \
            return node;                                \
        } else {                                        \
            NO_MATCHING_RPAREN;                         \
            return std::make_unique<AST_base>(Token{}); \
        }                                               \
    } while (0)

namespace austlisp {

struct Eval {
    using vec_iter = std::vector<Token>::iterator;
    using vec_stmt = std::vector<Token>;

    Eval(Env* env) : env(env), paren_stack(0) {}

private:
    struct AST_base {
        AST_base() = default;
        AST_base(Token&& _t) : t(std::move(_t)) {}
        Token t;
        std::unique_ptr<AST_base> left;
        std::unique_ptr<AST_base> right;
    };
    struct AST_if : public AST_base {
        std::unique_ptr<AST_base> cond;
    };

public:
    constexpr bool match_rparen(const Token& t) noexcept {
        return t.token_type == Tokens::RPAREN;
    }

    std::unique_ptr<AST_base> parser(std::vector<Token>& token_list, size_t& t) {
        std::unique_ptr<AST_base> node;
        switch (token_list[t].token_type) {
        case Tokens::LPAREN:
            {
                paren_stack++;
                node = std::move(parser(token_list, ++t));
                if (paren_stack == 0 && (t != token_list.size() - 1)) {
                    UNEXCEPTED_RPAREN;
                    return std::make_unique<AST_base>(Token{});
                }
                return node;
            }
        case Tokens::PLUS:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::PLUS;
            node->left         = parser(token_list, ++t);
            node->right        = parser(token_list, ++t);
            paren_handler();
            break;
        case Tokens::MINUS:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::MINUS;
            node->left         = parser(token_list, ++t);
            node->right        = parser(token_list, ++t);
            paren_handler();
            break;
        case Tokens::STAR:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::STAR;
            node->left         = parser(token_list, ++t);
            node->right        = parser(token_list, ++t);
            paren_handler();
            break;
        case Tokens::DIVISION:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::DIVISION;
            node->left         = parser(token_list, ++t);
            node->right        = parser(token_list, ++t);
            paren_handler();
            break;
        case Tokens::INTEGER:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::INTEGER;
            node->t.value      = std::move(token_list[t].value);
            return node;
        case Tokens::DOUBLE:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::DOUBLE;
            node->t.value      = std::move(token_list[t].value);
            return node;
        case Tokens::STRING:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::STRING;
            node->t.value      = std::move(token_list[t].value);
            return node;
        case Tokens::QUOTE:
            {
                // QUOTE 后面的内容不解释执行，直接挂在AST上, 作为列表或符号类型？
                auto quoted        = std::make_unique<AST_base>();
                node               = std::make_unique<AST_base>();
                node->t.token_type = Tokens::QUOTE;

                quoted->t.token_type = Tokens::LIST;
                auto list            = std::make_unique<List>();
                int paren_holder     = this->paren_stack;
                for (;;) {
                    switch (token_list[++t].token_type) {
                    case Tokens::LPAREN:
                        this->paren_stack++;
                        list->emplace_back(std::move(token_list[t]));
                        break;
                    case Tokens::RPAREN:
                        this->paren_stack--;
                        list->emplace_back(std::move(token_list[t]));
                        break;
                    default:
                        list->emplace_back(std::move(token_list[t]));
                    }
                    if (paren_stack == paren_holder || t == token_list.size() - 1) {
                        break;
                    }
                }
                if (paren_holder != paren_stack) {
                    NO_MATCHING_RPAREN;
                    return std::make_unique<AST_base>(Token{});
                } else if (t + paren_stack != token_list.size() - 1) {
                    UNEXCEPTED_RPAREN;
                    return std::make_unique<AST_base>(Token{});
                }

                quoted->t.value = std::move(list);
                node->left      = std::move(quoted);
                return node;
            }
        case Tokens::K_DEFINE:
            {
                node               = std::make_unique<AST_base>();
                node->t.token_type = Tokens::K_DEFINE;
                if (token_list[++t].token_type != Tokens::IDENT) {
                    std::cerr << "error!: define后必须跟一个符号名称.\n";
                    return std::make_unique<AST_base>(Token{});
                }
                node->left  = std::make_unique<AST_base>(std::move(token_list[t]));
                node->right = parser(token_list, ++t);
                if (node->right->t.token_type == Tokens::NONE) {
                    std::cerr << "error!: define需要一个赋给变量的值.\n";
                    return std::make_unique<AST_base>(Token{});
                }
                paren_handler();
                break;
            }
        case Tokens::K_LAMBDA:
            {
                /*      [LAMBDA]
                 *      /      \
                 * [params]   [body] */

                node               = std::make_unique<AST_base>();
                node->t.token_type = Tokens::K_LAMBDA;

                // params
                if (token_list[++t].token_type != Tokens::LPAREN) {
                    std::cerr << "error!: 语法错误, lambda 缺失参数列表.\n";
                    return std::make_unique<AST_base>(Token{});
                }
                auto params = std::make_unique<List>();
                while (token_list[++t].token_type != Tokens::RPAREN && t < token_list.size()) {
                    params->emplace_back(std::move(token_list[t]));
                }
                node->left = std::make_unique<AST_base>(Token{Tokens::LIST, std::move(params)});

                // body:
                if (token_list[++t].token_type != Tokens::LPAREN) {
                    std::cerr << "error!: 语法错误, lambda 缺失body.\n";
                    return std::make_unique<AST_base>(Token{});
                }
                auto body        = std::make_unique<List>();
                int paren_holder = this->paren_stack++; // ++ because we match a '('
                body->emplace_back(std::move(token_list[t]));
                while (paren_stack != paren_holder && t != token_list.size() - 1) {
                    switch (token_list[++t].token_type) {
                    case Tokens::LPAREN:
                        paren_stack++;
                        body->emplace_back(std::move(token_list[t]));
                        break;
                    case Tokens::RPAREN:
                        paren_stack--;
                        body->emplace_back(std::move(token_list[t]));
                        break;
                    default:
                        body->emplace_back(std::move(token_list[t]));
                    }
                }
                if (paren_holder != paren_stack) {
                    NO_MATCHING_RPAREN;
                    return std::make_unique<AST_base>(Token{});
                } else if (t + paren_stack != token_list.size() - 1) {
                    UNEXCEPTED_RPAREN;
                    return std::make_unique<AST_base>(Token{});
                }
                node->right = std::make_unique<AST_base>(Token{Tokens::LIST, std::move(body)});
                return node;
            }
        case Tokens::IDENT:
            {
                node = std::make_unique<AST_base>();
                // 前面是一个 '(' 说明是一个调用
                if (t - 1 >= 0 && token_list[t - 1].token_type == Tokens::LPAREN) {
                    node->t.token_type = Tokens::IDENT_C;
                    auto params_list   = std::make_unique<List>();
                    params_list->emplace_back(std::move(token_list[t]));
                    Token tmp {};
                    while (token_list[++t].token_type != Tokens::RPAREN) {
                        if (token_list[t].token_type == Tokens::LPAREN) {
                            auto res = parser(token_list, t);
                            tmp = eval(res);
                        } else {
                            tmp = std::move(token_list[t]);
                        }
                        params_list->emplace_back(std::move(tmp));
                    }
                    node->t.value = std::move(params_list);
                } else {
                    node->t.token_type = Tokens::IDENT;
                    node->t.value      = std::move(token_list[t].value);
                }
                return node;
            }
        default:
            break;
        }
        return std::make_unique<AST_base>(Token{});
    }

    Token do_plus(Token&& left, Token&& right) noexcept {
        using _STR_ptr = std::unique_ptr<std::string>;
        Token ret{};
        if ((left.token_type == Tokens::DOUBLE || left.token_type == Tokens::INTEGER)
            && (right.token_type == Tokens::DOUBLE || right.token_type == Tokens::INTEGER)) {
            if (left.token_type == Tokens::DOUBLE || right.token_type == Tokens::DOUBLE) {
                ret.token_type = Tokens::DOUBLE;
                double v1, v2;
                auto _type_helper = [](const Token& t) {
                    switch (t.token_type) {
                    case Tokens::DOUBLE:
                        return std::get<double>(t.value);
                        break;
                    case Tokens::INTEGER:
                        return static_cast<double>(std::get<int64_t>(t.value));
                    default:
                        return .0;
                        break;
                    }
                };
                v1        = _type_helper(left);
                v2        = _type_helper(right);
                ret.value = v1 + v2;
            } else {
                ret.token_type = Tokens::INTEGER;
                ret.value      = std::get<int64_t>(left.value) + std::get<int64_t>(right.value);
            }
        } else if (left.token_type == Tokens::STRING && right.token_type == Tokens::STRING) {
            ret.token_type = Tokens::STRING;
            ret.value =
                std::make_unique<std::string>(*std::get<_STR_ptr>(left.value) + *std::get<_STR_ptr>(right.value));
        } else {
            std::cerr << "不是可加的类型！\n";
        }
        return ret;
    }

    Token do_minus(Token&& left, Token&& right) noexcept {
        using _STR_ptr = std::unique_ptr<std::string>;
        Token ret{};
        if ((left.token_type == Tokens::INTEGER || left.token_type == Tokens::DOUBLE)
            && (right.token_type == Tokens::INTEGER || right.token_type == Tokens::DOUBLE)) {
            if (left.token_type == Tokens::DOUBLE || right.token_type == Tokens::DOUBLE) {
                ret.token_type = Tokens::DOUBLE;
                double v1, v2;
                auto _type_helper = [](const Token& t) {
                    switch (t.token_type) {
                    case Tokens::DOUBLE:
                        return std::get<double>(t.value);
                        break;
                    case Tokens::INTEGER:
                        return static_cast<double>(std::get<int64_t>(t.value));
                    default:
                        return .0;
                        break;
                    }
                };
                v1        = _type_helper(left);
                v2        = _type_helper(right);
                ret.value = v1 - v2;
            } else {
                ret.token_type = Tokens::INTEGER;
                ret.value      = std::get<int64_t>(left.value) - std::get<int64_t>(right.value);
            }
        } else {
            std::cerr << "不是可减的类型！\n";
        }
        return ret;
    }

    Token do_multiple(Token&& left, Token&& right) noexcept {
        using _STR_ptr = std::unique_ptr<std::string>;
        Token ret{};
        if ((left.token_type == Tokens::INTEGER || left.token_type == Tokens::DOUBLE)
            && (right.token_type == Tokens::INTEGER || right.token_type == Tokens::DOUBLE)) {
            if (left.token_type == Tokens::DOUBLE || right.token_type == Tokens::DOUBLE) {
                ret.token_type = Tokens::DOUBLE;
                double v1, v2;
                auto _type_helper = [](const Token& t) {
                    switch (t.token_type) {
                    case Tokens::DOUBLE:
                        return std::get<double>(t.value);
                        break;
                    case Tokens::INTEGER:
                        return static_cast<double>(std::get<int64_t>(t.value));
                    default:
                        return .0;
                        break;
                    }
                };
                v1        = _type_helper(left);
                v2        = _type_helper(right);
                ret.value = v1 * v2;
            } else {
                ret.token_type = Tokens::INTEGER;
                ret.value      = std::get<int64_t>(left.value) * std::get<int64_t>(right.value);
            }
        } else {
            std::cerr << "不是可乘的类型！\n";
        }
        return ret;
    }

    Token do_division(Token&& left, Token&& right) noexcept {
        using _STR_ptr = std::unique_ptr<std::string>;
        Token ret{};
        if ((left.token_type == Tokens::INTEGER || left.token_type == Tokens::DOUBLE)
            && (right.token_type == Tokens::INTEGER || right.token_type == Tokens::DOUBLE)) {
            if (left.token_type == Tokens::DOUBLE || right.token_type == Tokens::DOUBLE) {
                ret.token_type = Tokens::DOUBLE;
                double v1, v2;
                auto _type_helper = [](const Token& t) {
                    switch (t.token_type) {
                    case Tokens::DOUBLE:
                        return std::get<double>(t.value);
                        break;
                    case Tokens::INTEGER:
                        return static_cast<double>(std::get<int64_t>(t.value));
                    default:
                        return .0;
                        break;
                    }
                };
                v1        = _type_helper(left);
                v2        = _type_helper(right);
                ret.value = v1 / v2;
            } else {
                ret.token_type = Tokens::INTEGER;
                ret.value      = std::get<int64_t>(left.value) / std::get<int64_t>(right.value);
            }
        } else {
            std::cerr << "不是可乘的类型！\n";
        }
        return ret;
    }

    Token do_define(Token&& left, Token&& right, Env* env) {
        env->add(*std::get<std::unique_ptr<std::string>>(left.value), std::move(right));
        return Token{Tokens::K_DEFINE, 0};
    }
    Token _func_call(Lambda* func, List& params, Env* outer_env) {
        using _Ptr_Str_t = std::unique_ptr<std::string>;
        auto local_env = std::make_unique<Env>(outer_env);
        auto local_eval = std::make_unique<Eval>(local_env.get());

        for (int i = 1; i < params.size(); ++i) {
            local_env->add(*std::get<_Ptr_Str_t>(func->params[i - 1].value), std::move(params[i]));
        }
        size_t count = 0;
        return local_eval->eval(local_eval->parser(func->body, count));
    }
    Token do_getident_Call(const Token& ident, Env* env) {
        using _Ptr_List_t = std::unique_ptr<List>;
        using _Ptr_Str_t = std::unique_ptr<std::string>;
        auto& _params_list = *(std::get<_Ptr_List_t>(ident.value).get());
        const auto& name = *std::get<_Ptr_Str_t>(_params_list.at(0).value);

        auto tt = env->find(name);
        if (tt != nullptr && tt->token_type == Tokens::K_LAMBDA) {
            // std::cout << "BIG FUCK LAMBDA.\n";
            return _func_call(std::get<std::unique_ptr<Lambda>>(tt->value).get(), _params_list, env);
        } else {
            std::cerr << "没有发现变量：" << name << '\n';
            return Token{};
        }
    }
    Token do_getident(const std::string& name, Env* env) {
        auto tt = env->find(name);
        if (tt != nullptr) {
            return tt->copy();
        } else {
            std::cerr << "没有发现变量：" << name << '\n';
            return Token{};
        }
    }

    // 1. Lambda->params    = contain( node->left )
    // 2. Lambda->body      = contain( node->right )
    Token do_gen_lambda(AST_base* lambda, Env* env) {
        auto left  = std::move(std::get<std::unique_ptr<List>>(lambda->left->t.value));
        auto right = std::move(std::get<std::unique_ptr<List>>(lambda->right->t.value));
        auto pack  = std::make_unique<Lambda>(std::move(*left), std::move(*right));
        return Token{Tokens::K_LAMBDA, std::move(pack)};
    }

    Token eval(const std::unique_ptr<AST_base>& node) {
        auto tt = node->t.token_type;
        // NOTE: 没有问题, 无视clangd报警即可
        switch (tt) {
        case Tokens::QUOTE:
            return std::move(node->left->t);
        case Tokens::K_DEFINE:
            return do_define(std::move(node->left->t), std::move(eval(node->right)), env);
        case Tokens::IDENT_C:
            return do_getident_Call(node->t, env);
        case Tokens::IDENT:
            return do_getident(*std::get<std::unique_ptr<std::string>>(node->t.value), env);
        case Tokens::K_LAMBDA:
            return do_gen_lambda(node.get(), env);
        }

        Token left, right;
        if (node->left) {
            left = eval(node->left);
        }
        if (node->right) {
            right = eval(node->right);
        }

        switch (tt) {
        case Tokens::PLUS:
            return do_plus(std::move(left), std::move(right));
        case Tokens::MINUS:
            return do_minus(std::move(left), std::move(right));
        case Tokens::STAR:
            return do_multiple(std::move(left), std::move(right));
        case Tokens::DIVISION:
            return do_division(std::move(left), std::move(right));
        default:
            return std::move(node->t);
        }
    }
    // 如果上一条语句执行失败，paren_stack很有可能没有归0，对下一次执行产生影响
    constexpr void clear_status() noexcept {
        this->paren_stack = 0;
    }

private:
    Env* env;
    int paren_stack;
};

} // namespace austlisp

#endif
