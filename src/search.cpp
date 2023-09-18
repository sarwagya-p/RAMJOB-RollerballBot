#include "search.hpp"
#include <algorithm>
#include <float.h>
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

#define cw_90_pos(p) cw_90[p]
#define cw_180_pos(p) cw_180[p]
#define acw_90_pos(p) acw_90[p]
#define cw_90_move(m) move_promo(cw_90[getp0(m)], cw_90[getp1(m)], getpromo(m))
#define acw_90_move(m) move_promo(acw_90[getp0(m)], acw_90[getp1(m)], getpromo(m))
#define cw_180_move(p) move_promo(cw_180[getp0(m)], cw_180[getp1(m)], getpromo(m))
#define color(p) ((PlayerColor)(p & (WHITE | BLACK)))

bool compare(move_eval mve1, move_eval mve2)
{
    return (mve1.eval > mve2.eval)
}

Node::Node(Board* board_state) 
    :board_state(board_state)
    {
        legal_moves = board_state->get_legal_moves();
    }

void Node::Order_Children()
{
    move_eval_arr = std::vector<move_eval>(legal_moves.size());
    move_eval temp;
    for (auto i = legal_moves.begin(); i != legal_moves.end(); ++i)
    {
        temp.move = *i;
        temp.eval = evaluator->evaluate(board_to_dioble(board_state));

    move_eval_arr[i] = temp;
    }
    std::sort(move_eval_arr.begin(), move_eval_arr.end(), compare);
}


void undo_last_move(Board* b, U16 move){

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
        piecetype = (piecetype & (WHITE | BLACK)) | ROOK;
    }
    else if (promo == PAWN_BISHOP) {
        piecetype = (piecetype & (WHITE | BLACK)) | BISHOP;
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
    // std::cout << "Undid last move\n";
    // std::cout << all_boards_to_str(*b);
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
    undo_last_move(b, maxnode->move_eval_arr[0].move);
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
        undo_last_move(b, maxnode->move_eval_arr[j].move);
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
    undo_last_move(b, minnode->move_eval_arr[0].move);
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
        undo_last_move(b, minnode->move_eval_arr[j].move);
    }
    return minmove;  
}