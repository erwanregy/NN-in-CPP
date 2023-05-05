#pragma once

#include "Layer.hpp"

enum class PoolingType : char {
    Max,
    Average,
    Min,
};

struct PoolingLayer : public Layer {};