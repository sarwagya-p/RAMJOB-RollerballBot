#include "search.hpp"
#include <float.h>

struct CompareMoveEval{
    bool operator()(const move_eval mve1, const move_eval mve2)
    {
        return (mve1.eval > mve2.eval);
    }
};

Node::Node(Board* board_state) 
    :board_state(board_state), num_moves(0)
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

double Node::score()
{
    bool win, draw, lose;
    
    win = board_state->in_check() && (board_state->data.player_to_play >> 6);
    draw = !board_state->in_check();

    lose = ~draw & ~win;
    double margin_score = 0;
    margin_score -= (board_state->data.b_rook_ws != DEAD)*3;
    margin_score -= (board_state->data.b_rook_bs != DEAD)*3;
    margin_score -= (board_state->data.b_bishop != DEAD)*5;
    margin_score -= (board_state->data.b_pawn_ws != DEAD);
    margin_score -= (board_state->data.b_pawn_bs != DEAD);

    margin_score += (board_state->data.w_rook_ws != DEAD)*3;
    margin_score += (board_state->data.w_rook_bs != DEAD)*3;
    margin_score += (board_state->data.w_bishop != DEAD)*5;
    margin_score += (board_state->data.w_pawn_ws != DEAD);
    margin_score += (board_state->data.w_pawn_bs != DEAD);

    return margin_score + win * 100 + (lose - win) * (5 * (num_moves/20) + std::min(10, num_moves)) + 40*draw;
}

//ADVERSARIAL SEARCH
void search_move(Board* b, std::atomic<bool>& search, std::atomic<U16>& best_move, bool training=false)
{
    double alpha = -DBL_MAX;
    double beta = DBL_MAX;
    int cutoff = 1;
    move_eval optimum;
    if (b->data.player_to_play == WHITE)
    {
        while (search)
        {
            Node* maxnode = new Node(b);
            if (maxnode->legal_moves.empty())
            {
                return;
            }
            maxnode->Order_Children();
            // double maxmove;
            double d;
            b->do_move(maxnode->move_eval_arr[0].move);
            d = MIN_VAL(b, alpha, beta, 0, cutoff);
            alpha = std::max(alpha, d);
            optimum.eval = d;
            optimum.move = maxnode->move_eval_arr[0].move;
            b->undo_last_move(maxnode->move_eval_arr[0].move);
            for(int j = 1; j < maxnode->move_eval_arr.size(); j++)
            {
                b->do_move(maxnode->move_eval_arr[j].move);
                d = MIN_VAL(b, alpha, beta, 0, cutoff);
                alpha = std::max(alpha, d);
                
                if (optimum.eval < d)
                {
                    optimum.eval = d;
                    optimum.move = maxnode->move_eval_arr[j].move;
                }
                b->undo_last_move(maxnode->move_eval_arr[j].move);
            }
            best_move = optimum.move;
            ++cutoff;

            // return optimum.move;
        }
        
        // return optimum.move;
    }
    else
    {
        while (search)
        {
            Node* minnode = new Node(b);
            if (minnode->legal_moves.empty())
            {
                return;
            }
            minnode->Order_Children();
            // double maxmove;
            double d;
            b->do_move(minnode->move_eval_arr[-1].move);
            d = MAX_VAL(b, alpha, beta, 0, cutoff);
            beta = std::min(beta, d);
            optimum.eval = d;
            optimum.move = minnode->move_eval_arr[-1].move;
            b->undo_last_move(minnode->move_eval_arr[-1].move);
            for(int j = 2; j < minnode->move_eval_arr.size()+1; j++)
            {
                b->do_move(minnode->move_eval_arr[-j].move);
                d = MAX_VAL(b, alpha, beta, 0, cutoff);
                beta = std::min(beta, d);
                
                if (optimum.eval > d)
                {
                    optimum.eval = d;
                    optimum.move = minnode->move_eval_arr[-j].move;
                }
                b->undo_last_move(minnode->move_eval_arr[-j].move);
            }
            ++cutoff;
            best_move = optimum.move;
            // return optimum.move;
        }
        // return optimum.move;
    }
    if (training)
    {
        evaluator->update(board_to_dioble(b), optimum.eval);
    }
    return;
}

double MAX_VAL(Board* b, double alpha, double beta, int i, int cutoff)
{
    Node* maxnode = new Node(b);
    if (maxnode->legal_moves.empty())
    {
        return maxnode->score();
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
        return minnode->score();
    }
    if (i == cutoff)
    {
        return evaluator->evaluate(board_to_dioble(b));
    }
    minnode->Order_Children();
    double minmove;
    double d;
    b->do_move(minnode->move_eval_arr[-1].move);
    d = MAX_VAL(b, alpha, beta, i+1, cutoff);
    beta = std::min(beta, d);
    if (alpha>=beta)
    {
        return d;
    }    
    minmove = d;
    b->undo_last_move(minnode->move_eval_arr[-1].move);
    for(int j = 2; j < minnode->move_eval_arr.size()+1; j++)
    {
        b->do_move(minnode->move_eval_arr[-j].move);
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
        b->undo_last_move(minnode->move_eval_arr[-j].move);
    }
    return minmove;  
}