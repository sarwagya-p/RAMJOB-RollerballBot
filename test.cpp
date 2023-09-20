#include <random>
#include <thread>
#include<unistd.h>
#include "src/search.hpp"
#include "src/board.hpp"

std::mt19937 rd;

std::shared_ptr<Board> create_random_board(int num_pieces){
    std::shared_ptr<Board> b = std::shared_ptr<Board>(new Board());
    b->data = {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD};

    std::vector<int> shuffled_pieces= {0,1,3,4,5,6,7,9,10,11};

    for (size_t i=9; i>=num_pieces; i--){
        std::uniform_int_distribution<size_t> uniform(0, i);
        size_t j = uniform(rd);

        shuffled_pieces.erase(shuffled_pieces.begin()+j);
    }
    
    std::cout << "Remaining pieces: ";

    for (int x: shuffled_pieces){
        std::cout << x << " ";
    }
    std::cout << std::endl;
    shuffled_pieces.push_back(2);
    shuffled_pieces.push_back(8);

    std::uniform_int_distribution<size_t> uniform(0, 42);
    std::vector<U8> random_pos;
    for (int i=0; i<num_pieces+2; i++){
        U8 random_square = uniform(rd);
        int x, y;

        if (random_square < 14){
            x = random_square/7;
            y = random_square%7;
        }
        else if (random_square < 28){
            x = (random_square-14)/4;
            if (x>2) x+=3;

            y = (random_square-14)%4;
        }
        else {
            x = (random_square-28)/4 + 5;
            y = (random_square-28)%4;
        }
        U8 p = pos(x, y);
        random_pos.push_back(p);
    }

    U8* pieces = (U8*)(&b->data);
    for (int i=0; i<shuffled_pieces.size(); i++){
        pieces[shuffled_pieces[i]] = random_pos[i];
    }

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

void train(int num_pieces){
    std::shared_ptr<Board> board = std::shared_ptr<Board>(new Board());

    std::cout << board_to_str((U8*)board.get()) << std::endl;
    std::shared_ptr<NeuralNetwork> a = std::shared_ptr<NeuralNetwork>(new NeuralNetwork(25, {10, 10}, true, true));
    std::shared_ptr<NeuralNetwork> b = std::shared_ptr<NeuralNetwork>(new NeuralNetwork(25, {10, 10}, true, false));

    a->print_weights();

    std::atomic<bool> a_search=true, b_search=true, stop=false;
    std::atomic<U16> best_move_a, best_move_b;

    bool train_a = true, train_b = false;

    while (board->get_legal_moves().size() > 0){
        // std::cout << "Doing move" << std::endl;
        if (board->data.player_to_play == WHITE){
            search_move(board, a_search, best_move_a, true, a);
            std::cout << "Doing move." << std::endl;
            board->do_move(best_move_a);
        }
        else{
            search_move(board, b_search, best_move_b, false, b);
            board->do_move(best_move_b);
        }
    }
    std::cout << "Stopping, since legal_moves = " << board->get_legal_moves().size() << std::endl;
    stop = true;
}

int main(){
    while (true){
        train(2);
    }
}