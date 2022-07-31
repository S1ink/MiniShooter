#include <array>
#include <vector>

#include <CameraServer.h>

#include <core/visionserver2.h>
#include <core/extensions.h>
#include <core/vision.h>

#include "input.h"
#include "pwm_l298n.h"
#include "stepper.h"


class TPipeline : public vs2::VPipeline<TPipeline> {
public:
	inline static const std::array<cv::Point3f, 4> world{
		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
		cv::Point3f(0.25f, -0.25f, 0.f),	//bottom-right
		cv::Point3f(-0.25f, -0.25f, 0.f),	//bottom-left
		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
	};

	TPipeline() : VPipeline<TPipeline>("Retroreflective square tracking") {
		// ntables
	}

	inline void connect(const VisionCamera& v) { this->setCamera(v); }
	void invoke(cv::Mat& io_frame) {
		this->getFrame(io_frame);
		this->process(io_frame);
		this->PutFrame(io_frame);
	}
	void process(cv::Mat& io_frame) override {

	}

	inline float getX() const { return this->tvec[0][0]; }
	inline float getY() const { return this->tvec[1][0]; }
	inline float getZ() const { return this->tvec[2][0]; }
	double getDist() const { return sqrt(pow(getX(), 2) + pow(getY(), 2) + pow(getZ(), 2)); }
	double getLR() const { return atan2(getX(), getZ()) * 180/M_PI; }
	double getUD() const { return atan2(getY(), getZ()) * -180/M_PI; } 
	const cv::Mat_<float>& getTranslation() const { return this->tvec; }

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

	gpioInitialise();
	
	W24BYJ_Stepper<6, 13, 19, 26> hood_motor{};
	PWM_L298N<21, 20, 16> shooter_motor{};
	PWM_L298N<25, 7, 8> feed_motor{};

	std::vector<VisionCamera> cameras;
	TPipeline vpipe;
	cv::Mat frame;
	cs::MjpegServer stream{frc::CameraServer::GetInstance()->AddServer("Stream")};

	readConfig(cameras);
	for(size_t i = 0; i < cameras.size(); i++) {
		if(cameras[i].IsConnected()) {
			std::iter_swap(cameras.begin(), cameras.begin() + i);
			break;
		}
	}
	vpipe.connect(cameras[0]);
	stream.SetSource(vpipe);

	for(;;) {
		vpipe.invoke(frame);	// grab frame, process, and output to stream

		// update motors accordingly
	}


}