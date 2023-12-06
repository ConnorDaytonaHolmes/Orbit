#pragma once

#include "message.h"

class IMidiReceiver {
	virtual void on_message_received(MIDIMessage message) = 0;
};