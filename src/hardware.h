#pragma once

#include <array>
#include <pigpio.h>


static inline constexpr uint32_t stepper_steps{
	0b1001 |
	0b1000 << 4 |
	0b1100 << 8 |
	0b0100 << 12 |
	0b0110 << 16 |
	0b0010 << 20 |
	0b0011 << 24 |
	0b0001 << 28
};
static inline constexpr size_t
	stepper_indices{8},
	stepper_steps_per_rotation{4096}
;


struct Stepper;
struct PWMV;

void init_stepper(const Stepper&);
void stop_stepper(const Stepper&);
void set_stepper(const Stepper&, uint);

void init_pwm(const PWMV&);
void stop_pwm(const PWMV&);
void forward_pwm(const PWMV&);
void backward_pwm(const PWMV&);
void set_pwm(uint8_t, float);
void set_pwm_v(const PWMV&, float);

inline int init_output(uint p) { return gpioSetMode(p, PI_OUTPUT); }
inline int init_input(uint p) { return gpioSetMode(p, PI_INPUT); }


struct Stepper : std::array<uint8_t, 4> {
	inline uint8_t A1() const { return this->at(0); }
	inline uint8_t A2() const { return this->at(1); }
	inline uint8_t B1() const { return this->at(2); }
	inline uint8_t B2() const { return this->at(3); }

	inline void init() const { init_stepper(*this); }
	inline void stop() const { stop_stepper(*this); }
	inline void set(uint i) const { set_stepper(*this, i); }
};
struct PWMV : std::array<uint8_t, 3> {
	inline uint8_t PWM() const { return this->at(0); }
	inline uint8_t F() const { return this->at(1); }
	inline uint8_t B() const { return this->at(2); }

	inline void init() const { init_pwm(*this); }
	inline void stop() const { stop_pwm(*this); }
	inline void forward() const { forward_pwm(*this); }
	inline void backward() const { backward_pwm(*this); }
	inline void set(float s) const { set_pwm(this->PWM(), s); }
	inline void setv(float v) const { set_pwm_v(*this, v); }
};
struct Input {
	const uint8_t I;

	inline void init() const { gpioSetMode(I, PI_INPUT); }

	inline void pullUp() const { gpioSetPullUpDown(I, PI_PUD_UP); }
	inline void pullDown() const { gpioSetPullUpDown(I, PI_PUD_DOWN); }
	inline void pudOff() const { gpioSetPullUpDown(I, PI_PUD_OFF); }

	inline int get() const { return gpioRead(I); }
	inline void registerChange(gpioISRFunc_t f) const { gpioSetISRFunc(I, EITHER_EDGE, 0, f); }
	inline void registerRisingEdge(gpioISRFunc_t f) const { gpioSetISRFunc(I, RISING_EDGE, 0, f); }
	inline void registerFallingEdge(gpioISRFunc_t f) const { gpioSetISRFunc(I, FALLING_EDGE, 0, f); }


};