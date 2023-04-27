#pragma once

#include "DenseLayer.hpp"

#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>

enum class LayerType : char {
    Dense,
    Convolutional,
    Pooling,
};

struct LayerBuilder {
    LayerType type;
    size_t size;
    ActivationType activation_type;    
};

struct NeuralNetwork {

    //std::vector<std::unique_ptr<Layer>> layers;
    std::vector<std::unique_ptr<DenseLayer>> layers; // temp
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

    void save(std::string model_path) {
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
            std::filesystem::create_directory(layer_path);
            for (size_t n = 0; n < layers[l]->neurons.size(); n++) {
                std::string neuron_path = layer_path + "/neuron_" + std::to_string(n);
                std::filesystem::create_directory(neuron_path);
                std::ofstream weights_file(neuron_path + "/weights.txt");
                for (auto weight : layers[l]->neurons[n].weights) {
                    weights_file << weight << '\n';
                }
                std::ofstream bias_file(neuron_path + "/bias.txt");
                bias_file << layers[l]->neurons[n].bias << '\n';
                bias_file.close();
            }
        }
    }

    void load(std::string model_path) {
        if (not std::filesystem::exists(model_path)) {
            throw std::runtime_error("Folder '" + model_path + "' does not exist.");
        }
        for (size_t l = 0; l < layers.size(); l++) {
            std::string layer_path = model_path + "/layer_" + std::to_string(l);
            for (size_t n = 0; n < layers[l]->neurons.size(); n++) {
                std::string neuron_path = layer_path + "/neuron_" + std::to_string(n);
                std::ifstream weights_file(neuron_path + "/weights.txt");
                for (auto& weight : layers[l]->neurons[n].weights) {
                    std::string line;
                    std::getline(weights_file, line);
                    weight = std::stod(line);
                }
                weights_file.close();
                std::ifstream bias_file(neuron_path + "/bias.txt");
                std::string line;
                std::getline(bias_file, line);
                layers[l]->neurons[n].bias = std::stod(line);
                bias_file.close();
            }
        }
    }
};