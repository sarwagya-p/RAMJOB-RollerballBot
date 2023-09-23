#include <algorithm>
#include <random>
#include <iostream>
#include <memory>
#include <float.h>
#include <fstream>
#include<vector>
#include <unordered_map>

#include "board.hpp"
#include "engine.hpp"

class EvaluationFunc {
public:
    virtual void load_weights(std::string filename) = 0;
    virtual void dump_weights(std::string filename) = 0;

    virtual std::vector<double> prepare_features(std::shared_ptr<Board> b, int legal_moves_size);
    virtual double evaluate(std::vector<double> features) = 0;
    virtual void update(std::vector<double> features, double evaluated_output) = 0;

    virtual void print_weights() = 0;
};

class NeuralNetwork : public EvaluationFunc {
public:
    NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes, std::string filename,
        bool randomize_weights=true);
    
    void load_weights(std::string filename);
    void dump_weights(std::string filename);

    // std::vector<double> prepare_features(std::shared_ptr<Board> b);
    double evaluate(std::vector<double> features);
    void update(std::vector<double> features, double evaluated_output);

    void print_weights();

private:
    std::vector<int> layer_sizes;
    std::string filename;
    
    double learning_rate;
    std::vector<std::vector<std::vector<double>>> weights;
    std::vector<std::vector<double>> biases;

    std::vector<std::vector<double>> forward_prop_outputs(std::vector<double> features);
};

class WSum : public EvaluationFunc {
public:
    WSum(int input_size, std::string filename, bool randomize = false);

    void load_weights(std::string filename);
    void dump_weights(std::string filename);

    // std::vector<double> prepare_features(std::shared_ptr<Board> b);
    double evaluate(std::vector<double> features);
    void update(std::vector<double> features, double evaluated_output);

    void print_weights();

private:
    std::vector<double> weights;
    double learning_rate = 0.05;
    std::string filename;
};

double get_margin_score(std::shared_ptr<Board> board_state);


double sigmoid(double x){
        return 1/(1+std::exp(-x));
}
double inv_sigmoid(double y)
{
    return log(1/(1-y));
}

double sigmoid_derivative(double x){
    return sigmoid(x)*(1-sigmoid(x));
}

NeuralNetwork::NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes, std::string filename, 
    bool randomize_weights):filename(filename){
    learning_rate = 0.01;
    layer_sizes = hidden_layers_sizes;
    layer_sizes.insert(layer_sizes.begin(), input_size);
    layer_sizes.push_back(1);

    weights = std::vector<std::vector<std::vector<double>>>(layer_sizes.size()-1);
    biases = std::vector<std::vector<double>>(layer_sizes.size()-1);

    for (size_t i=0;i<layer_sizes.size()-1; i++){
        weights[i] = std::vector<std::vector<double>>(layer_sizes[i+1], std::vector<double>(layer_sizes[i]));
        biases[i] = std::vector<double>(layer_sizes[i+1]);
    }

    if (!randomize_weights){
        load_weights(filename);
        return;
    }
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<double> normal_random(0,1);

    
    for (size_t i=0; i<layer_sizes.size()-1; i++){
        for (int j =0; j<layer_sizes[i+1]; j++){
            for (int k=0; k<layer_sizes[i]; k++){
                weights[i][j][k] = normal_random(generator);
            }
        }

        for (int j=0; j<layer_sizes[i+1]; j++){
            biases[i][j] = normal_random(generator);
        }
    }

    dump_weights(filename);
}

double get_margin_score(std::shared_ptr<Board> board_state)
{
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
    return margin_score;
}


std::vector<std::vector<double>> NeuralNetwork::forward_prop_outputs(std::vector<double> features){
    std::vector<std::vector<double>> outputs(layer_sizes.size());
    outputs[0] = features;

    for (size_t layer=1; layer<layer_sizes.size(); layer++){
        outputs[layer] = std::vector<double>(layer_sizes[layer]);

        for (int i=0; i<layer_sizes[layer]; i++){
            double weighted_sum = biases[layer-1][i];

            for (int j=0; j<layer_sizes[layer-1]; j++){
                weighted_sum += outputs[layer-1][j]*weights[layer-1][i][j];
            }

            outputs[layer][i] = (sigmoid(weighted_sum));
        }
    }
    return outputs;
}

