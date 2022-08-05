#pragma once

#define MAIN_PWM_PIN		5

#define FEED_PWM_PIN		11
#define FEED_FORWARD_PIN	24
#define FEED_BACKWARD_PIN	25
#define FEED_PINS			{ FEED_PWM_PIN, FEED_FORWARD_PIN, FEED_BACKWARD_PIN }
#define FEED_INPUT_PIN		9
#define FEED_OUTPUT_PIN		10

#define HOOD_A1_PIN			6
#define HOOD_A2_PIN			13
#define HOOD_B1_PIN			19
#define HOOD_B2_PIN			26
#define HOOD_PINS			{ HOOD_A1_PIN, HOOD_A2_PIN, HOOD_B1_PIN, HOOD_B2_PIN }
#define HOOD_INC_VEC		1	// 1 or -1
#define HOOD_HOME_PIN		8

#define TURN_A1_PIN			12
#define TURN_A2_PIN			16
#define TURN_B1_PIN			20
#define TURN_B2_PIN			21
#define TURN_PINS			{ TURN_A1_PIN, TURN_A2_PIN, TURN_B1_PIN, TURN_B2_PIN }
#define TURN_INC_VEC		1	// 1 or -1
#define TURN_HOME_PIN		7