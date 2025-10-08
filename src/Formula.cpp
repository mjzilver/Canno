#include "Formula.hpp"

#include <cctype>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Cell.hpp"
#include "Sheet.hpp"
#include "Utils.hpp"

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

    failed = false;

    if (!root) {
        if (!err_msg.empty()) {
            return err_msg;
        }
        return set_err("No root node");
    }

    return evaluate_node(sheet, root);
}

std::string Formula::evaluate_binary_op(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> left,
                                        std::shared_ptr<Node> right, const std::function<double(double, double)>& op) {
    auto left_val = evaluate_node(sheet, left);
    auto right_val = evaluate_node(sheet, right);

    try {
        double l = std::stod(left_val);
        double r = std::stod(right_val);

        return pretty_print_double(op(l, r));
    } catch (...) {
        return set_err("Invalid binary operation");
    }
}

std::string Formula::evaluate_func(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> node) {
    // Flatten any ranges into individual CELL_REF nodes
    std::vector<std::shared_ptr<Node>> args;
    for (auto& arg : node->args) {
        if (arg->type == Node::Type::CELL_RANGE) {
            auto range_nodes = evaluate_range(arg);
            args.insert(args.end(), range_nodes.begin(), range_nodes.end());
        } else {
            args.push_back(arg);
        }
    }

    auto get_numeric_values =
        [&](const std::vector<std::shared_ptr<Node>>& nodes) -> std::optional<std::vector<double>> {
        std::vector<double> values;
        for (auto& n : nodes) {
            auto val_str = evaluate_node(sheet, n);
            double val;
            if (!parse_double(val_str, val)) return std::nullopt;
            values.push_back(val);
        }
        return values;
    };

    auto numeric_vals_opt = get_numeric_values(args);
    if (!numeric_vals_opt.has_value()) return set_err("Expected number");
    const auto& numeric_vals = numeric_vals_opt.value();

    if (node->value == "SUM") {
        double total = 0.0;
        for (double v : numeric_vals) total += v;
        return pretty_print_double(total);
    } else if (node->value == "AVG") {
        if (numeric_vals.empty()) return set_err("No values to average");
        double total = 0.0;
        for (double v : numeric_vals) total += v;
        return pretty_print_double(total / numeric_vals.size());
    } else if (node->value == "MIN") {
        if (numeric_vals.empty()) return set_err("No values for MIN");
        double min_val = numeric_vals[0];
        for (double v : numeric_vals) min_val = std::min(min_val, v);
        return pretty_print_double(min_val);
    } else if (node->value == "MAX") {
        if (numeric_vals.empty()) return set_err("No values for MAX");
        double max_val = numeric_vals[0];
        for (double v : numeric_vals) max_val = std::max(max_val, v);
        return pretty_print_double(max_val);
    } else if (node->value == "COUNT") {
        return std::to_string(numeric_vals.size());
    }

    return set_err("Unknown function: " + node->value);
}

std::vector<std::shared_ptr<Node>> Formula::evaluate_range(std::shared_ptr<Node> node) {
    auto delim_pos = node->value.find(':');
    if (delim_pos == std::string::npos) {
        throw std::runtime_error("Cell range must contain ':'");
    }
    std::string first = node->value.substr(0, delim_pos);
    std::string second = node->value.substr(delim_pos + 1);

    auto f_cell_i = cell_ref_to_indices(first);
    auto s_cell_i = cell_ref_to_indices(second);

    std::vector<std::shared_ptr<Node>> node_collection;

    for (int x = f_cell_i->first; x <= s_cell_i->first; ++x) {
        for (int y = f_cell_i->second; y <= s_cell_i->second; ++y) {
            std::string ref_name = indices_to_cell_ref(x, y);
            node_collection.push_back(std::make_shared<Node>(Node::Type::CELL_REF, ref_name));
        }
    }

    return node_collection;
}

std::string Formula::evaluate_node(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> node) {
    if (node->type == Node::Type::NUMBER) {
        return pretty_print_double(std::stod(node->value));
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
    } else if (node->type == Node::Type::CELL_RANGE) {
        return set_err("Invalid cell range context");
    } else if (node->type == Node::Type::ADD) {
        return evaluate_binary_op(sheet, node->left, node->right, [](double a, double b) { return a + b; });
    } else if (node->type == Node::Type::SUBTRACT) {
        return evaluate_binary_op(sheet, node->left, node->right, [](double a, double b) { return a - b; });
    } else if (node->type == Node::Type::MULTIPLY) {
        return evaluate_binary_op(sheet, node->left, node->right, [](double a, double b) { return a * b; });
    } else if (node->type == Node::Type::DIVIDE) {
        return evaluate_binary_op(sheet, node->left, node->right, [](double a, double b) { return a / b; });
    } else if (node->type == Node::Type::FUNCTION) {
        return evaluate_func(sheet, node);
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

                if (expr[i + 1] == ':') {
                    ++i;
                    cell_val += expr[i];
                    // consume until it is not alpanumeric
                    while (i + 1 < expr.size() && std::isalnum(expr[i + 1])) {
                        ++i;
                        cell_val += expr[i];
                    }
                    tokens.push_back({Token::CELL_RANGE_TOK, cell_val});
                } else {
                    tokens.push_back({Token::CELL_REF_TOK, cell_val});
                }

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

    if (tok.type == Token::NUM_TOK) {
        advance();
        return std::make_shared<Node>(Node::Type::NUMBER, tok.value);
    }
    if (tok.type == Token::CELL_REF_TOK) {
        advance();
        return std::make_shared<Node>(Node::Type::CELL_REF, tok.value);
    }
    if (tok.type == Token::CELL_RANGE_TOK) {
        advance();
        return std::make_shared<Node>(Node::Type::CELL_RANGE, tok.value);
    }
    if (tok.type == Token::FUNC_TOK) {
        advance();
        auto func_node = std::make_shared<Node>(Node::Type::FUNCTION, tok.value);

        if (!match({Token::LPAR_TOK})) {
            set_err("Expected '(' after function name");
            return nullptr;
        }

        if (!check(Token::RPAR_TOK)) {
            do {
                func_node->args.push_back(parse_expression());
            } while (match({Token::COMM_TOK}));
        }

        if (!match({Token::RPAR_TOK})) {
            set_err("Expected ')' after function arguments");
            return nullptr;
        }

        return func_node;
    }

    // TODO: parens

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

bool Formula::check(Token type) const {
    if (at_end()) return false;
    return peek().type == type;
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

    for (auto& arg : node->args) {
        calc_node_deps(sheet, arg);
    }

    if (node->type == Node::Type::CELL_RANGE) {
        auto cells = evaluate_range(node);
        for (auto& cell_ref : cells) {
            calc_node_deps(sheet, cell_ref);
        }
    }
}

std::vector<std::shared_ptr<Cell>> Formula::calc_deps(std::shared_ptr<Sheet> sheet) {
    if (!root) return deps;
    calc_node_deps(sheet, root);

    return deps;
}