#pragma once

#include "Vector2.hpp"

#include <Windows.h>
#include <exception>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>

bool in_range(const Vector2<int>& coordinate, const Vector2<int>& dimensions) {
	return coordinate >= Vector2<int>(0, 0) and coordinate < dimensions;
}

int coordinate_to_index(const Vector2<int>& coordinate, const int width) {
	return coordinate.y * width + coordinate.x;
}

enum class Colour : WORD {
	Black = 0x0000,
	DarkBlue = 0x0001,
	DarkGreen = 0x0002,
	DarkCyan = 0x0003,
	DarkRed = 0x0004,
	Purple = 0x0005,
	Brown = 0x0006,
	LightGrey = 0x0007,
	DarkGrey = 0x0008,
	Blue = 0x0009,
	Green = 0x000A,
	Cyan = 0x000B,
	Red = 0x000C,
	Magenta = 0x000D,
	Yellow = 0x000E,
	White = 0x000F,
};

enum class Shade : WCHAR {
	Empty = 0x0020,
	Quarter = 0x2591,
	Half = 0x2592,
	ThreeQuarters = 0x2593,
	Full = 0x2588,
};

class Pixel {

private:
	
	Colour colour;
	Shade shade;

public:

	Pixel() : colour(Colour::White), shade(Shade::Full) {}
	Pixel(const Colour colour) : colour(colour), shade(Shade::Full) {}
	Pixel(const Shade shade) : colour(Colour::White), shade(shade) {}
	Pixel(const Colour colour, const Shade shade) : colour(colour), shade(shade) {}

	Colour get_colour() const {
        return colour;
    }
	Shade get_shade() const {
        return shade;
    }

	friend std::wostream& operator<<(std::wostream& stream, const Pixel& pixel) {
		return stream << static_cast<unsigned short>(pixel.colour) << L' ' << static_cast<wchar_t>(pixel.shade);
	}
	friend std::wistream& operator>>(std::wistream& stream, Pixel& pixel) {
		int colour, shade;
		stream >> colour >> shade;
		pixel = Pixel(Colour(colour), Shade(shade));
		return stream;
	}
};

class Sprite {

private:

	Vector2<int> dimensions;
	std::vector<Pixel> texture;

public:

	Sprite() : dimensions({ 0, 0 }), texture() {}
	Sprite(Vector2<int> dimensions) : dimensions(dimensions), texture(dimensions.x* dimensions.y) {}
	Sprite(std::string filename) {
		load(filename);
	}

	const Vector2<int>& get_dimensions() const {
		return dimensions;
	}
	int get_width() const {
        return dimensions.x;
    }
	int get_height() const {
        return dimensions.y;
    }
	Pixel get_pixel(const Vector2<int>& coordinate) const {
		if (in_range(coordinate, dimensions)) {
			return texture[coordinate_to_index(coordinate, dimensions.x)];
		} else {
			return { Colour::White, Shade::Empty };
		}
	}

	void save(const std::string& filename) const {
		std::wofstream filestream(filename, std::ios::out | std::ios::trunc);
		if (not filestream.is_open()) {
			throw std::runtime_error("Unable to open file '" + filename + "'");
		}
		filestream << dimensions;
		for (const Pixel& pixel : texture) {
			filestream << ' ' << pixel;
		}
		filestream.close();
	}
	void load(const std::string& filename) {
		std::wifstream filestream(filename, std::ios::in);
		if (not filestream.is_open()) {
			throw std::runtime_error("Unable to open file '" + filename + "'");
		}
		filestream >> dimensions;
		texture = std::vector<Pixel>(dimensions.x * dimensions.y);
		for (Pixel& pixel : texture) {
			filestream >> pixel;
		}
		filestream.close();
	}
};

class Entity {

	Sprite sprite;
	Vector2<int> size;
	Vector2<double> position;
	Vector2<double> velocity;
	Vector2<double> acceleration;

public:

	Entity() : sprite(), size({ 0, 0 }), position({ 0, 0 }), velocity({ 0, 0 }), acceleration({ 0, 0 }) {}
	Entity(const std::string& filename) : sprite(filename), size({ 0, 0 }), position({ 0, 0 }), velocity({ 0, 0 }), acceleration({ 0, 0 }) {}

