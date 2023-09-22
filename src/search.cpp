#include <algorithm>
#include <iostream>
#include <float.h>
#include "search.hpp"


constexpr U8 cw_90[64] = {
    48, 40, 32, 24, 16, 8,  0,  7,
    49, 41, 33, 25, 17, 9,  1,  15,
    50, 42, 18, 19, 20, 10, 2,  23,
    51, 43, 26, 27, 28, 11, 3,  31,
    52, 44, 34, 35, 36, 12, 4,  39,
    53, 45, 37, 29, 21, 13, 5,  47,
    54, 46, 38, 30, 22, 14, 6,  55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 acw_90[64] = {
     6, 14, 22, 30, 38, 46, 54, 7,
     5, 13, 21, 29, 37, 45, 53, 15,
     4, 12, 18, 19, 20, 44, 52, 23,
     3, 11, 26, 27, 28, 43, 51, 31,
     2, 10, 34, 35, 36, 42, 50, 39,
     1,  9, 17, 25, 33, 41, 49, 47,
     0,  8, 16, 24, 32, 40, 48, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 cw_180[64] = {
    54, 53, 52, 51, 50, 49, 48, 7,
    46, 45, 44, 43, 42, 41, 40, 15,
    38, 37, 18, 19, 20, 33, 32, 23,
    30, 29, 26, 27, 28, 25, 24, 31,
    22, 21, 34, 35, 36, 17, 16, 39,
    14, 13, 12, 11, 10,  9,  8, 47,
     6,  5,  4,  3,  2,  1,  0, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 id[64] = {
     0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

struct CompareMoveEval{
    bool reverse = false;

    CompareMoveEval(bool reverse = false): reverse(reverse){}
    bool operator()(const move_eval mve1, const move_eval mve2)
    {
        return (mve1.eval > mve2.eval) != reverse;
    }
};

Node::Node(std::shared_ptr<Board> board_state, std::shared_ptr<EvaluationFunc> evaluator) 
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
        temp.eval = evaluator->evaluate(evaluator->prepare_features(board_state));
        undo_last_move(board_state, test_move);

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
std::shared_ptr<EvaluationFunc> evaluator, bool to_find_min, std::atomic<bool>& search){
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
    undo_last_move(b, move);

    return d;
}

//ADVERSARIAL SEARCH
void search_move(std::shared_ptr<Board> b, std::atomic<bool>& search, std::atomic<U16>& best_move, 
    bool training, std::shared_ptr<EvaluationFunc> evaluator)
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
            // if (maxnode->legal_moves.empty())
            // {
            //     return;
            // }
            maxnode->Order_Children(search, true);

            std::cout << "LEGALS WHITE: " << std::endl;
            for (U16 test_move: maxnode->legal_moves){
                std::cout << test_move << " ";
            }
            std::cout << std::endl;
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
                    // std::cout << "SETTING : " << optimum.movement << std::endl;
                    best_move = optimum.movement;
                    // std::cout << "SET AT : " << cutoff << std::endl;
                }
                
            }
            std::cout << "SETTING : " << optimum.movement << std::endl;
            best_move = optimum.movement;
            std::cout << "SET AT : " << cutoff << std::endl;
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
            std::cout << "LEGALS BLACK: " << std::endl;
            for (U16 test_move: minnode->legal_moves){
                std::cout << test_move << " ";
            }
            std::cout << std::endl;
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
                    // std::cout << "SETTING : " << optimum.movement << std::endl;
                    best_move = optimum.movement;
                    // std::cout << "SET AT : " << cutoff << std::endl;
                }
                
            }
            ++cutoff;
            std::cout << "SETTING : " << optimum.movement << std::endl;
            best_move = optimum.movement;
            std::cout << "SET AT : " << cutoff << std::endl;
            // return optimum.movement;
        }
        // return optimum.movement;
    }
    std::cout << "BLYATTT  " << cutoff << "  SUUUUKAAAA" << std::endl; 
    if (training)
    {
        std::cout << "Updating" << std::endl;
        evaluator->update(evaluator->prepare_features(b), optimum.eval);
        std::cout << "Updated" << std::endl;
    }
    return;
}

double MAX_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<EvaluationFunc> evaluator)
{
    if (!search) return 0;
    std::shared_ptr<Node> maxnode = std::shared_ptr<Node>(new Node(b, evaluator));
    if (maxnode->legal_moves.empty())
    {
        return maxnode->score();
    }
    if (i == cutoff)
    {
        return evaluator->evaluate(evaluator->prepare_features(b));
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
    std::atomic<bool>& search, std::shared_ptr<EvaluationFunc> evaluator)
{
    if (!search) return 0;

    std::shared_ptr<Node> minnode = std::shared_ptr<Node>(new Node(b, evaluator));
    if (minnode->legal_moves.empty())
    {
        return minnode->score();
    }
    if (i == cutoff)
    {
        return evaluator->evaluate(evaluator->prepare_features(b));
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



void undo_last_move(std::shared_ptr<Board> b, U16 move)
{
    U8 p0 = getp0(move);
    U8 p1 = getp1(move);
    U8 promo = getpromo(move);

    U8 piecetype = b->data.board_0[p1];
    U8 deadpiece = b->data.last_killed_piece;
    b->data.last_killed_piece = 0;

    // scan and get piece from coord
    U8 *pieces = (U8*)(&(b->data));
    for (int i=0; i<12; i++) {
        if (pieces[i] == p1) {
            pieces[i] = p0;
            break;
        }
    }
    if (b->data.last_killed_piece_idx >= 0) {
        pieces[b->data.last_killed_piece_idx] = p1;
        b->data.last_killed_piece_idx = -1;
    }

    if (promo == PAWN_ROOK) {
        piecetype = ((piecetype & (WHITE | BLACK)) ^ ROOK) | PAWN;
    }
    else if (promo == PAWN_BISHOP) {
        piecetype = ((piecetype & (WHITE | BLACK)) ^ BISHOP) | PAWN;
    }

    b->data.board_0[p0]           = piecetype;
    b->data.board_90[cw_90[p0]]   = piecetype;
    b->data.board_180[cw_180[p0]] = piecetype;
    b->data.board_270[acw_90[p0]] = piecetype;

    b->data.board_0[p1]           = deadpiece;
    b->data.board_90[cw_90[p1]]   = deadpiece;
    b->data.board_180[cw_180[p1]] = deadpiece;
    b->data.board_270[acw_90[p1]] = deadpiece;


    b->data.player_to_play = (PlayerColor)(b->data.player_to_play ^ (WHITE | BLACK));
}