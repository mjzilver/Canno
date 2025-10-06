#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

// forward declaration
class Cell;
class Sheet;

enum class Token {
    NUM_TOK,       // 12 or 12.12 or .012
    STR_TOK,       // "string"
    EQ_TOK,        // =
    MIN_TOK,       // +
    PLUS_TOK,      // -
    DIV_TOK,       // /
    MULT_TOK,      // *
    COMM_TOK,      // ,
    LPAR_TOK,      // (
    RPAR_TOK,      // )
    CELL_REF_TOK,  // A1
    FUNC_TOK       // SUM
};

struct TokenData {
    Token type;
    std::string value;
};

struct Node {
    enum class Type {
        NUMBER,    // float
        STRING,    // string
        CELL_REF,  // ptr
        FUNCTION,
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE
    } type;
    std::string value;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    Node(Type t, const std::string& val) : type(t), value(val) {}

    Node(Type t, const std::string& val, const std::shared_ptr<Node>& l, const std::shared_ptr<Node>& r)
        : type(t), value(val), left(l), right(r) {}
};

class Formula {
public:
    explicit Formula(const std::shared_ptr<Cell>& cell, const std::string& expr);

    std::string evaluate(std::shared_ptr<Sheet> sheet);

    std::vector<std::shared_ptr<Cell>> calc_deps(std::shared_ptr<Sheet> sheet);

    std::string get_text() { return text; }

private:
    std::string err_msg = "";
    bool failed = false;
    size_t current = 0;

    std::shared_ptr<Cell> containing_cell = nullptr;
    std::string text = "";

    std::shared_ptr<Node> root;
    std::vector<TokenData> tokens;
    std::vector<std::shared_ptr<Cell>> deps;

    void parse(const std::string& expr);
    void tokenize(const std::string& expr);

    std::string set_err(const std::string& err);
    std::string evaluate_node(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> node);
    std::string evaluate_binary_op(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> left,
                                   std::shared_ptr<Node> right, const std::function<int(int, int)>& op);

    std::shared_ptr<Node> parse_expression();
    std::shared_ptr<Node> parse_term();
    std::shared_ptr<Node> parse_factor();

    bool match(std::initializer_list<Token> types);
    const TokenData& advance();
    const TokenData& peek() const;
    const TokenData& previous() const;
    bool at_end() const;

    void calc_node_deps(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> node);
};
