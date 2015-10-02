/*********************************************************************
 *  TwoVehicleLocalizationModel.cpp
 *
 *  Created on: Sep 22, 2015
 *      Author: Filip Mandic
 *
 ********************************************************************/

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
*********************************************************************/
#include <labust/navigation/TwoVehicleLocalizationModel.hpp>

using namespace labust::navigation;

#include <vector>

#include <ros/ros.h>

TwoVehicleLocalizationModel::TwoVehicleLocalizationModel():
		dvlModel(0),
		xdot(0),
		ydot(0){

	this->initModel();
};

TwoVehicleLocalizationModel::~TwoVehicleLocalizationModel(){};

void TwoVehicleLocalizationModel::initModel(){

  x = vector::Zero(stateNum);
  xdot = 0;
  ydot = 0;
  /*** Setup the transition matrix ***/
  derivativeAW();
  R0 = R;
  V0 = V;
  //std::cout<<"R:"<<R<<"\n"<<V<<std::endl;
}

void TwoVehicleLocalizationModel::calculateXYInovationVariance(const TwoVehicleLocalizationModel::matrix& P, double& xin,double &yin){

	xin = sqrt(P(xp,xp)) + sqrt(R0(xp,xp));
	yin = sqrt(P(yp,yp)) + sqrt(R0(yp,yp));
}


void TwoVehicleLocalizationModel::calculateUVInovationVariance(const TwoVehicleLocalizationModel::matrix& P, double& uin,double &vin){

	uin = sqrt(P(u,u)) + sqrt(R0(u,u));
	//vin = sqrt(P(v,v)) + sqrt(R0(v,v));
}

void TwoVehicleLocalizationModel::step(const input_type& input){

  x(u) += 0;
  x(w) += 0;
  x(r) += 0;

  xdot = x(u)*cos(x(psi));
  ydot = x(u)*sin(x(psi));
  x(xp) += Ts * xdot;
  x(yp) += Ts * ydot;
  x(zp) += Ts * x(w);
  x(psi) += Ts * x(r);

  xk_1 = x;

  derivativeAW();
}

void TwoVehicleLocalizationModel::derivativeAW(){

	A = matrix::Identity(stateNum, stateNum);

	A(xp,u) = Ts*cos(x(psi));
	//A(xp,v) = -Ts*sin(x(psi));
	A(xp,psi) = Ts*(-x(u)*sin(x(psi)));
	//A(xp,xc) = Ts;

	A(yp,u) = Ts*sin(x(psi));
	//A(yp,v) = Ts*cos(x(psi));
	A(yp,psi) = Ts*(x(u)*cos(x(psi)));
	//A(yp,yc) = Ts;

	A(zp,w) = Ts;
	A(psi,r) = Ts;

	A(xb,ub) = Ts*cos(x(psib));
	//A(xp,v) = -Ts*sin(x(psi));
	A(xb,psib) = Ts*(-x(ub)*sin(x(psib)));
	//A(xp,xc) = Ts;

	A(yb,ub) = Ts*sin(x(psib));
	//A(yp,v) = Ts*cos(x(psi));
	A(yb,psib) = Ts*(x(ub)*cos(x(psib)));
	//A(yp,yc) = Ts;

	A(zb,wb) = Ts;
	A(psib,rb) = Ts;
}

