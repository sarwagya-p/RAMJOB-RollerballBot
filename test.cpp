#include "src/neural_network.hpp"

int main(){
    NeuralNetwork* evaluator = new NeuralNetwork(10, {5});

    evaluator->print_weights();
}