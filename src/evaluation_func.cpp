#include <unordered_map>
#include<random>
#include "evaluation_func.hpp"

double sigmoid(double x){
        return 1/(1+std::exp(-x));
        return 1/(1+std::exp(-x));
}

double sigmoid_derivative(double x){
    return sigmoid(x)*(1-sigmoid(x));
}

NeuralNetwork::NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes, std::string filename, 
    bool randomize_weights):filename(filename){
    learning_rate = 0.1;
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

std::vector<double> NeuralNetwork::prepare_features(std::shared_ptr<Board> b)
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

            outputs[layer][i] = sigmoid(weighted_sum);
        }
    }
    return outputs;
}

double NeuralNetwork::evaluate(std::vector<double> features){
    // actual
    return forward_prop_outputs(features).back()[0]*100;
    
}

void NeuralNetwork::update(std::vector<double> features, double evaluated_output){
    evaluated_output = evaluated_output/100;
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

    return sigmoid(0.169*weightedSum)*100;
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

int sign_alive(U8 piece){
    if (piece == DEAD) return -1;
    return 1;
}

int manhattan_to_promotion(U8 piece, int final_x, int final_y){
    if (piece == DEAD) return 0;
    
    int x = getx(piece);
    int y = gety(piece);

    return 7 - (std::abs(final_x - x) + std::abs(final_y - y));
}

std::vector<double> WSum::prepare_features(std::shared_ptr<Board> board){
    std::vector<double> features;

    // Pawn advantage
    double pawn_adv = 0;

    pawn_adv -= sign_alive(board->data.b_pawn_bs);
    pawn_adv -= sign_alive(board->data.b_pawn_ws);
    pawn_adv += sign_alive(board->data.w_pawn_bs);
    pawn_adv += sign_alive(board->data.w_pawn_ws);

    features.push_back(pawn_adv/2);

    // Rook Adv

    double rook_adv = 0;

    rook_adv -= sign_alive(board->data.b_rook_bs);
    rook_adv -= sign_alive(board->data.b_rook_ws);
    rook_adv += sign_alive(board->data.w_rook_bs);
    rook_adv += sign_alive(board->data.w_rook_ws);

    features.push_back(rook_adv/2);

    // Bishop Adv
    double bishop_adv = 0;

    bishop_adv -= sign_alive(board->data.b_bishop);
    bishop_adv += sign_alive(board->data.w_bishop);

    features.push_back(bishop_adv);

    // Pawn promotion manhattan distance

    double promotion_adv = manhattan_to_promotion(board->data.w_pawn_bs, 4, 6) 
                            - manhattan_to_promotion(board->data.b_pawn_bs, 2, 1);

    promotion_adv += manhattan_to_promotion(board->data.w_pawn_ws, 4, 6) 
                            - manhattan_to_promotion(board->data.b_pawn_ws, 2, 1);

    features.push_back(promotion_adv/14);
    std::cout << "Man dist: " << promotion_adv << std::endl;
    // In check

    double check = 0;

    if (board->in_check()){
        check += int(board->data.player_to_play == WHITE) - int(board->data.player_to_play == BLACK);
    }

    features.push_back(check);

    // In threat, for each piece type
    std::unordered_set<U16> white_moves, black_moves;
    PlayerColor curr_player = board->data.player_to_play;

    std::unordered_map<U8, int> being_attacked;
    being_attacked[PAWN] = 0;
    being_attacked[ROOK] = 0;
    being_attacked[BISHOP] = 0;

    board->data.player_to_play = WHITE;
    white_moves = board->get_legal_moves();

    board->data.player_to_play = BLACK;
    black_moves = board->get_legal_moves();
    board->data.player_to_play = curr_player;

    for (U16 w_move: white_moves){
        if (board->data.board_0[getp1(w_move)] & BLACK){
            being_attacked[board->data.board_0[getp1(w_move)] & 0xf]++;
        }
    }
    
    for (U16 b_move: black_moves){
        if (board->data.board_0[getp1(b_move)] & WHITE){
            being_attacked[board->data.board_0[getp1(b_move)] & 0xf]++;
        }
    }

    features.push_back(being_attacked[PAWN]/4);
    features.push_back(being_attacked[ROOK]/4);
    features.push_back(being_attacked[BISHOP]/4);
    // Defendend, for each piece type

    std::cout << "Features made" << std::endl;
    return features;
}