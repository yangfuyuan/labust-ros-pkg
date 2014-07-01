/*
 * mzos.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: Filip Mandic
 */

/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2014, LABUST, UNIZG-FER#include <sensor_msgs/Joy.h>
*
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
*********************************************************************/

//#ifndef EVENTEVALUATION_HPP_
//#define EVENTEVALUATION_HPP_
//
#include <labust_mission/labustMission.hpp>
////#include <exprtk/exprtk.hpp>
#include <sensor_msgs/Joy.h>
#include <geometry_msgs/PoseStamped.h>

//
namespace labust {
	namespace data {

		class DataManager{

		public:

			/*****************************************************************
			 ***  Class functions
			 ****************************************************************/

			DataManager();

			void updateData(const auv_msgs::NavSts::ConstPtr& data);

			/*********************************************************************
			 ***  Class variables
			 ********************************************************************/

			//ros::Subscriber subExternalEvents;
			//ros::Subscriber subStateHatAbs;

			/* State Hat variables */
			enum {u=0, v, w, r, x, y, z, psi, x_var, y_var, z_var, psi_var, alt, stateHatNum};
			std::vector<double> stateHatVar;

			/* Events data variables */
			std::vector<double> eventsVar;

			/* Mission specific variables */ // Svki specificni projekt ima klasu koja extenda DataManager klasu
			std::vector<double> missionVar;
		};

		DataManager::DataManager(){

			ros::NodeHandle nh;
			//subExternalEvents= nh.subscribe<misc_msgs::ExternalEvent>("externalEvent",1, &EventEvaluation::onReceiveExternalEvent, this);
			//subStateHatAbs= nh.subscribe<auv_msgs::NavSts>("stateHatAbs",1, &DataManager::onStateHat, this);


			//externalEventContainer.resize(5); // 5 eksternih evenata
			stateHatVar.resize(stateHatNum);
		}

		void DataManager::updateData(const auv_msgs::NavSts::ConstPtr& data){

			stateHatVar[u] = data->body_velocity.x;
			stateHatVar[v] = data->body_velocity.y;
			stateHatVar[w] = data->body_velocity.z;
			stateHatVar[r] = data->orientation_rate.yaw;

			stateHatVar[x] = data->position.north;
			stateHatVar[y] = data->position.east;
			stateHatVar[z] = data->position.depth;
			stateHatVar[psi] = data->orientation.yaw;

			stateHatVar[x_var] = data->position_variance.north;
			stateHatVar[y_var] = data->position_variance.east;
			stateHatVar[z_var] = data->position_variance.depth;
			stateHatVar[psi_var] = data->orientation_variance.yaw;

			stateHatVar[alt] = data->altitude;

		}
	}
}


class MZOS : public labust::data::DataManager{

public:

	ros::Subscriber subStateHatAbs, subRelativeDistance, subTargetPos, subAgvPos;
	ros::Publisher pubEvent, pubKFmode, pubDeltaPos;

	ros::Timer timer;

	auv_msgs::NED offset;

	bool measAvailable, fineApproachFlag;

	double deltaXposTarget, deltaYposTarget, targetXpos, targetYpos, agvXpos, agvYpos;

	enum {initial = 0, coarseApproach, fineApproach, Tugging};


