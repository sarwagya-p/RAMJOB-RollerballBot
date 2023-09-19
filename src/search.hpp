#pragma once

#include "neural_network.hpp"
#include "board.hpp"
#include "engine.hpp"

// #include <algorithm>

typedef struct move_eval
{
    U16 movement;
    double eval;
}move_eval;


class Node {
public:
    Board* board_state;
    int num_moves;
    std::unordered_set<U16> legal_moves;
    NeuralNetwork* evaluator;
    // Node* parent_node;
    // std::vector<Node*> children;

    std::vector<move_eval> move_eval_arr; 

    Node(Board* board_state, NeuralNetwork* evaluator);
    
    double MAX_VAL(Board* b, double alpha, double beta, int i, int cutoff);
    double MIN_VAL(Board* b, double alpha, double beta, int i, int cutoff);
    void Order_Children();
    double score();
};

void search_move(Board* b, std::atomic<bool>& search, std::atomic<U16>& best_move, bool training, NeuralNetwork* evaluator);
std::vector<double> board_to_dioble(Board* b);
