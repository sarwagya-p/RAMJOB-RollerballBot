#include "src/neural_network.hpp"

int main(){
    NeuralNetwork* evaluator = new NeuralNetwork(10, {5}, false);
    evaluator->load_weights("data/weights.txt");
    // evaluator->dump_weights("data/weights.txt");

    std::vector<double> inp = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<std::vector<double>> outputs = evaluator->forward_prop_outputs(inp);

    for (std::vector<double> row: outputs){
        std::cout << "Layer:\n";

        for (double x: row){
            std::cout << x << " ";
        }
        std::cout << "\n";
    }

    // std::cout << "Final Evaluation: " << evaluator->evaluate(inp) << std::endl;

    for (int i=0; i<1000; i++){
        evaluator->update(inp, 10.0);
        std::cout << "Final Evaluation: " << evaluator->evaluate(inp) << std::endl;
    }

    evaluator->print_weights();
}