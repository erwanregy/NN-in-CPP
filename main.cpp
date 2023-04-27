#include "NeuralNetwork.hpp"
#include "mnist.hpp"

#include "ConsoleGameEngine.hpp"

class InteractiveNeuralNetwork : public ConsoleGameEngine {

    NeuralNetwork neural_network;
    std::vector<double> inputs;
    unsigned short brush_size;

public:

    using ConsoleGameEngine::ConsoleGameEngine;

    void initialise() override {
        neural_network = NeuralNetwork(
            784,
            {
                { LayerType::Dense, 16, ActivationType::ReLU },
                { LayerType::Dense, 16, ActivationType::ReLU },
                { LayerType::Dense, 10, ActivationType::Sigmoid },
            }
        );

        neural_network.load("models/3b1b");

        inputs = std::vector<double>(784, 0.0);

        brush_size = 1;
    }

    void brush(const unsigned short brush_size, const double time_elapsed) {
        const Coordinate& mouse_position = get_mouse_position();
        Coordinate current;
        for (current.x = -brush_size; current.x <= brush_size; current.x++) {
            for (current.y = -brush_size; current.y <= brush_size; current.y++) {
                if (current.magnitude() <= brush_size and in_range(mouse_position + current, { 28, 28 })) {
                    double& input = inputs[coordinate_to_index(mouse_position + current, 28)];
                    input += 100.0 * time_elapsed;
                    if (input > 1.0) {
                        input = 1.0;
                    }
                }
            }
        }
    }

    void erase(const unsigned short brush_size, const double time_elapsed) {
        const Coordinate& mouse_position = get_mouse_position();
        Coordinate current;
        for (current.x = -brush_size; current.x <= brush_size; current.x++) {
            for (current.y = -brush_size; current.y <= brush_size; current.y++) {
                if (current.magnitude() <= brush_size and in_range(mouse_position + current, { 28, 28 })) {
                    double& input = inputs[coordinate_to_index(mouse_position + current, 28)];
                    input -= 100.0 * time_elapsed;
                    if (input < 0.0) {
                        input = 0.0;
                    }
                }
            }
        }
    }

    void update(const double time_elapsed) override {
        clear_screen();
        
        if (get_key(UpArrow) == Pressed and brush_size < 2) {
            brush_size++;
        }
        if (get_key(DownArrow) == Pressed and brush_size > 0) {
            brush_size--;
        }

        if (get_mouse_button(Left) == Held) {
            brush(brush_size, time_elapsed);
        }
        if (get_mouse_button(Right) == Held) {
            erase(brush_size, time_elapsed);
        }
        if (get_key(Space) == Pressed) {
            inputs = std::vector<double>(784, 0.0);
        }
        if (get_key(Esc) == Pressed) {
            stop();
        }

        for (short y = 0; y < 28; y++) {
            for (short x = 0; x < 28; x++) {
                const auto& input = inputs[y * 28 + x];
                Pixel pixel;
                if (input < 0.33) {
                    pixel = { Black, Full };
                } else if (input < 0.66) {
                    pixel = { DarkGrey, Full };
                } else if (input < 0.99) {
                    pixel = { LightGrey, Full };
                } else {
                    pixel = { White, Full };
                }
                draw_pixel({ x, y }, pixel);
            }
        }

        if (in_range(get_mouse_position(), { 28, 28 })) {
            draw_circle(get_mouse_position(), brush_size, Blue);
        }

        neural_network.feed_forward(inputs);

        std::wstring message = L"I see ";
        size_t prediction = neural_network.prediction();
        double confidence = neural_network.outputs[prediction];
        if (confidence < 0.7) {
            message += L"...";
        } else {
            message += L"a " + std::to_wstring(prediction) + L'!';
        }
        draw_string({ 0, 28 }, message);

        for (short i = 0; i <= 9; i++) {
            std::wstring bar = L" ";
            for (short j = 1; j < neural_network.outputs[i] * (get_screen_width() - 2); j++) {
                bar += Full;
            }
            draw_string({ 0, 29 + i }, std::to_wstring(i) + bar);
        }
    }
};

int main() {
    InteractiveNeuralNetwork inn({ 28, 28 + 1 + 10 }, { 16, 16 }, L"Interactive Neural Network Demo");
    inn.start();
}