#pragma once

#include "Vector2.hpp"

#include <Windows.h>
#include <exception>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <optional>

using Dimensions = Vector2<unsigned short>;
using Coordinate = Vector2<short>;
using Vector = Vector2<double>;

bool in_range(const Coordinate& coordinate, const Dimensions& dimensions) {
	return coordinate >= Coordinate(0, 0) and coordinate < Coordinate(dimensions.x, dimensions.y);
}

unsigned short coordinate_to_index(const Coordinate& coordinate, const unsigned short width) {
	return coordinate.y * width + coordinate.x;
}

enum Colour : unsigned short {
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

enum Shade : wchar_t {
	Empty = 0x0020,
	Quarter = 0x2591,
	Half = 0x2592,
	ThreeQuarters = 0x2593,
	Full = 0x2588,
};

struct Pixel {
	Colour colour;
	Shade shade;
	Pixel() : colour(White), shade(Full) {}
	Pixel(const Colour colour) : colour(colour), shade(Full) {}
	Pixel(const Shade shade) : colour(White), shade(shade) {}
	Pixel(const Colour colour, const Shade shade) : colour(colour), shade(shade) {}
	friend std::wostream& operator<<(std::wostream& stream, const Pixel& pixel) { return stream << pixel.colour << L' ' << pixel.shade; }
	friend std::wistream& operator>>(std::wistream& stream, Pixel& pixel) {
		short colour, shade;
		stream >> colour >> shade;
		pixel = Pixel(Colour(colour), Shade(shade));
		return stream;
	}
};

class Sprite {

	Dimensions dimensions;

	std::vector<Pixel> texture;

public:

	Sprite() : dimensions({ 0, 0 }), texture() {}

	Sprite(Dimensions dimensions) : dimensions(dimensions), texture(dimensions.x* dimensions.y) {}

	Sprite(std::string filename) {
		load(filename);
	}

	Dimensions get_dimensions() const {
		return dimensions;
	}

