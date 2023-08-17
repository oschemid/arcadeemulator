#pragma once

#include <string>
#include <cstdint>
#include <functional>
#include <fstream>
#include <vector>


namespace aos {
	using std::string;
	using std::uint8_t;
	using std::uint16_t;
	using std::uint32_t;
	using std::size_t;

	using fnuint8_t = std::function<uint8_t(const uint8_t)>;

	struct geometry_t {
		uint16_t width;
		uint16_t height;
		enum class rotation_t {
			NONE, 
			ROT90
		} rotation;
	};
	struct rgb_t {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};
	struct rgba_t {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha{ 0xff };

		operator rgb_t() const { return rgb_t{ .red = red, .green = green, .blue = blue }; }
	};

	using palette_t = std::vector<rgba_t>;

	struct rect_t {
		uint16_t left;
		uint16_t top;
		uint16_t right;
		uint16_t bottom;
	};

	class File {
	protected:
		std::ifstream _ifs;
		size_t _size;

	public:
		File(const string&);
		~File();

		size_t getSize() const { return _size; }
		bool read(const uint16_t, const uint16_t, const uint8_t*);
	};
};