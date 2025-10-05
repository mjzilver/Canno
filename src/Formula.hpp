#pragma once
#include <memory>
#include <string>
#include <vector>

class Sheet;  // forward declaration
class Cell;

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
        Number,   // float
        String,   // string
        CellRef,  // unique ptr
        Fumction,
        Add,
        Subtract,
        Multiply,
        Divide
    } type;
    std::string value;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    Node(Type t, const std::string& val): type(t), value(val) {}

    Node(Type t, const std::string& val, const std::shared_ptr<Node>& l, const std::shared_ptr<Node>& r)
        : type(t), value(val), left(l), right(r) {}
};

class Formula {
public:
    explicit Formula(const std::string& expr);

    std::string evaluate(std::shared_ptr<Sheet> sheet);

    std::vector<std::pair<int, int>> dependencies() const;

private:
    std::string err_msg = "";
    bool failed = false;
    size_t current = 0;

    std::shared_ptr<Node> root;
    std::vector<TokenData> tokens;
    std::vector<std::pair<int, int>> deps = {};

    void parse(const std::string& expr);
    void tokenize(const std::string& expr);

    std::string evaluate_node(std::shared_ptr<Sheet> sheet, std::shared_ptr<Node> node);

    std::shared_ptr<Node> parse_expression();
    std::shared_ptr<Node> parse_term();
    std::shared_ptr<Node> parse_factor();

    bool match(std::initializer_list<Token> types);
    const TokenData& advance();
    const TokenData& peek() const;
    const TokenData& previous() const;
    bool at_end() const;
};
