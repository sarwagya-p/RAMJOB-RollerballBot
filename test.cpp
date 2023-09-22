#include <random>
#include <thread>
#include <algorithm>
#include<unistd.h>
#include "src/search.hpp"
#include "src/board.hpp"

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

std::mt19937 rd(std::time(0));

double board_temp_eval(std::shared_ptr<Board> b)
{
    int num_moves = 40;
    double margin_score = get_margin_score(b);
    bool white_win, black_win;
    white_win = false;
    black_win = false;
    std::unordered_set<U16> legals;
    if (b->data.player_to_play == WHITE)
    {
        legals = b->get_legal_moves();
        if (legals.empty())
        {
            black_win = true;
        }
        b->data.player_to_play = BLACK;
        legals = b->get_legal_moves();
        if (b->in_check() || legals.empty())
        {
            white_win = true;
        }
        b->data.player_to_play = WHITE;
    }
    else
    {
        legals = b->get_legal_moves();
        if (legals.empty())
        {
            white_win = true;
        }
        b->data.player_to_play = WHITE;
        legals = b->get_legal_moves();
        if (b->in_check() || legals.empty())
        {
            black_win = true;
        }
        b->data.player_to_play = BLACK;
    }
    std::cout << "V or D:: " << white_win - black_win << std::endl;
    return margin_score + (white_win - black_win) * 100 + (black_win - white_win) * (5 * (num_moves/20) + std::min(10, num_moves)) + 40*(white_win == black_win);

}

void rotate_board1(U8 *src, U8 *tgt, const U8 *transform) {

    for (int i=0; i<64; i++) {
        tgt[transform[i]] = src[i];
    }
}

U8 get_random_pos()
{
    std::uniform_int_distribution<size_t> uniform(0, 48);
    U8 random_square = uniform(rd);
        int x, y;

        if (random_square < 14){
            x = random_square/7;
            y = random_square%7;
        }
        else if (random_square < 35){
            x = (random_square-14)/7 + 2;
            

            y = (random_square-14)%4;
            if (y>1) y+=3;
        }
        else {
            x = (random_square-35)/7 + 5;
            y = (random_square-35)%7;
        }
        return pos(x, y);
}

