#pragma once
#include <algorithm>
#include "wavetable/wavetable.h"
#include "engine/iaudiooutput.h"
#include <wavetable/wavetablecollection.h>
#include <engine.h>
#include "engine/igenerator.h"

const double MINIMUM_HZ = 0.01;
const double MAXIMUM_HZ = 20000.0f;

namespace wt = wavetable;
namespace osc {
	struct Oscillator : public IAudioOutput, public IGenerator {
	public:
		Oscillator(int num_channels, int buffer_size, double sample_rate, wt::Wavetable* wt);
		Oscillator(int num_channels, int buffer_size, double sample_rate);
		~Oscillator();

		void set_sample_rate(double new_sample_rate) override;
		void set_wavetable(wt::Wavetable* wt);
		void set_hz(double hz);
		double get_hz();
		void process_output() override;

	private:
		wt::Wavetable* _wavetable = nullptr;
		double _sample_index = 0.0;
		double _hz = 1000.0;
		double _index_offset = wt::WAVETABLE_SIZE * _hz / get_sample_rate();
	};
}

wt::WavetableCollection* oscillator_test(AudioEngine* e);