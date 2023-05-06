#include "NeuralNetwork.hpp"
#include "mnist.hpp"

#include <chrono>
#include <vector>
#include <utility>
#include <iostream>

int main() {
    std::iostream::sync_with_stdio(false);

    NeuralNetwork nn(
        784,
        {
            { LayerType::Dense, 16, ActivationType::ReLU },
            { LayerType::Dense, 16, ActivationType::ReLU },
            { LayerType::Dense, 10, ActivationType::Sigmoid },
        }
    );

    nn.load("models/3b1b");

    const auto train_inputs = extract_inputs("data/mnist/train_images.idx3-ubyte");
    const auto test_inputs = extract_inputs("data/mnist/test_images.idx3-ubyte");

    // Sanity check to make sure the model is working
    nn.test(test_inputs, extract_labels("data/mnist/test_labels.idx1-ubyte"));

    std::vector<input_t> inputs(train_inputs.size() + test_inputs.size());
    std::move(train_inputs.begin(), train_inputs.end(), inputs.begin());
    std::move(test_inputs.begin(), test_inputs.end(), inputs.begin() + train_inputs.size());


    // temp
    const auto train_labels = extract_labels("data/mnist/train_labels.idx1-ubyte");
    const auto test_labels = extract_labels("data/mnist/test_labels.idx1-ubyte");
    std::vector<label_t> labels(train_labels.size() + test_labels.size());
    std::move(train_labels.begin(), train_labels.end(), labels.begin());
    std::move(test_labels.begin(), test_labels.end(), labels.begin() + train_labels.size());


    const size_t num_iterations = 1000;

    size_t time_ns = 0;

    for (size_t i = 0; i < num_iterations; i++) {
        std::cout << "\r[";
        const size_t progress = (i + 1) * 100 / num_iterations;
        for (size_t j = 0; j < progress / 4; j++) std::cout << '=';
        if (progress == 100) std::cout << '='; else std::cout << '>';
        for (size_t j = progress / 4; j < 25; j++) std::cout << ' ';
        std::cout << "] " << progress << "%" << std::flush;
        for (const auto& input : inputs) {
            const auto start = std::chrono::high_resolution_clock::now();
            nn.feed_forward(input);
            const auto stop = std::chrono::high_resolution_clock::now();
            time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
        }
    }
    std::cout << '\n';

    const double time_s = static_cast<double>(time_ns) / 1e9;
    const double time_us = static_cast<double>(time_ns) / 1e3;

    const size_t num_images = inputs.size() * num_iterations;

    std::cout
        << "Time taken: " << time_s << " seconds\n"
        << "Images processed: " << num_images << '\n'
        << static_cast<double>(num_images) / time_s << " images per second\n"
        << time_us / static_cast<double>(num_images) << " microseconds per image\n"
        << std::flush;
}