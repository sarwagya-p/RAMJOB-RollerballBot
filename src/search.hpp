#pragma once
#include <memory>

#include "evaluation_func.hpp"
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
    std::shared_ptr<EvaluationFunc> evaluator;
    // Node* parent_node;
    // std::vector<Node*> children;

    std::vector<move_eval> move_eval_arr; 

    Node(std::shared_ptr<Board> board_state, std::shared_ptr<EvaluationFunc> evaluator);
    
    
    void Order_Children(std::atomic<bool>& search, bool reverse = false);
    double score();
};
double MAX_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<EvaluationFunc> evaluator);

double MIN_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<EvaluationFunc> evaluator);

void search_move(std::shared_ptr<Board> b, std::atomic<bool>& search, std::atomic<U16>& best_move, 
    bool training, std::shared_ptr<EvaluationFunc> evaluator);

double get_margin_score(std::shared_ptr<Board> board_state);
void undo_last_move(std::shared_ptr<Board> b, U16 move);