double NeuralNetwork::evaluate(std::vector<double> features){
    // actual
    return (forward_prop_outputs(features).back()[0])*100;
    
}

void NeuralNetwork::update(std::vector<double> features, double evaluated_output){
    evaluated_output = sigmoid(evaluated_output);
    std::vector<std::vector<double>> outputs = forward_prop_outputs(features);

    std::vector<std::vector<double>> errors(outputs.size()-1);
    for (size_t layer=0; layer<errors.size(); layer++){
        errors[layer] = std::vector<double>(layer_sizes[layer]);
    }
    errors.back() = {pow(evaluated_output-outputs.back()[0], 2)};
    for (int layer=errors.size()-2; layer>=0; --layer){
        for (int i=0; i<layer_sizes[layer]; i++){
            double weighted_sum = 0;
            for (size_t j=0; j<errors[layer+1].size(); j++){
                
                weighted_sum += errors[layer+1][j]*weights[layer][j][i];
            }

            errors[layer][i] = weighted_sum;
        }
    }
    for (size_t layer=0; layer<weights.size(); layer++){
        for (size_t i=0; i<weights[layer].size(); i++){
            for (size_t j=0; j<weights[layer][i].size(); j++){
                weights[layer][i][j] += learning_rate*errors[layer][i]*outputs[layer][j];
            }
        }
    }

    dump_weights(filename);
}

void NeuralNetwork::load_weights(std::string filename){
    std::ifstream input_file(filename);

    for (size_t layer=0; layer<weights.size(); layer++){
        for (std::vector<double>& row: weights[layer]){
            for (size_t i=0; i<row.size(); i++){
                input_file >> row[i];
            }
        }
    } 
    input_file.close();
}

void NeuralNetwork::dump_weights(std::string filename){
    std::ofstream output_file(filename);

    for (size_t layer=0; layer<weights.size(); layer++){
        for (std::vector<double>& row: weights[layer]){
            for (size_t i=0; i<row.size(); i++){
                output_file << row[i] << " ";
            }
            output_file << std::endl;
        }
        output_file << std::endl;
    } 
    output_file.close();
}

