#include <algorithm>
#include <iostream>
#include "search.hpp"
#include <float.h>

struct CompareMoveEval{
    bool operator()(const move_eval mve1, const move_eval mve2)
    {
        return (mve1.eval > mve2.eval);
    }
};

Node::Node(Board* board_state, NeuralNetwork* evaluator) 
    :board_state(board_state), evaluator(evaluator)
    {
        num_moves = 0;
        legal_moves = board_state->get_legal_moves();
    }

void Node::Order_Children()
{
    move_eval temp;
    for (U16 test_move: legal_moves)
    {
        // std::cout << "Move: " << test_move << std::endl;
        temp.movement = test_move;
        board_state->do_move(test_move);
        temp.eval = evaluator->evaluate(board_to_dioble(board_state));
        board_state->undo_last_move(test_move);

        move_eval_arr.push_back(temp);
    }
    std::sort(move_eval_arr.begin(), move_eval_arr.end(), CompareMoveEval());

    // std::cout << "Move order: " << std::endl;
    // for (move_eval test_move: move_eval_arr){
    //     std::cout << test_move.movement << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "TIME TO GET ORDERRED NODE CHILDREN" << std::endl;
    // for (move_eval t: move_eval_arr)
    // {
    //    std::cout << "CHILD MOVE : " << t.movement << " VAL : " << t.eval << std::endl;
    // }
    // std::cout << "ALL CHILDREN DONE" << std::endl;

}

std::vector<double> board_to_dioble(Board* b)
{
    std::vector<double> dio = std::vector<double>(25);
    dio[0] = (double)getx(b->data.b_rook_ws);
    dio[2] = (double)getx(b->data.b_rook_bs);
    dio[4] = (double)getx(b->data.b_king);
    dio[6] = (double)getx(b->data.b_bishop);
    dio[8] = (double)getx(b->data.b_pawn_ws);
    dio[10] = (double)getx(b->data.b_pawn_bs);

    dio[12] = (double)getx(b->data.w_rook_ws);
    dio[14] = (double)getx(b->data.w_rook_bs);
    dio[16] = (double)getx(b->data.w_king);
    dio[18] = (double)getx(b->data.w_bishop);
    dio[20] = (double)getx(b->data.w_pawn_ws);
    dio[22] = (double)getx(b->data.w_pawn_bs);

    dio[1] = (double)gety(b->data.b_rook_ws);
    dio[3] = (double)gety(b->data.b_rook_bs);
    dio[5] = (double)gety(b->data.b_king);
    dio[7] = (double)gety(b->data.b_bishop);
    dio[9] = (double)gety(b->data.b_pawn_ws);
    dio[11] = (double)gety(b->data.b_pawn_bs);

    dio[13] = (double)gety(b->data.w_rook_ws);
    dio[15] = (double)gety(b->data.w_rook_bs);
    dio[17] = (double)gety(b->data.w_king);
    dio[19] = (double)gety(b->data.w_bishop);
    dio[21] = (double)gety(b->data.w_pawn_ws);
    dio[23] = (double)gety(b->data.w_pawn_bs);


    dio[24] = get_margin_score(b);

    return dio;
}

double get_margin_score(Board* board_state)
{
    double margin_score;
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
    return margin_score;
}

double Node::score()
{
    bool win, draw, lose;
    
    win = board_state->in_check() && (board_state->data.player_to_play >> 6);
    draw = !board_state->in_check();

    lose = !draw & !win;
    double margin_score = get_margin_score(board_state);
    

    return margin_score + win * 100 + (lose - win) * (5 * (num_moves/20) + std::min(10, num_moves)) + 40*draw;
}

