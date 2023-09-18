#include<vector>

class NeuralNetwork {
    NeuralNetwork(int input_size, int num_hidden_layers, int hidden_size, int output_size);

    double evaluate(std::vector<double> features);
    void update(double correct_output);
    void update(double correct_output, double evaluated_output);

private:
    int input_size;
    int num_hidden_layers;
    int hidden_size;
    int output_size;

    std::vector<std::vector<double>> weights;
    std::vector<std::vector<double>> biases;

    double sigmoid(double x);
    double sigmoid_derivative(double x);
};