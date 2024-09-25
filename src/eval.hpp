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
        virtual ~AST_base() = default; // 没有什么意义，就是为了用 dynmaic_cast
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
        if (token_list.empty()) 
            return std::make_unique<AST_base>();

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
        case Tokens::TRUE:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::TRUE;
            node->t.value      = 1;
            return node;
        case Tokens::FALSE:
            node               = std::make_unique<AST_base>();
            node->t.token_type = Tokens::TRUE;
            node->t.value      = 0;
            return node;
        case Tokens::K_IF:
            {
                auto if_node          = std::make_unique<AST_if>();
                if_node->t.token_type = Tokens::K_IF;
                if_node->cond         = parser(token_list, ++t);
                if_node->left         = parser(token_list, ++t);
                if_node->right        = parser(token_list, ++t);
                return if_node;
            }
        case Tokens::K_WHILE:
            {
                auto loop_node = std::make_unique<AST_base>();
                loop_node->t.token_type = Tokens::K_WHILE;
                loop_node->left = parser(token_list, ++t);
                loop_node->right = parser(token_list, ++t);
                return loop_node;
            }
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
                        paren_holder++;
                        list->emplace_back(std::move(token_list[t]));
                        break;
                    case Tokens::RPAREN:
                        paren_holder--;
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
                }
                // else if (t + paren_holder != token_list.size() - 1) {
                //     UNEXCEPTED_RPAREN;
                //     return std::make_unique<AST_base>(Token{});
                // }

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
        case Tokens::K_SETQ:
            {
                node               = std::make_unique<AST_base>();
                node->t.token_type = Tokens::K_SETQ;
                if (token_list[++t].token_type != Tokens::IDENT) {
                    std::cerr << "error!: setq后必须跟一个符号名称.\n";
                    return std::make_unique<AST_base>(Token{});
                }
                node->left  = std::make_unique<AST_base>(std::move(token_list[t]));
                node->right = parser(token_list, ++t);
                if (node->right->t.token_type == Tokens::NONE) {
                    std::cerr << "error!: setq需要一个赋给变量的值.\n";
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
                using _Ptr_Str_t = std::unique_ptr<std::string>;
                node             = std::make_unique<AST_base>();
                // 前面是一个 '(' 说明是一个调用
                if (t - 1 >= 0 && token_list[t - 1].token_type == Tokens::LPAREN) {
                    node->t.token_type = Tokens::IDENT_C;
                    auto params_list   = std::make_unique<List>();
                    params_list->emplace_back(token_list[t].copy());
                    Token tmp{};
                    int paren_holder = this->paren_stack;
                    // auto res = parser(token_list, t);
                    // tmp      = eval(res);
                    // (area 2 '(2 2))
                    do {
                        switch (token_list[++t].token_type) {
                        case Tokens::LPAREN:
                            paren_holder++;
                            break;
                        case Tokens::RPAREN:
                            paren_holder--;
                            break;
                        }
                        params_list->emplace_back(token_list[t]);
                    } while (!(token_list[t].token_type == Tokens::RPAREN && paren_holder == paren_stack - 1));
                    node->t.value = std::move(params_list);
                } else {
                    node->t.token_type = Tokens::IDENT;
                    auto ident_name = *std::get<_Ptr_Str_t>(token_list[t].value);
                    node->t.value   = std::make_unique<std::string>(ident_name);
                    // lambda表达式的body部分的token原来用下面的代码会被移走，我用上面👆的代码
                    // 解决了，去tmd的，不在乎这一点性能
                    // node->t.value = std::move(token_list[t].value);
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
    Token do_while(AST_base* loop_node, Env* env) {
        auto cond = eval(loop_node->left);
        cond = eval(loop_node->left);
        Token ret{};
        while (cond.token_type != Tokens::FALSE) {
            ret = eval(loop_node->right);
            cond = eval(loop_node->left);
        }
        return ret;
    }
    Token do_setq(Token&& left, Token&& right, Env* env) {
        env->update(*std::get<_Ptr_Str_t>(left.value), right.token_type, std::move(right.value));
        return Token{};
    }
    Token _func_call(Lambda* func, List& params, Env* outer_env) {
        using _Ptr_Str_t = std::unique_ptr<std::string>;
        auto local_env   = std::make_unique<Env>(outer_env);
        auto local_eval  = std::make_unique<Eval>(local_env.get());

        for (int i = 1; i < params.size(); ++i) {
            local_env->add(*std::get<_Ptr_Str_t>(func->params[i - 1].value), std::move(params[i]));
        }
        size_t count      = 0;
        auto _holder_body = func->body;
        return local_eval->eval(local_eval->parser(_holder_body, count));
    }
    Token do_getident_Call(const Token& ident, Env* env) {
        using _Ptr_List_t = std::unique_ptr<List>;
        using _Ptr_Str_t  = std::unique_ptr<std::string>;
        std::unique_ptr<AST_base> res;
        // DONE: 把求参数推迟到这里，前面就是记录参数
        auto& contain    = *std::get<_Ptr_List_t>(ident.value);
        const auto& name = *std::get<_Ptr_Str_t>(contain.at(0).value);

        size_t count = 0; // ignore indent name
        List _params_list{};
        _params_list.emplace_back(contain.at(0));
        while (contain[++count].token_type != Tokens::RPAREN) {
            // 我不得不放弃之前使用引用的想法，除非我再添加一个字符串字面量的类型，但我不想再在为
            // 这个项目花更多的时间了。一个没有注册到sym_table中的字面量在用了智能指针管理内存的情况下
            // 其挂在AST上的Token会在出当前作用域就被释放掉，这将引发很多问题，包括让我之前在eq，equal函数上的
            // 工作白费了，所以Token的reference函数目前只对define过的变量有效果，实际上在当前代码下弃用了
            // 除非我在花精力搞字符串字面量的token_type。
            // 也怪我没有做好前期的设计😡，C++写一个解释器比我预想的要麻烦很多。
            // Anyway, 就这样吧。
            auto res  = parser(contain, count);
            Token tmp = eval(res);
            _params_list.emplace_back(std::move(tmp));
        }

        auto tt = env->find(name);
        if (tt != nullptr) {
            // std::cout << "BIG FUCK LAMBDA.\n";
            switch (tt->token_type) {
            case Tokens::K_LAMBDA:
                {
                    auto _lambda = std::get<std::unique_ptr<Lambda>>(tt->value).get();
                    return _func_call(_lambda, _params_list, env);
                }
            case Tokens::_BUILDIN_CAR:
                return env->_buildin_func_car(_params_list);
            case Tokens::_BUILDIN_CDR:
                return env->_buildin_func_cdr(_params_list);
            case Tokens::_BUILDIN_EQ:
                try {
                    return env->_buildin_func_eq(_params_list);
                } catch (std::logic_error* e) {
                    e->what();
                    return Token{};
                }
            case Tokens::_BUILDIN_EQUAL:
                try {
                    return env->_buildin_func_equal(_params_list);
                } catch (std::logic_error* e) {
                    e->what();
                    return Token{};
                }
            default:
                std::cerr << "未知的lambda:" << name << '\n';
                return Token{};
            }
        } else {
            std::cerr << "没有发现变量：" << name << '\n';
            return Token{};
        }
    }

    Token do_getident(const std::string& name, Env* env) {
        auto tt = env->find(name);
        if (tt != nullptr) {
            // 复杂类型返回引用，基本类型复制
            switch (tt->token_type) {
            case Tokens::INTEGER:
            case Tokens::DOUBLE:
                return tt->copy();
            case Tokens::LIST:
            case Tokens::STRING:
                return tt->copy();
            default:
                return tt->copy();
            }
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

    Token do_condition(AST_if* if_stmt, Env* env) {
        auto ret = eval(if_stmt->cond);
        if (ret.token_type == Tokens::TRUE) {
            return eval(if_stmt->left);
        } else {
            return eval(if_stmt->right);
        }
    }
    Token eval(AST_if* node) noexcept {
        return do_condition(node, env);
    }
    Token eval(const std::unique_ptr<AST_base>& node) {
        auto tt = node->t.token_type;
        // NOTE: 没有问题, 无视clangd报警即可
        switch (tt) {
        case Tokens::QUOTE:
            return std::move(node->left->t);
        case Tokens::K_IF:
            return eval(dynamic_cast<AST_if*>(node.get()));
        case Tokens::K_WHILE:
            return do_while(node.get(), env);
        case Tokens::K_DEFINE:
            return do_define(std::move(node->left->t), std::move(eval(node->right)), env);
        case Tokens::K_SETQ:
            return do_setq(std::move(node->left->t), std::move(eval(node->right)), env);
        case Tokens::IDENT_C:
            return do_getident_Call(node->t, env);
        case Tokens::IDENT:
            return do_getident(*std::get<std::unique_ptr<std::string>>(node->t.value), env);
        case Tokens::TRUE:
            return Token{Tokens::TRUE, 1};
        case Tokens::FALSE:
            return Token{Tokens::FALSE, 0};
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
        // case Tokens::STRING:
        //     return node->t.reference();
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
