#include "core/math/MathExpressionEvaluator.h"
#include <vector>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace catchim::core {

namespace {

enum class TokenType {
    Number,
    Plus,
    Minus,
    Multiply,
    Divide,
    LParen,
    RParen
};

struct Token {
    TokenType type;
    double value{0.0};
};

bool tokenize(std::string_view input, std::vector<Token>& tokens) {
    size_t i = 0;
    const size_t n = input.size();

    while (i < n) {
        char c = input[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            size_t start = i;
            bool hasDot = (c == '.');
            i++;
            while (i < n && (std::isdigit(static_cast<unsigned char>(input[i])) || input[i] == '.')) {
                if (input[i] == '.') {
                    if (hasDot) return false; // Multiple dots in one number
                    hasDot = true;
                }
                i++;
            }
            std::string numStr(input.substr(start, i - start));
            try {
                double val = std::stod(numStr);
                tokens.push_back({TokenType::Number, val});
            } catch (...) {
                return false;
            }
            continue;
        }

        switch (c) {
            case '+': tokens.push_back({TokenType::Plus, 0.0}); break;
            case '-': tokens.push_back({TokenType::Minus, 0.0}); break;
            case '*': tokens.push_back({TokenType::Multiply, 0.0}); break;
            case '/': tokens.push_back({TokenType::Divide, 0.0}); break;
            case '(': tokens.push_back({TokenType::LParen, 0.0}); break;
            case ')': tokens.push_back({TokenType::RParen, 0.0}); break;
            default: return false; // Invalid character
        }
        i++;
    }
    return !tokens.empty();
}

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

    std::optional<double> parse() {
        if (tokens_.empty()) return std::nullopt;
        index_ = 0;
        auto res = parseExpression();
        if (!res.has_value() || index_ != tokens_.size()) {
            return std::nullopt;
        }
        if (!std::isfinite(*res)) return std::nullopt;
        return res;
    }

private:
    const Token* peek() const {
        if (index_ < tokens_.size()) return &tokens_[index_];
        return nullptr;
    }

    const Token* consume() {
        if (index_ < tokens_.size()) return &tokens_[index_++];
        return nullptr;
    }

    std::optional<double> parseExpression() {
        auto left = parseTerm();
        if (!left.has_value()) return std::nullopt;

        while (true) {
            const auto* t = peek();
            if (t && t->type == TokenType::Plus) {
                consume();
                auto right = parseTerm();
                if (!right.has_value()) return std::nullopt;
                *left += *right;
            } else if (t && t->type == TokenType::Minus) {
                consume();
                auto right = parseTerm();
                if (!right.has_value()) return std::nullopt;
                *left -= *right;
            } else {
                break;
            }
        }
        return left;
    }

    std::optional<double> parseTerm() {
        auto left = parseFactor();
        if (!left.has_value()) return std::nullopt;

        while (true) {
            const auto* t = peek();
            if (t && t->type == TokenType::Multiply) {
                consume();
                auto right = parseFactor();
                if (!right.has_value()) return std::nullopt;
                *left *= *right;
            } else if (t && t->type == TokenType::Divide) {
                consume();
                auto right = parseFactor();
                if (!right.has_value() || std::abs(*right) < 1e-12) return std::nullopt; // Div by zero
                *left /= *right;
            } else {
                break;
            }
        }
        return left;
    }

    std::optional<double> parseFactor() {
        const auto* t = peek();
        if (!t) return std::nullopt;

        if (t->type == TokenType::Number) {
            consume();
            return t->value;
        }

        if (t->type == TokenType::Minus) {
            consume();
            auto val = parseFactor();
            if (!val.has_value()) return std::nullopt;
            return -(*val);
        }

        if (t->type == TokenType::LParen) {
            consume();
            auto val = parseExpression();
            if (!val.has_value()) return std::nullopt;
            const auto* closeParen = peek();
            if (!closeParen || closeParen->type != TokenType::RParen) {
                return std::nullopt;
            }
            consume();
            return val;
        }

        return std::nullopt;
    }

    const std::vector<Token>& tokens_;
    size_t index_{0};
};

} // namespace

std::optional<double> MathExpressionEvaluator::evaluateMathExpression(std::string_view input) {
    std::vector<Token> tokens;
    if (!tokenize(input, tokens)) {
        return std::nullopt;
    }
    Parser parser(tokens);
    return parser.parse();
}

double MathExpressionEvaluator::snapToStep(double value, double step) noexcept {
    if (step <= 0.0 || !std::isfinite(step) || !std::isfinite(value)) {
        return value;
    }
    double snapped = std::round(value / step) * step;
    return snapped;
}

std::string MathExpressionEvaluator::formatNumberForDisplay(
    double value,
    int minFractionDigits,
    int maxFractionDigits
) {
    if (!std::isfinite(value)) {
        return "0";
    }

    int maxDigits = std::clamp(maxFractionDigits, 0, 10);
    int minDigits = std::clamp(minFractionDigits, 0, maxDigits);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(maxDigits) << value;
    std::string s = ss.str();

    if (maxDigits > 0) {
        auto dotPos = s.find('.');
        if (dotPos != std::string::npos) {
            // Trim zeros back to minDigits
            size_t targetLen = dotPos + 1 + static_cast<size_t>(minDigits);
            while (s.size() > targetLen && s.back() == '0') {
                s.pop_back();
            }
            if (minDigits == 0 && s.back() == '.') {
                s.pop_back();
            }
        }
    }

    if (s == "-0" || s == "-0.0") return "0";
    return s;
}

bool MathExpressionEvaluator::isNearlyEqual(double a, double b, double epsilon) noexcept {
    return std::abs(a - b) <= epsilon;
}

} // namespace catchim::core
