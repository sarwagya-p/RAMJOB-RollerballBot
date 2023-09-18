#pragma once

#include<vector>

class NeuralNetwork {
public:
    NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes);

    double evaluate(std::vector<double> features);
    void update(std::vector<double> features, double evaluated_output);

private:
    std::vector<int> layer_sizes;
    
    double learning_rate;
    std::vector<std::vector<std::vector<double>>> weights;
    std::vector<std::vector<double>> biases;

    std::vector<std::vector<double>> forward_prop_outputs(std::vector<double> features);
    double sigmoid(double x);
    double sigmoid_derivative(double x);
};