	Pixel get_pixel(const Coordinate& coordinate) const {
		if (in_range(coordinate, dimensions)) {
			return texture[coordinate_to_index(coordinate, dimensions.x)];
		} else {
			return Pixel(White, Empty);
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
public: // temp
	Dimensions size;
	Vector position;
	Vector velocity;
	Vector acceleration;
public:

	Entity() : sprite(), size({ 0, 0 }), position({ 0, 0 }), velocity({ 0, 0 }), acceleration({ 0, 0 }) {}

	Entity(const std::string& filename) : sprite(filename), size({ 0, 0 }), position({ 0, 0 }), velocity({ 0, 0 }), acceleration({ 0, 0 }) {}

	void set_position(const Vector& position) {
		this->position = position;
	}

	Vector get_position() const {
		return position;
	}

	Sprite get_sprite() const {
		return sprite;
	}

	void update(const double time) {
		position += velocity * time;
		velocity += acceleration * time;
	}
};

class ConsoleGameEngine {

protected:

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

	enum ButtonState : char {
		Released,
		Pressed,
		Held,
	};

	enum MouseWheelState : char {
		Stationary,
		Up,
		Down,
	};

	enum Key : char {
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

	enum MouseButton : char {
		Left = VK_LBUTTON,
		Right = VK_RBUTTON,
		Middle = VK_MBUTTON,
	};

private:

	const Dimensions screen_dimensions;

	const Dimensions font_dimensions;

	const std::wstring title;

	const struct {
		HANDLE output, input, original;
	} console;

	std::vector<CHAR_INFO> buffer;

	bool running;

	std::vector<Sprite>	sprites;

	SMALL_RECT window_region;

public:

	ConsoleGameEngine(const Dimensions& screen_dimensions = { 80, 30 }, const Dimensions& font_dimensions = { 1, 1 }, const std::wstring& title = L"Console Graphics Engine")
		: screen_dimensions(screen_dimensions), font_dimensions(font_dimensions), title(title), buffer(screen_dimensions.x* screen_dimensions.y), console({ GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_INPUT_HANDLE) }), running(false), sprites() {
		if (console.output == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Failed to get output console handle");
		}

		if (console.input == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Failed to get input console handle");
		}

		CONSOLE_CURSOR_INFO cursor_info;
		GetConsoleCursorInfo(console.output, &cursor_info);
		cursor_info.bVisible = FALSE;
		SetConsoleCursorInfo(console.output, &cursor_info);

		window_region = { 0, 0, 1, 1 };
		if (not SetConsoleWindowInfo(console.output, TRUE, &window_region)) {
			throw std::runtime_error("Failed to set console window info");
		}

		if (not SetConsoleScreenBufferSize(console.output, { (short)screen_dimensions.x, (short)screen_dimensions.y })) {
			throw std::runtime_error("Failed to set console screen_dimensions buffer screen_dimensions");
		}

		if (not SetConsoleActiveScreenBuffer(console.output)) {
			throw std::runtime_error("Failed to set console active screen_dimensions buffer");
		}

		CONSOLE_FONT_INFOEX font_info = {
			sizeof(CONSOLE_FONT_INFOEX),
			0,
			{ (short)font_dimensions.x, (short)font_dimensions.y },
			FF_DONTCARE,
			FW_NORMAL,
			L""
		};

		if (not SetCurrentConsoleFontEx(console.output, FALSE, &font_info)) {
			throw std::runtime_error("Failed to set console font");
		}

		CONSOLE_SCREEN_BUFFER_INFO screen_info;
		if (not GetConsoleScreenBufferInfo(console.output, &screen_info)) {
			throw std::runtime_error("Failed to get console screen_dimensions buffer info");
		} else {
			Dimensions window_dimensions = Dimensions(screen_info.dwMaximumWindowSize.X, screen_info.dwMaximumWindowSize.Y);
			if (screen_dimensions > window_dimensions) {
				throw std::runtime_error("Screen dimensions are too large, maximum dimensions allowed are '" + (std::string)window_dimensions + "'");
			}
		}

		window_region = { 0, 0, (short)(screen_dimensions.x - 1), (short)(screen_dimensions.y - 1) };
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
	};

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
	}

protected:

	virtual void initialise() = 0;

	virtual void update(const double time_elapsed) = 0;

	void stop() {
		running = false;
		SetConsoleActiveScreenBuffer(console.original);
	}

private:

	void render(const double frame_rate) {
		if (not SetConsoleTitleW((title + L" - FPS: " + std::to_wstring(frame_rate)).c_str())) {
			throw std::runtime_error("Failed to set console title");
		}

		if (not WriteConsoleOutputW(console.output, buffer.data(), { (short)screen_dimensions.x, (short)screen_dimensions.y }, { 0, 0 }, &window_region)) {
			throw std::runtime_error("Failed to draw to console");
		}
	}

protected:

	Dimensions get_screen_dimensions() {
		return screen_dimensions;
	}

	unsigned short get_screen_width() {
		return screen_dimensions.x;
	}

	unsigned short get_screen_height() {
		return screen_dimensions.y;
	}

	ButtonState get_key(Key key) {
		return get_button(key);
	}

	ButtonState get_key(char key) {
		if ((key >= 'A' and key <= 'Z') or (key >= '0' and key <= '9')) {
			return get_button(key);
		} else {
			throw std::runtime_error("Invalid key requested '" + key + '\'');
		}
	}

	ButtonState get_mouse_button(MouseButton mouse_button) {
		return get_button(mouse_button);
	}

	Coordinate get_mouse_position() {
		const auto& input_records = get_input_record();
		for (const auto& input_record : input_records) {
			if (input_record.EventType == MOUSE_EVENT and input_record.Event.MouseEvent.dwEventFlags == MOUSE_MOVED) {
				const auto& position = input_record.Event.MouseEvent.dwMousePosition;
				mouse_position = Coordinate(position.X, position.Y);
			}
		}
		return mouse_position;
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

	short get_mouse_x() {
		return get_mouse_position().x;
	}

	short get_mouse_y() {
		return get_mouse_position().y;
	}

	const std::vector<CHAR_INFO>& get_buffer() {
		return buffer;
	}

private:

	ButtonState get_button(char button) {
		short state = GetAsyncKeyState(button);
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

	std::vector<INPUT_RECORD> get_input_record() {
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

	Coordinate mouse_position = Coordinate(0, 0);

protected:

	// Drawing functions

	void clear_screen(const Pixel& pixel = Pixel(Black, Empty)) {
		for (unsigned short x = 0; x < screen_dimensions.x; x++) {
			for (unsigned short y = 0; y < screen_dimensions.y; y++) {
				draw_pixel(Coordinate(x, y), pixel);
			}
		}
	}

	void draw_character(const Coordinate& coordinate, const wchar_t character, const Colour colour = White) {
		if (in_range(coordinate, screen_dimensions)) {
			unsigned short index = coordinate_to_index(coordinate, screen_dimensions.x);
			buffer[index].Char.UnicodeChar = character;
			buffer[index].Attributes = colour;
		}
	}

	void draw_pixel(const Coordinate& coordinate, const Pixel& pixel = Pixel()) {
		draw_character(coordinate, pixel.shade, pixel.colour);
	}

	void draw_sprite(const Coordinate& coordinate, const Sprite& sprite, const double scale = 1.0) {
		const Dimensions& dimensions = sprite.get_dimensions() * scale;
		for (short x = 0; x < dimensions.x; x++) {
			for (short y = 0; y < dimensions.y; y++) {
				const Pixel& pixel = sprite.get_pixel(Coordinate(x, y) / scale);
				if (pixel.shade != Empty) {
					draw_pixel(coordinate + Coordinate(x, y), pixel);
				}
			}
		}
	}

	void draw_entity(const Entity& entity) {
		draw_sprite(Coordinate((short)entity.get_position().x, (short)entity.get_position().y), entity.get_sprite());
	}

	void draw_string(const Coordinate& coordinate, const std::wstring& string, Colour colour = White) {
		for (unsigned short i = 0; i < string.length(); i++) {
			draw_character(coordinate + Coordinate(i, 0), string[i], colour);
		}
	}

	template <typename type> type abs(type x) { return x < 0 ? -x : x; }

	void draw_line(const Coordinate& start, const Coordinate& end, const Pixel& pixel = Pixel()) {
		Coordinate current = start;
		Coordinate delta = end - start;
		Coordinate step = { (delta.x > 0) - (delta.x < 0), (delta.y > 0) - (delta.y < 0) };
		delta = { abs(delta.x), abs(delta.y) };

		if (delta.x > delta.y) {
			short error = delta.x / 2;
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
			short error = delta.y / 2;
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

	void draw_triangle(const Coordinate coordinates[3], const Pixel& pixel = Pixel()) {
		draw_line(coordinates[0], coordinates[1], pixel);
		draw_line(coordinates[1], coordinates[2], pixel);
		draw_line(coordinates[2], coordinates[0], pixel);
	}

	void draw_circle(const Coordinate& centre, const unsigned short radius = 1, const Pixel& pixel = Pixel()) {
		Coordinate current;
		for (current.x = -radius; current.x <= radius; current.x++) {
			for (current.y = -radius; current.y <= radius; current.y++) {
				if (current.magnitude() == radius) {
					draw_pixel(centre + current, pixel);
				}
			}
		}
	}

	void draw_filled_circle(const Coordinate& centre, const unsigned short radius = 1, const Pixel& pixel = Pixel()) {
		Coordinate current;
		for (current.x = -radius; current.x <= radius; current.x++) {
			for (current.y = -radius; current.y <= radius; current.y++) {
				if (current.magnitude() <= radius) {
					draw_pixel(centre + current, pixel);
				}
			}
		}
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