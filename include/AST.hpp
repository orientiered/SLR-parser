#pragma once

#include <memory>
#include <ostream>
namespace AST {

    enum DumpType {GRAPHVIZ = 0, SERIALIZE};
    struct Node;
    using NodePtr = std::shared_ptr<Node>;
    using WeakNodePtr = std::weak_ptr<Node>;

    struct Node {
        std::size_t id;
        WeakNodePtr parent;

    protected:
        Node(): id(++next_id) {}

    public:

        virtual void dump(std::ostream& os, DumpType type = GRAPHVIZ) = 0;
        virtual ~Node() = default;

    private:
        static std::size_t next_id;
    };


    enum Operator {
        PLUS, MINUS, MUL, DIV
    };

    struct BinOpNode final : Node {
        NodePtr left;
        NodePtr right;
        Operator op;

        void dump(std::ostream& os, DumpType type = GRAPHVIZ) override;

        ~BinOpNode() override = default;
    };

    NodePtr makeBinOp(NodePtr left, Operator op, NodePtr right);

    struct IdNode final : Node {
        std::string id_name;

        void dump(std::ostream& os, DumpType type = GRAPHVIZ) override;
        ~IdNode() override = default;
    };

    NodePtr makeId(const std::string& name);

    struct NumNode final: Node {
        int num;

        void dump(std::ostream& os, DumpType type = GRAPHVIZ) override;

        ~NumNode() override = default;
    };

    NodePtr makeNum(int value);

    void dumpTreeAsGraphviz(const NodePtr& root, std::ostream& os);
    void dumpTreeAsString(const NodePtr& root, std::ostream& os);
};
