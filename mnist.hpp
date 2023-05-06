#pragma once

#include <array>
#include <string>
#include <cstdint>
#include <tuple>
#include <fstream>
#include <vector>
#include <iostream>

using input_t = std::vector<double>;
using pixel_t = uint8_t;
using image_t = std::array<std::array<pixel_t, 28>, 28>;
using label_t = uint8_t;

uint32_t swap_endianness(uint32_t word) {
    return ((word & 0xff) << 24) | ((word & 0xff00) << 8) | ((word & 0xff0000) >> 8) | ((word & 0xff000000) >> 24);
}

std::ifstream open_file(std::string path) {
    std::ifstream file(path, std::ios::binary);

    if (not file.is_open()) {
        throw std::runtime_error("Could not open file '" + path + "'");
    }

    return file;
}

template <size_t num_dimensions>
size_t extract_header(std::ifstream& file, size_t num) {
    uint32_t temp = 0;
    std::streamsize size = sizeof(temp);

    file.read(reinterpret_cast<char*>(&temp), size);

    file.read(reinterpret_cast<char*>(&temp), size);
    if (num == 0) {
        num = swap_endianness(temp);
    }

    for (size_t i = 0; i < num_dimensions - 1; i++) {
        file.read(reinterpret_cast<char*>(&temp), size);
    }

    return num;
}

std::vector<image_t> extract_images(std::string images_path, size_t num_images = 0) {
    std::ifstream file = open_file(images_path);

    num_images = extract_header<3>(file, num_images);

    std::vector<image_t> images(num_images);

    for (auto& image : images) {
        for (auto& row : image) {
            for (auto& pixel : row) {
                label_t value = 0;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                pixel = value;
            }
        }
    }

    return images;
}

std::vector<input_t> extract_inputs(std::string images_path, size_t num_images = 0) {
    std::ifstream file = open_file(images_path);

    num_images = extract_header<3>(file, num_images);

    std::vector<input_t> inputs(num_images, input_t(28 * 28));

    for (auto& input : inputs) {
        for (auto& pixel : input) {
            label_t value = 0;
            file.read(reinterpret_cast<char*>(&value), sizeof(value));
            pixel = static_cast<double>(value) / 255.0;
        }
    }

    return inputs;
}

std::tuple<std::vector<image_t>, std::vector<input_t>> extract_images_and_inputs(std::string images_path, size_t num_images = 0) {
    std::ifstream file = open_file(images_path);

    num_images = extract_header<3>(file, num_images);

    std::vector<image_t> images(num_images);
    std::vector<input_t> inputs(num_images, input_t(28 * 28));

    for (size_t image = 0; image < num_images; image++) {
        for (size_t row = 0; row < 28; row++) {
            for (size_t column = 0; column < 28; column++) {
                label_t value = 0;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                images[image][row][column] = value;
                inputs[image][row * 28 + column] = static_cast<double>(value) / 255.0;
            }
        }
    }

    return std::make_tuple(images, inputs);
}

std::vector<label_t> extract_labels(std::string labels_path, size_t num_labels = 0) {
    std::ifstream file = open_file(labels_path);

    num_labels = extract_header<1>(file, num_labels);

    std::vector<label_t> labels(num_labels);

    for (auto& label : labels) {
        label_t value = 0;
        file.read(reinterpret_cast<char*>(&value), sizeof(value));
        label = value;
    }

    return labels;
}

std::tuple<std::vector<image_t>, std::vector<input_t>, std::vector<label_t>> extract(std::string images_path, std::string labels_path, size_t num_images) {
    std::vector<image_t> images;
    std::vector<input_t> inputs;
    std::vector<label_t> labels;

    std::tie(images, inputs) = extract_images_and_inputs(images_path, num_images);
    labels = extract_labels(labels_path, num_images);

    return std::make_tuple(images, inputs, labels);
}

const std::string ascii_scale = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

void print_image(image_t image) {
    std::cout << '+';
    for (size_t i = 0; i < image[0].size(); i++) {
        std::cout << "--";
    }
    std::cout << "+\n";
    for (const auto& row : image) {
        std::cout << '|';
        for (const auto pixel : row) {
            size_t brightness = static_cast<size_t>((static_cast<double>(pixel) / 256.0) * ascii_scale.length());
            std::cout << ascii_scale[brightness] << ascii_scale[brightness];
        }
        std::cout << "|\n";
    }
    std::cout << '+';
    for (size_t i = 0; i < image[0].size(); i++) {
        std::cout << "--";
    }
    std::cout << "+\n" << std::flush;
}