#pragma once

#include "Layer.hpp"

struct DenseLayer : public Layer {

    std::vector<Neuron> neurons;
    ActivationType activation_type;
    std::vector<double> outputs;

    DenseLayer(const size_t num_inputs, const size_t num_neurons, const ActivationType activation_type) : neurons(num_neurons), activation_type(activation_type), outputs(num_neurons) {
        for (auto& neuron : neurons) {
            neuron = Neuron(num_inputs, activation_type);
        }
    }

    const std::vector<double>& calc_outputs(const std::vector<double>& inputs) override {
        for (size_t n = 0; n < neurons.size(); n++) {
            outputs[n] = neurons[n].calc_output(inputs);
        }
        return outputs;
    }

    void calc_deltas(const std::vector<double>& errors) override {
        for (size_t i = 0; i < neurons.size(); i++) {
            neurons[i].calc_delta(errors[i]);
        }
    }

    void update_parameters(const std::vector<double>& inputs, const double learning_rate) override {
        for (auto& neuron : neurons) {
            neuron.update_parameters(inputs, learning_rate);
        }
    }

    void save(const std::string& layer_path) override {
        std::filesystem::create_directory(layer_path);
        for (size_t n = 0; n < neurons.size(); n++) {
            std::string neuron_path = layer_path + "/neuron_" + std::to_string(n);
            neurons[n].save(neuron_path);
        }
    }

    void load(const std::string& layer_path) override {
        if (not std::filesystem::exists(layer_path)) {
            throw std::runtime_error("Folder '" + layer_path + "' does not exist");
        }
        for (size_t n = 0; n < neurons.size(); n++) {
            std::string neuron_path = layer_path + "/neuron_" + std::to_string(n);
            neurons[n].load(neuron_path);
        }
    }
};