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
	AudioBuffer();
	AudioBuffer(unsigned int buffer_size);

	std::shared_ptr<float> buf;
	unsigned int size;

	void resize(unsigned int new_size);
	void clear();

	void copy_from(AudioBuffer* src);
	void copy_from(AudioBuffer* src, float volume);
	void add(AudioBuffer* src);
	void add(AudioBuffer* src, float volume);
	void clip();
};

