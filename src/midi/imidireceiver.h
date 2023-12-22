#pragma once

#include "midi.h"
#include "message.h"

//void IMIDIReceiver_on_message_received(MIDIMessage message);

class IMIDIReceiver {
public:
	IMIDIReceiver() {
		//MIDI::events.OnMidiMessageReceived += IMIDIReceiver_on_message_received;
	}
	
	virtual void on_message_received(MIDIMessage message) = 0;

};
