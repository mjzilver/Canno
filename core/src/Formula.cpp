#include "../include/Formula.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "../include/Cell.hpp"
#include "../include/Sheet.hpp"
#include "../include/Utils.hpp"

Formula::Formula(const Cell* cell, const std::string& expr) {
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

std::string Formula::evaluate(Sheet& sheet) {
    deps.clear();

    if (failed) {
        deps = calc_deps(sheet);
        if (failed) {
            return err_msg;
        }
    }

    if (!root) return set_err("Empty formula");
    return evaluate_node(sheet, *root);
}

std::string Formula::evaluate_division(Sheet& sheet, const Node& left, const Node& right) {
    auto left_val = evaluate_node(sheet, left);
    auto right_val = evaluate_node(sheet, right);

    double l = 0, r = 0;
    if (parse_double(left_val, l) && parse_double(right_val, r)) {
        if (roughly_equal(r, 0.0)) return set_err("Divide by zero");
        return pretty_print_double(l / r);
    } else {
        return set_err("Cannot parse " + left_val + " or " + right_val);
    }
}

std::string Formula::evaluate_binary_op(Sheet& sheet, const Node& left, const Node& right,
                                        const std::function<double(double, double)>& op) {
    auto left_val = evaluate_node(sheet, left);
    auto right_val = evaluate_node(sheet, right);

    double l = 0, r = 0;
    if (parse_double(left_val, l) && parse_double(right_val, r)) {
        return pretty_print_double(op(l, r));
    } else {
        return set_err("Cannot parse " + left_val + " or " + right_val);
    }
}

std::vector<Node*> Formula::flatten_range(const Node& node) {
    std::vector<Node*> args;

    range_nodes_storage.clear();

    for (auto& arg : node.args) {
        if (arg->type == Node::Type::CELL_RANGE) {
            auto range_nodes = evaluate_range(*arg);
            for (auto& n : range_nodes) {
                args.push_back(n.get());
                range_nodes_storage.push_back(std::move(n));
            }
        } else {
            args.push_back(arg.get());
        }
    }
    return args;
}

std::optional<std::vector<double>> Formula::get_numeric_values(Sheet& sheet, const std::vector<Node*>& nodes) {
    std::vector<double> values;
    for (auto& n : nodes) {
        auto val_str = evaluate_node(sheet, *n);
        double val;
        if (!parse_double(val_str, val)) return std::nullopt;
        values.push_back(val);
    }
    return values;
}

std::string Formula::evaluate_func(Sheet& sheet, const Node& node) {
    std::vector<Node*> args = flatten_range(node);
    auto numeric_vals_opt = get_numeric_values(sheet, args);
    if (!numeric_vals_opt.has_value()) return set_err("Expected number");
    const auto& numeric_vals = numeric_vals_opt.value();

    if (node.value == "SUM") {
        double total = 0.0;
        for (double v : numeric_vals) total += v;
        return pretty_print_double(total);
    } else if (node.value == "AVG") {
        if (numeric_vals.empty()) return set_err("No values to average");
        double total = 0.0;
        for (double v : numeric_vals) total += v;
        return pretty_print_double(total / numeric_vals.size());
    } else if (node.value == "MIN") {
        if (numeric_vals.empty()) return set_err("No values for MIN");
        double min_val = numeric_vals[0];
        for (double v : numeric_vals) min_val = std::min(min_val, v);
        return pretty_print_double(min_val);
    } else if (node.value == "MAX") {
        if (numeric_vals.empty()) return set_err("No values for MAX");
        double max_val = numeric_vals[0];
        for (double v : numeric_vals) max_val = std::max(max_val, v);
        return pretty_print_double(max_val);
    } else if (node.value == "COUNT") {
        return std::to_string(numeric_vals.size());
    }

    return set_err("Unknown function: " + node.value);
}

std::vector<std::unique_ptr<Node>> Formula::evaluate_range(const Node& node) {
    auto delim_pos = node.value.find(':');
    if (delim_pos == std::string::npos) throw std::runtime_error("Cell range must contain ':'");

    std::string first = node.value.substr(0, delim_pos);
    std::string second = node.value.substr(delim_pos + 1);

    auto f_cell_i = cell_ref_to_indices(first);
    auto s_cell_i = cell_ref_to_indices(second);

    std::vector<std::unique_ptr<Node>> node_collection;
    for (int x = f_cell_i->first; x <= s_cell_i->first; ++x) {
        for (int y = f_cell_i->second; y <= s_cell_i->second; ++y) {
            std::string ref_name = indices_to_cell_ref(x, y);
            node_collection.push_back(std::make_unique<Node>(Node::Type::CELL_REF, ref_name));
        }
    }
    return node_collection;
}

std::string Formula::evaluate_node(Sheet& sheet, const Node& node) {
    switch (node.type) {
        case Node::Type::NUMBER:
            return pretty_print_double(std::stod(node.value));
        case Node::Type::STRING:
            return node.value;
        case Node::Type::CELL_REF: {
            Cell* ref_cell = sheet.get_cell(node.value);
            if (ref_cell) return ref_cell->get_value();
            return set_err("unknown ref " + node.value);
        }
        case Node::Type::CELL_RANGE:
            return set_err("Invalid cell range context");
        case Node::Type::ADD:
            return evaluate_binary_op(sheet, *node.left, *node.right, [](double a, double b) { return a + b; });
        case Node::Type::SUBTRACT:
            return evaluate_binary_op(sheet, *node.left, *node.right, [](double a, double b) { return a - b; });
        case Node::Type::MULTIPLY:
            return evaluate_binary_op(sheet, *node.left, *node.right, [](double a, double b) { return a * b; });
        case Node::Type::DIVIDE:
            return evaluate_division(sheet, *node.left, *node.right);
        case Node::Type::FUNCTION:
            return evaluate_func(sheet, node);
    }
    return set_err("Unexpected node type");
}

void Formula::tokenize(const std::string& expr) {
    tokens.clear();
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
            std::string num(1, c);
            bool has_dot = (c == '.');
            while (i + 1 < expr.size() && (isdigit(expr[i + 1]) || (!has_dot && expr[i + 1] == '.'))) {
                ++i;
                if (expr[i] == '.') has_dot = true;
                num += expr[i];
            }
            tokens.push_back({Token::NUM_TOK, num});
            continue;
        }

        // CellRef or Func
        if (isalpha(c)) {
            std::string cell_val(1, c);
            while (i + 1 < expr.size() && isalpha(expr[i + 1])) {
                ++i;
                cell_val += expr[i];
            }

            if (i + 1 < expr.size() && isdigit(expr[i + 1])) {
                while (i + 1 < expr.size() && isdigit(expr[i + 1])) {
                    ++i;
                    cell_val += expr[i];
                }

                if (i + 1 < expr.size() && expr[i + 1] == ':') {
                    ++i;
                    cell_val += expr[i];
                    while (i + 1 < expr.size() && isalnum(expr[i + 1])) {
                        ++i;
                        cell_val += expr[i];
                    }
                    tokens.push_back({Token::CELL_RANGE_TOK, cell_val});
                } else {
                    tokens.push_back({Token::CELL_REF_TOK, cell_val});
                }
            } else {
                tokens.push_back({Token::FUNC_TOK, cell_val});
            }
        }
    }
}

