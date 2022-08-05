#include "vision.h"

#include <core/vision.h>
#include <core/mem.h>


void TargetVision::invoke(cv::Mat& io_frame) {
		this->getFrame(io_frame);
		this->process(io_frame);
		this->PutFrame(io_frame);
	}
double TargetVision::getDist() const {
	return sqrt(pow(getX(), 2) + pow(getY(), 2) + pow(getZ(), 2));
}
double TargetVision::getLR() const {
	return atan2(getX(), getZ()) * 180/M_PI;
}
double TargetVision::getUD() const {
	return atan2(getY(), getZ()) * -180/M_PI;
}

void TargetVision::resizeBuffers(cv::Size size) {
	this->buffer = cv::Mat(size/this->scale, CV_8UC3);
	this->binary = cv::Mat(size/this->scale, CV_8UC1);
	for(size_t i = 0; i < this->channels.size(); i++) {
		channels[i] = cv::Mat(size/this->scale, CV_8UC1);
	}
}

void TargetVision::process(cv::Mat& io_frame) {
	if(io_frame.size() != this->buffer.size()*(int)this->scale) {
		this->resizeBuffers(io_frame.size());
	}
	cv::resize(io_frame, this->buffer, cv::Size(), 1.f / this->scale, 1.f / this->scale);
	cv::split(this->buffer, this->channels);
	cv::addWeighted(
		this->channels[vs2::weights_map[~this->color][0]], this->alpha,
		this->channels[vs2::weights_map[~this->color][1]], this->beta,
		this->gamma, this->binary
	);
	cv::subtract(this->channels[~this->color], this->binary, this->binary);
	memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->thresh);

	cv::findContours(this->binary, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	this->target_idx = -1;
	this->area_largest = 0;
	for(size_t i = 0; i < this->contours.size(); i++) {
		this->area_buff = cv::contourArea(this->contours[i]);
		if(this->area_buff > this->area_largest) {
			this->area_largest = this->area_buff;
			this->target_idx = i;
		}
	}
	if(this->target_idx >= 0) {
		cv::convexHull(this->contours[this->target_idx], this->target_points);
		cv::approxPolyDP(this->target_points, this->target_points, 0.1*cv::arcLength(this->contours[this->target_idx], false), true);
		rescale(this->target_points, this->scale);

		// draw points

		if(this->target_points.size() == world.size()) {
			reorderClockWise(this->target_points);
			for(size_t i = 0; i < world.size(); i++) {
				this->target_quad[i] = this->target_points[i];
			}
			for(size_t i = 0; i < this->target_quad.size(); i++) {
				cv::circle(io_frame, this->target_quad[i], 1, markup_map[~this->color][0]);
			}
			cv::solvePnP(world, this->target_quad, this->getSrcMatrix(), this->getSrcDistort(), this->rvec, this->tvec);

			cv::projectPoints(this->projected_src, this->rvec, this->tvec, this->getSrcMatrix(), this->getSrcDistort(), this->projected_dest);
			cv::line(io_frame, this->projected_dest[0], this->projected_dest[1], markup_map[~this->color][1]);

		} else {
			for(size_t i = 0; i < this->target_quad.size(); i++) {
				cv::circle(io_frame, this->target_quad[i], 1, markup_map[2][0]);
			}
		}
	}

}