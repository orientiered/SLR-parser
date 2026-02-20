#include "AST.hpp"
#include <iostream>
#include <memory>
#include <ostream>

namespace AST {
    // initializing id for nodes
    std::size_t Node::next_id = 0;

    void BinOpNode::dump(std::ostream& os, DumpType type) {
        auto label = [&]() {
            switch(op) {
                case PLUS: return '+';
                case MINUS: return '-';
                case DIV: return '/';
                case MUL: return '*';
                default: return '?';
            }
        };

        switch(type) {
            case GRAPHVIZ:
            // dumping itself
            os << "  node" << id << " [label=\"" << label()
                << "\", shape=rectangle, style=filled, fillcolor=\"#e1f5ff\"];\n";

            // dumping children
            if (left) {
                left->dump(os, type);
                os << "  node" << id << " -> node" << left->id
                    << " [label=\"left\"];\n";
            }
            if (right) {
                right->dump(os, type);
                os << "  node" << id << " -> node" << right->id
                    << " [label=\"right\"];\n";
            }

            break;

            case SERIALIZE:
            os << "(BINOP:" << label();
            if (left) left->dump(os, type);
            if (right) right->dump(os, type);
            os << ")";
            break;
            default:
            std::cerr << "Can't dump BinOp to this type\n";
            break;
        }
    }

    void IdNode::dump(std::ostream& os, DumpType type) {
        switch(type) {
            case GRAPHVIZ:
            os << "  node" << id << " [label=\"" << "ID:" << id_name
                << "\", shape=rectangle, style=filled, fillcolor=\"#f1e364\"];\n";
            break;
            case SERIALIZE:
            os << "(ID:" << id_name << ")";
            break;
            default:
            std::cerr << "Can't dump IdNode to this type\n";
            break;
        }
    }

    void NumNode::dump(std::ostream& os, DumpType type) {
        switch(type) {
            case GRAPHVIZ:
            os << "  node" << id << " [label=\"" << "NUM:" << num
                << "\", shape=rectangle, style=filled, fillcolor=\"#38d878\"];\n";
            break;
            case SERIALIZE:
            os << "(NUM:" << num << ")";
            break;
            default:
            std::cerr << "Can't dump NumNode to this type\n";
            break;
        }
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


    void dumpTreeAsGraphviz(const NodePtr& root, std::ostream& os) {
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

        root->dump(os, GRAPHVIZ);

        os << "}\n";
    }

    void dumpTreeAsString(const NodePtr& root, std::ostream& os) {
        if (!root) {
            os << "<EMPTY_TREE>";
            return;
        }

        root->dump(os, SERIALIZE);
    }

};

