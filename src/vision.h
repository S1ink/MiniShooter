#pragma once

#include <array>
#include <vector>

#include <core/visionserver2.h>
#include <core/extensions.h>


class TargetVision : public vs2::VPipeline<TargetVision> {
public:
	inline static const std::array<cv::Point3f, 4> world{
		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
		cv::Point3f(0.25f, -0.25f, 0.f),	//bottom-right
		cv::Point3f(-0.25f, -0.25f, 0.f),	//bottom-left
		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
	};

	inline TargetVision() : VPipeline<TargetVision>("Retroreflective square tracking") {
		// ntables
	}

	inline void connect(const VisionCamera& v) { this->setCamera(v); }
	void invoke(cv::Mat& io_frame);
	void process(cv::Mat& io_frame) override;

	inline float getX() const { return this->tvec[0][0]; }
	inline float getY() const { return this->tvec[1][0]; }
	inline float getZ() const { return this->tvec[2][0]; }
	inline const cv::Mat_<float>& getTranslation() const { return this->tvec; }
	double getDist() const;
	double getLR() const;
	double getUD() const;

protected:
	void resizeBuffers(cv::Size);

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
	std::array<cv::Point2f, 4> target_quad;
	cv::Mat_<float>
		rvec{1, 3},
		tvec{1, 3}
	;

	std::array<cv::Point3d, 2> projected_src{
		cv::Point3d(0.0, 0.0, 0.0),
		cv::Point3d(0.0, 0.0, 0.5)
	};
	std::array<cv::Point2d, 2> projected_dest;


};