#include "search.hpp"
#include <float.h>

struct CompareMoveEval{
    bool operator()(const move_eval mve1, const move_eval mve2)
    {
        return (mve1.eval > mve2.eval);
    }
};

Node::Node(Board* board_state) 
    :board_state(board_state)
    {
        legal_moves = board_state->get_legal_moves();
    }

void Node::Order_Children()
{
    move_eval temp;
    for (U16 move: legal_moves)
    {
        temp.move = move;
        temp.eval = evaluator->evaluate(board_to_dioble(board_state));

        move_eval_arr.push_back(temp);
    }
    std::sort(move_eval_arr.begin(), move_eval_arr.end(), CompareMoveEval());
}

std::vector<double> board_to_dioble(Board* b)
{
    std::vector<double> dio = std::vector<double>(12);
    dio[0] = (double)b->data.b_rook_ws;
    dio[1] = (double)b->data.b_rook_bs;
    dio[2] = (double)b->data.b_king;
    dio[3] = (double)b->data.b_bishop;
    dio[4] = (double)b->data.b_pawn_ws;
    dio[5] = (double)b->data.b_pawn_bs;

    dio[6] = (double)b->data.w_rook_ws;
    dio[7] = (double)b->data.w_rook_bs;
    dio[8] = (double)b->data.w_king;
    dio[9] = (double)b->data.w_bishop;
    dio[10] = (double)b->data.w_pawn_ws;
    dio[11] = (double)b->data.w_pawn_bs;

    return dio;
}


//ADVERSARIAL SEARCH
U16 search_move(Board* b, bool training=false)
{
    double alpha = -DBL_MAX;
    double beta = DBL_MAX;
}

double MAX_VAL(Board* b, double alpha, double beta, int i, int cutoff)
{
    Node* maxnode = new Node(b);
    if (maxnode->legal_moves.empty())
    {
        return score(b);
    }
    if (i == cutoff)
    {
        return evaluator->evaluate(board_to_dioble(b));
    }
    maxnode->Order_Children();
    double maxmove;
    double d;
    b->do_move(maxnode->move_eval_arr[0].move);
    d = MIN_VAL(b, alpha, beta, i+1, cutoff);
    alpha = std::max(alpha, d);
    if (alpha>=beta)
    {
        return d;
    }    
    maxmove = d;
    b->undo_last_move(maxnode->move_eval_arr[0].move);
    for(int j = 1; j < maxnode->move_eval_arr.size(); j++)
    {
        b->do_move(maxnode->move_eval_arr[j].move);
        d = MIN_VAL(b, alpha, beta, i+1, cutoff);
        alpha = std::max(alpha, d);
        if (alpha>=beta)
        {
            return d;
        }
        if (maxmove < d)
        {
            maxmove = d;
        }
        b->undo_last_move(maxnode->move_eval_arr[j].move);
    }
    return maxmove;  
}

double MIN_VAL(Board* b, double alpha, double beta, int i, int cutoff)
{
    Node* minnode = new Node(b);
    if (minnode->legal_moves.empty())
    {
        return score(b);
    }
    if (i == cutoff)
    {
        return evaluator->evaluate(board_to_dioble(b));
    }
    minnode->Order_Children();
    double minmove;
    double d;
    b->do_move(minnode->move_eval_arr[0].move);
    d = MAX_VAL(b, alpha, beta, i+1, cutoff);
    beta = std::min(beta, d);
    if (alpha>=beta)
    {
        return d;
    }    
    minmove = d;
    b->undo_last_move(minnode->move_eval_arr[0].move);
    for(int j = 1; j < minnode->move_eval_arr.size(); j++)
    {
        b->do_move(minnode->move_eval_arr[j].move);
        d = MAX_VAL(b, alpha, beta, i+1, cutoff);
        beta = std::min(beta, d);
        if (alpha>=beta)
        {
            return d;
        }
        if (minmove > d)
        {
            minmove = d;
        }
        b->undo_last_move(minnode->move_eval_arr[j].move);
    }
    return minmove;  
}