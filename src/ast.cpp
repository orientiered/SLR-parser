#include "AST.hpp"
#include <memory>
#include <ostream>

namespace AST {
    // initializing id for nodes
    std::size_t Node::next_id = 0;

    void BinOpNode::dump(std::ostream& os) {
        auto label = [&]() {
            switch(op) {
                case PLUS: return '+';
                case MINUS: return '-';
                case DIV: return '/';
                case MUL: return '*';
                default: return '?';
            }
        };

        // dumping itself
        os << "  node" << id << " [label=\"" << label()
            << "\", shape=rectangle, style=filled, fillcolor=\"#e1f5ff\"];\n";

        // dumping children
        if (left) {
            left->dump(os);
            os << "  node" << id << " -> node" << left->id
                << " [label=\"left\"];\n";
        }
        if (right) {
            right->dump(os);
            os << "  node" << id << " -> node" << right->id
                << " [label=\"right\"];\n";
        }
    }

    void IdNode::dump(std::ostream& os) {
        os << "  node" << id << " [label=\"" << "ID:" << id_name
            << "\", shape=rectangle, style=filled, fillcolor=\"#f1e364\"];\n";
    }

    void NumNode::dump(std::ostream& os) {
        os << "  node" << id << " [label=\"" << "NUM:" << num
            << "\", shape=rectangle, style=filled, fillcolor=\"#38d878\"];\n";
    }

    NodePtr makeBinOp(NodePtr left, Operator op, NodePtr right) {
        auto node = std::make_shared<BinOpNode>();
        node->left = left; node->right = right;
        node->op = op;

        if (left) left->parent = node;
        if (right) right->parent = node;

        return node;
    }

    NodePtr makeId(const std::string& name) {
        auto node = std::make_shared<IdNode>();
        node->id_name = name;
        return node;
    }

    NodePtr makeNum(int value) {
        auto node = std::make_shared<NumNode>();
        node->num = value;
        return node;
    }


    void dumpTree(const NodePtr& root, std::ostream& os) {
        if (!root) {
            os << "digraph AST {\n";
            os << "  label=\"Empty tree\";\n";
            os << "}\n";
            return;
        }

        os << "digraph AST {\n";
        os << "  rankdir=TB;\n";  // Top to Bottom
        os << "  node [fontname=\"Courier\", fontsize=10];\n";
        os << "  edge [fontname=\"Courier\", fontsize=8];\n";
        os << "\n";

        root->dump(os);

        os << "}\n";
    }
};

