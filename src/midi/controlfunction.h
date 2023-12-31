#pragma once

enum ControlFunction {
	BANK_SELECT,
	MOD_WHEEL,
	BREATH_CONTROLLER,
	FOOT_CONTROLLER = 0x4,
	PORTAMENTO_TIME,
	DATA_ENTRY_MSB,
	CHANNEL_VOLUME,
	BALANCE,
	PAN = 0xA,
	EXPRESSION_CONTROLLER,
	EFFECT_CONTROL_1,
	EFFECT_CONTROL_2,
	GENERAL_PURPOSE_CONTROLLER_1 = 0x10,
	GENERAL_PURPOSE_CONTROLLER_2,
	GENERAL_PURPOSE_CONTROLLER_3,
	GENERAL_PURPOSE_CONTROLLER_4,

	LSB_CONTROL_0 = 0x20,	// BANK_SELECT,
	LSB_CONTROL_1,			// MOD_WHEEL,
	LSB_CONTROL_2,			// BREATH_CONTROLLER,
	LSB_CONTROL_3,			// UNDEFINED
	LSB_CONTROL_4,			// FOOT_CONTROLLER,
	LSB_CONTROL_5,			// PORTAMENTO_TIME,
	LSB_CONTROL_6,			// DATA_ENTRY_MSB,
	LSB_CONTROL_7,			// CHANNEL_VOLUME,
	LSB_CONTROL_8,			// BALANCE,
	LSB_CONTROL_9,			// UNDEFINED
	LSB_CONTROL_10,			// PAN,
	LSB_CONTROL_11,			// EXPRESSION_CONTROLLER,
	LSB_CONTROL_12,			// EFFECT_CONTROL_1,
	LSB_CONTROL_13,			// EFFECT_CONTROL_2,
	LSB_CONTROL_14,			// UNDEFINED
	LSB_CONTROL_15,			// UNDEFINED
	LSB_CONTROL_16,			// GENERAL_PURPOSE_CONTROLLER_1,
	LSB_CONTROL_17,			// GENERAL_PURPOSE_CONTROLLER_2,
	LSB_CONTROL_18,			// GENERAL_PURPOSE_CONTROLLER_3,
	LSB_CONTROL_19,			// GENERAL_PURPOSE_CONTROLLER_4,

	DAMPER_PEDAL_ON_OFF = 0x40, // sustain pedal
	PORTAMENTO_ON_OFF,
	SOSTENUTO_ON_OFF,
	SOFT_PEDAL_ON_OFF,
	LEGATO_FOOTSWITCH,
	HOLD_2,

	SOUND_CONTROLLER_1, // sound variation
	SOUND_CONTROLLER_2, // timbre/harmonic intens
	SOUND_CONTROLLER_3, // release time
	SOUND_CONTROLLER_4, // attack time
	SOUND_CONTROLLER_5, // brightness
	SOUND_CONTROLLER_6, // decay time
	SOUND_CONTROLLER_7, // vibrato rate
	SOUND_CONTROLLER_8, // vibrato depth
	SOUND_CONTROLLER_9, // vibrato delay
	SOUND_CONTROLLER_10, // undefined

	GENERAL_PURPOSE_CONTROLLER_5,
	GENERAL_PURPOSE_CONTROLLER_6,
	GENERAL_PURPOSE_CONTROLLER_7,
	GENERAL_PURPOSE_CONTROLLER_8,

	PORTAMENTO_CONTROL,
	HIGH_RESOLUTION_VELOCITY_PREFIX = 0x58,

	EFFECTS_1_DEPTH = 0x5B, // reverb send level
	EFFECTS_2_DEPTH, // tremolo depth
	EFFECTS_3_DEPTH, // chorus send level
	EFFECTS_4_DEPTH, // detune depth
	EFFECTS_5_DEPTH, // phaser depth

	DATA_INCREMENT,
	DATA_DECREMENT,
	NON_REGISTERED_PARAMETER_NUMBER_LSB,
	NON_REGISTERED_PARAMETER_NUMBER_MSB,
	REGISTERED_PARAMETER_NUMBER_LSB,
	REGISTERED_PARAMETER_NUMBER_MSB,

	/* CHANNEL MODE MESSAGES */

	ALL_SOUND_OFF = 120,
	RESET_ALL_CONTROLLERS,
	LOCAL_CONTROL_ON_OFF,
	ALL_NOTES_OFF,
	OMNI_MODE_OFF,
	OMNI_MODE_ON,
	MONO_MODE_ON,
	POLY_MODE_ON,
};