void NeuralNetwork::print_weights(){
    for (size_t layer=0; layer<weights.size(); layer++){
        std::cout << "Weights for layer: " << layer << std::endl;

        for (std::vector<double>& row: weights[layer]){
            for (double w: row){
                std::cout << w << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n\n";
    }
}

WSum::WSum(int input_size, std::string filename, bool randomize): filename(filename){
    if (!randomize){
        weights = {1, 10, 3, 0.5, -0.5, 0.02, 0.8};
        // load_weights(filename);
        return;
    }

    weights = std::vector<double>(input_size);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<double> uniform(0,1);
    for (int i=0; i<input_size; i++){
        weights[i] = uniform(generator);
    }
}

void WSum::load_weights(std::string filename){
    std::ifstream input_file(filename);

    for (size_t i=0; i<weights.size(); i++){
        input_file >> weights[i];
    }

    input_file.close();
}

void WSum::dump_weights(std::string filename){
    std::ofstream output_file(filename);

    for (size_t i=0; i<weights.size(); i++){
        output_file << weights[i] << " ";
    }
    output_file << std::endl;
    output_file.close();
}

double WSum::evaluate(std::vector<double> features){
    double weightedSum = 0;
    if (features.size() != weights.size()){
        std::cout << "Feature size mismatch" << std::endl;
        return 0;
    }
    for (size_t i=0; i<weights.size(); i++){
        weightedSum += weights[i]*features[i];
    }
    std::cout << "SUKAA SUM: " << weightedSum << std::endl;
    std::cout << "Man val: " << weights[5]*features[5];
    return weightedSum;
}

void WSum::update(std::vector<double> features, double evaluated_output){
    std::vector<double> weighted_features(features.size());
    double weighted_sum = 0;
    double prior_output = evaluate(features);

    for (size_t i=0; i<features.size(); i++){
        weighted_features[i] = features[i]*weights[i];
        weighted_sum += weighted_features[i];
    }

    for (size_t i=0; i<weights.size(); i++){
        weights[i] += learning_rate*(evaluated_output - prior_output) * weighted_features[i]/weighted_sum;
    }

    dump_weights(filename);
}

void WSum::print_weights(){
    for (size_t i=0; i<weights.size(); i++){
        std::cout << weights[i] << std::endl;
    }

    std::cout << std::endl << std::endl;
}

int sign_alive(std::shared_ptr<Board> board, U8 piece, PieceType p_type){
    if (piece == DEAD) return 0;
    
    if (board->data.board_0[piece] & p_type) return 1;
    return 0;
}

int manhattan_to_promotion(std::shared_ptr<Board> b, U8 piece, PlayerColor col){
    if (piece == DEAD || !(b->data.board_0[piece] & PAWN)) return 0;
    
    int x = getx(piece);
    int y = gety(piece);

    if (col == BLACK){
        int d1 = std::abs(x - 2) + std::abs(y - 0);
        int d2 = std::abs(x - 2) + std::abs(y - 1);

        return 7-std::min(d1, d2);
    }

    int d1 = std::abs(x-4)+std::abs(y-5);
    int d2 = std::abs(x-4)+std::abs(y-6);

    return 7-std::min(d1, d2);
}

double is_pawns_connected(std::shared_ptr<Board> board, PlayerColor col){
    U8 pos1, pos2;

    if (col == WHITE){
        pos1 = board->data.w_pawn_bs;
        pos1 = board->data.w_pawn_ws;
    }
    else {
        pos1 = board->data.b_pawn_bs;
        pos1 = board->data.b_pawn_ws;
    }

    if (pos1 == DEAD || !(board->data.board_0[pos1] & PAWN)) return 0;
    if (pos2 == DEAD || !(board->data.board_0[pos2] & PAWN)) return 0;

    return 10-(std::abs(getx(pos1)-getx(pos2)) + std::abs(gety(pos1)+gety(pos2)));
}

std::vector<double> EvaluationFunc::prepare_features(std::shared_ptr<Board> board, int legal_moves_size){
    std::vector<double> features;

    double white_pieces = 1, black_pieces = 1;
    // Pawn advantage

    double num_b_pawns = sign_alive(board, board->data.b_pawn_bs, PAWN) + sign_alive(board, board->data.b_pawn_ws, PAWN);
    double num_w_pawns = sign_alive(board, board->data.w_pawn_bs, PAWN)+sign_alive(board, board->data.w_pawn_ws, PAWN);
    white_pieces += num_w_pawns;
    black_pieces += num_b_pawns;

    features.push_back((num_w_pawns-num_b_pawns));
    // std::cout << "Pawns: " << num_w_pawns << " " << num_b_pawns << std::endl;
    // Rook Adv

    double num_b_rooks = 0;
    num_b_rooks += sign_alive(board, board->data.b_rook_bs, ROOK);
    num_b_rooks += sign_alive(board, board->data.b_rook_ws, ROOK);
    num_b_rooks += sign_alive(board, board->data.b_pawn_ws, ROOK);
    num_b_rooks += sign_alive(board, board->data.b_pawn_bs, ROOK);
    
    double num_w_rooks = 0;
    num_w_rooks += sign_alive(board, board->data.w_rook_bs, ROOK);
    num_w_rooks += sign_alive(board, board->data.w_rook_ws, ROOK);
    num_w_rooks += sign_alive(board, board->data.w_pawn_ws, ROOK);
    num_w_rooks += sign_alive(board, board->data.w_pawn_bs, ROOK);

    // std::cout << "Rooks: " << num_w_rooks << " " << num_b_rooks << std::endl;

    white_pieces += 5*num_w_rooks;
    black_pieces += 5*num_b_rooks;

    features.push_back((num_w_rooks-num_b_rooks));

    // Bishop Adv
    double num_w_bishops = 0;
    double num_b_bishops = 0;

    num_b_bishops += sign_alive(board, board->data.b_bishop, BISHOP);
    num_b_bishops += sign_alive(board, board->data.b_pawn_bs, BISHOP);
    num_b_bishops += sign_alive(board, board->data.b_pawn_ws, BISHOP);

    num_w_bishops += sign_alive(board, board->data.w_bishop, BISHOP);
    num_w_bishops += sign_alive(board, board->data.w_pawn_bs, BISHOP);
    num_w_bishops += sign_alive(board, board->data.w_pawn_ws, BISHOP);

    // std::cout << "Bishops: " << num_w_bishops << " " << num_b_bishops << std::endl;
    white_pieces += 3*num_w_bishops;
    black_pieces += 3*num_b_bishops;
    features.push_back(num_w_bishops-num_b_bishops);

    // Ratio of alive pieces

    features.push_back(white_pieces/black_pieces);
    features.push_back(black_pieces/white_pieces);

    // std::cout << "Ratio: " << white_pieces/black_pieces << " " << black_pieces/white_pieces << std::endl;
    // Pawn promotion manhattan distance

    double promotion_adv = manhattan_to_promotion(board, board->data.w_pawn_bs, WHITE) 
                            + manhattan_to_promotion(board, board->data.w_pawn_ws, WHITE);

    promotion_adv -= manhattan_to_promotion(board, board->data.b_pawn_bs, BLACK) 
                            + manhattan_to_promotion(board, board->data.b_pawn_ws, BLACK);

    features.push_back(promotion_adv);
    // std::cout << "Man dist: " << promotion_adv << std::endl;

    // Connected Pawns

    double connected_pawns = is_pawns_connected(board, WHITE) - is_pawns_connected(board, BLACK);

    // features.push_back(connected_pawns);

    // In check

    double check = 0;

    if (board->in_check()){
        check += int(board->data.player_to_play == WHITE) - int(board->data.player_to_play == BLACK);
        if (legal_moves_size == 0)
        {
            std::cout << "IS MATEEE" << std::endl;
            if(board->data.player_to_play == WHITE)
            {
                check = -100000;
            }
            else
            {
                check = 100000;
            }
        }
    }

    features.push_back(check);



    // std::cout << "Features made" << std::endl;
    return features;
}

typedef struct
{
    U16 movement;
    double eval;
}move_eval;


class Node {
public:
    std::shared_ptr<Board> board_state;
    std::shared_ptr<EvaluationFunc> evaluator;

    Node(std::shared_ptr<Board> board_state, std::shared_ptr<EvaluationFunc> evaluator);
    std::vector<move_eval> Order_Children(std::atomic<bool>& search, bool reverse = false);
};

double MAX_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<EvaluationFunc> evaluator);

double MIN_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<EvaluationFunc> evaluator);

