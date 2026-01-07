#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

// forward declaration
class Cell;
class Sheet;

enum class Token {
    NUM_TOK,         // 11.11
    STR_TOK,         // string
    EQ_TOK,          // =
    MIN_TOK,         // +
    PLUS_TOK,        // -
    DIV_TOK,         // /
    MULT_TOK,        // *
    COMM_TOK,        // ,
    LPAR_TOK,        // (
    RPAR_TOK,        // )
    CELL_REF_TOK,    // A1
    CELL_RANGE_TOK,  // A1:A3
    FUNC_TOK         // SUM
};

struct TokenData {
    Token type;
    std::string value;
};

struct Node {
    enum class Type { NUMBER, STRING, CELL_REF, CELL_RANGE, FUNCTION, ADD, SUBTRACT, MULTIPLY, DIVIDE } type;

    std::string value;

    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
    std::vector<std::unique_ptr<Node>> args;

    explicit Node(Type t, std::string val) : type(t), value(std::move(val)) {}

    Node(Type t, std::string val, std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : type(t), value(std::move(val)), left(std::move(l)), right(std::move(r)) {}
};

class Formula {
public:
    explicit Formula(const Cell* cell, const std::string& expr);

    std::string evaluate(Sheet& sheet);

    std::vector<Cell*> calc_deps(Sheet& sheet);

    std::string get_text() { return text; }
    const Node* get_root() const { return root.get(); }

private:
    std::string err_msg = "";
    bool failed = false;
    size_t current = 0;

    const Cell* containing_cell;
    std::string text = "";

    std::unique_ptr<Node> root;
    std::vector<TokenData> tokens;
    std::vector<Cell*> deps;

    void parse(const std::string& expr);
    void tokenize(const std::string& expr);

    std::string set_err(const std::string& err);

    std::string evaluate_node(Sheet& sheet, const Node& node);
    std::string evaluate_func(Sheet& sheet, const Node& node);
    std::string evaluate_binary_op(Sheet& sheet, const Node& left, const Node& right,
                                   const std::function<double(double, double)>& op);
    std::string evaluate_division(Sheet& sheet, const Node& left, const Node& right);

    std::vector<Node*> evaluate_range(const Node& node);

    std::unique_ptr<Node> parse_expression();
    std::unique_ptr<Node> parse_term();
    std::unique_ptr<Node> parse_factor();

    std::vector<Node*> flatten_range(const Node& node);
    std::optional<std::vector<double>> get_numeric_values(Sheet& sheet, const std::vector<Node*>& nodes);

    bool match(std::initializer_list<Token> types);
    const TokenData& advance();
    const TokenData& peek() const;
    const TokenData& previous() const;
    bool at_end() const;
    bool check(Token type) const;

    bool calc_node_deps(Sheet& sheet, const Node& node, std::unordered_set<Cell*>& visited);
};
