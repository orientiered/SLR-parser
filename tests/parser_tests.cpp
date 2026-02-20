#include "gtest/gtest.h"
#include <sstream>
#include <utility>
#include "AST.hpp"
#include "syntax_analyzer.hpp"


TEST(ParserInterface, Init) {
    SyntaxAnalyzer parser;

    EXPECT_EQ(0, parser.init());
};

using SerializedAST = std::string;
using ParseStatus = SyntaxAnalyzer::ParseStatus;

struct ParserTestCase {
    std::string input;
    SerializedAST out;
    ParseStatus status = ParseStatus::SUCCESS;
};

class ParserTest : public ::testing::Test {
protected:
    SyntaxAnalyzer parser;

    void SetUp() override {
        parser.init();
    }

    std::pair<ParseStatus, std::string> parse_serialized(const std::string& expr) {
        ParseStatus status = parser.parse(expr);

        std::ostringstream out;
        AST::dumpTreeAsString(parser.get_root(), out);

        return std::make_pair(status, out.str());
    }

};


TEST_F(ParserTest, Basics) {

    std::vector<ParserTestCase> cases = {
        {"135"      , "(NUM:135)"                   },
        {"x"        , "(ID:x)"                      },
        {"17+19"    , "(BINOP:+(NUM:17)(NUM:19))"   },
        {"id*5"     , "(BINOP:*(ID:id)(NUM:5))"     },
        {"0/yy"     , "(BINOP:/(NUM:0)(ID:yy))"     },
        {"18-xyz"   , "(BINOP:-(NUM:18)(ID:xyz))"   },
    };

    for (auto& test: cases) {
        auto [status, tree] = parse_serialized(test.input);

        EXPECT_EQ(test.status, status);
        EXPECT_EQ(test.out, tree);
    }
}

TEST_F(ParserTest, Priority) {
    std::vector<ParserTestCase> cases = {
        {"1+x*y/2+4", "(BINOP:+(BINOP:+(NUM:1)(BINOP:/(BINOP:*(ID:x)(ID:y))(NUM:2)))(NUM:4))"},
        {"1-x/y*2+4", "(BINOP:+(BINOP:-(NUM:1)(BINOP:*(BINOP:/(ID:x)(ID:y))(NUM:2)))(NUM:4))"},
        {"1-2+3-4", "(BINOP:-(BINOP:+(BINOP:-(NUM:1)(NUM:2))(NUM:3))(NUM:4))"},
        {"x*y-4/3", "(BINOP:-(BINOP:*(ID:x)(ID:y))(BINOP:/(NUM:4)(NUM:3)))"},
        {"a*b/c*d", "(BINOP:*(BINOP:/(BINOP:*(ID:a)(ID:b))(ID:c))(ID:d))"}
    };

    for (auto& test: cases) {
        auto [status, tree] = parse_serialized(test.input);

        EXPECT_EQ(test.status, status);
        EXPECT_EQ(test.out, tree);
    }
}

TEST_F(ParserTest, Parentheses) {

    std::vector<ParserTestCase> cases = {
        {"1+y*z", "(BINOP:+(NUM:1)(BINOP:*(ID:y)(ID:z)))"},
        {"(1+y)*z", "(BINOP:*(BINOP:+(NUM:1)(ID:y))(ID:z))"},
        {"1+((x)*z)", "(BINOP:+(NUM:1)(BINOP:*(ID:x)(ID:z)))"},
        {"(((x)))+(y/(43-x))", "(BINOP:+(ID:x)(BINOP:/(ID:y)(BINOP:-(NUM:43)(ID:x))))"},
        {"(x", "<EMPTY_TREE>", ParseStatus::SYNTAX_ERR},
        {"1+()", "<EMPTY_TREE>", ParseStatus::SYNTAX_ERR}
    };

    for (auto& test: cases) {
        auto [status, tree] = parse_serialized(test.input);

        EXPECT_EQ(test.status, status);
        EXPECT_EQ(test.out, tree);
    }
}

TEST_F(ParserTest, LexerTests) {
    std::vector<ParserTestCase> cases = {
        {"abcdefghijklmnopqrstuvwxyz", "(ID:abcdefghijklmnopqrstuvwxyz)"},
        {"ABCDEFGHIJKLMNOPQRSTUVYXYZ", "(ID:ABCDEFGHIJKLMNOPQRSTUVYXYZ)"},
        {"1234567890", "(NUM:1234567890)"},
        {"0", "(NUM:0)"},
        {"0123", "(NUM:123)"}, // numbers starting with leading zero are accepted
        {"var_1", "<EMPTY_TREE>", ParseStatus::LEXICAL_ERR},
        {"3^9", "<EMPTY_TREE>", ParseStatus::LEXICAL_ERR},
        {"xy~4", "<EMPTY_TREE>", ParseStatus::LEXICAL_ERR},
        {"x=y+5", "<EMPTY_TREE>", ParseStatus::LEXICAL_ERR},
    };

    for (auto& test: cases) {
        auto [status, tree] = parse_serialized(test.input);

        EXPECT_EQ(test.status, status);
        EXPECT_EQ(test.out, tree);
    }
}


TEST_F(ParserTest, Limits) {
    std::vector<ParserTestCase> cases = {
    // 300 nested parentheses
    {"((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))", "(ID:x)"},
    // 300 nested parentheses with 2 closing missing
    {"((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))", "<EMPTY_TREE>", ParseStatus::SYNTAX_ERR}

    };

    for (auto& test: cases) {
        auto [status, tree] = parse_serialized(test.input);

        EXPECT_EQ(test.status, status);
        EXPECT_EQ(test.out, tree);
    }
}

TEST_F(ParserTest, SyntaxErrors) {
    std::vector<ParserTestCase> cases = {
        {"x++3", "<EMPTY_TREE>", ParseStatus::SYNTAX_ERR},
        {"+4", "<EMPTY_TREE>", ParseStatus::SYNTAX_ERR},
        {"-52", "<EMPTY_TREE>", ParseStatus::SYNTAX_ERR},
        {"+-*/*", "<EMPTY_TREE>", ParseStatus::SYNTAX_ERR}
    };

    for (auto& test: cases) {
        auto [status, tree] = parse_serialized(test.input);

        EXPECT_EQ(test.status, status);
        EXPECT_EQ(test.out, tree);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
