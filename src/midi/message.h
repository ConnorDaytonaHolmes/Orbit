#pragma once
#include <cstdint>

// midi interface
// IMidiReceiver : OnMessageReceived(MidiMessage fniwefiew)


// midi message is status byte + data byte(s)
// status is 4 bits for message type, 4 bits for channel
// SSSSNNNN (SSSS = STATUS_ID) (NNNN = 0-15 -> MIDI Channel Number 1-16)

static constexpr uint8_t STATUSIDMASK = 0xF0;

enum StatusID : uint8_t {
	NOTE_OFF = 0b10000000,
	NOTE_ON = 0b10010000,
	POLYPHONIC_KEY_PRESSURE = 0b10100000,
	CONTROL_CHANGE = 0b10110000,
	PROGRAM_CHANGE = 0b11000000,
	CHANNEL_PRESSURE = 0b11010000,
	PITCH_BEND_CHANGE = 0b11100000,
	CHANNEL_MODE_MESSAGE = 0b10110000, // see controlfunction.h
	
	/* SYSTEM COMMON MESSAGES */

	SYSTEM_EXCLUSIVE = 0b11110000,
	MIDI_TIME_CODE_QUARTER_FRAME = 0b11110001,
	SONG_POSITION_POINTER = 0b11110010,
	SONG_SELECT = 0b11110011,
	TUNE_REQUEST = 0b11110110,
	END_OF_EXCLUSIVE = 0b11110111,

	/* SYSTEM REAL-TIME MESSAGES */
	TIMING_CLOCK = 0b11111000,
	START = 0b11111010,
	CONTINUE = 0b11111011,
	STOP = 0b11111100,
	ACTIVE_SENSING = 0b11111110,
	RESET = 0b11111111,
};

struct MIDIMessage {
	StatusID status;
	uint8_t channel;
	union {
		struct KeyEventInfo {
			uint8_t note_number;
			uint8_t velocity;
		} key_event_info;

		struct ControlChangeInfo {
			uint8_t controller_number;
			uint8_t value;
		} control_change_info;

		uint8_t program_number;
		uint8_t channel_pressure_value;

		struct PitchBendInfo {
			uint8_t lsb;
			uint8_t msb;
		} pitch_bend_info;

		struct MIDITimeCodeQuarterFrame {
			uint8_t message_type;
			uint8_t values;
		} midi_tc_qf_info;

		struct SongPositionPointer {
			uint8_t lsb;
			uint8_t msb;
		} song_position_pointer_info;

		uint8_t song_select;

		uint8_t* system_exclusive_message;
	};
};

MIDIMessage decode_message(uint8_t* message);
MIDIMessage decode_message(uint32_t message);