	MZOS():measAvailable(false),deltaXposTarget(0.0),deltaYposTarget(0.0), targetXpos(0.0), targetYpos(0.0), agvXpos(0.0), agvYpos(0.0),
			fineApproachFlag(false){

		ros::NodeHandle nh;

		/* Subscribers */
		subStateHatAbs = nh.subscribe<auv_msgs::NavSts>("stateHatAbsSlow",1, &MZOS::onStateHat, this);
		subRelativeDistance = nh.subscribe<geometry_msgs::PoseStamped>("target_pose", 1, &MZOS::onRelativeDistance, this);
		subTargetPos = nh.subscribe<auv_msgs::NED>("target_pos", 1, &MZOS::onTargetPos, this);
		subAgvPos = nh.subscribe<auv_msgs::NED>("agv_pos", 1, &MZOS::onAgvPos, this);

		/* Publishers */
		pubEvent = nh.advertise<misc_msgs::ExternalEvent>("externalEvent",1);
		pubDeltaPos = nh.advertise<auv_msgs::NED>("quad_delta_pos", 1);
		//pubKFmode = nh.advertise<std_msgs::Bool>("quad_delta_pos_available",1);
		pubKFmode = nh.advertise<std_msgs::Bool>("KFmode",1);

	}

	void onStateHat(const auv_msgs::NavSts::ConstPtr& data){

//		if(measAvailable){
//
//			measAvailable = false;
//
////			auv_msgs::NED msg;
////			msg.north = deltaXposTarget;
////			msg.east = deltaYposTarget;
////			pubDeltaPos.publish(msg);
//
//			std_msgs::Bool msg2;
//			msg2.data = true;
//			pubKFmode.publish(msg2);
//
//		} else {
//
//			std_msgs::Bool msg2;
//			msg2.data = false;
//			pubKFmode.publish(msg2);
//		}


	}


	void onTimeout(const ros::TimerEvent& timer){

	}

	void onTargetPos(const auv_msgs::NED::ConstPtr& msg){

		targetXpos = msg->north;
		targetYpos = msg->east;

		misc_msgs::ExternalEvent sendEvent;

		sendEvent.id = 1;
		sendEvent.value = targetXpos;
		pubEvent.publish(sendEvent);

		sendEvent.id = 2;
		sendEvent.value = targetYpos;
		pubEvent.publish(sendEvent);

	    /* Publish controller mode*/
	    std_msgs::Bool KFdeltaMode;
	    KFdeltaMode.data = false;
	    pubKFmode.publish(KFdeltaMode);

		if(fineApproachFlag == false){

			sendEvent.id = 5;
			sendEvent.value = coarseApproach;
			pubEvent.publish(sendEvent);

			fineApproachFlag = true;
		}
	}

	void onRelativeDistance(const geometry_msgs::PoseStamped::ConstPtr& msg){

		deltaXposTarget = -msg->pose.position.x;
		deltaYposTarget = -msg->pose.position.y;

		measAvailable = true;

		misc_msgs::ExternalEvent sendEvent;

		sendEvent.id = 1;
		sendEvent.value = deltaXposTarget;
		pubEvent.publish(sendEvent);

		sendEvent.id = 2;
		sendEvent.value = deltaYposTarget;
		pubEvent.publish(sendEvent);

		sendEvent.id = 5;
		sendEvent.value = fineApproach;
		pubEvent.publish(sendEvent);

	    /* Publish controller mode*/
	    std_msgs::Bool KFdeltaMode;
	    KFdeltaMode.data = true;
	    pubKFmode.publish(KFdeltaMode);
	}

	void onAgvPos(const auv_msgs::NED::ConstPtr& msg){

		agvXpos = msg->north;
		agvYpos = msg->east;

		misc_msgs::ExternalEvent sendEvent;

		sendEvent.id = 3;
		sendEvent.value = agvXpos;
		pubEvent.publish(sendEvent);

		sendEvent.id = 4;
		sendEvent.value = agvYpos;
		pubEvent.publish(sendEvent);

		sendEvent.id = 5;
		sendEvent.value = Tugging;
		pubEvent.publish(sendEvent);

	    /* Publish controller mode*/
	    std_msgs::Bool KFdeltaMode;
	    KFdeltaMode.data = false;
	    pubKFmode.publish(KFdeltaMode);
	}

};


int main(int argc, char** argv){

	ros::init(argc, argv, "mzos_mission");
	ros::NodeHandle nh;

	MZOS mzos;

	ros::spin();
	return 0;
}











