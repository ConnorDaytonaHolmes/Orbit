#pragma once
#include <winnt.h>
#include <winrt/Windows.Foundation.h>

enum class DeviceState {
    DeviceStateUninitialized,
    DeviceStateInError,
    DeviceStateDiscontinuity,
    DeviceStateFlushing,
    DeviceStateActivated,
    DeviceStateInitialized,
    DeviceStateStarting,
    DeviceStatePlaying,
    DeviceStateCapturing,
    DeviceStatePausing,
    DeviceStatePaused,
    DeviceStateStopping,
    DeviceStateStopped
};

class DeviceStateChangedEventArgs sealed {
public:
    DeviceStateChangedEventArgs(DeviceState new_state, HRESULT hr) :
        _device_state(new_state),
        _hr(hr) {
    };

    DeviceState get_state() { return _device_state; }
    __declspec(property(get = get_state)) DeviceState State;

    HRESULT get_hr() { return _hr; }
    __declspec(property(get = get_hr)) HRESULT hr;

private:
    DeviceState const _device_state;
    HRESULT     const _hr;
};
