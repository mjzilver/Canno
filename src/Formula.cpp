#include "Formula.hpp"

#include <cctype>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Sheet.hpp"

std::string token_to_string(Token t) {
    switch (t) {
        case Token::NUM_TOK:
            return "NUM_TOK";
        case Token::STR_TOK:
            return "STR_TOK";
        case Token::EQ_TOK:
            return "EQ_TOK";
        case Token::MIN_TOK:
            return "MIN_TOK";
        case Token::PLUS_TOK:
            return "PLUS_TOK";
        case Token::DIV_TOK:
            return "DIV_TOK";
        case Token::MULT_TOK:
            return "MULT_TOK";
        case Token::COMM_TOK:
            return "COMM_TOK";
        case Token::LPAR_TOK:
            return "LPAR_TOK";
        case Token::RPAR_TOK:
            return "RPAR_TOK";
        case Token::CELL_REF_TOK:
            return "CELL_REF_TOK";
        case Token::FUNC_TOK:
            return "FUNC_TOK";
        default:
            return "UNKNOWN";
    }
}

Formula::Formula(const std::string& expr) { parse(expr); }

std::string Formula::evaluate(std::shared_ptr<Sheet> sheet) {
    if (failed) {
        return err_msg;
    }

    if (!root) {
        return "#ERR: No root";
    }

    return evaluate_node(sheet, root);
}

std::string Formula::evaluate_node(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> node) {
    if (node->type == Node::Type::Number) {
        return node->value;
    } else if (node->type == Node::Type::CellRef) {
        auto ref_val = sheet->get_cell_val(node->value);

        if (ref_val.has_value()) {
            return ref_val.value();
        } else {
            return "#ERR: unknown ref " + node->value;
        }
    }

    return "#ERR: Unexpected node type";
}

std::vector<std::pair<int, int>> Formula::dependencies() const { return deps; }

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
            bool hasDot = (c == '.');
            num += c;
            while (i + 1 < expr.size() && (isdigit(expr[i + 1]) || (!hasDot && expr[i + 1] == '.'))) {
                ++i;
                if (expr[i] == '.') hasDot = true;
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
            node = std::make_shared<Node>(Node::Type::Add, "+", node, right);
        else
            node = std::make_shared<Node>(Node::Type::Subtract, "-", node, right);
    }

    return node;
}

// Medium precedence = / *
std::shared_ptr<Node> Formula::parse_term() {
    auto node = parse_factor();
    while (match({Token::DIV_TOK, Token::MULT_TOK})) {
      Token op = previous().type;
        auto right = parse_term();

        if (op == Token::DIV_TOK)
            node = std::make_shared<Node>(Node::Type::Divide, "/", node, right);
        else
            node = std::make_shared<Node>(Node::Type::Multiply, "*", node, right);
    }

    return node;
}


// highest precedence = nums, (), cell_ref, funcs
std::shared_ptr<Node> Formula::parse_factor() {
    if (at_end()) {
        failed = true;
        err_msg = "#ERR: unexpected end of formula";
        return nullptr;
    }

    const auto& tok = peek();
    // if num
    if (tok.type == Token::NUM_TOK) {
        advance();
        return std::make_shared<Node>(Node::Type::Number, tok.value);
    }
    // if cell ref
    if (tok.type == Token::CELL_REF_TOK) {
        advance();
        return std::make_shared<Node>(Node::Type::CellRef, tok.value);
    }
    // TODO: function & parens

    failed = true;
    err_msg = "#ERR: unexpected token '" + tok.value + "'";
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
