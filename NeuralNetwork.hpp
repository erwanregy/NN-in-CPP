#pragma once

#include "DenseLayer.hpp"
//#include "ConvolutionalLayer.hpp"
#include "PoolingLayer.hpp"

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

    std::vector<std::unique_ptr<DenseLayer>> layers;
    std::vector<double> outputs;

    NeuralNetwork() : layers(), outputs() {}

    NeuralNetwork(size_t num_inputs, const std::vector<LayerBuilder>& layer_builders) : layers(layer_builders.size()), outputs(layer_builders.back().size, 0.0) {
        for (size_t i = 0; i < layers.size(); i++) {
            switch (layer_builders[i].type) {
                case LayerType::Dense:
                    layers[i] = std::make_unique<DenseLayer>(num_inputs, layer_builders[i].size, layer_builders[i].activation_type);
                    break;
                case LayerType::Convolutional:
                    break;
                case LayerType::Pooling:
                    break;
            }
            num_inputs = layer_builders[i].size;
        }
    }

    void feed_forward(std::vector<double> inputs) {
        for (auto& layer : layers) {
            inputs = layer->calc_outputs(inputs);
        }
        outputs = inputs;
    }

    size_t predict(const std::vector<double>& inputs) {
        feed_forward(inputs);
        return prediction();
    }

    size_t prediction() const {
        return std::distance(outputs.begin(), std::max_element(outputs.begin(), outputs.end()));
    }

    void back_propagate(size_t label) {
        auto& output_layer = layers.back();
        for (size_t n = 0; n < output_layer->neurons.size(); n++) {
            double error = outputs[n] - static_cast<double>(n == label);
            output_layer->neurons[n].calc_delta(error);
        }
        for (size_t l = layers.size() - 2; l != -1; l--) {
            auto& layer = layers[l];
            auto& next_layer = layers[l + 1];
            for (size_t n = 0; n < layer->neurons.size(); n++) {
                double error = 0.0;
                for (auto& next_layer_neuron : next_layer->neurons) {
                    error += next_layer_neuron.weights[n] * next_layer_neuron.delta;
                }
                layer->neurons[n].calc_delta(error);
            }
        }
    }

    void update_parameters(const std::vector<double>& inputs, const double learning_rate) {
        for (auto& layer : layers) {
            layer->update_parameters(inputs, learning_rate);
        }
    }

    void train(const std::vector<std::vector<double>>& inputs, const std::vector<size_t>& labels, const size_t num_epochs, const double learning_rate, const size_t batch_size) {
        for (size_t epoch_num = 0; epoch_num < num_epochs; epoch_num++) {
            size_t batch_start = rand() % (inputs.size() - batch_size);
            for (size_t b = batch_start; b < batch_start + batch_size; b++) {
                feed_forward(inputs[b]);
                back_propagate(labels[b]);
                update_parameters(inputs[b], learning_rate);
            }
            if (num_epochs < 10 or epoch_num % (num_epochs / 10) == 0 or epoch_num == 1 or epoch_num == num_epochs) {
                std::cout << "Epoch " << epoch_num << "/" << num_epochs << std::flush << " - ";
                test(inputs, labels);
            }
        }
    }

    void test(const std::vector<std::vector<double>>& inputs, const std::vector<size_t>& labels) {
        size_t num_correct = 0;
        for (size_t i = 0; i < inputs.size(); i++) {
            if (predict(inputs[i]) == labels[i]) {
                num_correct++;
            }
        }
        std::cout << "Accuracy: " << static_cast<double>(num_correct) / inputs.size() << '%' << std::endl;
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
            layers[l]->save(layer_path);
        }
    }

    void load(const std::string& model_path) {
        if (not std::filesystem::exists(model_path)) {
            throw std::runtime_error("Folder '" + model_path + "' does not exist.");
        }
        for (size_t l = 0; l < layers.size(); l++) {
            std::string layer_path = model_path + "/layer_" + std::to_string(l);
            layers[l]->load(layer_path);
        }
    }
};