void search_move(std::shared_ptr<Board> b, std::atomic<bool>& search, std::atomic<U16>& best_move, 
    bool training, std::shared_ptr<EvaluationFunc> evaluator);

double get_margin_score(std::shared_ptr<Board> board_state);
void undo_last_move(std::shared_ptr<Board> b, U16 move);

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
    bool operator()(const move_eval mve1, const move_eval mve2)
    {
        return (mve1.eval > mve2.eval);
    }
};

struct CompareMoveEvalReverse{
    bool operator()(const move_eval mve1, const move_eval mve2)
    {
        return (mve1.eval < mve2.eval);
    }
};


Node::Node(std::shared_ptr<Board> board_state, std::shared_ptr<EvaluationFunc> evaluator) 
    :board_state(board_state), evaluator(evaluator)
    {
        std::cout << "Node set." << std::endl;
    }

std::vector<move_eval> Node::Order_Children(std::atomic<bool>& search, bool reverse)
{
    std::unordered_set<U16> legal_moves = board_state->get_legal_moves();
    std::vector<move_eval> move_eval_arr;

    move_eval temp;
    for (U16 test_move: legal_moves)
    {
        if (!search) return {};
        // std::cout << "Move: " << test_move << std::endl;
        temp.movement = test_move;
        board_state->do_move(test_move);
        // std::cout << "Evaluating move: " << move_to_str(test_move) << std::endl;
        temp.eval = evaluator->evaluate(evaluator->prepare_features(board_state, board_state->get_legal_moves().size()));
        undo_last_move(board_state, test_move);

        move_eval_arr.push_back(temp);
    }
    // std::cout << "Evalutions done, sorting" << std::endl;
    
    if (!reverse)
    std::sort(move_eval_arr.begin(), move_eval_arr.end(), CompareMoveEval());
    else
    std::sort(move_eval_arr.begin(), move_eval_arr.end(), CompareMoveEvalReverse());
    std::cout << "Sorted" << std::endl;
    
    return move_eval_arr;
}

