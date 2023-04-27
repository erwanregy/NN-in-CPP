#pragma once

#include "Activation.hpp"

#include <random>
#include <functional>

std::default_random_engine generator;
std::normal_distribution normal_distribution(0.0, 0.5);

struct Neuron {
    
    std::vector<double> weights;
    double bias;
    struct {
        std::function<double(double)> function;
        std::function<double(double)> derivative;
    } activation;
    double output;
    double delta;

    Neuron() : weights(), bias(0.0), activation(), output(0.0), delta(0.0) {}

    Neuron(const size_t num_inputs, const ActivationType activation_type) : weights(num_inputs), bias(0.0), activation(), output(0.0), delta(0.0) {
        for (auto& weight : weights) {
            weight = normal_distribution(generator);
        }
        switch (activation_type) {
            case ActivationType::Linear:
                activation = {
                    [](double x) { return x; },
                    [](double x) { return 1.0; }
                };
                break;
            case ActivationType::Sigmoid:
                activation = {
                    [](double x) { return 1.0 / (1.0 + exp(-x)); },
                    [](double x) { return x * (1.0 - x); }
                };
                break;
            case ActivationType::ReLU:
                activation = {
                    [](double x) { return x > 0.0 ? x : 0.0; },
                    [](double x) { return x > 0.0 ? 1.0 : 0.0; }
                };
                break;
            default:
                throw std::runtime_error("Invalid activation type");
                break;
        }
    }

    const double& calc_output(const std::vector<double>& inputs) {
        if (inputs.size() != weights.size()) {
            throw std::runtime_error("Number of inputs does not match number of weights");
        }
        double sum = bias;
        for (size_t i = 0; i < inputs.size(); i++) {
            sum += weights[i] * inputs[i];
        }
        return output = activation.function(sum);
    }

    void calc_delta(const double error) {
        delta = error * activation.derivative(output);
    }

    void update_parameters(const std::vector<double>& inputs, double learning_rate) {
        if (inputs.size() != weights.size()) {
            throw std::runtime_error("Number of inputs does not match number of weights");
        }
        for (size_t i = 0; i < inputs.size(); i++) {
            weights[i] -= learning_rate * delta * inputs[i];
        }
        bias -= learning_rate * delta;
    }
};