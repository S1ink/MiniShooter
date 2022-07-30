#pragma once

#include <pigpio.h>


template<uint8_t I>
class Input {
public:
	inline Input() { gpioSetMode(I, PI_INPUT); }

	inline void pullUp() { gpioSetPullUpDown(I, PI_PUD_UP); }
	inline void pullDown() { gpioSetPullUpDown(I, PI_PUD_DOWN); }
	inline void pudOff() { gpioSetPullUpDown(I, PI_PUD_OFF); }

	inline int get() const { return gpioRead(I); }
	inline void registerChange(gpioISRFunc_t f) const { gpioSetISRFunc(I, EITHER_EDGE, 0, f); }
	inline void registerRisingEdge(gpioISRFunc_t f) const { gpioSetISRFunc(I, RISING_EDGE, 0, f); }
	inline void registerFallingEdge(gpioISRFunc_t f) const { gpioSetISRFunc(I, FALLING_EDGE, 0, f); }


};