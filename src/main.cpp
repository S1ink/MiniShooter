#include <array>
#include <vector>

#include <pigpio.h>

#include <frc/DriverStation.h>

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


int main() {

	// for(;;) {
	// 	std::cout << 
	// 		"DS Status: " << (frc::DriverStation::IsDSAttached() ? "Online\n" : "Offline\n");
	// 	for(size_t s = 0; s < frc::DriverStation::kJoystickPorts; s++) {
	// 		std::cout << "\tStick " << (s + 1) << " - Connected?: " << frc::DriverStation::IsJoystickConnected(s) <<
	// 			"  Axis Count: " << frc::DriverStation::GetStickAxisCount(s) << 
	// 			"  Button Count: " << frc::DriverStation::GetStickButtonCount(s) << '\n';
	// 	}
	// 	std::cout << "\n\n";
	// 	std::cout.flush();
	// 	std::this_thread::sleep_for(std::chrono::seconds(5));
	// }

	gpioInitialise();

	init_output(MAIN_PWM_PIN);
	init_output(FEED_FORWARD_PIN);
	init_output(FEED_BACKWARD_PIN);

	/*
	space - toggle main
	enter - toggle feed
	u/d arrows - main pwm
	l/r arrows - change feed direction
	*/

	float main_speed = 50.f;
	bool main = false, feed = false, forward = true;

	std::cout <<
		"Controls:\n[space] - toggle main\n[enter] - toggle feed\n[u/d] - adjust main speed\n[l/r] - set feed direction\n[e] - exit\n"
	<< std::endl;
	char i[5] = {0}, last[5] = {0};
	int s = 1;
	for(;;) {
		std::cout <<
			"| MAIN: " << (main ? "on  | FEED: ":"off | FEED: ") <<
			(feed ? "on  | PWM: ":"off | PWM: ") << main_speed <<
			" | DIRECTION: " << forward << " |   " <<
		std::endl;

		i[0] = getch();
		if(i[0] == 'e') {
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
		if(!strncmp(i, A_LEFT, 3) || !strncmp(i, A_RIGHT, 3)) {
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


		// i = std::cin.get();
		// if(i >= '0' && i <= '9') {
		// 	std::cin.unget();
		// 	uint16_t n;
		// 	std::cin >> n;
		// 	gpioPWM(PWM, (float)n / 100 * 255);
		// 	std::cout << "Set PWM to " << n << "%\n";
		// } else {
		// 	switch(i) {
		// 		case 'f': {
		// 			gpioWrite(FORWARD, PI_HIGH);
		// 			gpioWrite(BACKWARD, PI_LOW);
		// 			std::cout << "Forward\n";
		// 			break;
		// 		}
		// 		case 'b': {
		// 			gpioWrite(FORWARD, PI_LOW);
		// 			gpioWrite(BACKWARD, PI_HIGH);
		// 			std::cout << "Backward\n";
		// 			break;
		// 		}
		// 		case 's': {
		// 			gpioWrite(FORWARD, PI_LOW);
		// 			gpioWrite(BACKWARD, PI_LOW);
		// 			std::cout << "Stop\n";
		// 			break;
		// 		}
		// 		case 'r': {
		// 			gpioWrite(FORWARD, PI_HIGH);
		// 			gpioWrite(BACKWARD, PI_LOW);
		// 			std::cout << "Run\n";
		// 			break;
		// 		}
		// 		case 'e': {
		// 			std::cout << "Exit\n";
		// 			gpioWrite(FORWARD, PI_LOW);
		// 			gpioWrite(BACKWARD, PI_LOW);
		// 			gpioWrite(PWM, PI_LOW);
		// 			gpioTerminate();
		// 			return;
		// 		}
		// 		case '\r':
		// 		case '\n':
		// 		{
		// 			break;
		// 		}
		// 		default: {
		// 			std::cout << "Invalid option: " << i << std::endl;
		// 		}
		// 	}
		// }

	}

	gpioTerminate();

}