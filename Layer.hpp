#pragma once

#include "Neuron.hpp"

struct Layer {

    std::vector<double> outputs;

    virtual const std::vector<double>& calc_outputs(const std::vector<double>& inputs) = 0;

    virtual void calc_deltas(const std::vector<double>& errors) = 0;

    virtual void update_parameters(const std::vector<double>& inputs, const double learning_rate) = 0;

    virtual void save(const std::string& layer_path) = 0;

    virtual void load(const std::string& layer_path) = 0;
};