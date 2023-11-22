#pragma once

#include <windows.h>
#include <iostream>
#include <unordered_map>
#include <timeapi.h>

#define WINDOWS 1

#define stringify( name ) #name

void print_host_error(long err);
void print_hresult(HRESULT hr);
unsigned long get_sys_reference_time();
unsigned long get_sys_reference_time_samples();

const long HOST_ERROR_MASK = 0x0FFFF000;

enum HostError : long {
	HOST_OK = 0,
	FailedToLoadASIODriver = 1 << 12,
	FailedToInitializeASIODriver,
	FailedToInitializeStaticData,
	FailedToCreateBuffers,
	FailedToStartASIO,
	FailedToGetChannelCount,
	FailedToGetChannelInfo,
	FailedToGetBufferSize,
	FailedToGetSampleRate,
	FailedToSetSampleRate,
	FailedToGetSampleRateAfterSetting,
	FailedToGetSystemTime,
};

const std::unordered_map<long, const char* const> host_error_to_string = {
	{ HOST_OK,								stringify(HOST_OK) },
	{ FailedToLoadASIODriver,				stringify(FailedToLoadASIODriver) },
	{ FailedToInitializeASIODriver,			stringify(FailedToInitializeASIODriver) },
	{ FailedToInitializeStaticData,			stringify(FailedToInitializeStaticData) },
	{ FailedToCreateBuffers,				stringify(FailedToCreateBuffers) },
	{ FailedToStartASIO,					stringify(FailedToStartASIO) },
	{ FailedToGetChannelCount,				stringify(FailedToGetChannelCount) },
	{ FailedToGetChannelInfo,				stringify(FailedToGetChannelInfo) },
	{ FailedToGetBufferSize,				stringify(FailedToGetBufferSize) },
	{ FailedToGetSampleRate,				stringify(FailedToGetSampleRate) },
	{ FailedToSetSampleRate,				stringify(FailedToSetSampleRate) },
	{ FailedToGetSampleRateAfterSetting,	stringify(FailedToGetSampleRateAfterSetting) },
};

float lerp(float a, float b, float t);
float clamp(float value, float min, float max);
float clamp_0_1(float value);
float clamp_n1_1(float value);