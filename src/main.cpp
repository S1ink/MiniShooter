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

// int main(int argc, char** argv) {

// 	//SigHandle::get();

// 	gpioInitialise();
// 	//shooter.init();
// 	init_output(MAIN_PWM_PIN);
// 	init_stepper(TURN_PINS);

// 	std::vector<VisionCamera> cameras;
// 	TargetVision vpipe;

// 	readConfig(cameras);
// 	// for(size_t i = 0; i < cameras.size(); i++) {
// 	// 	if(cameras[i].IsConnected()) {
// 	// 		std::iter_swap(cameras.begin(), cameras.begin() + i);
// 	// 		break;
// 	// 	}
// 	// }

// 	VisionServer::Init();
// 	VisionServer::addCameras(std::move(cameras));
// 	VisionServer::addStream();
// 	VisionServer::addPipeline(&vpipe);
// 	//VisionServer::compensate();
// 	VisionServer::runSingleThread(60);

// 	float d, lr, diff_lr;
// 	int64_t steps = 0;
// 	hrc::time_point t;
// 	for(;;) {
// 		d = vpipe.getDist();
// 		lr = vpipe.getLR();
// 		if(d > 0 && d <= 5.f) {
// 			//std::cout << d << std::endl;
// 			set_pwm(MAIN_PWM_PIN, d / 5.f * 100.f);
// 		}
// 		t = hrc::now();
// 		while(
// 			((diff_lr = (lr / 360 - steps / (float)stepper_steps_per_rotation)) && abs(diff_lr) > (2.f / stepper_steps_per_rotation)) ||
// 			(hrc::now() - t).count() < 9000000
// 		) {
// 			steps += sgn(diff_lr);
// 			set_stepper(TURN_PINS, steps);
// 			std::this_thread::sleep_for(std::chrono::milliseconds(2));
// 		}
// 		std::cout << (hrc::now() - t).count() / 1000000 << "ms :: " << sgn(diff_lr) << " :: " << steps << std::endl;
// 	}


// }


#include <unistd.h>
#include <termios.h>

char getch(int block = 1) {
	char buf = 0;
	struct termios old = {0};

	if (tcgetattr(0, &old) < 0) {
		perror("tcsetattr()");
	}

	old.c_lflag &= ~ICANON;
	old.c_lflag &= ~ECHO;
	old.c_cc[VMIN] = block;
	old.c_cc[VTIME] = 0;

	if (tcsetattr(0, TCSANOW, &old) < 0) {
		perror("tcsetattr ICANON");
	}
	if (read(0, &buf, 1) < 0) {
		perror ("read()");
	}

	old.c_lflag |= ICANON;
	old.c_lflag |= ECHO;

	if (tcsetattr(0, TCSADRAIN, &old) < 0) {
		perror ("tcsetattr ~ICANON");
	}

	return (buf);
}

#define A_UP	"\e[A"
#define A_DOWN	"\e[B"
#define A_RIGHT	"\e[C"
#define A_LEFT	"\e[D"

#define SAVE_POS	"\e7"
#define RESTORE_POS	"\e8"
#define ERASE_BELOW	"\e[J"


void run_stepper(const Stepper& s, const float& t, std::atomic_bool& reset, bool& exit) {
	float error, ratio = 28.f / 42.f;
	int64_t index = 0;
	for(;!exit;) {
		if(reset) {
			index = 0;
			reset = false;
		}
		error = t - ((float)index / stepper_steps_per_rotation * 360.f * ratio);
		if((int)(error * 8) != 0) {		// eighth of a degree resolution
			index += sgn(error);
			set_stepper_(s, index);
		} else {
			stop_stepper(s);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(4));
	}
}

int main() {

	gpioInitialise();

	init_output(MAIN_PWM_PIN);
	init_output(FEED_FORWARD_PIN);
	init_output(FEED_BACKWARD_PIN);
	init_stepper(HOOD_PINS);

	// int x = 0;
	// for(; x < 1000; x++) {
	// 	set_stepper_(HOOD_PINS, x);
	// 	std::this_thread::sleep_for(std::chrono::microseconds(2000));
	// }
	// for(; x > 0; x--) {
	// 	set_stepper_(HOOD_PINS, x);
	// 	std::this_thread::sleep_for(std::chrono::microseconds(2000));
	// }
	// stop_stepper(HOOD_PINS);
	// gpioTerminate();
	// return;

	float main_speed = 50.f, hood_angle = 0.f;
	bool main = false, feed = false, forward = true, end = false;
	std::atomic_bool reset_hood{false};

	std::cout <<
		"Controls:\n"
		"[space] - toggle main\n"
		"[enter] - toggle feed\n"
		"[u/d] - adjust main speed\n"
		"[l/r] - adjust hood angle\n"
		"[z] - zero hood\n"
		"[f] - toggle feed direction\n"
		"[e] - exit\n"
	<< std::endl;
	char i[5] = {0}, last[5] = {0};
	int s = 1;

	std::thread stepper_runner(run_stepper, Stepper(HOOD_PINS), std::cref(hood_angle), std::ref(reset_hood), std::ref(end));

	for(;;) {
		std::cout <<
			"| MAIN: " << (main ? "on  | FEED: ":"off | FEED: ") <<
			(feed ? "on  | PWM: ":"off | PWM: ") << main_speed <<
			" | ANGLE: " << hood_angle << " | DIRECTION: " << forward << " |   " <<
		std::endl;

		i[0] = getch();
		if(i[0] == 'e') {
			end = true;
			stepper_runner.join();
			break;
		}
		while(i[s] = getch(0)) {
			s++;
		}

		if(!strncmp(i, A_UP, 3)) {
			main_speed += 1.f;
			main_speed = (main_speed > 100.f ? 100.f : main_speed);
			if(main) {
				set_pwm(MAIN_PWM_PIN, main_speed);
			}
		}
		if(!strncmp(i, A_DOWN, 3)) {
			main_speed -= 1.f;
			main_speed = (main_speed < 0.f ? 0.f : main_speed);
			if(main) {
				set_pwm(MAIN_PWM_PIN, main_speed);
			}
		}
		if(!strncmp(i, A_LEFT, 3)) {
			hood_angle -= 1.f;
		}
		if(!strncmp(i, A_RIGHT, 3)) {
			hood_angle += 1.f;
		}
		if(i[0] == 'z') {
			hood_angle = 0.f;
			reset_hood = true;
		}
		if(i[0] == 'f') {
			forward = !forward;
			gpioWrite(FEED_FORWARD_PIN, feed && forward);
			gpioWrite(FEED_BACKWARD_PIN, feed && !forward);
		}
		if(i[0] == ' ') {
			main = !main;
			set_pwm(MAIN_PWM_PIN, (main ? main_speed : 0.f));
		}
		if(i[0] == '\n') {
			feed = !feed;
			gpioWrite(FEED_FORWARD_PIN, feed && forward);
			gpioWrite(FEED_BACKWARD_PIN, feed && !forward);
		}

		std::cout << "\e[1A"ERASE_BELOW;
		std::cout.flush();

		// for(int x = 0; x < s; x++) {
		// 	std::cout << (int)i[x];
		// }
		// std::cout << std::endl;
		s = 1;

	}

	gpioTerminate();

}