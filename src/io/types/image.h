#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <cstdlib>

enum ImageFormat {
	IMAGE_FORMAT_R8,
	IMAGE_FORMAT_RG8,
	IMAGE_FORMAT_RGBA8,
};

class Image {
public:
	static const char *formatName(ImageFormat format) {
		switch (format) {
			case IMAGE_FORMAT_R8:
				return "R8";
			case IMAGE_FORMAT_RG8:
				return "RG8";
			case IMAGE_FORMAT_RGBA8:
				return "RGBA8";
		}
	}

	static size_t formatChannelCount(ImageFormat format) {
		switch (format) {
			case IMAGE_FORMAT_R8:
				return 1;
			case IMAGE_FORMAT_RG8:
				return 2;
			case IMAGE_FORMAT_RGBA8:
				return 4;
		}
	}

private:
	uint32_t m_width;
	uint32_t m_height;

	ImageFormat m_format;
	uint8_t *m_data;

public:
	inline uint32_t width() const {
		return m_width;
	}

	inline uint32_t height() const {
		return m_height;
	}

	inline ImageFormat format() const {
		return m_format;
	}

	inline uint8_t *data() const {
		return m_data;
	}

	inline uint64_t size() const {
		return m_width * m_height * formatChannelCount(m_format);
	}

	Image() {}

	Image(uint32_t _width, uint32_t _height, ImageFormat _format, uint8_t *_data) {
		m_width = _width;
		m_height = _height;
		m_format = _format;
		m_data = _data;
	}

	~Image() {
		if (m_data == nullptr)
			return;

		free(m_data);
	}
};

#endif // !IMAGE_H