	const Sprite& get_sprite() const {
		return sprite;
	}
	const Vector2<double>& get_position() const {
		return position;
	}

	void set_position(const Vector2<double>& position) {
		this->position = position;
	}

	void update(const double time) {
		position += velocity * time;
		velocity += acceleration * time;
	}
};

class ConsoleGraphicsEngine {

private:

	class Timer {
		struct {
			std::chrono::high_resolution_clock::time_point start, finish;
		} time;
	public:
		void start() {
			time.start = std::chrono::high_resolution_clock::now();
		}
		void stop() {
			time.finish = std::chrono::high_resolution_clock::now();
		}
		double elapsed() {
			return std::chrono::duration_cast<std::chrono::duration<double>>(time.finish - time.start).count();
		}
		void reset() {
			time.start = time.finish;
		}
	} timer;

protected:

	enum class ButtonState : char {
		Released,
		Pressed,
		Held,
	};

	// enum class MouseWheelState : char {
	// 	Stationary,
	// 	Up,
	// 	Down,
	// };

	enum class Key : char {
		Backspace = VK_BACK,
		Tab = VK_TAB,
		Enter = VK_RETURN,
		Shift = VK_SHIFT,
		Control = VK_CONTROL,
		Alt = VK_MENU,
		CapsLock = VK_CAPITAL,
		Esc = VK_ESCAPE,
		Space = VK_SPACE,
		LeftArrow = VK_LEFT,
		UpArrow = VK_UP,
		RightArrow = VK_RIGHT,
		DownArrow = VK_DOWN,
		// Comma = ?,
		// Period = ?,
		Delete = VK_DELETE,
	};

	enum class MouseButton : char {
		Left = VK_LBUTTON,
		Right = VK_RBUTTON,
		Middle = VK_MBUTTON,
	};

private:

	const Vector2<int> screen_dimensions;

	const Vector2<int> font_dimensions;

	const std::wstring title;

	const struct {
		HANDLE output, input, original;
	} console;

	std::vector<CHAR_INFO> buffer;

	bool running;

	std::vector<Sprite>	sprites;

	Vector2<int> mouse_position;

public:

	ConsoleGraphicsEngine(const Vector2<int>& screen_dimensions = { 80, 30 }, const Vector2<int>& font_dimensions = { 1, 1 }, const std::wstring& title = L"Console Graphics Engine")
	: screen_dimensions(screen_dimensions), font_dimensions(font_dimensions), title(title), buffer(screen_dimensions.x* screen_dimensions.y), console({ GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_INPUT_HANDLE) }), running(false), sprites(), prev_mouse_position(0, 0) {
		if (console.output == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Failed to get output console handle");
		}

		if (console.input == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Failed to get input console handle");
		}

		CONSOLE_CURSOR_INFO cursor_info;
		if (not GetConsoleCursorInfo(console.output, &cursor_info)) {
			throw std::runtime_error("Failed to get console cursor info");
		}
		cursor_info.bVisible = FALSE;
		if (not SetConsoleCursorInfo(console.output, &cursor_info)) {
			throw std::runtime_error("Failed to set console cursor info");
		}

		SMALL_RECT window_region = { 0, 0, 1, 1 };
		if (not SetConsoleWindowInfo(console.output, TRUE, &window_region)) {
			throw std::runtime_error("Failed to set console window info");
		}

		if (not SetConsoleScreenBufferSize(console.output, { static_cast<short>(screen_dimensions.x), static_cast<short>(screen_dimensions.y) })) {
			throw std::runtime_error("Failed to set console screen_dimensions buffer screen_dimensions");
		}

		if (not SetConsoleActiveScreenBuffer(console.output)) {
			throw std::runtime_error("Failed to set console active screen_dimensions buffer");
		}

		CONSOLE_FONT_INFOEX font_info = {
			sizeof(CONSOLE_FONT_INFOEX),
			0,
            { static_cast<short>(font_dimensions.x), static_cast<short>(font_dimensions.y) },
			FF_DONTCARE,
			0,
			L""
		};
		if (not SetCurrentConsoleFontEx(console.output, FALSE, &font_info)) {
			throw std::runtime_error("Failed to set console font");
		}

		CONSOLE_SCREEN_BUFFER_INFO screen_info;
		if (not GetConsoleScreenBufferInfo(console.output, &screen_info)) {
			throw std::runtime_error("Failed to get console screen_dimensions buffer info");
		} else {
			Vector2<int> window_dimensions = Vector2<int>(screen_info.dwMaximumWindowSize.X, screen_info.dwMaximumWindowSize.Y);
			if (screen_dimensions > window_dimensions) {
				throw std::runtime_error("Screen dimensions are too large, maximum dimensions allowed are '" + (std::string)window_dimensions + "'");
			}
		}

		window_region = { 0, 0, static_cast<short>(screen_dimensions.x - 1), static_cast<short>(screen_dimensions.y - 1) };
		if (not SetConsoleWindowInfo(console.output, TRUE, &window_region)) {
			throw std::runtime_error("Failed to set console window info");
		}

		if (not SetConsoleMode(console.input, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)) {
			throw std::runtime_error("Failed to set console mode");
		}

		if (not SetConsoleTitleW(title.c_str())) {
			throw std::runtime_error("Failed to set console title");
		}

		if (not SetConsoleCtrlHandler((PHANDLER_ROUTINE)close_handler, TRUE)) {
			throw std::runtime_error("Failed to set console control handler");
		}
	}

	void start() {
		timer.start();

		initialise();

		running = true;

		while (running) {
			timer.stop();
			double time_elapsed = timer.elapsed();
			double frame_rate = 1.0 / time_elapsed;
			timer.reset();

			update(time_elapsed);

			render(frame_rate);
		}

		close();
	}