const TwoVehicleLocalizationModel::output_type& TwoVehicleLocalizationModel::update(vector& measurements, vector& newMeas){

	std::vector<size_t> arrived;
	std::vector<double> dataVec;

	for (size_t i=0; i<newMeas.size(); ++i)
	{
		if (newMeas(i))
		{
			arrived.push_back(i);
			dataVec.push_back(measurements(i));
			newMeas(i) = 0;
		}
	}

	if (dvlModel != 0) derivativeH();

	measurement.resize(arrived.size());
	H = matrix::Zero(arrived.size(),stateNum);
	y = vector::Zero(arrived.size());
	R = matrix::Zero(arrived.size(),arrived.size());
	V = matrix::Zero(arrived.size(),arrived.size());

	for (size_t i=0; i<arrived.size();++i)
	{
		measurement(i) = dataVec[i];

		if (dvlModel != 0)
		{
			H.row(i)=Hnl.row(arrived[i]);
			y(i) = ynl(arrived[i]);
		}
		else
		{
			H(i,arrived[i]) = 1;
			y(i) = x(arrived[i]);
		}

		for (size_t j=0; j<arrived.size(); ++j)
		{
			R(i,j)=R0(arrived[i],arrived[j]);
			V(i,j)=V0(arrived[i],arrived[j]);
		}
	}

	/*** Debug Print
	std::cout<<"Setup H:"<<H<<std::endl;
	std::cout<<"Setup R:"<<R<<std::endl;
	std::cout<<"Setup V:"<<V<<std::endl;
	***/
	return measurement;
}

void TwoVehicleLocalizationModel::estimate_y(output_type& y){
	y=this->y;
}