void Formula::parse(const std::string& expr) {
    tokens.clear();
    tokenize(expr);

    if (tokens.empty() || tokens[0].type != Token::EQ_TOK) throw std::runtime_error("Formula must start with '='");

    current = 1;
    root = parse_expression();
}

std::unique_ptr<Node> Formula::parse_expression() {
    auto node = parse_term();
    while (match({Token::PLUS_TOK, Token::MIN_TOK})) {
        Token op = previous().type;
        auto right = parse_term();
        if (op == Token::PLUS_TOK)
            node = std::make_unique<Node>(Node::Type::ADD, "+", std::move(node), std::move(right));
        else
            node = std::make_unique<Node>(Node::Type::SUBTRACT, "-", std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Node> Formula::parse_term() {
    auto node = parse_factor();
    while (match({Token::DIV_TOK, Token::MULT_TOK})) {
        Token op = previous().type;
        auto right = parse_term();
        if (op == Token::DIV_TOK)
            node = std::make_unique<Node>(Node::Type::DIVIDE, "/", std::move(node), std::move(right));
        else
            node = std::make_unique<Node>(Node::Type::MULTIPLY, "*", std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Node> Formula::parse_factor() {
    if (at_end()) {
        set_err("Unexpected end of formula");
        return nullptr;
    }

    const auto& tok = peek();
    if (tok.type == Token::NUM_TOK) {
        advance();
        return std::make_unique<Node>(Node::Type::NUMBER, tok.value);
    }
    if (tok.type == Token::CELL_REF_TOK) {
        advance();
        return std::make_unique<Node>(Node::Type::CELL_REF, tok.value);
    }
    if (tok.type == Token::CELL_RANGE_TOK) {
        advance();
        return std::make_unique<Node>(Node::Type::CELL_RANGE, tok.value);
    }
    if (tok.type == Token::FUNC_TOK) {
        advance();
        auto func_node = std::make_unique<Node>(Node::Type::FUNCTION, tok.value);
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
    set_err("Unexpected token '" + tok.value + "'");
    return nullptr;
}

bool Formula::match(std::initializer_list<Token> types) {
    if (at_end()) return false;
    for (auto type : types)
        if (peek().type == type) {
            advance();
            return true;
        }
    return false;
}

bool Formula::check(Token type) const { return !at_end() && peek().type == type; }

const TokenData& Formula::advance() {
    if (!at_end()) current++;
    return tokens[current - 1];
}
const TokenData& Formula::peek() const { return tokens[current]; }
const TokenData& Formula::previous() const { return tokens[current - 1]; }
bool Formula::at_end() const { return current >= tokens.size(); }

bool Formula::calc_node_deps(Sheet& sheet, const Node& node, std::unordered_set<Cell*>& visited) {
    if (failed) return false;

    if (node.type == Node::Type::CELL_REF) {
        Cell* cell = sheet.get_cell(node.value);
        if (cell) {
            if (visited.count(cell)) {
                set_err("Circular reference detected");
                return false;
            }
            deps.push_back(cell);
            if (auto cell_form = cell->get_formula()) {
                visited.insert(cell);
                if (auto p_root = cell_form->get_root())
                    if (!calc_node_deps(sheet, *p_root, visited)) return false;
                visited.erase(cell);
            }
        }
    }

    if (node.left && !calc_node_deps(sheet, *node.left, visited)) return false;
    if (node.right && !calc_node_deps(sheet, *node.right, visited)) return false;
    for (auto& arg : node.args)
        if (!calc_node_deps(sheet, *arg, visited)) return false;

    if (node.type == Node::Type::CELL_RANGE) {
        auto cells = evaluate_range(node);
        for (auto& cell_ref : cells)
            if (!calc_node_deps(sheet, *cell_ref, visited)) return false;
    }

    return true;
}

std::vector<Cell*> Formula::calc_deps(Sheet& sheet) {
    deps.clear();
    if (!root) return deps;
    std::unordered_set<Cell*> visited;
    calc_node_deps(sheet, *root, visited);
    return deps;
}
