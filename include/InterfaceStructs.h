/****************************************************************************
 *
 *   Copyright (c) 2015-2016 AIT, ETH Zurich. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name AIT nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/*
 * InterfaceStructs.h
 *
 *  Created on: Aug 25, 2015
 *      Author: nicolas
 */

#ifndef VIO_ROS_SRC_PARSEYAML_H_
#define VIO_ROS_SRC_PARSEYAML_H_

#include <yaml-cpp/yaml.h>
#include <string>
#include <ros/console.h>
#include "Precision.h"
//#include "SLAM_includes.h"

// VIOParameters
// =========================================================
struct VIOParameters
{
	int num_points_per_anchor;
	int num_anchors;
	int max_ekf_iterations;
	bool fixed_feature;
	bool delayed_initialization;
	bool mono;
	bool RANSAC;
	bool full_stereo;
};

// ProcessNoise
// =========================================================
struct ProcessNoise
{
	FloatType qv;
	FloatType qw;
	FloatType qao;
	FloatType qwo;
	FloatType qR_ci;
};

// NoiseParamters
// =========================================================
struct NoiseParameters
{
	ProcessNoise process_noise;
	FloatType image_noise;
	FloatType inv_depth_initial_unc;
	FloatType gyro_bias_initial_unc[3];
	FloatType acc_bias_initial_unc[3];
};

// VIOMeasurements
// =========================================================
struct VIOMeasurements
{
	FloatType gyr[3];
	FloatType acc[3];
};

// IMUOffset
// =========================================================
struct IMUState
{
	FloatType pos[3];
	FloatType att[4];
	FloatType gyro_bias[3];
	FloatType acc_bias[3];
};

// RobotState
// =========================================================
struct RobotState
{
	FloatType pos[3];
	FloatType att[4];
	FloatType vel[3];
	IMUState IMU;
};

// AnchorPose
// =========================================================
struct AnchorPose
{
	FloatType pos[3];
	FloatType att[4];
};

// ReferenceCommand
// =========================================================
struct ReferenceCommand
{
	FloatType position[4]; // x, y, z, yaw
	FloatType velocity[4];
};

// cameraParameters
// =========================================================
struct CameraParameters // parameters of one camera
{
	FloatType RadialDistortion[3];
	FloatType TangentialDistortion[2];
	FloatType FocalLength[2];
	FloatType PrincipalPoint[2];
	enum {PLUMB_BOB = 0, ATAN = 1};
	int DistortionModel;
};

struct DUOParameters // parameters of both cameras plus stereo calibration
{
	CameraParameters CameraParameters1;
	CameraParameters CameraParameters2;

	FloatType r_lr[3];
	FloatType R_lr[9];
	FloatType R_rl[9];
	FloatType R_ci[9];
	FloatType t_ci[3];
	FloatType gyro_bias[3];
	FloatType acc_bias[3];
	FloatType time_shift;

};


