/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, LABUST, UNIZG-FER
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the LABUST nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Author: Dula Nad
 *  Created: 01.02.2013.
 *********************************************************************/
#include <labust/tools/conversions.hpp>

#include <nav_msgs/Odometry.h>
#include <geometry_msgs/TwistStamped.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Bool.h>
#include <ros/ros.h>

#include <Eigen/Dense>

struct DvlSim
{
	DvlSim():
		maxBottomLock(100),
		maxDepth(100),
		dvl_pub(1)
	{
		ros::NodeHandle nh,ph("~");

		ph.param("MaxBottomLock",maxBottomLock, maxBottomLock);
		ph.param("MaxDepth", maxDepth, maxDepth);
		ph.param("DvlPub", dvl_pub, dvl_pub);
		std::vector<double> offset(3,0), orot(3,0);
		ph.param("offset", offset, offset);
		ph.param("orot", orot, orot);
		this->offset<<offset[0],offset[1],offset[2];
		labust::tools::quaternionFromEulerZYX(M_PI*orot[0]/180,
				M_PI*orot[1]/180,
				M_PI*orot[2]/180,
				this->orot);

		odom = nh.subscribe<nav_msgs::Odometry>("meas_odom",1,&DvlSim::onOdom, this);
		dvl_nu = nh.advertise<geometry_msgs::TwistStamped>("dvl",1);
		dvl_ned = nh.advertise<geometry_msgs::TwistStamped>("dvl_ned",1);
		altitude_pub = nh.advertise<std_msgs::Float32>("altitude",1);
		dvl_bottom = nh.advertise<std_msgs::Bool>("dvl_bottom",1);

		last_pos[0] = last_pos[1] = last_pos[2] = 0;
	}

	void onOdom(const typename nav_msgs::Odometry::ConstPtr& msg)
	{
		static int i=0;
		i=++i%dvl_pub;

		if (i == 0)
		{
			double dT = (ros::Time::now() - lastTime).toSec();
			lastTime = ros::Time::now();
			geometry_msgs::TwistStamped::Ptr dvl(new geometry_msgs::TwistStamped());
			dvl->header.stamp = ros::Time::now();
			dvl->header.frame_id = msg->header.frame_id;
			double pos[3],v[3];
			labust::tools::pointToVector(msg->pose.pose.position, pos);
			for (int i=0; i<3; ++i)
			{
				v[i] = (pos[i] - last_pos[i])/dT;
				last_pos[i] = pos[i];
			}
			labust::tools::vectorToPoint(v, dvl->twist.linear);
			dvl_ned.publish(dvl);

			dvl.reset(new geometry_msgs::TwistStamped());
			dvl->header.stamp = ros::Time::now();
			dvl->header.frame_id = "dvl_frame";
			//Calculate body-fixed speeds
			Eigen::Quaternion<double> qv(msg->pose.pose.orientation.w,
					msg->pose.pose.orientation.x,
					msg->pose.pose.orientation.y,
					msg->pose.pose.orientation.z);
			Eigen::Vector3d nu = qv.matrix().transpose() * Eigen::Vector3d(v[0],v[1],v[2]);

			//Incorporate offsets for DVL frame
			Eigen::Vector3d nur;
			labust::tools::pointToVector(msg->twist.twist.angular, nur);
			Eigen::Matrix3d omega;
			enum {p=0,q,r};
			omega<<0,-nur(r),nur(q),
					nur(r),0,-nur(p),
					-nur(q),nur(p),0;

			nu = orot.toRotationMatrix().transpose()*(nu + omega*offset);
			dvl->twist.linear.x = nu(0);
			dvl->twist.linear.y = nu(1);
			dvl->twist.linear.z = nu(2);
			dvl_nu.publish(dvl);

			if ((maxDepth - msg->pose.pose.position.z) < maxBottomLock)
			{
				std_msgs::Float32::Ptr altitude(new std_msgs::Float32());
				altitude->data = maxDepth - msg->pose.pose.position.z;
				altitude_pub.publish(altitude);
				std_msgs::Bool bl;
				bl.data = true;
				dvl_bottom.publish(bl);
			}
			else
			{
				std_msgs::Bool bl;
				bl.data = false;
				dvl_bottom.publish(bl);
			}
		}
	}

private:
	ros::Subscriber odom;
	ros::Publisher dvl_nu, altitude_pub, dvl_ned, dvl_bottom;
	ros::Time lastTime;
	Eigen::Vector3d offset;
	Eigen::Quaternion<double> orot;
	double last_pos[3];
	int dvl_pub;
	double maxBottomLock, maxDepth;
};

int main(int argc, char* argv[])
{
	ros::init(argc,argv,"dvl_sim");
	ros::NodeHandle nh;
	DvlSim dvl;
	ros::spin();
	return 0;
}


