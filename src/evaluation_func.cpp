#include "evaluation_func.hpp"
#include<random>

double sigmoid(double x){
        return 1/(1+std::exp(x));
}

double sigmoid_derivative(double x){
    return sigmoid(x)*(1-sigmoid(x));
}

NeuralNetwork::NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes, bool randomize_weights, bool to_train)
:to_train(to_train){
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

NeuralNetwork::NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes, std::string filename, bool to_train)
:to_train(to_train){
    NeuralNetwork(input_size, hidden_layers_sizes, false);
    load_weights(filename);
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

WSum::WSum(int input_size, bool train){
    weights = std::vector<double>(input_size);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<double> uniform(0,1);
    for (int i=0; i<input_size; i++){
        weights[i] = uniform(generator);
    }
}

WSum::WSum(int input_size, std::string filename){
    weights = std::vector<double>(input_size);
    load_weights(filename);
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

    for (int i=0; i<weights.size(); i++){
        weightedSum += weights[i]*features[i];
    }

    return sigmoid(weightedSum)*100;
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
}

void WSum::print_weights(){
    for (int i=0; i<weights.size(); i++){
        std::cout << weights[i] << std::endl;
    }

    std::cout << std::endl << std::endl;
}