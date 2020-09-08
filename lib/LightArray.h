#ifndef HAVE_LIGHT_ARRAY_H
#define HAVE_LIGHT_ARRAY_H

#include <array>
#include <cstdint>
#include <cstring>
#include <gsl/span>

struct LightLevel {
	static const unsigned int MAX = 15;
	const uint8_t v;

	LightLevel(uint8_t value) : v(value) {
		if (value > MAX) {
			throw std::invalid_argument("value out of range");
		}
	}

	operator int() const {
		return v;
	}
};

class LightArray {
public:
	static const size_t DATA_SIZE = 2048;
private:
	std::array<uint8_t, DATA_SIZE> mValues;

	static inline void index(const uint8_t x, const uint8_t y, const uint8_t z, unsigned int &offset, unsigned int &shift) {
		offset = (x << 7) | (z << 3) | (y >> 1);
		shift = (y & 1) << 2;
	}

public:
	LightArray() {
		mValues.fill(0);
	}

	LightArray(const gsl::span<const uint8_t, DATA_SIZE> &values){
		memcpy(mValues.data(), values.data(), sizeof(mValues));
	}

	LightArray(const LightLevel fillValue) {
		mValues.fill((fillValue << 4) | fillValue);
	}

	LightLevel get(const uint8_t x, const uint8_t y, const uint8_t z) const {
		unsigned int offset, shift;
		index(x, y, z, offset, shift);

		return (mValues[offset] >> shift) & 0xf;
	}

	void set(const uint8_t x, const uint8_t y, const uint8_t z, const LightLevel value) {
		unsigned int offset, shift;
		index(x, y, z, offset, shift);

		auto byte = mValues[offset];
		mValues[offset] = (byte & ~(0xf << shift)) | (value << shift);
	}

	const gsl::span<const uint8_t, DATA_SIZE> getRawData() const {
		return gsl::span<const uint8_t>(mValues);
	}
};
#endif
