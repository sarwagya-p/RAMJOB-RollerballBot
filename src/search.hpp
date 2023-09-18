#include "board.hpp"
#include "engine.hpp"
#include "neural_network.hpp"
#include <bits/stdc++.h>

typedef struct
{
    U16 move;
    double eval;
}move_eval;


class Node {
    public:
    Board* board_state;
    std::unordered_set<U16> legal_moves;

    // Node* parent_node;
    // std::vector<Node*> children;

    std::vector<move_eval> move_eval_arr; 

    Node(Board* board_state);

    void Order_Children();
};

NeuralNetwork* evaluator;
std::vector<double> board_to_dioble(Board* b);
U16 search_move(Board* b, bool training=false);
double MAX_VAL(Board* b);
double MIN_VAL(Board* b);