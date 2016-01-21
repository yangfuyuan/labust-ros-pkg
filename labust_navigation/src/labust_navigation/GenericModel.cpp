/*********************************************************************
 *  GenericModel.cpp
 *
 *  Created on: Mar 26, 2013
 *      Author: Dula Nad
 *
 *   Modified on: Jan 13, 2015
 *      Author: Filip Mandic
 *
 ********************************************************************/

/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2016, LABUST, UNIZG-FER
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
#include <labust/navigation/GenericModel.hpp>


using namespace labust::navigation;

#include <vector>

#include <ros/ros.h>

GenericModel::GenericModel()
		//dvlModel(0),
		//xdot(0),
		//ydot(0)
				{

	this->initializeModel();
};

GenericModel::~GenericModel(){};

void GenericModel::initializeModel(){

//  x = vector::Zero(state_num);
//  xdot = 0;
//  ydot = 0;
//  /*** Setup the transition matrix ***/
//  derivativeAW();
//  R0 = R;
//  V0 = V;
//  //std::cout<<"R:"<<R<<"\n"<<V<<std::endl;
}

/*
void GenericModel::calculateXYInovationVariance(const GenericModel::matrix& P, double& xin,double &yin){

	xin = sqrt(P(xp,xp)) + sqrt(R0(xp,xp));
	yin = sqrt(P(yp,yp)) + sqrt(R0(yp,yp));
}

double GenericModel::calculateAltInovationVariance(const GenericModel::matrix& P){

	return sqrt(P(altitude,altitude)) + sqrt(R0(altitude,altitude));
}

void GenericModel::calculateUVInovationVariance(const GenericModel::matrix& P, double& uin,double &vin){

	uin = sqrt(P(u,u)) + sqrt(R0(v,v));
	vin = sqrt(P(v,v)) + sqrt(R0(v,v));
}
*/

/*********************************************************************
 *** Model data
 ********************************************************************/

void GenericModel::step(const input_type& input)
{
	  x(u) += Ts*(-surge.Beta(x(u))/surge.alpha*x(u) + 1/surge.alpha * input(X));
	  x(v) += Ts*(-sway.Beta(x(v))/sway.alpha*x(v) + 1/sway.alpha * input(Y));
	  x(w) += Ts*(-heave.Beta(x(w))/heave.alpha*x(w) + 1/heave.alpha * (input(Z) + x(buoyancy)));
	  //x(p) += Ts*(-roll.Beta(x(p))/roll.alpha*x(p) + 1/roll.alpha * (input(Kroll) + x(roll_restore)));
	  //x(q) += Ts*(-pitch.Beta(x(p))/pitch.alpha*x(q) + 1/pitch.alpha * (input(M) + x(pitch_restore)));
	  x(r) += Ts*(-yaw.Beta(x(r))/yaw.alpha*x(r) + 1/yaw.alpha * input(N) + x(b));

	  xdot = x(u)*cos(x(psi)) - x(v)*sin(x(psi)) + x(xc);
	  ydot = x(u)*sin(x(psi)) + x(v)*cos(x(psi)) + x(yc);
	  x(xp) += Ts * xdot;
	  x(yp) += Ts * ydot;
	  x(zp) += Ts * x(w);
	  x(altitude) += -Ts * x(w);
	  //\todo This is not actually true since angles depend on each other
	  //\todo Also x,y are dependent on the whole rotation matrix.
	  //\todo We make a simplification here for testing with small angles ~10°
	  //x(phi) += Ts * x(p);
	  //x(theta) += Ts * x(q);
	  x(psi) += Ts * x(r);

	  ////!!!!!!!!!!!!!!!!!!!!!!xk_1 = x;

	  ///!!!!!!!!!!!!!11derivativeAW();
}

