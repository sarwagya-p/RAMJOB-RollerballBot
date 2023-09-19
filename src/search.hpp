#pragma once

#include "board.hpp"
#include "engine.hpp"
#include "neural_network.hpp"
#include <algorithm>

typedef struct move_eval
{
    U16 move;
    double eval;
}move_eval;


class Node {
public:
    Board* board_state;
    int num_moves;
    std::unordered_set<U16> legal_moves;

    // Node* parent_node;
    // std::vector<Node*> children;

    std::vector<move_eval> move_eval_arr; 

    Node(Board* board_state);

    void Order_Children();
    double score();
};

NeuralNetwork* evaluator;
std::vector<double> board_to_dioble(Board* b);
void search_move(Board* b, std::atomic<bool>& search, std::atomic<U16>& best_move, bool training=false);
double MAX_VAL(Board* b, double alpha, double beta, int i, int cutoff);
double MIN_VAL(Board* b, double alpha, double beta, int i, int cutoff);