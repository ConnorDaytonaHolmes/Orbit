#pragma once

#include "../event.h"
#include <message.h>
// struct midimessage
// parse midi files
// sync with OS/device midi time

class MIDIEventHandler {
public:
	static Event<MIDIMessage> OnMidiMessageReceived;
};

namespace MIDI {
	static MIDIEventHandler events;
}