double move_and_eval(std::shared_ptr<Board> b, U16 move, int i, int cutoff, double alpha, double beta,
std::shared_ptr<EvaluationFunc> evaluator, bool to_find_min, std::atomic<bool>& search){
    if (!search) return 0;
    std::cout << "Doing move for eval." << std::endl;
    b->do_move(move);
    U8 last_killed_piece_temp = b->data.last_killed_piece;
    int last_killed_piece_idx_temp = b->data.last_killed_piece_idx;
    std::cout << "Done move." << std::endl;
    double d = 0;
    std::cout << "Making Node: " << std::endl;
    if (to_find_min){
        d = MIN_VAL(b, alpha, beta, i, cutoff, search, evaluator);
    }
    else {
        d = MAX_VAL(b, alpha, beta, i, cutoff, search, evaluator);
    }
    std::cout << "Search done" << std::endl;
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
        while (search)
        {
            double alpha = -DBL_MAX;
            double beta = DBL_MAX;
            std::shared_ptr<Node> maxnode = std::make_shared<Node>(b, evaluator);
            std::vector<move_eval> legal_moves = maxnode->Order_Children(search, false);
            std::cout << "Legal moves made" << std::endl;
            if (legal_moves.empty())
            {
                std::cout << "No legals, returning" << std::endl;
                return;
            }
            

            std::cout << "LEGALS WHITE: " << std::endl;
            for (move_eval test_move: legal_moves){
                std::cout << move_to_str(test_move.movement) << " ";
            }
            std::cout << std::endl;

            double d;
            optimum.eval = -DBL_MAX;

            for(size_t j = 0; j < legal_moves.size(); j++)
            {
                std::cout << "Evaling index: " << j << " move: " << move_to_str(legal_moves[j].movement) << std::endl;
                d = move_and_eval(b, legal_moves[j].movement, 0, cutoff,
                alpha, beta, evaluator, true, search);
                std::cout << "Done evaling index: " << j << std::endl;
                if (!search) break;
                alpha = std::max(alpha, d);
                
                if (optimum.eval < d)
                {
                    optimum.eval = d;
                    optimum.movement = legal_moves[j].movement;
                    std::cout << "SETTING : " << optimum.movement << std::endl;
                    best_move = optimum.movement;
                    std::cout << "SET AT : " << cutoff << std::endl;
                }
                
            }
            if (!search) break;
            std::cout << "SETTING : " << optimum.movement << std::endl;
            best_move = optimum.movement;
            std::cout << "SET AT : " << cutoff << std::endl;
            cutoff += 2;
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
            std::shared_ptr<Node> minnode = std::make_shared<Node>(b, evaluator);
            std::vector<move_eval> legal_moves = minnode->Order_Children(search, true);
            if (legal_moves.empty())
            {
                std::cout << "No legals, returning" << std::endl;
                return;
            }

            std::cout << "LEGALS BLACK: " << std::endl;
            for (move_eval test_move: legal_moves){
                std::cout << test_move.movement << " ";
            }
            std::cout << std::endl;
            // double maxmove;
            double d = move_and_eval(b, legal_moves[0].movement, 0, cutoff,
                alpha, beta, evaluator, false, search);
            if (!search) break;
            beta = std::min(beta, d);
            // std::cout << "Check1" << std::endl;
            optimum.eval = DBL_MAX;
            // optimum.movement = minnode->move_eval_arr.end()[-1].movement;
            
            
            // std::cout << "SETTING : " << optimum.movement << std::endl;
            // best_move = optimum.movement;
            // std::cout << "SET AT : " << cutoff << std::endl;
            for(size_t j = 0; j < legal_moves.size(); j++)
            {
                std::cout << "Evaling index: " << j << " move: " << move_to_str(legal_moves[j].movement) << std::endl;
                double d = move_and_eval(b, legal_moves[j].movement, 0, cutoff,
                alpha, beta, evaluator, false, search);
                if (!search) break;
                beta = std::min(beta, d);

                if (optimum.eval > d)
                {
                    optimum.eval = d;
                    optimum.movement = legal_moves[j].movement;
                    // if (!search)
                    // {
                    //     return;
                    // }
                    std::cout << "SETTING : " << optimum.movement << std::endl;
                    best_move = optimum.movement;
                    std::cout << "SET AT : " << cutoff << std::endl;
                }
                
            }
            if (!search) break;
            cutoff += 2;
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
        // evaluator->update(evaluator->prepare_features(b), optimum.eval);
        std::cout << "Updated" << std::endl;
    }
    return;
}