inline DUOParameters parseYaml(const YAML::Node& node)
{
	DUOParameters v;

	YAML::Node r_lr = node["r_lr"];

	for (std::size_t i = 0; i < r_lr.size(); i++)
	{
		v.r_lr[i] = r_lr[i][0].as<double>();
	}

	YAML::Node R_lr = node["R_lr"];
	for (std::size_t i = 0; i < R_lr.size(); i++)
	{
		YAML::Node row = R_lr[i];
		for (std::size_t j = 0; j < row.size(); j++) // matlab is column major
			v.R_lr[i + j*3] = row[j].as<double>();
	}

	YAML::Node R_rl = node["R_rl"];
	for (std::size_t i = 0; i < R_rl.size(); i++)
	{
		YAML::Node row = R_rl[i];
		for (std::size_t j = 0; j < row.size(); j++) // matlab is column major
			v.R_rl[i + j*3] = row[j].as<double>();
	}

	if(const YAML::Node R_ci = node["R_ci"]) {
		for (std::size_t i = 0; i < R_ci.size(); i++)
		{
			YAML::Node row = R_ci[i];
			for (std::size_t j = 0; j < row.size(); j++) // matlab is column major
				v.R_ci[i + j*3] = row[j].as<double>();
		}
	} else {
		ROS_WARN("Did not find R_ci, using default");
		for (std::size_t i = 0; i < 3; i++)
		{
			for (std::size_t j = 0; j < 3; j++)
			{
				if (i == j)
					v.R_ci[i + j*3] = 1.0;
				else
					v.R_ci[i + j*3] = 0.0;
			}
		}
	}

	if(const YAML::Node t_ci = node["t_ci"]) {
		for (std::size_t i = 0; i < t_ci.size(); i++)
		{
			v.t_ci[i] = t_ci[i][0].as<double>();
		}
	} else {
		ROS_WARN("Did not find t_ci, using default");
		for (std::size_t i = 0; i < 3; i++)
		{
			v.t_ci[i] = 0.0;
		}
	}

	if(const YAML::Node gyro_bias = node["gyro_bias"]) {
		for (std::size_t i = 0; i < gyro_bias.size(); i++)
		{
			v.gyro_bias[i] = gyro_bias[i][0].as<double>();
		}
	} else {
		ROS_WARN("Did not find gyro_bias, using default");
		for (std::size_t i = 0; i < 3; i++)
		{
			v.gyro_bias[i] = 0.0;
		}
	}

	if(const YAML::Node acc_bias = node["acc_bias"]) {
		for (std::size_t i = 0; i < acc_bias.size(); i++)
		{
			v.acc_bias[i] = acc_bias[i][0].as<double>();
		}
	} else {
		ROS_WARN("Did not find acc_bias, using default");
		for (std::size_t i = 0; i < 3; i++)
		{
			v.acc_bias[i] = 0.0;
		}
	}

	if(const YAML::Node time_shift = node["time_shift"]) {
		v.time_shift = time_shift.as<double>();
	} else {
		ROS_WARN("Did not find time_shift, using default");
		v.time_shift = 0.0;
	}

	std::string plumb_bob = "plumb_bob";
	std::string atan = "atan";


	// camera 1
	YAML::Node DistortionModel = node["CameraParameters1"]["DistortionModel"];
	if (!atan.compare(DistortionModel.as<std::string>()))
		v.CameraParameters1.DistortionModel = v.CameraParameters1.ATAN;
	else
		v.CameraParameters1.DistortionModel = v.CameraParameters1.PLUMB_BOB;

	YAML::Node RadialDistortion = node["CameraParameters1"]["RadialDistortion"];
	for (std::size_t i = 0; i < RadialDistortion.size(); i++)
		v.CameraParameters1.RadialDistortion[i] = RadialDistortion[i].as<double>();
	for (std::size_t i = RadialDistortion.size(); i < 3; i++)
		v.CameraParameters1.RadialDistortion[i] = 0.0;

//	YAML::Node TangentialDistortion = node["CameraParameters1"]["TangentialDistortion"];
//	for (std::size_t i = 0; i < TangentialDistortion.size(); i++)
//		v.CameraParameters1.TangentialDistortion[i] = TangentialDistortion[i].as<double>();

	YAML::Node FocalLength = node["CameraParameters1"]["FocalLength"];
	for (std::size_t i = 0; i < FocalLength.size(); i++)
		v.CameraParameters1.FocalLength[i] = FocalLength[i].as<double>();

	YAML::Node PrincipalPoint = node["CameraParameters1"]["PrincipalPoint"];
	for (std::size_t i = 0; i < PrincipalPoint.size(); i++)
		v.CameraParameters1.PrincipalPoint[i] = PrincipalPoint[i].as<double>();

	// camera 2
	DistortionModel = node["CameraParameters2"]["DistortionModel"];
	if (!atan.compare(DistortionModel.as<std::string>()))
		v.CameraParameters2.DistortionModel = v.CameraParameters2.ATAN;
	else
		v.CameraParameters2.DistortionModel = v.CameraParameters2.PLUMB_BOB;

	RadialDistortion = node["CameraParameters2"]["RadialDistortion"];
	for (std::size_t i = 0; i < RadialDistortion.size(); i++)
		v.CameraParameters2.RadialDistortion[i] = RadialDistortion[i].as<double>();
	for (std::size_t i = RadialDistortion.size(); i < 3; i++)
		v.CameraParameters2.RadialDistortion[i] = 0.0;

//	TangentialDistortion = node["CameraParameters2"]["TangentialDistortion"];
//	for (std::size_t i = 0; i < TangentialDistortion.size(); i++)
//		v.CameraParameters2.TangentialDistortion[i] = TangentialDistortion[i].as<double>();

	FocalLength = node["CameraParameters2"]["FocalLength"];
	for (std::size_t i = 0; i < FocalLength.size(); i++)
		v.CameraParameters2.FocalLength[i] = FocalLength[i].as<double>();

	PrincipalPoint = node["CameraParameters2"]["PrincipalPoint"];
	for (std::size_t i = 0; i < PrincipalPoint.size(); i++)
		v.CameraParameters2.PrincipalPoint[i] = PrincipalPoint[i].as<double>();

	return v;
}

#endif /* VIO_ROS_SRC_PARSEYAML_H_ */
