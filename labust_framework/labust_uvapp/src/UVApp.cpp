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
#include <labust/vehicles/UVApp.hpp>
#include <labust/xml/XMLReader.hpp>
#include <labust/xml/XMLWriter.hpp>
#include <labust/vehicles/VehicleDriver.hpp>
#include <labust/navigation/NavDriver.hpp>
#include <labust/control/ControlDriver.hpp>

using labust::vehicles::UVApp;

UVApp::UVApp(const labust::xml::ReaderPtr reader, const std::string& id)
{
	_xmlNode* org_node = reader->currentNode();
	reader->useNode(reader->value<_xmlNode*>("UVApp" + (id.empty()?"":("[@id='" + id + "']"))));

	hdCon.loadPlugin("hdcontrol-plug",reader);
	lfCon.loadPlugin("lfcontrol-plug",reader);
	//manCon.loadPlugin("mancontrol-plug",reader);
	//ident.loadPlugin("ident-plug",reader);

	nav.loadPlugin("lfnavigation-plug",reader);

	uuv.loadPlugin("uvsim-plug","Vehicle",reader);

	applist.push_back(&hdCon());
	applist.push_back(&lfCon());
	applist.push_back(&nav());
	applist.push_back(&uuv());

	reader->useNode(org_node);

	//Initalize elements.
	labust::vehicles::zero(tau);
}

void UVApp::calculateTau()
{
	switch (mode)
	{
	case idle:
		//Stop operation
		this->stop();
		break;
	case identification:
		//Initialize the identification
		break;
	case manual: break;
	case headingDepth:
		//Do heading depth control.
		hdCon().getTAU(stateRef, stateHat,tau);
		break;
	case lineFollowing:
		//Do line following control.
		lfCon().getTAU(stateRef,stateHat,tau);
		break;
	default:
		//Stop operation
		this->stop();
		break;
	}
}

void UVApp::stop()
{
	mode = idle;
	labust::vehicles::zero(tau);
}

const labust::vehicles::stateMapRef UVApp::step(labust::vehicles::stateMapRef measurements)
{
	//For now a separate handle for simulation.
	///\todo Change the simulation to give only UUV sensor states, and to separately publish direct simulation measurements in a getData encoding.
	///When that is done this will not be needed in simulation.
	bool isSimulation = true;
	bool newMeasurement = ((measurements.find(state::x) != measurements.end()) && (measurements.find(state::y) != measurements.end()));
	double xuuv(0),yuuv(0),xbackup(0),ybackup(0);
	if (isSimulation && newMeasurement)
	{
		using namespace labust::vehicles;

		if (newMeasurement)
		{
			//Recover external modem measurements.
			xuuv = measurements[state::x];
			yuuv = measurements[state::y];
		}
	}

	uuv().getState(measurements);
	nav().prediction(tauk_1);

	if (isSimulation)
	{
		//Remove measurements so that the filter does not correct
		//in each step.
		xbackup = measurements[state::x];
		ybackup = measurements[state::y];
		measurements.erase(state::x);
		measurements.erase(state::y);

		if (newMeasurement)
		{
			//Add the measurements when new ones arrived.
			measurements[state::x] = xuuv;
			measurements[state::y] = yuuv;
		}
	}

	//Make a correction with edited measurements.
	nav().correction(measurements,stateHat);

	calculateTau();
	uuv().setTAU(tau);
	tauk_1 = tau;

	//Return original measurements
	if (isSimulation)
	{
		measurements[state::x] = xbackup;
		measurements[state::y] = ybackup;
	}

	return stateHat;
}

void UVApp::setCommand(const labust::apps::stringRef cmd)
{
	labust::xml::Reader reader(cmd);
	reader.useNode(reader.value<_xmlNode*>("/uvapp"));

	for(size_t i=0; i< applist.size(); ++i)
	{
		try
		{
			applist[i]->setCommand(cmd);
		}
		catch (std::exception& e){};
	}
}

void UVApp::getData(labust::apps::stringPtr data)
{
	labust::xml::Writer writer;
	writer.startElement("uvapp");

	for(size_t i=0; i<applist.size(); ++i)
	{
		labust::apps::stringPtr str(new labust::apps::string());
		applist[i]->getData(str);
		writer.addXML(*str);
	}
	writer.endDocument();
	(*data) = writer.toString();
}