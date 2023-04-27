#pragma once

#include "Layer.hpp"

enum class PoolingType : char {
    Max,
    Average,
    Min,
};

struct Dimensions {
    size_t width;
    size_t height;
};

struct PoolingLayer : public Layer {

    PoolingType pooling_type;
    Dimensions input_dimensions;
    Dimensions output_dimensions;
    size_t stride;
    std::vector<double> outputs;

    PoolingLayer(const PoolingType pooling_type, const Dimensions& input_dimensions, const size_t stride) : pooling_type(pooling_type), input_dimensions(input_dimensions), output_dimensions({ input_dimensions.width / stride, input_dimensions.height / stride }), stride(stride), outputs(output_dimensions.width * output_dimensions.height) {}

    const std::vector<double>& calc_outputs(const std::vector<double>& inputs) override {
        if (inputs.size() != input_dimensions.width * input_dimensions.height) {
            throw std::runtime_error("Number of inputs does not match number of neurons");
        }
        outputs.resize(output_dimensions.width * output_dimensions.height);
        for (size_t i = 0; i < output_dimensions.width; i++) {
            for (size_t j = 0; j < output_dimensions.height; j++) {
                double value;
                switch (pooling_type) {
                    case PoolingType::Max:
                        value = std::numeric_limits<double>::lowest();
                        break;
                    case PoolingType::Average:
                        value = 0.0;
                        break;
                    case PoolingType::Min:
                        value = std::numeric_limits<double>::max();
                        break;
                    default:
                        throw std::runtime_error("Invalid pooling type");
                        break;
                }
                for (size_t k = 0; k < stride; k++) {
                    for (size_t l = 0; l < stride; l++) {
                        size_t index = (i * stride + k) * input_dimensions.height + (j * stride + l);
                        switch (pooling_type) {
                            case PoolingType::Max:
                                value = std::max(value, inputs[index]);
                                break;
                            case PoolingType::Average:
                                value += inputs[index];
                                break;
                            case PoolingType::Min:
                                value = std::min(value, inputs[index]);
                                break;
                            default:
                                throw std::runtime_error("Invalid pooling type");
                                break;
                        }
                    }
                }
                if (pooling_type == PoolingType::Average) {
                    value /= stride * stride;
                }
                outputs[i * output_dimensions.height + j] = value;
            }
        }
        return outputs;
    }

    void calc_deltas(const std::vector<double>& errors) override {
        throw std::runtime_error("Pooling layer does not support backpropagation");
    }

    void update_parameters(const std::vector<double>& inputs, const double learning_rate) override {
        throw std::runtime_error("Pooling layer does not support backpropagation");
    }
};