//ADVERSARIAL SEARCH
void search_move(Board* b, std::atomic<bool>& search, std::atomic<U16>& best_move, bool training, NeuralNetwork* evaluator)
{
    
    int cutoff = 1;
    U8 last_killed_piece_temp;
    int last_killed_piece_idx_temp;
    move_eval optimum;
    if (b->data.player_to_play == WHITE)
    {
        // std::cout << "Checking white search" << std::endl;
        while (search)
        {
            // std::cout << "Cutoff: " << cutoff << std::endl;
            double alpha = -DBL_MAX;
            double beta = DBL_MAX;
            Node* maxnode = new Node(b, evaluator);
            std::cout << "LEGALS: " << std::endl;
            for (U16 test_move: maxnode->legal_moves){
                std::cout << test_move << " ";
            }
            std::cout << std::endl;
            // if (maxnode->legal_moves.empty())
            // {
            //     return;
            // }
            maxnode->Order_Children();
            // std::cout << "LEGALS OREDERRED: " << std::endl;
            // for (move_eval test_move: maxnode->move_eval_arr){
            //     std::cout << test_move.movement << " ";
            // }
            // std::cout << std::endl;
            // double maxmove;
            double d;
            b->do_move(maxnode->move_eval_arr[0].movement);
            last_killed_piece_temp = b->data.last_killed_piece;
            last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
            d = MIN_VAL(b, alpha, beta, 0, cutoff, search, evaluator);
            b->data.last_killed_piece = last_killed_piece_temp;
            b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
            b->undo_last_move(maxnode->move_eval_arr[0].movement);
            alpha = std::max(alpha, d);
            // if (!search)
            // {
            //     break;
            // }
            optimum.eval = d;
            optimum.movement = maxnode->move_eval_arr[0].movement;
            
            std::cout << "SETTING : " << optimum.movement << std::endl;
            best_move = optimum.movement;
            std::cout << "SET AT : " << cutoff << std::endl;
            
            for(size_t j = 1; j < maxnode->move_eval_arr.size(); j++)
            {
                b->do_move(maxnode->move_eval_arr[j].movement);
                last_killed_piece_temp = b->data.last_killed_piece;
                last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
                d = MIN_VAL(b, alpha, beta, 0, cutoff, search, evaluator);
                b->data.last_killed_piece = last_killed_piece_temp;
                b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
                b->undo_last_move(maxnode->move_eval_arr[j].movement);
                alpha = std::max(alpha, d);
                
                // if (!search)
                // {
                //     break;
                // }
                if (optimum.eval < d)
                {
                    optimum.eval = d;
                    optimum.movement = maxnode->move_eval_arr[j].movement;
                    std::cout << "SETTING : " << optimum.movement << std::endl;
                    best_move = optimum.movement;
                    std::cout << "SET AT : " << cutoff << std::endl;
                }
                
            }
            // std::cout << "SETTING : " << optimum.movement << std::endl;
            // best_move = optimum.movement;
            // std::cout << "SET AT : " << cutoff << std::endl;
            ++cutoff;
            delete maxnode;
            // return optimum.movement;
        }
        
        // return optimum.movement;
    }
    else
    {
        // std::cout << "Checking black search" << std::endl;
        while (search)
        {
            // std::cout << "Cutoff: " << cutoff << std::endl;
            double alpha = -DBL_MAX;
            double beta = DBL_MAX;
            Node* minnode = new Node(b, evaluator);
            // if (minnode->legal_moves.empty())
            // {
            //     return;
            // }
            minnode->Order_Children();
            // double maxmove;
            double d;
            b->do_move(minnode->move_eval_arr.end()[-1].movement);
            last_killed_piece_temp = b->data.last_killed_piece;
            last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
            d = MAX_VAL(b, alpha, beta, 0, cutoff, search, evaluator);
            U8 last_killed_piece_temp = b->data.last_killed_piece;
            int last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
            b->undo_last_move(minnode->move_eval_arr.end()[-1].movement);
            beta = std::min(beta, d);
            // if (!search)
            // {
            //     break;
            // }
            optimum.eval = d;
            optimum.movement = minnode->move_eval_arr.end()[-1].movement;
            
            
            std::cout << "SETTING : " << optimum.movement << std::endl;
            best_move = optimum.movement;
            std::cout << "SET AT : " << cutoff << std::endl;
            for(size_t j = 2; j < minnode->move_eval_arr.size()+1; j++)
            {
                b->do_move(minnode->move_eval_arr.end()[-j].movement);
                last_killed_piece_temp = b->data.last_killed_piece;
                last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
                d = MAX_VAL(b, alpha, beta, 0, cutoff, search, evaluator);
                b->data.last_killed_piece = last_killed_piece_temp;
                b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
                b->undo_last_move(minnode->move_eval_arr.end()[-j].movement);
                beta = std::min(beta, d);
                
                // if (!search)
                // {
                //     break;
                // }
                if (optimum.eval > d)
                {
                    optimum.eval = d;
                    optimum.movement = minnode->move_eval_arr.end()[-j].movement;
                    // if (!search)
                    // {
                    //     return;
                    // }
                    std::cout << "SETTING : " << optimum.movement << std::endl;
                    best_move = optimum.movement;
                    std::cout << "SET AT : " << cutoff << std::endl;
                }
                
            }
            ++cutoff;
            // std::cout << "SETTING : " << optimum.movement << std::endl;
            // best_move = optimum.movement;
            // std::cout << "SET AT : " << cutoff << std::endl;
            // return optimum.movement;
            delete minnode;
        }
        // return optimum.movement;
    }
    std::cout << "BLYATTT  " << cutoff << "  SUUUUKAAAA" << std::endl; 
    if (training)
    {
        evaluator->update(board_to_dioble(b), optimum.eval);
        evaluator->dump_weights("data/weights.txt");
    }
    return;
}

