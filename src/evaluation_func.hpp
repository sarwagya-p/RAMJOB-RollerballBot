#pragma once

#include<memory>
#include <fstream>
#include<vector>
#include <iostream>

#include "board.hpp"

class EvaluationFunc {
public:
    virtual void load_weights(std::string filename) = 0;
    virtual void dump_weights(std::string filename) = 0;

    virtual std::vector<double> prepare_features(std::shared_ptr<Board> b);
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