void GenericModel::derivativeAX()
{
	A = matrix::Identity(stateNum, stateNum);

	A(u,u) = 1-Ts*(surge.beta + 2*surge.betaa*fabs(x(u)))/surge.alpha;
	A(v,v) = 1-Ts*(sway.beta + 2*sway.betaa*fabs(x(v)))/sway.alpha;
	A(w,w) = 1-Ts*(heave.beta + 2*heave.betaa*fabs(x(w)))/heave.alpha;
	A(w,buoyancy) = Ts/heave.alpha;
	//A(p,p) = 1-Ts*(roll.beta + 2*roll.betaa*fabs(x(p)))/roll.alpha;
	//A(p,roll_restore) = Ts/roll.alpha;
	//A(q,q) = 1-Ts*(pitch.beta + 2*pitch.betaa*fabs(x(q)))/pitch.alpha;
	//A(q,pitch_restore) = Ts/pitch.alpha;
	A(r,r) = 1-Ts*(yaw.beta + 2*yaw.betaa*fabs(x(r)))/yaw.alpha;
	A(r,b) = Ts;

	A(xp,u) = Ts*cos(x(psi));
	A(xp,v) = -Ts*sin(x(psi));
	A(xp,psi) = Ts*(-x(u)*sin(x(psi)) - x(v)*cos(x(psi)));
	A(xp,xc) = Ts;

	A(yp,u) = Ts*sin(x(psi));
	A(yp,v) = Ts*cos(x(psi));
	A(yp,psi) = Ts*(x(u)*cos(x(psi)) - x(v)*sin(x(psi)));
	A(yp,yc) = Ts;

	A(zp,w) = Ts;
	//\todo If you don't want the altitude to contribute comment this out.
	A(altitude,w) = -Ts;

	//A(phi,p) = Ts;
	//A(theta,q) = Ts;
	A(psi,r) = Ts;
}
void GenericModel::derivativeAW()
{

}
void GenericModel::derivativeHX()
{

	    Hnl = matrix::Zero(measSize,stateNum); // Prije je bilo identity
		Hnl.topLeftCorner(stateNum,stateNum) = matrix::Identity(stateNum,stateNum);

		ynl = vector::Zero(measSize);
		ynl.head(stateNum) = matrix::Identity(stateNum,stateNum)*x;

		//switch (dvlModel){
		//case 1:
			/*** Correct the nonlinear part ***/
			ynl(u) = x(u)+x(xc)*cos(x(psi))+x(yc)*sin(x(psi));
			ynl(v) = x(v)-x(xc)*sin(x(psi))+x(yc)*cos(x(psi));

			//Correct for the nonlinear parts
			Hnl(u,u) = 1;
			Hnl(u,xc) = cos(x(psi));
			Hnl(u,yc) = sin(x(psi));
			Hnl(u,psi) = -x(xc)*sin(x(psi)) + x(yc)*cos(x(psi));

			Hnl(v,v) = 1;
			Hnl(v,xc) = -sin(x(psi));
			Hnl(v,yc) = cos(x(psi));
			Hnl(v,psi) = -x(xc)*cos(x(psi)) - x(yc)*sin(x(psi));
//			break;
//
//		case 2:
//			/*** Correct the nonlinear part ***/
//			y(u) = x(u)*cos(x(psi)) - x(v)*sin(x(psi)) + x(xc);
//			y(v) = x(u)*sin(x(psi)) + x(v)*cos(x(psi)) + x(yc);
//
//		    /*** Correct for the nonlinear parts ***/
//			Hnl(u,xc) = 1;
//			Hnl(u,u) = cos(x(psi));
//			Hnl(u,v) = -sin(x(psi));
//			Hnl(u,psi) = -x(u)*sin(x(psi)) - x(v)*cos(x(psi));
//
//			Hnl(v,yc) = 1;
//			Hnl(v,u) = sin(x(psi));
//			Hnl(v,v) = cos(x(psi));
//			Hnl(v,psi) = x(u)*cos(x(psi)) - x(v)*sin(x(psi));
//			break;
//		}



		double rng  = sqrt(pow((x(xp)-x(xb)),2)+pow((x(yp)-x(yb)),2)+pow((x(zp)-x(zb)),2));
		double delta_x = (x(xb)-x(xp));
		double delta_y = (x(yb)-x(yp));

		if(rng<0.00001)
			rng = 0.00001;

		if(abs(delta_x)<0.00001)
			delta_x = (delta_x<0)?-0.00001:0.00001;

		if(abs(delta_y)<0.00001)
			delta_y = (delta_y<0)?-0.00001:0.00001;

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
void GenericModel::derivativeHV()
{

}