double MAX_VAL(Board* b, double alpha, double beta, int i, int cutoff, std::atomic<bool>& search, NeuralNetwork* evaluator)
{
    // if (!search)
    // {
    //     return 0;
    // }
    Node* maxnode = new Node(b, evaluator);
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
    b->do_move(maxnode->move_eval_arr[0].movement);
    U8 last_killed_piece_temp = b->data.last_killed_piece;
    int last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
    d = MIN_VAL(b, alpha, beta, i+1, cutoff, search, evaluator);
    b->data.last_killed_piece = last_killed_piece_temp;
    b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
    b->undo_last_move(maxnode->move_eval_arr[0].movement);
    alpha = std::max(alpha, d);
    if (alpha>=beta)
    {
        return d;
    }    
    maxmove = d;
    
    for(size_t j = 1; j < maxnode->move_eval_arr.size(); j++)
    {
        b->do_move(maxnode->move_eval_arr[j].movement);
        last_killed_piece_temp = b->data.last_killed_piece;
        last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
        d = MIN_VAL(b, alpha, beta, i+1, cutoff, search, evaluator);
        b->data.last_killed_piece = last_killed_piece_temp;
        b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
        b->undo_last_move(maxnode->move_eval_arr[j].movement);
        alpha = std::max(alpha, d);
        if (alpha>=beta)
        {
            return d;
        }
        if (maxmove < d)
        {
            maxmove = d;
        }
        
    }
    delete maxnode;
    return maxmove;  
}

double MIN_VAL(Board* b, double alpha, double beta, int i, int cutoff, std::atomic<bool>& search, NeuralNetwork* evaluator)
{
    // if (!search)
    // {
    //     return 0;
    // }
    Node* minnode = new Node(b, evaluator);
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
    b->do_move(minnode->move_eval_arr.end()[-1].movement);
    U8 last_killed_piece_temp = b->data.last_killed_piece;
    int last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
    d = MAX_VAL(b, alpha, beta, i+1, cutoff, search, evaluator);
    b->data.last_killed_piece = last_killed_piece_temp;
    b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
    b->undo_last_move(minnode->move_eval_arr.end()[-1].movement);
    beta = std::min(beta, d);
    if (alpha>=beta)
    {
        return d;
    }    
    minmove = d;
    
    for(size_t j = 2; j < minnode->move_eval_arr.size()+1; j++)
    {
        b->do_move(minnode->move_eval_arr.end()[-j].movement);
        last_killed_piece_temp = b->data.last_killed_piece;
        last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
        d = MAX_VAL(b, alpha, beta, i+1, cutoff, search, evaluator);
        b->data.last_killed_piece = last_killed_piece_temp;
        b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
        b->undo_last_move(minnode->move_eval_arr.end()[-j].movement);
        beta = std::min(beta, d);
        if (alpha>=beta)
        {
            return d;
        }
        if (minmove > d)
        {
            minmove = d;
        }
        
    }
    delete minnode;
    return minmove;  
}