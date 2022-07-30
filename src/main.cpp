#include <array>

#include <core/visionserver2.h>
#include <core/extensions.h>

#include "input.h"
#include "pwm_l298n.h"
#include "stepper.h"


class SquareTarget : public vs2::VPipeline<SquareTarget> {
public:
	inline static const std::array<cv::Point3f, 4> world{
		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
		cv::Point3f(0.25f, -0.25f, 0.f),	//bottom-right
		cv::Point3f(-0.25f, -0.25f, 0.f),	//bottom-left
		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
	};

	SquareTarget() : VPipeline<SquareTarget>("Retroreflective square tracking") {}

	void process(cv::Mat& io_frame) override {

	}

private:
	std::array<cv::Mat, 3> channels;
	cv::Mat binary, buffer;
	double alpha{0.5}, beta{0.5}, gamma{0.0};
	uint8_t thresh{50};
	size_t scale{1};
	vs2::BGR color{vs2::BGR::BLUE};

	std::vector<std::vector<cv::Point2i> > contours;
	double area_largest{0.f}, area_buff{0.f};
	int16_t target_idx{-1};

	std::vector<cv::Point2i> target_points;
	cv::Mat_<float>
		rvec{1, 3},
		tvec{1, 3}
	;


};


int main(int argc, char** argv) {
	W24BYJ_Stepper<6, 13, 19, 26> hood_motor{};
	PWM_L298N<21, 20, 16> shooter_motor{};
	PWM_L298N<25, 7, 8> feed_motor{};


}