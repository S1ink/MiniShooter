#include "hardware.h"

#include <pigpio.h>


void init_stepper(const Stepper& s) {
	gpioSetMode(s.A1(), PI_OUTPUT);
	gpioSetMode(s.A2(), PI_OUTPUT);
	gpioSetMode(s.B1(), PI_OUTPUT);
	gpioSetMode(s.B2(), PI_OUTPUT);
}
void stop_stepper(const Stepper& s) {
	gpioWrite(s.A1(), 0);
	gpioWrite(s.A2(), 0);
	gpioWrite(s.B1(), 0);
	gpioWrite(s.B2(), 0);
}
void set_stepper(const Stepper& s, uint i) {	// i % stepper_indices
	gpioWrite(s.A1(), 0b1 & (stepper_steps >> (i & (stepper_indices - 1) * 4 + 3)));
	gpioWrite(s.A2(), 0b1 & (stepper_steps >> (i & (stepper_indices - 1) * 4 + 2)));
	gpioWrite(s.B1(), 0b1 & (stepper_steps >> (i & (stepper_indices - 1) * 4 + 1)));
	gpioWrite(s.B2(), 0b1 & (stepper_steps >> (i & (stepper_indices - 1) * 4)));
}

void init_pwm(const PWMV& p) {
	gpioSetMode(p.PWM(), PI_OUTPUT);
	gpioSetMode(p.F(), PI_OUTPUT);
	gpioSetMode(p.B(), PI_OUTPUT);
}
void stop_pwm(const PWMV& p) {
	gpioWrite(p.PWM(), 0);
	gpioWrite(p.F(), 0);
	gpioWrite(p.B(), 0);
}
void forward_pwm(const PWMV& p) {
	gpioWrite(p.F(), 1);
	gpioWrite(p.B(), 0);
}
void backward_pwm(const PWMV& p) {
	gpioWrite(p.F(), 0);
	gpioWrite(p.B(), 1);
}
void set_pwm(uint8_t p, float per) {
	gpioPWM(p, per / 100 * 255);
}
void set_pwm_v(const PWMV& p, float v) {
	if(v > 0) { forward_pwm(p); }
	if(v < 0) { backward_pwm(p); }
	set_pwm(p.PWM(), abs(v));
}