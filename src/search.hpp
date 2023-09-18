#include "board.hpp"
#include "engine.hpp"
#include "neural_network.hpp"

class Node {
    Board* board_state;
    std::unordered_set<U16> legal_moves;

    Node* parent_node;
    std::vector<Node*> children;

    NeuralNetwork* evaluator;
};

U16 search_move(Board* b, bool training=false);