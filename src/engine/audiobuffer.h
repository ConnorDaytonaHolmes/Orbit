#pragma once
#include <memory>

template<typename T>
struct array_deleter {
	void operator ()(T const* p) {
		delete[] p;
	}
};

// Buffer size is the size in FRAMES
// (1 frame = 1 sample for each channel)
struct AudioBuffer {
public:
	AudioBuffer(uint32_t _size);

	std::shared_ptr<float> buf;
	uint32_t size; // in frames

	void resize(uint32_t new_size);
	void clear();

	void copy_from(AudioBuffer* src);
	void copy_from(AudioBuffer* src, float volume);
	void add(AudioBuffer* src);
	void add(AudioBuffer* src, float volume);
	void clip();
};