double MAX_VAL(std::shared_ptr<Board> b, double alpha, double beta, int i, int cutoff, 
    std::atomic<bool>& search, std::shared_ptr<EvaluationFunc> evaluator)
{
    std::cout << "Called max val for cutoff = " << cutoff << " depth = " << i << std::endl;
    if (!search) return 0;
    
    std::shared_ptr<Node> maxnode = std::make_shared<Node>(b, evaluator);
    std::cout << "Node memory alloted" << std::endl;
    std::vector<move_eval> legal_moves = maxnode->Order_Children(search, false);

    if (legal_moves.empty())
    {   
        return evaluator->evaluate(evaluator->prepare_features(b, 0));
    }
    if (i == cutoff)
    {
        return evaluator->evaluate(evaluator->prepare_features(b, legal_moves.size()));
    }
    std::cout << "Ordering" << std::endl;
    maxnode->Order_Children(search);
    std::cout << "Ordered" << std::endl;
    if (!search) return 0;
    
    double maxmove = -DBL_MAX;
    double d;
    
    for(size_t j = 0; j < legal_moves.size(); j++)
    {
        d = move_and_eval(b, legal_moves[j].movement, i+1, cutoff,
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

    std::shared_ptr<Node> minnode = std::make_shared<Node>(b, evaluator);
    std::cout << "Node memory alloted" << std::endl;
    std::vector<move_eval> legal_moves = minnode->Order_Children(search, true);

    if (legal_moves.empty())
    {
        // std::cout << "Check4 : " << std::endl;
        return evaluator->evaluate(evaluator->prepare_features(b, 0));
        // std::cout << "Check5 : " << std::endl;
    }
    if (i == cutoff)
    {   
        return evaluator->evaluate(evaluator->prepare_features(b, legal_moves.size()));
    }
    minnode->Order_Children(search, true);
    if (!search) return 0;

    double minmove = DBL_MAX;
    double d;
    
    for(size_t j = 0; j < legal_moves.size(); j++)
    {
        d = move_and_eval(b, legal_moves[j].movement, i+1, cutoff,
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

// static std::shared_ptr<EvaluationFunc> evaluator = std::shared_ptr<EvaluationFunc>(new NeuralNetwork(25, {10, 10}, "data/weights.txt"));
static std::shared_ptr<EvaluationFunc> evaluator = std::make_shared<WSum>(7, "./data/wsum_weights.txt", false);


void Engine::find_best_move(const Board& b) {

    std::shared_ptr<Board> board_state = std::shared_ptr<Board>(b.copy());
    
    std::cout << "Calling from Engine" << std::endl;
    search_move(board_state, search, best_move, false, evaluator);
}