protected:

	virtual void initialise() {}

	virtual void update(const double time_elapsed) {}

	virtual void close() {}

	void stop() {
		running = false;
		SetConsoleActiveScreenBuffer(console.original);
	}

private:

	void render(const double frame_rate) {
		if (not SetConsoleTitleW((title + L" - FPS: " + std::to_wstring(frame_rate)).c_str())) {
			throw std::runtime_error("Failed to set console title");
		}

		SMALL_RECT window_region = { 0, 0, static_cast<short>(screen_dimensions.x - 1), static_cast<short>(screen_dimensions.y - 1) };
		if (not WriteConsoleOutputW(console.output, buffer.data(), { static_cast<short>(screen_dimensions.x), static_cast<short>(screen_dimensions.y) }, { 0, 0 }, &window_region)) {
			throw std::runtime_error("Failed to draw to console");
		}
	}

protected:

	const Vector2<int>& get_screen_dimensions() const {
		return screen_dimensions;
	}

	int get_screen_width() const {
		return screen_dimensions.x;
	}

	int get_screen_height() const {
		return screen_dimensions.y;
	}

	ButtonState get_key(Key key) const {
		return get_button(static_cast<char>(key));
	}

	ButtonState get_key(const char key) const {
		if ((key >= 'A' and key <= 'Z') or (key >= '0' and key <= '9')) {
			return get_button(key);
		} else {
			throw std::runtime_error("Invalid key requested '" + key + '\'');
		}
	}

	ButtonState get_mouse_button(const MouseButton mouse_button) const {
		return get_button(static_cast<const char>(mouse_button));
	}

	const Vector2<int>& get_mouse_position() {
		const auto& input_records = get_input_record();
		for (const auto& input_record : input_records) {
			if (input_record.EventType == MOUSE_EVENT and input_record.Event.MouseEvent.dwEventFlags == MOUSE_MOVED) {
				const auto& position = input_record.Event.MouseEvent.dwMousePosition;
				mouse_position = Vector2<int>(position.X, position.Y);
			}
		}
		return mouse_position;
	}

	int get_mouse_x() {
		return get_mouse_position().x;
	}

	int get_mouse_y() {
		return get_mouse_position().y;
	}

	// MouseWheelState get_mouse_wheel() {
	// 	const auto& input_records = get_input_record();
	// 	for (const auto& input_record : input_records) {
	// 		if (input_record.EventType == MOUSE_EVENT and input_record.Event.MouseEvent.dwEventFlags == MOUSE_WHEELED) {
	// 			const auto& wheel = input_record.Event.MouseEvent.dwButtonState;
	// 			if (HIWORD(wheel) > 0) {
    //                 return MouseWheelState::Up;
	// 			} else {
    //                 return MouseWheelState::Down;
    //             }
	// 		}
	// 	}
	// 	return MouseWheelState::Stationary;
	// }