void TwoVehicleLocalizationModel::derivativeH(){

	Hnl = matrix::Zero(measSize,stateNum);
	Hnl.topLeftCorner(stateNum,stateNum) = matrix::Identity(stateNum,stateNum);

	ynl = vector::Zero(measSize);
	ynl.head(stateNum) = matrix::Identity(stateNum,stateNum)*x;

//	switch (dvlModel){
//	case 1:
//		/*** Correct the nonlinear part ***/
//		ynl(u) = x(u)+x(xc)*cos(x(psi))+x(yc)*sin(x(psi));
//		ynl(v) = x(v)-x(xc)*sin(x(psi))+x(yc)*cos(x(psi));
//
//		//Correct for the nonlinear parts
//		Hnl(u,u) = 1;
//		Hnl(u,xc) = cos(x(psi));
//		Hnl(u,yc) = sin(x(psi));
//		Hnl(u,psi) = -x(xc)*sin(x(psi)) + x(yc)*cos(x(psi));
//
//		Hnl(v,v) = 1;
//		Hnl(v,xc) = -sin(x(psi));
//		Hnl(v,yc) = cos(x(psi));
//		Hnl(v,psi) = -x(xc)*cos(x(psi)) - x(yc)*sin(x(psi));
//		break;
//
//	case 2:
//		/*** Correct the nonlinear part ***/
//		y(u) = x(u)*cos(x(psi)) - x(v)*sin(x(psi)) + x(xc);
//		y(v) = x(u)*sin(x(psi)) + x(v)*cos(x(psi)) + x(yc);
//
//	    /*** Correct for the nonlinear parts ***/
//		Hnl(u,xc) = 1;
//		Hnl(u,u) = cos(x(psi));
//		Hnl(u,v) = -sin(x(psi));
//		Hnl(u,psi) = -x(u)*sin(x(psi)) - x(v)*cos(x(psi));
//
//		Hnl(v,yc) = 1;
//		Hnl(v,u) = sin(x(psi));
//		Hnl(v,v) = cos(x(psi));
//		Hnl(v,psi) = x(u)*cos(x(psi)) - x(v)*sin(x(psi));
//		break;
//	}

//	double rng  = sqrt(pow((x(xp)-x(xb)),2)+pow((x(yp)-x(yb)),2)+pow((x(zp)-x(zb)),2));
//
//	//if(rng == 0) rng = 0.1;
//
//	if(rng<0.0001){
//		rng = 0.0001;
//	}
//
//	double delta_xbp = x(xb)-x(xp);
//	//ROS_ERROR("delta: %f",delta_xbp);
//	if(abs(delta_xbp)<0.000000001){
//		delta_xbp = (delta_xbp<0)?-0.000000001:0.000000001;
//	}
//
//	//ROS_ERROR("delta after: %f",delta_xbp);
//
//	ynl(range) = rng;
//	ynl(bearing) = atan2((x(yp)-x(yb)),(x(xp)-x(xb)))-0*x(psi);
//	ynl(elevation) = asin((x(zp)-x(zb))/rng);
//
//	Hnl(range, xp)  = (x(xp)-x(xb))/rng;
//	Hnl(range, yp)  = (x(yp)-x(yb))/rng;
//	Hnl(range, zp)  = (x(zp)-x(zb))/rng;
//
//	Hnl(range, xb)  = -(x(xp)-x(xb))/rng;
//	Hnl(range, yb)  = -(x(yp)-x(yb))/rng;
//	Hnl(range, zb)  = -(x(zp)-x(zb))/rng;
//
//	Hnl(bearing, xp) = (x(yb)-x(yp))/(pow(delta_xbp,2)*(pow((x(yb)-x(yp))/delta_xbp,2) + 1));
//	Hnl(bearing, yp) = -1/((delta_xbp)*(pow((x(yb) - x(yp))/delta_xbp,2) + 1));
//	Hnl(bearing, xb) = -(x(yb) - x(yp))/(pow(delta_xbp,2)*(pow((x(yb) - x(yp))/delta_xbp,2) + 1));
//	Hnl(bearing, yb) = 1/((delta_xbp)*(pow((x(yb) - x(yp))/delta_xbp,2) + 1));
//	//Hnl(bearing, psi) = -1;


	double rng  = sqrt(pow((x(xp)-x(xb)),2)+pow((x(yp)-x(yb)),2)+pow((x(zp)-x(zb)),2));
	double delta_x = (x(xb)-x(xp));
	double delta_y = (x(yb)-x(yp));

	double eps = 0.00001;

	if(rng<eps)
		rng = eps;

	if(abs(delta_x)<eps)
		delta_x = (delta_x<0)?-eps:eps;

	if(abs(delta_y)<eps)
		delta_y = (delta_y<0)?-eps:eps;

	ynl(range) = rng;
	ynl(bearing) = atan2(delta_y,delta_x) -1*x(psi);
	ynl(elevation) = asin((x(zp)-x(zb))/rng);

	Hnl(range, xp)  = -(x(xb)-x(xp))/rng;
	Hnl(range, yp)  = -(x(yb)-x(yp))/rng;
	Hnl(range, zp)  = -(x(zb)-x(zp))/rng;

	Hnl(range, xb)  = (x(xb)-x(xp))/rng;
	Hnl(range, yb)  = (x(yb)-x(yp))/rng;
	Hnl(range, zb)  = (x(zb)-x(zp))/rng;

	Hnl(bearing, xp) = delta_y/(delta_x*delta_x+delta_y*delta_y);
	Hnl(bearing, yp) = -delta_x/(delta_x*delta_x+delta_y*delta_y);
	Hnl(bearing, xb) = -delta_y/(delta_x*delta_x+delta_y*delta_y);
	Hnl(bearing, yb) = delta_x/(delta_x*delta_x+delta_y*delta_y);

	Hnl(bearing, psi) = -1;



	// Nadi gresku u elevationu i sredi singularitete
//	double part1 = (x(zb) - x(zp))/(sqrt(1 - pow((x(zb) - x(zp)),2)/pow(rng,2))*(pow((rng),3)));
//	double part2 = (x(xb)*x(xb) - 2*x(xb)*x(xp) + x(xp)*x(xp) + x(yb)*x(yb) - 2*x(yb)*x(yp) + x(yp)*x(yp))/(sqrt(1 - pow((x(zb) - x(zp)),2)/pow(rng,2))*(pow((rng),3)));
//
//	Hnl(elevation,xp) = -((x(xb) - x(xp))*part1);
//	Hnl(elevation,yp) = -((x(yb) - x(yp))*part1);
//	Hnl(elevation,zp) = part2;
//	Hnl(elevation,xb) = ((x(xb) - x(xp))*part1);
//	Hnl(elevation,yb) = ((x(yb) - x(yp))*part1);
//	Hnl(elevation,zb) = -part2;
}
