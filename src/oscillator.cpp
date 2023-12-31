#include "oscillator.h"
#include <engine/masterbuffer.h>
#include <engine.h>

osc::Oscillator::Oscillator(int num_channels, int buffer_size, double sample_rate, const PanningLaw& panning_law, wt::Wavetable* wt)
: Oscillator(num_channels, buffer_size, sample_rate, panning_law) {
	_wavetable = wt;
}

osc::Oscillator::Oscillator(int num_channels, int buffer_size, double sample_rate, const PanningLaw& panning_law)
	: IAudioOutput(num_channels, buffer_size, sample_rate, panning_law), IGenerator(this) {
}

osc::Oscillator::~Oscillator() {
	if (_wavetable) {
		_wavetable->release();
	}
}

void osc::Oscillator::set_wavetable(wt::Wavetable* new_wt) {
	new_wt->add_ref();
	if (_wavetable != nullptr) {
		_wavetable->release();
	}
	_wavetable = new_wt;
}

void osc::Oscillator::set_hz(double hz) {
	_hz = std::clamp(hz, MINIMUM_HZ, MAXIMUM_HZ);
	_index_offset = wt::WAVETABLE_SIZE * _hz / sample_rate;
}

double osc::Oscillator::get_hz() {
	return _hz;
}

void osc::Oscillator::set_sample_rate(double sample_rate) {
	IAudioOutput::set_sample_rate(sample_rate);
	_index_offset = wt::WAVETABLE_SIZE * _hz / sample_rate;
}

void osc::Oscillator::process_output() {
	if (!_wavetable || !playing) {
		// No assigned wavetable? all zero baby
		clear_buffer();
		output_ready = true;
	}
	else {
		float* f_out = out.buf.get();
		// Populate buffer with sampled wavetable data
		// two writes, one to left, one to right (interleaved)
		// oscillators are always mono
		for (unsigned int i = 0; i < get_buffer_size(); i += channels) {
			float value = _wavetable->get_sample((float)_sample_index);
			for (int n = 0; n < channels; n++) {
				f_out[i + n] = value;
			}
			_sample_index += _index_offset;
			_sample_index -= (_sample_index >= wt::WAVETABLE_SIZE) * wt::WAVETABLE_SIZE;
		}
		IAudioOutput::process_output();
		output_ready = true;
	}
}

wt::WavetableCollection* oscillator_test(AudioEngine* e) {
	// Load wt library
	wt::WavetableCollection* wt_lib = load_wavetable_library();
	if (!wt_lib)
		return nullptr;

	// Get sine wavetable
	const char* wt_name = "Sine";
	wt::Wavetable* test_wavetable;
	bool found = wt_lib->find_wavetable(wt_name, &test_wavetable);
	if (!found) {
		printf("Couldn't find '%s' wavetable.\n", wt_name);
		return nullptr;
	}

	std::vector<osc::Oscillator*> oscillators{};   // notes F#, A#, C#, F, G#
	std::vector<double> frequencies { 50.0, 100.0, 200.0 };// , 369.0, 466.0, 554.0, 698.0, 830.0 };
	for (int i = 0; i < frequencies.size(); i++) {
		osc::Oscillator* o =
			new osc::Oscillator(
				2,
				e->master->out.size,
				e->master->sample_rate,
				e->audio_settings->panning_law
			);
		o->volume = 1.0f / frequencies.size();
		o->set_wavetable(test_wavetable);
		o->set_hz(frequencies[i]);
		o->clear_buffer();
		e->mixer.get()->get_track(1)->assign_input(o);
		oscillators.push_back(o);
	}

	return wt_lib;
}