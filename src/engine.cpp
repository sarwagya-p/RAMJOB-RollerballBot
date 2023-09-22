#include <algorithm>
#include <random>
#include <iostream>
#include <memory>

#include "search.hpp"
#include "board.hpp"
#include "engine.hpp"

// static std::shared_ptr<EvaluationFunc> evaluator = std::shared_ptr<EvaluationFunc>(new NeuralNetwork(25, {10, 10}, "data/weights.txt"));
static std::shared_ptr<EvaluationFunc> evaluator = std::make_shared<WSum>(8, "./data/wsum_weights.txt", false);


void Engine::find_best_move(const Board& b) {

    std::shared_ptr<Board> board_state = std::shared_ptr<Board>(b.copy());
    
    std::cout << "Calling from Engine" << std::endl;
    // std::cout << "LEGALS: " << std::endl;
    // for (U16 test_move: b.get_legal_moves()){
    //     std::cout << test_move << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "LEGALS COPY: " << std::endl;
    // for (U16 test_move: board_state->get_legal_moves()){
    //     std::cout << test_move << " ";
    // }
    // std::cout << std::endl;

    search_move(board_state, search, best_move, false, evaluator);

    // pick a random move
    
    // auto moveset = b.get_legal_moves();
    // if (moveset.size() == 0) {
    //     this->best_move = 0;
    // }
    // else {
    //     std::vector<U16> moves;
    //     std::cout << all_boards_to_str(b) << std::endl;
    //     for (auto m : moveset) {
    //         std::cout << move_to_str(m) << " ";
    //     }
    //     std::cout << std::endl;
    //     std::sample(
    //         moveset.begin(),
    //         moveset.end(),
    //         std::back_inserter(moves),
    //         1,
    //         std::mt19937{std::random_device{}()}
    //     );
    //     this->best_move = moves[0];
    // }
}
