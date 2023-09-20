#include <algorithm>
#include <iostream>
#include "search.hpp"
#include <float.h>

struct CompareMoveEval{
    bool reverse = false;

    CompareMoveEval(bool reverse = false): reverse(reverse){}
    bool operator()(const move_eval mve1, const move_eval mve2)
    {
        return (mve1.eval > mve2.eval) != reverse;
    }
};

Node::Node(std::shared_ptr<Board> board_state, std::shared_ptr<NeuralNetwork> evaluator) 
    :board_state(board_state), evaluator(evaluator)
    {
        num_moves = 0;
        legal_moves = board_state->get_legal_moves();
    }

void Node::Order_Children(std::atomic<bool>& search, bool reverse)
{
    move_eval temp;
    for (U16 test_move: legal_moves)
    {
        if (!search) return;
        // std::cout << "Move: " << test_move << std::endl;
        temp.movement = test_move;
        board_state->do_move(test_move);
        temp.eval = evaluator->evaluate(board_to_dioble(board_state));
        board_state->undo_last_move(test_move);

        move_eval_arr.push_back(temp);
    }
    std::sort(move_eval_arr.begin(), move_eval_arr.end(), CompareMoveEval(reverse));

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

std::vector<double> board_to_dioble(std::shared_ptr<Board> b)
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

double get_margin_score(std::shared_ptr<Board> board_state)
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

double move_and_eval(std::shared_ptr<Board> b, U16 move, int i, int cutoff, double alpha, double beta,
std::shared_ptr<NeuralNetwork> evaluator, bool to_find_min, std::atomic<bool>& search){
    if (!search) return 0;
    b->do_move(move);
    U8 last_killed_piece_temp = b->data.last_killed_piece;
    int last_killed_piece_idx_temp = b->data.last_killed_piece_idx;

    double d = 0;
    if (to_find_min){
        d = MIN_VAL(b, alpha, beta, i, cutoff, search, evaluator);
    }
    else {
        d = MAX_VAL(b, alpha, beta, i, cutoff, search, evaluator);
    }
    b->data.last_killed_piece = last_killed_piece_temp;
    b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
    b->undo_last_move(move);

    return d;
}

//ADVERSARIAL SEARCH
void search_move(std::shared_ptr<Board> b, std::atomic<bool>& search, std::atomic<U16>& best_move, 
    bool training, std::shared_ptr<NeuralNetwork> evaluator)
{
    int cutoff = 1;
    move_eval optimum;

    if (b->data.player_to_play == WHITE)
    {
        // std::cout << "Checking white search" << std::endl;
        while (search && cutoff < 6)
        {
            // std::cout << "Cutoff: " << cutoff << std::endl;
            double alpha = -DBL_MAX;
            double beta = DBL_MAX;
            std::shared_ptr<Node> maxnode = std::shared_ptr<Node>(new Node(b, evaluator));
            std::cout << "LEGALS: " << std::endl;
            for (U16 test_move: maxnode->legal_moves){
                std::cout << test_move << " ";
            }
            std::cout << std::endl;
            // if (maxnode->legal_moves.empty())
            // {
            //     return;
            // }
            maxnode->Order_Children(search, true);
            // std::cout << "LEGALS OREDERRED: " << std::endl;
            // for (move_eval test_move: maxnode->move_eval_arr){
            //     std::cout << test_move.movement << " ";
            // }
            // std::cout << std::endl;
            // double maxmove;
            double d = move_and_eval(b, maxnode->move_eval_arr[0].movement, 0, cutoff, 
            alpha, beta, evaluator, true, search);
            if (!search) return;
            alpha = std::max(alpha, d);
            
            optimum.eval = -DBL_MAX;
            // optimum.movement = maxnode->move_eval_arr[0].movement;
            
            // std::cout << "SETTING : " << optimum.movement << std::endl;
            // best_move = optimum.movement;
            // std::cout << "SET AT : " << cutoff << std::endl;
            
            for(size_t j = 0; j < maxnode->move_eval_arr.size(); j++)
            {
                d = move_and_eval(b, maxnode->move_eval_arr[j].movement, 0, cutoff,
                alpha, beta, evaluator, true, search);
                if (!search) return;

                alpha = std::max(alpha, d);
                
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
            // return optimum.movement;
        }
        
        // return optimum.movement;
    }
    else
    {
        // std::cout << "Checking black search" << std::endl;
        while (search && cutoff < 6)
        {
            // std::cout << "Cutoff: " << cutoff << std::endl;
            double alpha = -DBL_MAX;
            double beta = DBL_MAX;
            std::shared_ptr<Node> minnode = std::shared_ptr<Node>(new Node(b, evaluator));
            // if (minnode->legal_moves.empty())
            // {
            //     return;
            // }
            minnode->Order_Children(search, true);
            // double maxmove;
            double d = move_and_eval(b, minnode->move_eval_arr[0].movement, 0, cutoff,
                alpha, beta, evaluator, false, search);
            if (!search) return;
            beta = std::min(beta, d);

            optimum.eval = DBL_MAX;
            // optimum.movement = minnode->move_eval_arr.end()[-1].movement;
            
            
            // std::cout << "SETTING : " << optimum.movement << std::endl;
            // best_move = optimum.movement;
            // std::cout << "SET AT : " << cutoff << std::endl;
            for(size_t j = 0; j < minnode->move_eval_arr.size(); j++)
            {
                double d = move_and_eval(b, minnode->move_eval_arr[j].movement, 0, cutoff,
                alpha, beta, evaluator, false, search);

                if (!search) return;
                beta = std::min(beta, d);

                if (optimum.eval > d)
                {
                    optimum.eval = d;
                    optimum.movement = minnode->move_eval_arr[j].movement;
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
        }
        // return optimum.movement;
    }
    std::cout << "BLYATTT  " << cutoff << "  SUUUUKAAAA" << std::endl; 
    if (training)
    {
        std::cout << "Updating" << std::endl;
        evaluator->update(board_to_dioble(b), optimum.eval);
        evaluator->dump_weights("data/weights.txt");
        std::cout << "Updated" << std::endl;
    }
    return;
}

double MAX_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<NeuralNetwork> evaluator)
{
    if (!search) return 0;
    std::shared_ptr<Node> maxnode = std::shared_ptr<Node>(new Node(b, evaluator));
    if (maxnode->legal_moves.empty())
    {
        return maxnode->score();
    }
    if (i == cutoff)
    {
        return evaluator->evaluate(board_to_dioble(b));
    }
    maxnode->Order_Children(search);
    if (!search) return 0;
    
    double maxmove = -DBL_MAX;
    double d;
    // double d = move_and_eval(b, maxnode->move_eval_arr[0].movement, i+1, cutoff,
    // alpha, beta, evaluator, true, search);
    // if (!search) return 0;

    // alpha = std::max(alpha, d);
    // if (alpha>=beta)
    // {
    //     return d;
    // }    
    // maxmove = d;
    
    for(size_t j = 0; j < maxnode->move_eval_arr.size(); j++)
    {
        d = move_and_eval(b, maxnode->move_eval_arr[j].movement, i+1, cutoff,
        alpha, beta, evaluator, true, search);
        if (!search) return 0;

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
    return maxmove;  
}

double MIN_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<NeuralNetwork> evaluator)
{
    if (!search) return 0;

    std::shared_ptr<Node> minnode = std::shared_ptr<Node>(new Node(b, evaluator));
    if (minnode->legal_moves.empty())
    {
        return minnode->score();
    }
    if (i == cutoff)
    {
        return evaluator->evaluate(board_to_dioble(b));
    }
    minnode->Order_Children(search, true);
    if (!search) return 0;

    double minmove = DBL_MAX;
    double d;
    // b->do_move(minnode->move_eval_arr.end()[-1].movement);
    // U8 last_killed_piece_temp = b->data.last_killed_piece;
    // int last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
    // d = MAX_VAL(b, alpha, beta, i+1, cutoff, search, evaluator);
    // b->data.last_killed_piece = last_killed_piece_temp;
    // b->data.last_killed_piece_idx = last_killed_piece_idx_temp;
    // b->undo_last_move(minnode->move_eval_arr.end()[-1].movement);
    // beta = std::min(beta, d);
    // if (alpha>=beta)
    // {
    //     return d;
    // }    
    // minmove = d;
    
    for(size_t j = 0; j < minnode->move_eval_arr.size(); j++)
    {
        d = move_and_eval(b, minnode->move_eval_arr[j].movement, i+1, cutoff,
        alpha, beta, evaluator, false, search);
        if (!search) return 0;

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
    return minmove;  
}