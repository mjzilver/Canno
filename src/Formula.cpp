#include "Formula.hpp"

#include <cctype>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>

#include "Cell.hpp"
#include "Sheet.hpp"

Formula::Formula(const std::shared_ptr<Cell>& cell, const std::string& expr) {
    text = expr;
    containing_cell = cell;
    parse(expr);
}

std::string Formula::set_err(const std::string& err) {
    auto full_error = "#ERR: " + err;
    failed = true;
    err_msg = full_error;
    return full_error;
}

std::string Formula::evaluate(std::shared_ptr<Sheet> sheet) {
    deps.clear();

    if (failed) {
        return err_msg;
    }

    if (!root) {
        return set_err("No root node");
    }

    return evaluate_node(sheet, root);
}

std::string Formula::evaluate_binary_op(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> left,
                                        std::shared_ptr<Node> right, const std::function<int(int, int)>& op) {
    auto left_val = evaluate_node(sheet, left);
    auto right_val = evaluate_node(sheet, right);

    try {
        int l = std::stoi(left_val);
        int r = std::stoi(right_val);
        return std::to_string(op(l, r));
    } catch (...) {
        return set_err("Invalid binary operation");
    }
}

std::string Formula::evaluate_node(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> node) {
    if (node->type == Node::Type::NUMBER) {
        return node->value;
    } else if (node->type == Node::Type::STRING) {
        return node->value;
    } else if (node->type == Node::Type::CELL_REF) {
        std::shared_ptr<Cell> ref_cell = sheet->get_cell(node->value);

        if (ref_cell != nullptr) {
            if (ref_cell == containing_cell) {
                return set_err("Circular ref");
            }

            return ref_cell->get_value();
        } else {
            return set_err("unknown ref " + node->value);
        }
    } else if (node->type == Node::Type::ADD) {
        return evaluate_binary_op(sheet, node->left, node->right, [](int a, int b) { return a + b; });
    } else if (node->type == Node::Type::SUBTRACT) {
        return evaluate_binary_op(sheet, node->left, node->right, [](int a, int b) { return a - b; });
    } else if (node->type == Node::Type::MULTIPLY) {
        return evaluate_binary_op(sheet, node->left, node->right, [](int a, int b) { return a * b; });
    } else if (node->type == Node::Type::DIVIDE) {
        return evaluate_binary_op(sheet, node->left, node->right, [](int a, int b) { return a / b; });
    }

    return set_err("Unexpected node type");
}

void Formula::tokenize(const std::string& expr) {
    for (size_t i = 0; i < expr.length(); ++i) {
        char c = expr[i];

        switch (c) {
            case '=':
                tokens.push_back({Token::EQ_TOK, "="});
                break;
            case '+':
                tokens.push_back({Token::PLUS_TOK, "+"});
                break;
            case '-':
                tokens.push_back({Token::MIN_TOK, "-"});
                break;
            case '*':
                tokens.push_back({Token::MULT_TOK, "*"});
                break;
            case '/':
                tokens.push_back({Token::DIV_TOK, "/"});
                break;
            case ',':
                tokens.push_back({Token::COMM_TOK, ","});
                break;
            case '(':
                tokens.push_back({Token::LPAR_TOK, "("});
                break;
            case ')':
                tokens.push_back({Token::RPAR_TOK, ")"});
                break;
        }

        // Number
        if (isdigit(c) || c == '.') {
            std::string num;
            bool has_dot = (c == '.');
            num += c;
            while (i + 1 < expr.size() && (isdigit(expr[i + 1]) || (!has_dot && expr[i + 1] == '.'))) {
                ++i;
                if (expr[i] == '.') has_dot = true;
                num += expr[i];
            }
            tokens.push_back({Token::NUM_TOK, num});
            continue;
        }

        // CellRef OR Func
        if (std::isalpha(c)) {
            std::string cell_val;
            cell_val += c;

            // Get all letters
            while (i + 1 < expr.size() && isalpha(expr[i + 1])) {
                ++i;
                cell_val += expr[i];
            }

            // A1, AA11 etc
            if (i + 1 < expr.size() && isdigit(expr[i + 1])) {
                while (i + 1 < expr.size() && isdigit(expr[i + 1])) {
                    ++i;
                    cell_val += expr[i];
                }
                tokens.push_back({Token::CELL_REF_TOK, cell_val});
                continue;
            } else {
                tokens.push_back({Token::FUNC_TOK, cell_val});
                continue;
            }
        }
    }
}

void Formula::parse(const std::string& expr) {
    tokens.clear();

    tokenize(expr);

    if (tokens.empty() || tokens[0].type != Token::EQ_TOK) {
        // Should never happen
        throw std::runtime_error("Formula must start with '='");
    }

    current = 1;

    root = parse_expression();
}

// lowest precedence = + -
std::shared_ptr<Node> Formula::parse_expression() {
    auto node = parse_term();
    while (match({Token::PLUS_TOK, Token::MIN_TOK})) {
        Token op = previous().type;
        auto right = parse_term();

        if (op == Token::PLUS_TOK)
            node = std::make_shared<Node>(Node::Type::ADD, "+", node, right);
        else
            node = std::make_shared<Node>(Node::Type::SUBTRACT, "-", node, right);
    }

    return node;
}

// Medium precedence = / *
std::shared_ptr<Node> Formula::parse_term() {
    auto node = parse_factor();
    while (match({Token::DIV_TOK, Token::MULT_TOK})) {
        Token op = previous().type;
        auto right = parse_term();

        if (op == Token::DIV_TOK) {
            node = std::make_shared<Node>(Node::Type::DIVIDE, "/", node, right);
        } else {
            node = std::make_shared<Node>(Node::Type::MULTIPLY, "*", node, right);
        }
    }

    return node;
}

// highest precedence = nums, (), cell_ref, funcs
std::shared_ptr<Node> Formula::parse_factor() {
    if (at_end()) {
        set_err("Unexpcted end of formula");
        return nullptr;
    }

    const auto& tok = peek();
    // if num
    if (tok.type == Token::NUM_TOK) {
        advance();
        return std::make_shared<Node>(Node::Type::NUMBER, tok.value);
    }
    // if cell ref
    if (tok.type == Token::CELL_REF_TOK) {
        advance();
        return std::make_shared<Node>(Node::Type::CELL_REF, tok.value);
    }
    // TODO: function & parens

    set_err("unexpected token '" + tok.value + "'");
    return nullptr;
}

bool Formula::match(std::initializer_list<Token> types) {
    if (at_end()) return false;

    for (auto type : types) {
        if (peek().type == type) {
            advance();
            return true;
        }
    }
    return false;
}

const TokenData& Formula::advance() {
    if (!at_end()) current++;
    return tokens[current - 1];
}

const TokenData& Formula::peek() const { return tokens[current]; }
const TokenData& Formula::previous() const { return tokens[current - 1]; }

bool Formula::at_end() const { return current >= tokens.size(); }

void Formula::calc_node_deps(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> node) {
    if (!node) return;

    if (node->type == Node::Type::CELL_REF) {
        if (auto cell = sheet->get_cell(node->value)) {
            deps.push_back(cell);
        }
    }

    calc_node_deps(sheet, node->left);
    calc_node_deps(sheet, node->right);
}

std::vector<std::shared_ptr<Cell>> Formula::calc_deps(std::shared_ptr<Sheet> sheet) {
    if (!root) return deps;
    calc_node_deps(sheet, root);

    return deps;
}