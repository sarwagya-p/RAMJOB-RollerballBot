#pragma once
#include <memory>

#include "neural_network.hpp"
#include "board.hpp"
#include "engine.hpp"

// #include <algorithm>

typedef struct
{
    U16 movement;
    double eval;
}move_eval;


class Node {
public:
    std::shared_ptr<Board> board_state;
    int num_moves;
    std::unordered_set<U16> legal_moves;
    std::shared_ptr<NeuralNetwork> evaluator;
    // Node* parent_node;
    // std::vector<Node*> children;

    std::vector<move_eval> move_eval_arr; 

    Node(std::shared_ptr<Board> board_state, std::shared_ptr<NeuralNetwork> evaluator);
    
    
    void Order_Children();
    double score();
};
double MAX_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<NeuralNetwork> evaluator);

double MIN_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<NeuralNetwork> evaluator);

void search_move(std::shared_ptr<Board> b, std::atomic<bool>& search, std::atomic<U16>& best_move, 
    bool training, std::shared_ptr<NeuralNetwork> evaluator);

std::vector<double> board_to_dioble(std::shared_ptr<Board> b);
double get_margin_score(std::shared_ptr<Board> board_state);
