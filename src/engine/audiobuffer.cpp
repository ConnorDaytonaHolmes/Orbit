#include "audiobuffer.h"
#include "../util.h"

AudioBuffer::AudioBuffer(uint32_t buffer_size) {
	size = buffer_size;
	std::shared_ptr<float> sp(new float[buffer_size] { 0.0f }, array_deleter<float>());
	sp.swap(buf);
}

//Should probably check if use_count == 1, oh well

void AudioBuffer::resize(uint32_t new_size) {
	if (size == new_size)
		return;

	size = new_size;
	float* new_buffer = new float[size];
	buf.reset(new_buffer, array_deleter<float>());
}

void AudioBuffer::clear() {
	memset(buf.get(), 0, size * sizeof(float));
}

void AudioBuffer::copy_from(AudioBuffer* src) {
	if (this->size != src->size) {
		printf("Buffer size mismatch in AudioBuffer::copy(AudioBuffer* src)");
		return;
	}
	memcpy(this->buf.get(), src->buf.get(), size * sizeof(float));
}

void AudioBuffer::copy_from(AudioBuffer* src, float volume) {
	if (this->size != src->size) {
		printf("Buffer size mismatch in AudioBuffer::copy(AudioBuffer* src)");
		return;
	}
	
	float vol = clamp_0_1(volume);
	if (vol == 1.0) {
		this->copy_from(src);
		return;
	}
	else if (vol == 0.0) {
		clear();
		return;
	}

	float* f_dst = this->buf.get();
	float* f_src = src->buf.get();
	for (unsigned int i = 0; i < size; i++) {
		f_dst[i] = f_src[i] * vol;
	}
}

void AudioBuffer::add(AudioBuffer* src) {
	if (this->size != src->size) {
		printf("Buffer size mismatch in AudioBuffer::copy(AudioBuffer* src)");
		return;
	}
	float* f_dst = this->buf.get();
	float* f_src = src->buf.get();
	for (unsigned int i = 0; i < size; i++) {
		f_dst[i] += f_src[i];
	}
}



void AudioBuffer::add(AudioBuffer* src, float volume) {
	if (this->size != src->size) {
		printf("Buffer size mismatch in AudioBuffer::copy(AudioBuffer* src)");
		return;
	}
	float vol = clamp_0_1(volume);
	if (vol >= 1.0) {
		this->add(src);
		return;
	}
	else if (vol <= 0.0) {
		return;
	}

	for (unsigned int i = 0; i < size; i++) {
		float* dst_array = this->buf.get();
		float* src_array = src->buf.get();
		dst_array[i] += src_array[i] * vol;
	}
}

void AudioBuffer::clip() {
	float* buffer = buf.get();
	for (int i = 0; i < size; i++) {
		buffer[i] = clamp_n1_1(buffer[i]);
	}
}