std::shared_ptr<Board> create_random_board(int num_pieces){
    std::vector<int> shuffled_pieces= {0, 1, 3, 4, 5, 6,7, 9, 10, 11};

    for (int i=9; i>=num_pieces; i--){
        std::uniform_int_distribution<size_t> uniform(0, i);
        size_t j = uniform(rd);

        shuffled_pieces.erase(shuffled_pieces.begin()+j);
    }
    shuffled_pieces.push_back(2);
    shuffled_pieces.push_back(8);

    std::cout << "Remaining pieces: ";

    for (int x: shuffled_pieces){
        std::cout << x << " ";
    }
    std::cout << std::endl;

    std::uniform_int_distribution<size_t> uniform(0, 48);
    std::vector<U8> random_pos;
    U8 p;
    while (random_pos.size() < shuffled_pieces.size()-1){
        // U8 random_square = uniform(rd);
        // int x, y;

        // if (random_square < 14){
        //     x = random_square/7;
        //     y = random_square%7;
        // }
        // else if (random_square < 35){
        //     x = (random_square-14)/7 + 2;
            

        //     y = (random_square-14)%4;
        //     if (y>1) y+=3;
        // }
        // else {
        //     x = (random_square-35)/7 + 5;
        //     y = (random_square-35)%7;
        // }
        p = get_random_pos();

        if (std::find(random_pos.begin(), random_pos.end(), p) != random_pos.end()) continue;
        random_pos.push_back(p);
    }

    U8 p_2 = get_random_pos();

    while (((getp0(p) - getp0(p_2)) <= 1 && (getp0(p) - getp0(p_2)) >= -1 && (getp1(p) - getp1(p_2)) <= 1 && (getp1(p) - getp1(p_2)) >= -1) || std::find(random_pos.begin(), random_pos.end(), p_2) != random_pos.end())
    {
        p_2 = get_random_pos();
    }

    random_pos.push_back(p_2);


    std::shared_ptr<Board> b = std::shared_ptr<Board>(new Board());
    b->data.board_0[b->data.b_rook_ws]  = 0;
    b->data.board_0[b->data.b_rook_bs]  = 0;
    b->data.board_0[b->data.b_king   ]  = 0;
    b->data.board_0[b->data.b_bishop ]  = 0;
    b->data.board_0[b->data.b_pawn_ws]  = 0;
    b->data.board_0[b->data.b_pawn_bs]  = 0;

    b->data.board_0[b->data.w_rook_ws]  = 0;
    b->data.board_0[b->data.w_rook_bs]  = 0;
    b->data.board_0[b->data.w_king   ]  = 0;
    b->data.board_0[b->data.w_bishop ]  = 0;
    b->data.board_0[b->data.w_pawn_ws]  = 0;
    b->data.board_0[b->data.w_pawn_bs]  = 0;

    b->data.b_rook_ws  = DEAD;
    b->data.b_rook_bs  = DEAD;
    b->data.b_king     = DEAD;
    b->data.b_bishop   = DEAD;
    b->data.b_pawn_ws  = DEAD;
    b->data.b_pawn_bs  = DEAD;

    b->data.w_rook_ws  = DEAD;
    b->data.w_rook_bs  = DEAD;
    b->data.w_king     = DEAD;
    b->data.w_bishop   = DEAD;
    b->data.w_pawn_ws  = DEAD;
    b->data.w_pawn_bs  = DEAD;

    U8* pieces = (U8*)(&b->data);

    for (int i=0; i<shuffled_pieces.size(); i++){
        std::cout << "Setting piece " << shuffled_pieces[i] << " to pos " << int(random_pos[i]) << std::endl;
        pieces[shuffled_pieces[i]] = random_pos[i];
    }

    b->data.board_0[b->data.b_rook_ws]  = (b->data.b_rook_ws != DEAD)*(BLACK | ROOK);
    b->data.board_0[b->data.b_rook_bs]  = (b->data.b_rook_bs != DEAD)*(BLACK | ROOK);
    b->data.board_0[b->data.b_king   ]  = (b->data.b_king != DEAD)*(BLACK | KING);
    b->data.board_0[b->data.b_bishop ]  = (b->data.b_bishop != DEAD)*(BLACK | BISHOP);
    b->data.board_0[b->data.b_pawn_ws]  = (b->data.b_pawn_ws != DEAD)*(BLACK | PAWN);
    b->data.board_0[b->data.b_pawn_bs]  = (b->data.b_pawn_bs != DEAD)*(BLACK | PAWN);

    b->data.board_0[b->data.w_rook_ws]  = (b->data.w_rook_ws != DEAD)*(WHITE | ROOK);
    b->data.board_0[b->data.w_rook_bs]  = (b->data.w_rook_bs != DEAD)*(WHITE | ROOK);
    b->data.board_0[b->data.w_king   ]  = (b->data.w_king != DEAD)*(WHITE | KING);
    b->data.board_0[b->data.w_bishop ]  = (b->data.w_bishop != DEAD)*(WHITE | BISHOP);
    b->data.board_0[b->data.w_pawn_ws]  = (b->data.w_pawn_ws != DEAD)*(WHITE | PAWN);
    b->data.board_0[b->data.w_pawn_bs]  = (b->data.w_pawn_bs != DEAD)*(WHITE | PAWN);

    rotate_board1(b->data.board_0, b->data.board_90, cw_90);
    rotate_board1(b->data.board_0, b->data.board_180, cw_180);
    rotate_board1(b->data.board_0, b->data.board_270, acw_90);

    return b;
}

void player(std::shared_ptr<Board> board, std::atomic<bool>& search, std::atomic<bool>& stop, std::atomic<U16>& best_move,
        std::shared_ptr<NeuralNetwork> evaluator, bool training){
    while (!stop){
        if (search){
            if (training) std::cout << "Getting move from a" << std::endl;
            else std::cout << "Getting move from b" << std::endl;
            search_move(board, search, best_move, training, evaluator);
        }
    }
}

