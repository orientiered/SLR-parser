#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "lexer.hpp"
#include "AST.hpp"
#include "syntax_analyzer.hpp"

struct CLIOptions {
    bool show_help = false;
    bool export_table = false;
    std::string table_file;
    std::string input_file;
    std::string input_string;
    std::string dot_file;
    std::string svg_file;
    bool interactive = true;  // input from stdin by default
};

CLIOptions parse_args(int argc, char* argv[]) {
    CLIOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return opts;
        }
        else if (arg == "--export-table") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Error: --export-table requires a filename argument");
            }
            opts.export_table = true;
            opts.table_file = argv[++i];
            opts.interactive = false;
        }
        else if (arg == "-f" || arg == "--file") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Error: -f/--file requires a filename argument");
            }
            opts.input_file = argv[++i];
            opts.interactive = false;
        }
        else if (arg == "-s" || arg == "--string") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Error: -s/--string requires an expression argument");
            }
            opts.input_string = argv[++i];
            opts.interactive = false;
        }
        else if (arg == "--dot") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Error: --dot requires a filename argument");
            }
            opts.dot_file = argv[++i];
        }
        else if (arg == "--svg") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Error: --svg requires a filename argument");
            }
            opts.svg_file = argv[++i];
        }
        else {
            throw std::runtime_error("Error: Unknown argument '" + arg + "'. Use -h for help.");
        }
    }

    return opts;
}

void show_help() {
    std::cout << R"(SLR Parser for Mathematical Expressions

Usage: parser [OPTIONS]

Options:
  -h, --help                Show this help message and exit
  -f FILE, --file FILE      Parse expressions from FILE (one per line)
  -s EXPR, --string EXPR    Parse single expression EXPR
  --export-table FILE       Export SLR action/goto tables to CSV FILE
  --dot FILE                Save AST to Graphviz DOT FILE after parsing
  --svg FILE                Save AST to SVG FILE (requires 'dot' utility)

Examples:
  parser -s "2 + 3 * 4"                # Parse single expression
  parser -f expressions.txt            # Parse multiple expressions from file
  parser --export-table tables.csv     # Export parser tables
  parser -s "a + b" --dot ast.dot      # Parse and save AST to DOT
  parser -s "1+2*3" --svg tree.svg     # Parse and generate SVG directly

Interactive mode:
  parser                               # Read expressions from stdin (Ctrl+D to exit)
)";
}

// Парсинг одного выражения с обработкой ошибок
bool parse_expression(SyntaxAnalyzer& parser, const std::string& expr) {
    try {
        auto status = parser.parse(expr);

        if (status == SyntaxAnalyzer::ParseStatus::SUCCESS) {
            AST::dumpTreeAsString(parser.get_root(), std::cout);
            std::cout << "\n";
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception when parsing " << expr << ": " << e.what() << "\n";
        return false;
    }
}

// Сохранение AST в формате Graphviz
bool save_ast_dot(const AST::NodePtr& root, const std::string& filename) {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file '" << filename << "' for writing\n";
            return false;
        }
        AST::dumpTreeAsGraphviz(root, file);
        std::cout << "AST saved to: " << filename << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving AST to '" << filename << "': " << e.what() << "\n";
        return false;
    }
}

// Generating svg from dot
bool generate_svg(const std::string& dot_file, const std::string& svg_file) {
    std::string cmd = "dot -Tsvg " + dot_file + " -o " + svg_file;
    int result = std::system(cmd.c_str());

    if (result == 0) {
        std::cout << "SVG generated in file: " << svg_file << "\n";
        return true;
    } else {
        std::cerr << "Error: Failed to generate SVG (dot command returned "
                  << result << ")\n";
        std::cerr << "  Hint: Make sure Graphviz is installed and 'dot' is in PATH\n";
        return false;
    }
}

int main(int argc, char* argv[]) {
    try {
        // Args parsing
        CLIOptions opts;
        try {
            opts = parse_args(argc, argv);
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n\n";
            std::cerr << "Use -h or --help for usage information.\n";
            return EXIT_FAILURE;
        }

        // Help msg
        if (opts.show_help) {
            show_help();
            return EXIT_SUCCESS;
        }

        // Initialization
        SyntaxAnalyzer parser;
        int init_error = parser.init();
        if (init_error) {
            std::cerr << "Parser initialization error (code: " << init_error << ")\n";
            return EXIT_FAILURE;
        }

        // Export first, follow and action/goto tables
        if (opts.export_table) {
            std::cout << "Exporting SLR tables to: " << opts.table_file << "\n";
            parser.dump_tables(opts.table_file);
            return EXIT_SUCCESS;
        }

        // Parse from string
        if (!opts.input_string.empty()) {
            if (!parse_expression(parser, opts.input_string)) {
                return EXIT_FAILURE;
            }
        }
        // Parse from file
        else if (!opts.input_file.empty()) {
            if (parser.parse_file(opts.input_file) != SyntaxAnalyzer::ParseStatus::SUCCESS) {
                return EXIT_FAILURE;
            }
        }

        // Ineractive mode
        else {
            std::cout << "SLR Parser (interactive mode)\n";
            std::cout << "Enter expressions (empty line or Ctrl+D to exit):\n\n";

            std::string expr;
            while (std::getline(std::cin, expr)) {
                if (expr.empty()) break;

                parse_expression(parser, expr);
            }
            std::cout << "\nExiting interactive mode.\n";
        }

        // save to DOT
        if (!opts.dot_file.empty()) {
            if (!save_ast_dot(parser.get_root(), opts.dot_file)) {
                return EXIT_FAILURE;
            }
        }

        // Generate SVG
        if (!opts.svg_file.empty()) {
            // Если не указан DOT-файл, используем временный
            std::string dot_file = opts.dot_file.empty() ? "temp_ast.dot" : opts.dot_file;

            if (opts.dot_file.empty()) {
                if (!save_ast_dot(parser.get_root(), dot_file)) {
                    return EXIT_FAILURE;
                }
            }

            generate_svg(dot_file, opts.svg_file);

            if (opts.dot_file.empty()) {
                std::remove(dot_file.c_str());
            }
        }

        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