private:

	ButtonState get_button(const char button) const {
		auto state = GetAsyncKeyState(button);
		if (state & 0x8000) {
			if (state & 0x0001) {
				return ButtonState::Pressed;
			} else {
				return ButtonState::Held;
			}
		} else {
			return ButtonState::Released;
		}
	}

	const std::vector<INPUT_RECORD> get_input_record() const {
		DWORD num_events = 0;
		if (not GetNumberOfConsoleInputEvents(console.input, &num_events)) {
			throw std::runtime_error("Failed to get number of console input events");
		}
		std::vector<INPUT_RECORD> input_records(num_events);
		if (num_events > 0) {
			ReadConsoleInputW(console.input, input_records.data(), num_events, &num_events);
		}
		return input_records;
	}

	Vector2<int> prev_mouse_position = Vector2<int>(0, 0);

protected:

	// Drawing functions

	void clear_screen(const Pixel& pixel = { Colour::Black, Shade::Empty }) {
		for (int x = 0; x < screen_dimensions.x; x++) {
			for (int y = 0; y < screen_dimensions.y; y++) {
				draw_pixel(Vector2<int>(x, y), pixel);
			}
		}
	}

	void draw_character(const Vector2<int>& coordinate, const wchar_t character, const Colour colour = Colour::White) {
		if (in_range(coordinate, screen_dimensions)) {
			const size_t index = coordinate_to_index(coordinate, screen_dimensions.x);
			buffer[index].Char.UnicodeChar = character;
			buffer[index].Attributes = static_cast<WORD>(colour);
		}
	}

	void draw_pixel(const Vector2<int>& coordinate, const Pixel& pixel = Pixel()) {
		draw_character(coordinate, static_cast<wchar_t>(pixel.get_shade()), pixel.get_colour());
	}

	void draw_sprite(const Vector2<int>& coordinate, const Sprite& sprite, const unsigned int scale = 1.0) {
		const Vector2<int>& dimensions = sprite.get_dimensions() * scale;
		for (int x = 0; x < dimensions.x; x++) {
			for (int y = 0; y < dimensions.y; y++) {
				const Pixel& pixel = sprite.get_pixel(Vector2<int>(x, y) / scale);
				if (pixel.get_shade() != Shade::Empty) {
					draw_pixel(coordinate + Vector2<int>(x, y), pixel);
				}
			}
		}
	}

	void draw_entity(const Entity& entity) {
		draw_sprite(Vector2<int>((int)entity.get_position().x, (int)entity.get_position().y), entity.get_sprite());
	}

	void draw_string(const Vector2<int>& coordinate, const std::wstring& string, Colour colour = Colour::White) {
		for (unsigned int i = 0; i < string.length(); i++) {
			draw_character(coordinate + Vector2<int>(i, 0), string[i], colour);
		}
	}

	void draw_line(const Vector2<int>& start, const Vector2<int>& end, const Pixel& pixel = Pixel()) {
		Vector2<int> current = start;
		Vector2<int> delta = end - start;
		Vector2<int> step = { (delta.x > 0) - (delta.x < 0), (delta.y > 0) - (delta.y < 0) };
		auto abs = [](int x) { return x > 0 ? x : -x; };
		delta = { abs(delta.x), abs(delta.y) };

		if (delta.x > delta.y) {
			int error = delta.x / 2;
			while (current.x != end.x) {
				draw_pixel(current, pixel);
				error -= delta.y;
				if (error < 0) {
					current.y += step.y;
					error += delta.x;
				}
				current.x += step.x;
			}
		} else {
			int error = delta.y / 2;
			while (current.y != end.y) {
				draw_pixel(current, pixel);
				error -= delta.x;
				if (error < 0) {
					current.x += step.x;
					error += delta.y;
				}
				current.y += step.y;
			}
		}
	}

	void draw_triangle(const Vector2<int> coordinates[3], const Pixel& pixel = Pixel()) {
		draw_line(coordinates[0], coordinates[1], pixel);
		draw_line(coordinates[1], coordinates[2], pixel);
		draw_line(coordinates[2], coordinates[0], pixel);
	}

	void draw_filled_triangle(const Vector2<int> coordinates[3], const Pixel& pixel = Pixel()) {
		throw std::runtime_error("To be implemented");
		// TODO
	}

	void draw_circle(const Vector2<int>& centre, const int radius = 1, const Pixel& pixel = Pixel()) {
		Vector2<int> current = { 0, radius };
        int p = 1 - radius;
		while (current.x <= current.y) {
            draw_pixel(centre + current, pixel);
            draw_pixel(centre - current, pixel);
            draw_pixel(centre + Vector2<int>(current.y, current.x), pixel);
            draw_pixel(centre - Vector2<int>(current.y, current.x), pixel);
            draw_pixel(centre + Vector2<int>(current.x, current.y), pixel);
            draw_pixel(centre - Vector2<int>(current.x, current.y), pixel);
            draw_pixel(centre + Vector2<int>(current.x, -current.y), pixel);
            draw_pixel(centre - Vector2<int>(current.x, -current.y), pixel);
			if (p < 0) {
                p += 2 * current.x + 3;
			} else {
                p += 2 * (current.x - current.y) + 5;
                current.y--;
            }
            current.x++;
        }
	}

	void draw_filled_circle(const Vector2<int>& centre, const int radius = 1, const Pixel& pixel = Pixel()) {
		Vector2<int> current = { 0, radius };
        int p = 1 - radius;
		while (current.x <= current.y) {
            draw_line(centre + Vector2<int>(-current.y, current.x), centre + Vector2<int>(current.y, current.x), pixel);
            draw_line(centre + Vector2<int>(-current.x, current.y), centre + Vector2<int>(current.x, current.y), pixel);
            draw_line(centre + Vector2<int>(-current.x, -current.y), centre + Vector2<int>(current.x, -current.y), pixel);
            draw_line(centre + Vector2<int>(-current.y, -current.x), centre + Vector2<int>(current.y, -current.x), pixel);
			if (p < 0) {
                p += 2 * current.x + 3;
			} else {
                p += 2 * (current.x - current.y) + 5;
                current.y--;
            }
            current.x++;
        }
	}

	void draw_rectangle(const Vector2<int>& top_left, const Vector2<int>& bottom_right, const Pixel& pixel = Pixel()) {
        Vector2<int> current;
		for (current.x = top_left.x; current.x <= bottom_right.x; current.x++) {
			for (current.y = top_left.y; current.y <= bottom_right.y; current.y++) {
				if (current.x == top_left.x or current.x == bottom_right.x or
					current.y == top_left.y or current.y == bottom_right.y) {
                    draw_pixel(current, pixel);
                }
            }
        }
    }

	void draw_filled_rectangle(const Vector2<int>& top_left, const Vector2<int>& bottom_right, const Pixel& pixel = Pixel()) {
        Vector2<int> current;
		for (current.x = top_left.x; current.x <= bottom_right.x; current.x++) {
			for (current.y = top_left.y; current.y <= bottom_right.y; current.y++) {
                draw_pixel(current, pixel);
            }
        }
    }

	void draw_polygon(const std::vector<Vector2<int>>& vertices, const Pixel& pixel = Pixel()) {
		for (unsigned int i = 0; i < vertices.size(); i++) {
			draw_line(vertices[i], vertices[(i + 1) % vertices.size()], pixel);
		}
		draw_line(vertices[vertices.size() - 1], vertices[0], pixel);
    }

private:

	static bool close_handler(DWORD event) {
		if (event == CTRL_CLOSE_EVENT) {
			return true;
		} else {
			return false;
		}
	}
};