void train(int num_pieces, std::shared_ptr<EvaluationFunc> a, std::shared_ptr<EvaluationFunc> b){
    std::shared_ptr<Board> board = create_random_board(num_pieces);

    std::cout << board_to_str(board->data.board_0) << std::endl;

    std::atomic<bool> a_search(true), b_search(true), stop(false);
    std::atomic<U16> best_move_a, best_move_b;

    bool train_a = true, train_b = false;
    int i = 0;
    U16 w1, w2, w3, w4, w5, w6;
    U16 b1, b2, b3, b4, b5, b6;
    while (board->get_legal_moves().size() > 0 && i < 100){
        // std::cout << "Doing move" << std::endl;
        if (board->data.player_to_play == WHITE){
            search_move(board, a_search, best_move_a, true, a);
            std::cout << "Doing move.\n\n\n" << std::endl;
            board->do_move(best_move_a);
            std::cout << board_to_str(board->data.board_0) << "\n\n\n\n\n" << std::endl;
            w6 = w5;
            w5 = w4;
            w4 = w3;
            w3 = w2;
            w2 = w1;
            w1 = best_move_a;
            
        }
        else{
            search_move(board, b_search, best_move_b, false, b);
            board->do_move(best_move_b);
            b6 = b5;
            b5 = b4;
            b4 = b3;
            b3 = b2;
            b2 = b1;
            b1 = best_move_b;
        }
        if(w6 == w4 && w4 == w2 && w5 == w3 && w3 == w1 && b6 == b4 && b4 == b2 && b5 == b3 && b3 == b1)
            break;
        i++;
    }
    double eval = board_temp_eval(board);
    std::cout << "BLYAAADD SUUUKAA EVAL ___ SCORE::" << a->evaluate(a->prepare_features(board)) << std::endl;
    std::cout << "BLYAAADD SUUUKAA TEMP ___ SCORE::" << eval << std::endl;
    a->update(a->prepare_features(board), eval);

    std::cout << "Stopping"  << std::endl;
    std::cout << "Dumping weights" << std::endl;

    a->dump_weights("data/weights.txt");
    stop = true;
}

void train_on_margin(std::shared_ptr<EvaluationFunc> evaluator)
{
    // std::shared_ptr<NeuralNetwork> a = std::shared_ptr<NeuralNetwork>(new NeuralNetwork(25, {10}, "data/weights.txt", true));
    double eval;
    std::uniform_int_distribution<size_t> uniform(0, 10);
    std::shared_ptr<Board> board = create_random_board(uniform(rd));
    std::cout << board_to_str(board->data.board_0) << std::endl;
    eval = board_temp_eval(board);
    std::cout << "BLYAAADD SUUUKAA EVAL FUNC ___ SCORE::" << evaluator->evaluate(evaluator->prepare_features(board)) << std::endl;
    std::cout << "BLYAAADD SUUUKAA BOARD MARGIN ___ SCORE::" << eval << std::endl;
    // evaluator->update(evaluator->prepare_features(board), eval);
    // evaluator->dump_weights("data/weights.txt");
    
}

void train_neural(int num_pieces){
    std::shared_ptr<NeuralNetwork> a = std::shared_ptr<NeuralNetwork>(new NeuralNetwork(25, {10}, "./data/weights.txt", true));
    std::shared_ptr<NeuralNetwork> b = std::shared_ptr<NeuralNetwork>(new NeuralNetwork(25, {10}, "./data/weights.txt", false));

    train(num_pieces, a, b);
}

void train_wsum(int num_pieces){
    // std::shared_ptr<EvaluationFunc> a = std::shared_ptr<EvaluationFunc>(new WSum(8, "./data/wsum_weights.txt", false, false));
    std::shared_ptr<EvaluationFunc> b = std::shared_ptr<EvaluationFunc>(new WSum(8, "./data/wsum_weights.txt", false));
    train_on_margin(b);
}

int main(){
    long long i = 1;
    while (i < 2)
    {
        std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nTRAIN NUMBER :: " << i << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"<< std::endl; 
        // sleep(2);
        i++;
        train_wsum(2);
        // break;
    }
    
}