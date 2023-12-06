#include "message.h"

MIDIMessage decode_message(uint8_t* message) {
	MIDIMessage m{};
	m.status = (StatusID)(message[0] & STATUSIDMASK);
	m.channel = message[0] & ~STATUSIDMASK;
	
	switch (m.status) {
	case NOTE_OFF:
	case NOTE_ON:
	case POLYPHONIC_KEY_PRESSURE:
		m.key_event_info.note_number = message[1] & 0x7F;
		m.key_event_info.velocity = message[2] & 0x7F;
		break;

	case CONTROL_CHANGE: // also CHANNEL_MODE_MESSAGES
		m.control_change_info.controller_number = message[1] & 0x7F;
		m.control_change_info.value = message[2] & 0x7F;
		break;

	case PROGRAM_CHANGE:
		m.program_number = message[1] & 0x7F;
		break;

	case CHANNEL_PRESSURE:
		m.channel_pressure_value = message[1] & 0x7F;
		break;

	case PITCH_BEND_CHANGE:
		m.pitch_bend_info.lsb = message[1] & 0x7F;
		m.pitch_bend_info.msb = message[2] & 0x7F;
		break;

	// definitely will not work, need to read message into block then process, blah blah blah cbf
	case SYSTEM_EXCLUSIVE:
		m.system_exclusive_message = &message[1];
		break;

	case MIDI_TIME_CODE_QUARTER_FRAME:
		m.midi_tc_qf_info.message_type = message[1] & 0x70;
		m.midi_tc_qf_info.values = message[1] & 0x0f;
		break;

	case SONG_POSITION_POINTER:
		m.song_position_pointer_info.lsb = message[1] & 0x7F;
		m.song_position_pointer_info.msb = message[2] & 0x7F;
		break;

	case SONG_SELECT:
		m.song_select = message[1] & 0x7F;
		break;
	}
	// handle some invalid message???
	// who knows :) through an optional
	return m;
}

MIDIMessage decode_message(uint32_t message) {
	return decode_message((uint8_t*)&message);	
}