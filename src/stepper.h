#pragma once

#include <chrono>
#include <pigpio.h>


struct W24BYJ_base {
	static inline size_t
		indices{8},
		rotation_steps{4096}
	;
	static inline uint32_t steps{
		0b1001 |
		0b1000 << 4 |
		0b1100 << 8 |
		0b0100 << 12 |
		0b0110 << 16 |
		0b0010 << 20 |
		0b0011 << 24 |
		0b0001 << 28
	};
};
template<
	uint8_t A1,
	uint8_t A2,
	uint8_t B1,
	uint8_t B2
>class W24BYJ_Stepper : public W24BYJ_base {
	using This_t = W24BYJ_Stepper<A1, A2, B1, B2>;
public:
	explicit W24BYJ_Stepper();
	~W24BYJ_Stepper();

	inline void increment() { this->mov(1); }
	inline void decrement() { this->mov(-1); }
	void step(int64_t i);	// increment by 'i' number of steps (negative values will decrement)
	void rotate(float d);	// rotate by 'd' number of degrees
	inline void zero() { this->count = 0; }		// zero the internal counter
	void stop();
	void setInterval(uint64_t us);

	inline int64_t getSteps() const { return this->count; }
	inline float getAngle() const { return (float)this->count / rotation_steps * 360; }
	inline int64_t getInterval() const { return this->interval_us; }

protected:
	int64_t count{0};
	uint64_t interval_us{5000};

	void mov(int8_t v);


};



template<uint8_t A1, uint8_t A2, uint8_t B1, uint8_t B2>
W24BYJ_Stepper<A1, A2, B1, B2>::W24BYJ_Stepper() {
	gpioSetMode(A1, PI_OUTPUT);
	gpioSetMode(A2, PI_OUTPUT);
	gpioSetMode(B1, PI_OUTPUT);
	gpioSetMode(B2, PI_OUTPUT);
}
template<uint8_t A1, uint8_t A2, uint8_t B1, uint8_t B2>
W24BYJ_Stepper<A1, A2, B1, B2>::~W24BYJ_Stepper() {
	this->stop();
}

template<uint8_t A1, uint8_t A2, uint8_t B1, uint8_t B2>
void W24BYJ_Stepper<A1, A2, B1, B2>::mov(int8_t v) {
	this->count += v;
	uint8_t index = (this->count % This_t::indices) * 4;
	gpioWrite(A1, 0b1 & (This_t::steps >> (index + 3)));
	gpioWrite(A2, 0b1 & (This_t::steps >> (index + 2)));
	gpioWrite(B1, 0b1 & (This_t::steps >> (index + 1)));
	gpioWrite(B2, 0b1 & (This_t::steps >> index));
}

template<uint8_t A1, uint8_t A2, uint8_t B1, uint8_t B2>
void W24BYJ_Stepper<A1, A2, B1, B2>::step(int64_t i) {
	uint8_t s = sgn(i);
	for(size_t n = 0; n < i * s; n++) {
		this->mov(s);
		std::this_thread::sleep_for(std::chrono::microseconds(this->interval_us));
	}
	this->stop();
}
template<uint8_t A1, uint8_t A2, uint8_t B1, uint8_t B2>
void W24BYJ_Stepper<A1, A2, B1, B2>::rotate(float d) {
	this->step(d / 360 * This_t::rotation_steps);
}
template<uint8_t A1, uint8_t A2, uint8_t B1, uint8_t B2>
void W24BYJ_Stepper<A1, A2, B1, B2>::stop() {
	gpioWrite(A1, 0);
	gpioWrite(A2, 0);
	gpioWrite(B1, 0);
	gpioWrite(B2, 0);
}