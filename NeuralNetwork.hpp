#pragma once

#include "DenseLayer.hpp"
//#include "ConvolutionalLayer.hpp"
#include "PoolingLayer.hpp"
#include "mnist.hpp"

#include <vector>
#include <memory>
#include <iostream>

enum class LayerType : char {
    Dense,
    Convolutional,
    Pooling,
};

struct LayerBuilder {
    LayerType type;
    size_t size;
    ActivationType activation_type;
    size_t input_width;
    size_t input_height;
    size_t kernel_size;
    size_t num_kernels;
    size_t stride;
    PoolingType pooling_type;
};

struct NeuralNetwork {

    //std::vector<std::unique_ptr<DenseLayer>> layers;
    std::vector<DenseLayer> layers;
    std::vector<double>& outputs;

    std::vector<DenseLayer> create_layers(size_t num_inputs, const std::vector<LayerBuilder>& layer_builders) {
        std::vector<DenseLayer> layers;
        for (const auto& layer_builder : layer_builders) {
            switch (layer_builder.type) {
            case LayerType::Dense:
                //layers.push_back(std::make_unique<DenseLayer>(num_inputs, layer_builder.size, layer_builder.activation_type));
                layers.push_back(DenseLayer(num_inputs, layer_builder.size, layer_builder.activation_type));
                break;
            case LayerType::Convolutional:
                break;
            case LayerType::Pooling:
                break;
            }
            num_inputs = layer_builder.size;
        }
        return layers;
    }

    NeuralNetwork(size_t num_inputs, const std::vector<LayerBuilder>& layer_builders) : layers(create_layers(num_inputs, layer_builders)), outputs(layers.back().outputs) {}

    label_t feed_forward(const input_t& inputs) {
        layers[0].calc_outputs(inputs);
        for (size_t l = 1; l < layers.size(); l++) {
            layers[l].calc_outputs(layers[l - 1].outputs);
        }
        return prediction();
    }

    label_t prediction() const {
        return static_cast<label_t>(std::distance(outputs.begin(), std::max_element(outputs.begin(), outputs.end())));
    }

    void back_propagate(label_t label) {
        for (size_t n = 0; n < layers.back().neurons.size(); n++) {
            double error = outputs[n] - static_cast<double>(n == label);
            layers.back().neurons[n].calc_delta(error);
        }
        for (size_t l = layers.size() - 2; l != -1; l--) {
            for (size_t n = 0; n < layers[l].neurons.size(); n++) {
                double error = 0.0;
                for (auto& next_layer_neuron : layers[l + 1].neurons) {
                    error += next_layer_neuron.weights[n] * next_layer_neuron.delta;
                }
                layers[l].neurons[n].calc_delta(error);
            }
        }
    }

    void update_parameters(const input_t& inputs, const double learning_rate) {
        layers[0].update_parameters(inputs, learning_rate);
        for (size_t l = 1; l < layers.size(); l++) {
            layers[l].update_parameters(layers[l - 1].outputs, learning_rate);
        }
    }

    void train(const std::vector<input_t>& inputs, const std::vector<label_t>& labels, const size_t num_epochs = 100, const double learning_rate = 0.01, const size_t batch_size = 128, const std::vector<input_t>& test_inputs = {}, const std::vector<label_t>& test_labels = {}) {
        if (inputs.size() != labels.size()) {
            throw std::invalid_argument("Number of train inputs and labels must be equal.");
        }
        for (size_t epoch_num = 1; epoch_num <= num_epochs; epoch_num++) {
            size_t batch_start = rand() % (inputs.size() - batch_size);
            for (size_t b = batch_start; b < batch_start + batch_size; b++) {
                feed_forward(inputs[b]);
                back_propagate(labels[b]);
                update_parameters(inputs[b], learning_rate);
            }
            if (num_epochs < 10 or epoch_num % (num_epochs / 10) == 0 or epoch_num == 1 or epoch_num == num_epochs) {
                std::cout << "Epoch " << epoch_num << "/" << num_epochs << std::flush << " - ";
                if (not test_inputs.empty()) {
                    test(test_inputs, test_labels);
                } else {
                    test(inputs, labels);
                }
            }
        }
    }

    void test(const std::vector<input_t>& inputs, const std::vector<label_t>& labels) {
        if (inputs.size() != labels.size()) {
            throw std::invalid_argument("Number of test inputs and labels must be equal.");
        }
        size_t num_correct = 0;
        for (size_t i = 0; i < inputs.size(); i++) {
            if (feed_forward(inputs[i]) == labels[i]) {
                num_correct++;
            }
        }
        std::cout << "Accuracy: " << static_cast<double>(num_correct) * 100.0 / static_cast<double>(inputs.size()) << '%' << std::endl;
    }

    void save(const std::string& model_path) {
        if (std::filesystem::exists(model_path)) {
            std::cout << "Folder '" << model_path << "' already exists. Overwrite? (y/N): " << std::flush;
            if (std::cin.get() != 'y') {
                return;
            } else {
                std::filesystem::remove_all(model_path);
            }
        }
        std::filesystem::create_directory(model_path);
        for (size_t l = 0; l < layers.size(); l++) {
            std::string layer_path = model_path + "/layer_" + std::to_string(l);
            layers[l].save(layer_path);
        }
    }

    void load(const std::string& model_path) {
        if (not std::filesystem::exists(model_path)) {
            throw std::runtime_error("Folder '" + model_path + "' does not exist.");
        }
        for (size_t l = 0; l < layers.size(); l++) {
            std::string layer_path = model_path + "/layer_" + std::to_string(l);
            layers[l].load(layer_path);
        }
    }
};