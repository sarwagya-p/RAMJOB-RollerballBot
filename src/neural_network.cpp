#include "neural_network.hpp"
#include<random>

double NeuralNetwork::sigmoid(double x){
        return 1/(1+std::exp(x));
}

double NeuralNetwork::sigmoid_derivative(double x){
    return sigmoid(x)*(1-sigmoid(x));
}

NeuralNetwork::NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes){
    learning_rate = 0.01;
    layer_sizes = hidden_layers_sizes;
    layer_sizes.insert(layer_sizes.begin(), input_size);
    layer_sizes.push_back(1);
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<double> normal_random(0,1);

    weights = std::vector<std::vector<std::vector<double>>>(layer_sizes.size()-1);
    biases = std::vector<std::vector<double>>(layer_sizes.size()-1);
    
    for (int i=0; i<layer_sizes.size()-1; i++){
        weights[i] = std::vector<std::vector<double>>(layer_sizes[i+1], std::vector<double>(layer_sizes[i]));

        for (int j =0; j<layer_sizes[i+1]; j++){
            for (int k=0; k<layer_sizes[i]; k++){
                weights[i][j][k] = normal_random(generator);
            }
        }

        biases[i] = std::vector<double>(layer_sizes[i+1]);

        for (int j=0; j<layer_sizes[i+1]; j++){
            biases[i][j] = normal_random(generator);
        }
    }
}

std::vector<std::vector<double>> NeuralNetwork::forward_prop_outputs(std::vector<double> features){
    std::vector<std::vector<double>> outputs(layer_sizes.size());
    outputs[0] = features;

    for (int layer=1; layer<layer_sizes.size(); layer++){
        outputs[layer] = std::vector<double>(layer_sizes[layer]);

        for (int i=0; i<layer_sizes[layer]; i++){
            double weighted_sum = biases[layer-1][i];

            for (int j=0; j<layer_sizes[layer-1]; j++){
                weighted_sum += features[j]*weights[layer-1][i][j];
            }

            outputs[layer][i] = sigmoid(weighted_sum);
        }
    }
    return outputs;
}

double NeuralNetwork::evaluate(std::vector<double> features){
    // for testing node search

    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<double> normal_random(0,10);
    return normal_random(generator) + 100;
    
    // actual
    // return forward_prop_outputs(features).back()[0];
    
}

void NeuralNetwork::update(std::vector<double> features, double evaluated_output){
    std::vector<std::vector<double>> outputs = forward_prop_outputs(features);

    std::vector<std::vector<double>> errors(outputs.size()-1);
    errors.back() = {pow(evaluated_output-outputs.back()[0], 2)};

    for (int layer=outputs.size()-3; layer>=0; layer--){
        for (int i=0; i<layer_sizes[layer]; i++){
            double weighted_sum = 0;

            for (int j=0; j<layer_sizes[layer+1]; j++){
                weighted_sum += errors[layer+1][j]*weights[layer][j][i];
            }

            errors[layer][i] = weighted_sum;
        }
    }

    for (int layer=0; layer<weights.size(); layer++){
        for (int i=0; i<weights[layer].size(); i++){
            for (int j=0; j<weights[layer][i].size(); j++){
                weights[layer][i][j] += learning_rate*errors[layer][i]*outputs[layer][j];
            }
        }
    }
}