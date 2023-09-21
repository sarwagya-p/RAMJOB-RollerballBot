#pragma once

#include <fstream>
#include<vector>
#include <iostream>

class EvaluationFunc {
public:
    virtual void load_weights(std::string filename) = 0;
    virtual void dump_weights(std::string filename) = 0;

    virtual double evaluate(std::vector<double> features) = 0;
    virtual void update(std::vector<double> features, double evaluated_output) = 0;

    virtual void print_weights() = 0;
};

class NeuralNetwork : public EvaluationFunc {
public:
    NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes, bool randomize_weights=true, bool to_train = false);
    NeuralNetwork(int input_size, std::vector<int> hidden_layers_sizes, std::string filename, bool to_train = false);

    void load_weights(std::string filename);
    void dump_weights(std::string filename);

    double evaluate(std::vector<double> features);
    void update(std::vector<double> features, double evaluated_output);

    void print_weights();

private:
    bool to_train;
    std::vector<int> layer_sizes;
    
    double learning_rate;
    std::vector<std::vector<std::vector<double>>> weights;
    std::vector<std::vector<double>> biases;

    std::vector<std::vector<double>> forward_prop_outputs(std::vector<double> features);
};

class WSum : public EvaluationFunc {
public:
    WSum(int input_size, bool train);
    WSum(int input_size, std::string filename);

    void load_weights(std::string filename);
    void dump_weights(std::string filename);

    double evaluate(std::vector<double> features);
    void update(std::vector<double> features, double evaluated_output);

    void print_weights();

private:
    std::vector<double> weights;
    double learning_rate = 0.05;
};