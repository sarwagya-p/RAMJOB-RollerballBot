#include <unordered_map>
#include <random>
#include "evaluation_func.hpp"

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

// std::vector<double> NeuralNetwork::prepare_features(std::shared_ptr<Board> b)
// {
    // std::vector<double> dio = std::vector<double>(25);
    // dio[0] = (double)getx(b->data.b_rook_ws);
    // dio[2] = (double)getx(b->data.b_rook_bs);
    // dio[4] = (double)getx(b->data.b_king);
    // dio[6] = (double)getx(b->data.b_bishop);
    // dio[8] = (double)getx(b->data.b_pawn_ws);
    // dio[10] = (double)getx(b->data.b_pawn_bs);

    // dio[12] = (double)getx(b->data.w_rook_ws);
    // dio[14] = (double)getx(b->data.w_rook_bs);
    // dio[16] = (double)getx(b->data.w_king);
    // dio[18] = (double)getx(b->data.w_bishop);
    // dio[20] = (double)getx(b->data.w_pawn_ws);
    // dio[22] = (double)getx(b->data.w_pawn_bs);

    // dio[1] = (double)gety(b->data.b_rook_ws);
    // dio[3] = (double)gety(b->data.b_rook_bs);
    // dio[5] = (double)gety(b->data.b_king);
    // dio[7] = (double)gety(b->data.b_bishop);
    // dio[9] = (double)gety(b->data.b_pawn_ws);
    // dio[11] = (double)gety(b->data.b_pawn_bs);

    // dio[13] = (double)gety(b->data.w_rook_ws);
    // dio[15] = (double)gety(b->data.w_rook_bs);
    // dio[17] = (double)gety(b->data.w_king);
    // dio[19] = (double)gety(b->data.w_bishop);
    // dio[21] = (double)gety(b->data.w_pawn_ws);
    // dio[23] = (double)gety(b->data.w_pawn_bs);

    // dio[24] = get_margin_score(b);

    // return dio;
// }

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
    weights = std::vector<double>(input_size);

    if (!randomize){
        load_weights(filename);
        return;
    }

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<double> uniform(0,1);
    for (int i=0; i<input_size; i++){
        weights[i] = uniform(generator);
    }
}

void WSum::load_weights(std::string filename){
    std::ifstream input_file(filename);

    for (int i=0; i<weights.size(); i++){
        input_file >> weights[i];
    }

    input_file.close();
}

void WSum::dump_weights(std::string filename){
    std::ofstream output_file(filename);

    for (int i=0; i<weights.size(); i++){
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
    for (int i=0; i<weights.size(); i++){
        weightedSum += weights[i]*features[i];
    }
    std::cout << "SUKAA SUM: " << weightedSum << std::endl;
    return weightedSum;
}

void WSum::update(std::vector<double> features, double evaluated_output){
    std::vector<double> weighted_features(features.size());
    double weighted_sum = 0;
    double prior_output = evaluate(features);

    for (int i=0; i<features.size(); i++){
        weighted_features[i] = features[i]*weights[i];
        weighted_sum += weighted_features[i];
    }

    for (int i=0; i<weights.size(); i++){
        weights[i] += learning_rate*(evaluated_output - prior_output) * weighted_features[i]/weighted_sum;
    }

    dump_weights(filename);
}

void WSum::print_weights(){
    for (int i=0; i<weights.size(); i++){
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
    if (piece == DEAD || b->data.board_0[piece] != PAWN) return 0;
    
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

std::vector<double> EvaluationFunc::prepare_features(std::shared_ptr<Board> board){
    std::vector<double> features;

    double white_pieces = 1, black_pieces = 1;
    // Pawn advantage

    double num_b_pawns = sign_alive(board, board->data.b_pawn_bs, PAWN) + sign_alive(board, board->data.b_pawn_ws, PAWN);
    double num_w_pawns = sign_alive(board, board->data.w_pawn_bs, PAWN)+sign_alive(board, board->data.w_pawn_ws, PAWN);
    white_pieces += num_w_pawns;
    black_pieces += num_b_pawns;

    features.push_back((num_w_pawns-num_b_pawns)/2);
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

    features.push_back((num_w_rooks-num_b_rooks)/2);

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

    features.push_back(promotion_adv/14);
    // std::cout << "Man dist: " << promotion_adv << std::endl;
    // In check

    double check = 0;

    if (board->in_check()){
        check += int(board->data.player_to_play == WHITE) - int(board->data.player_to_play == BLACK);
        if (board->get_legal_moves().empty())
        {
            std::cout << "IS MATEEE" << std::endl;
            if(board->data.player_to_play == WHITE)
            {
                check = -1000;
            }
            else
            {
                check = 1000;
            }
        }
    }

    features.push_back(check);

    // In threat, for each piece type
    // std::unordered_set<U16> white_moves, black_moves;
    // PlayerColor curr_player = board->data.player_to_play;

    // std::unordered_map<U8, int> being_attacked;
    // being_attacked[PAWN] = 0;
    // being_attacked[ROOK] = 0;
    // being_attacked[BISHOP] = 0;

    // board->data.player_to_play = WHITE;
    // white_moves = board->get_legal_moves();

    // board->data.player_to_play = BLACK;
    // black_moves = board->get_legal_moves();
    // board->data.player_to_play = curr_player;

    // for (U16 w_move: white_moves){
    //     if (board->data.board_0[getp1(w_move)] & BLACK){
    //         being_attacked[board->data.board_0[getp1(w_move)] & 0xf]++;
    //     }
    // }
    
    // for (U16 b_move: black_moves){
    //     if (board->data.board_0[getp1(b_move)] & WHITE){
    //         being_attacked[board->data.board_0[getp1(b_move)] & 0xf]++;
    //     }
    // }

    // features.push_back(being_attacked[PAWN]/4);
    // features.push_back(being_attacked[ROOK]/4);
    // features.push_back(being_attacked[BISHOP]/4);
    // Defendend, for each piece type

    // std::cout << "Features made" << std::endl;
    return features;
}