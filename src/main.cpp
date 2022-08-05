#include <array>
#include <vector>

#include <pigpio.h>

#include <core/visionserver2.h>
#include <core/vision.h>
//#include <cpp-tools/src/sighandle.h>

#include "config.h"
#include "hardware.h"
#include "vision.h"


// struct Shooter {
// 	Stepper
// 		hood_stepper{HOOD_PINS},
// 		turn_stepper{TURN_PINS};
// 	Input
// 		hood_home{HOOD_HOME_PIN},
// 		turn_home{TURN_HOME_PIN},
// 		feed_output{FEED_OUTPUT_PIN},
// 		feed_input{FEED_INPUT_PIN};


// 	void init() {
// 		init_output(MAIN_PWM_PIN);
// 		init_output(FEED_PWM_PIN);
// 		hood_stepper.init();
// 		turn_stepper.init();
// 		hood_home.init();
// 		turn_home.init();
// 		feed_output.init();
// 		feed_input.init();
// 	}


// } shooter;

using namespace vs2;
using hrc = std::chrono::high_resolution_clock;

int main(int argc, char** argv) {

	//SigHandle::get();

	gpioInitialise();
	//shooter.init();
	init_output(MAIN_PWM_PIN);
	init_stepper(TURN_PINS);

	std::vector<VisionCamera> cameras;
	TargetVision vpipe;

	readConfig(cameras);
	// for(size_t i = 0; i < cameras.size(); i++) {
	// 	if(cameras[i].IsConnected()) {
	// 		std::iter_swap(cameras.begin(), cameras.begin() + i);
	// 		break;
	// 	}
	// }

	VisionServer::Init();
	VisionServer::addCameras(std::move(cameras));
	VisionServer::addStream();
	VisionServer::addPipeline(&vpipe);
	//VisionServer::compensate();
	VisionServer::runSingleThread(60);

	float d, lr;
	int64_t steps = 0;
	hrc::time_point t;
	for(;;) {
		d = vpipe.getDist();
		lr = vpipe.getLR();
		if(d > 0 && d <= 5.f) {
			std::cout << d << std::endl;
			set_pwm(MAIN_PWM_PIN, d / 5.f * 100.f);
		}
		t = hrc::now();
		while(
			abs(lr) > abs(steps / (float)stepper_steps_per_rotation * 360) &&
			(hrc::now() - t).count() < 10000000
		) {
			steps += sgn(lr);
			set_stepper(TURN_PINS, steps);
		}
	}


}