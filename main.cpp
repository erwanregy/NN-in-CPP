#include "NeuralNetwork.hpp"
#include "mnist.hpp"

#include "ConsoleGraphicsEngine.hpp"


const int scale = 3;


class InteractiveNeuralNetwork : public ConsoleGraphicsEngine {

    NeuralNetwork neural_network;
    std::vector<input_t> example_inputs;
    input_t inputs;
    int brush_size;

public:

    using ConsoleGraphicsEngine::ConsoleGraphicsEngine;

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

        example_inputs = extract_inputs("data/mnist/test_images.idx3-ubyte", 1000);

        inputs = std::vector<double>(784, 0.0);

        brush_size = 1;
    }

    void brush(const double frame_time) {
        const Coordinate& mouse_position = get_mouse_position() / scale;
        Coordinate current;
        for (current.x = -brush_size; current.x <= brush_size; current.x++) {
            for (current.y = -brush_size; current.y <= brush_size; current.y++) {
                Coordinate coordinate = mouse_position + current;
                if (current.x * current.x + current.y * current.y <= brush_size * brush_size and coordinate.in_bounds({ 28, 28 })) {
                    double& input = inputs[coordinate.to_index(28)];
                    input += 100.0 * frame_time;
                    if (input > 1.0) {
                        input = 1.0;
                    }
                }
            }
        }
    }

    void erase(const double frame_time) {
        const Coordinate& mouse_position = get_mouse_position() / scale;
        Coordinate current;
        for (current.x = -brush_size; current.x <= brush_size; current.x++) {
            for (current.y = -brush_size; current.y <= brush_size; current.y++) {
                Coordinate coordinate = mouse_position + current;
                if (current.x * current.x + current.y * current.y <= brush_size * brush_size and coordinate.in_bounds({ 28, 28 })) {
                    double& input = inputs[coordinate.to_index(28)];
                    input -= 100.0 * frame_time;
                    if (input < 0.0) {
                        input = 0.0;
                    }
                }
            }
        }
    }

    void update(const double frame_time) override {
        clear_screen();
        
        if (get_key(Key::UpArrow) == ButtonState::Pressed and brush_size < 5) {
            brush_size++;
        }
        if (get_key(Key::DownArrow) == ButtonState::Pressed and brush_size > 0) {
            brush_size--;
        }

        if (get_mouse_button(MouseButton::Left) == ButtonState::Held) {
            brush(frame_time);
        }
        if (get_mouse_button(MouseButton::Right) == ButtonState::Held) {
            erase(frame_time);
        }
        if (get_key(Key::Space) == ButtonState::Pressed) {
            std::fill(inputs.begin(), inputs.end(), 0.0);
        }
        if (get_key(Key::Escape) == ButtonState::Pressed) {
            stop();
        }
        if (get_key(Key::Enter) == ButtonState::Pressed) {
            clear_screen();
            inputs = example_inputs[rand() % example_inputs.size()];
        }

        for (Coordinate coordinate = { 0, 0 }; coordinate.x < 28; coordinate.x++) {
            for (coordinate.y = 0; coordinate.y < 28; coordinate.y++) {
                const size_t index = coordinate.to_index(28);
                const auto& input = inputs[index];
                draw_filled_rectangle(coordinate * scale, coordinate * scale + Coordinate(scale, scale), Pixel(input));
            }
        }

        if (get_mouse_position().in_bounds({ 28 * scale, 28 * scale })) {
            draw_circle(get_mouse_position(), brush_size * scale, Pixel::Colour::Blue);
        }

        neural_network.feed_forward(inputs);

        std::wstring message = L"I see ";
        const size_t prediction = neural_network.prediction();
        const double confidence = neural_network.outputs[prediction];
        if (confidence < 0.7) {
            message += L"...";
        } else {
            message += L"a " + std::to_wstring(prediction) + L'!';
        }
        // Print prediction at top of right side of drawing area
        draw_string({ (28 + 1) * scale, 0 }, message);
        
        // for (Coordinate coordinate = { 0, 0 }; coordinate.y <= 9; coordinate.y++) {
        //     std::wstring bar = L"";
        //     for (coordinate.x = 1; coordinate.x < neural_network.outputs[coordinate.x] * (get_screen_width() - 3); coordinate.x++) {
        //         bar += static_cast<wchar_t>(Pixel::Shade::Full);
        //     }
        //     draw_string({ 0, get_screen_height() - ((9 - coordinate.x) * scale.y) }, std::to_wstring(coordinate.x) + L' ' + bar);
        // }

        // Draw border along right side of drawing area
        draw_line({ 28 * scale, 0 }, { 28 * scale, 28 * scale }, Pixel(0.2));

        // Print predictions along right side of drawing area
        for (int i = 0; i <= 9; i++) {
            Coordinate bottom = { (28 + 1 + i) * scale, get_screen_height() - 1 };

            double confidence = neural_network.outputs[i];

            // draw bar chart of confidence
            draw_line(bottom, bottom + Coordinate(0, static_cast<int>(-confidence * (get_screen_height() - 1))));

            // draw number at bottom right
            draw_character(bottom, i + '0');
        }
    }
};


int main() {
    Coordinate grid_size = { 28 + 1 + 10, 28 };
    InteractiveNeuralNetwork inn(grid_size * scale, { 20 / scale, 20 / scale }, L"Interactive Neural Network Demo");
    inn.start();
}