#pragma once

#include <memory>
#include <ostream>
namespace AST {

    struct Node;
    using NodePtr = std::shared_ptr<Node>;
    using WeakNodePtr = std::weak_ptr<Node>;

    struct Node {
        std::size_t id;
        WeakNodePtr parent;

    protected:
        Node(): id(++next_id) {}

    public:
        virtual void dump(std::ostream& os) = 0;
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

        void dump(std::ostream& os) override;

        ~BinOpNode() override = default;
    };

    NodePtr makeBinOp(NodePtr left, Operator op, NodePtr right);

    struct IdNode final : Node {
        std::string id_name;

        void dump(std::ostream& os) override;
        ~IdNode() override = default;
    };

    NodePtr makeId(const std::string& name);

    struct NumNode final: Node {
        int num;

        void dump(std::ostream& os) override;

        ~NumNode() override = default;
    };

    NodePtr makeNum(int value);

    void dumpTree(const NodePtr& root, std::ostream& os);
};
