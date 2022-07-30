#pragma once

#include <pigpio.h>


template<
	uint8_t PWM,
	uint8_t F,
	uint8_t B
>class PWM_L298N {
	using This_t = PWM_L298N<PWM, F, B>;
public:
	explicit PWM_L298N(float v = 0.f);
	~PWM_L298N();

	void forward();
	void backward();
	inline void setPercent(float p) { gpioPWM(PWM, p / 100 * 255); }
	void setVector(float v);
	void stop();


};



template<uint8_t PWM, uint8_t F, uint8_t B>
PWM_L298N<PWM, F, B>::PWM_L298N(float v) {
	gpioSetMode(PWM, PI_OUTPUT);
	gpioSetMode(F, PI_OUTPUT);
	gpioSetMode(B, PI_OUTPUT);

	this->setVector(v);
}
template<uint8_t PWM, uint8_t F, uint8_t B>
PWM_L298N<PWM, F, B>::~PWM_L298N() {
	this->stop();
}

template<uint8_t PWM, uint8_t F, uint8_t B>
void PWM_L298N<PWM, F, B>::forward() {
	gpioWrite(F, 1);
	gpioWrite(B, 0);
}
template<uint8_t PWM, uint8_t F, uint8_t B>
void PWM_L298N<PWM, F, B>::backward() {
	gpioWrite(F, 0);
	gpioWrite(B, 1);
}
template<uint8_t PWM, uint8_t F, uint8_t B>
void PWM_L298N<PWM, F, B>::setVector(float v) {
	if(v > 0) { this->forward(); }
	if(v < 0) { this->backward(); }
	this->setPercent(abs(v));
}
template<uint8_t PWM, uint8_t F, uint8_t B>
void PWM_L298N<PWM, F, B>::stop() {
	gpioWrite(PWM, 0);
	gpioWrite(F, 0);
	gpioWrite(B, 0);
}