/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "fncsLoad.h"
#include "gridCoreTemplates.h"
#include "gridBus.h"
#include "stringOps.h"
#include "fncsLibrary.h"
#include "vectorOps.hpp"
#include "fncsSupport.h"


fncsLoad::fncsLoad(const std::string &objName) : gridRampLoad(objName)
{

}

coreObject *fncsLoad::clone(coreObject *obj) const
{
	fncsLoad *nobj = cloneBase<fncsLoad, gridRampLoad>(this, obj);
	if (!(nobj))
	{
		return obj;
	}
	nobj->inputUnits = inputUnits;
	nobj->scaleFactor = scaleFactor;
	nobj->loadKey = loadKey;
	nobj->voltageKey = voltageKey;

	return nobj;
}


void fncsLoad::pFlowObjectInitializeA(gridDyn_time time0, unsigned long flags)
{
	gridRampLoad::pFlowObjectInitializeA(time0, flags);

	double V = bus->getVoltage();
	double A = bus->getAngle();
	

	
	if (!voltageKey.empty())
	{
		std::complex<double> Vc = std::polar(V, A);
		Vc *= baseVoltage;
		fncsSendComplex(voltageKey, Vc);
	}
	

	prevP = getP();
	prevQ = getQ();
}

void fncsLoad::pFlowObjectInitializeB()
{
	updateA(prevTime);
	//clear any ramps initially
	dPdt = 0.0;
	dQdt = 0.0;
}

void fncsLoad::updateA(gridDyn_time time)
{
	double V = bus->getVoltage();
	double A = bus->getAngle();

	if (!voltageKey.empty())
	{
		std::complex<double> Vc = std::polar(V, A);
		Vc *= baseVoltage;
		fncsSendComplex(voltageKey, Vc);
	}
	lastUpdateTime = time;
}

gridDyn_time fncsLoad::updateB()
{
	nextUpdateTime += updatePeriod;
	
	//now get the updates
	auto res = fncsGetComplex(loadKey);
	if (res.real() == kNullVal)
	{
		dPdt = 0.0;
		dQdt = 0.0;
		prevP = getP();
		prevQ = getQ();
		return nextUpdateTime;
	}
	res - res*scaleFactor;
	double newP = unitConversion(res.real(), inputUnits, gridUnits::puMW, systemBasePower, baseVoltage);
	double newQ = unitConversion(res.imag(), inputUnits, gridUnits::puMW, systemBasePower, baseVoltage);

	if (opFlags[use_ramp])
	{
		if (opFlags[predictive_ramp]) //ramp uses the previous change to guess into the future
		{
			setP(newP);
			setQ(newQ);
			if ((prevTime - lastUpdateTime) > 0.001)
			{
				dPdt = (newP - prevP) / (prevTime - lastUpdateTime);
				dQdt = (newQ - prevQ) / (prevTime - lastUpdateTime);
			}
			else
			{
				dPdt = 0.0;
				dQdt = 0.0;
			}
			prevP = newP;
			prevQ = newQ;
			prevTime = lastUpdateTime;
		}
		else // output will ramp up to the specified value in the update period
		{
			dPdt = (newP - getP()) / updatePeriod;
			dQdt = (newQ - getQ()) / updatePeriod;
			prevP = getP();
			prevQ = getQ();
		}
	}
	else
	{
		setP(newP);
		setQ(newQ);
		prevP = newP;
		prevQ = newQ;
		dPdt = 0;
		dQdt = 0;
	}
	return nextUpdateTime;
}

void fncsLoad::timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode)
{
	while (ttime > nextUpdateTime)
	{
		updateA(ttime);
		updateB();
	}

	gridRampLoad::timestep(ttime, args, sMode);
}


void fncsLoad::setFlag(const std::string &param, bool val)
{

	if (param == "initial_queury")
	{
		opFlags.set(initial_query, val);
	}
	else if (param == "predictive")
	{
		if (val)
		{
			opFlags.set(use_ramp, val);
			opFlags.set(predictive_ramp, val);
		}
		else
		{
			opFlags.set(predictive_ramp, false);
		}

	}
	else if (param == "interpolate")
	{
		opFlags.set(use_ramp, val);
		opFlags.set(predictive_ramp, !val);
	}
	else if (param == "step")
	{
		opFlags.set(use_ramp, !val);
	}
	else if (param == "use_ramp")
	{
		opFlags.set(use_ramp, val);
	}
	else
	{
		gridRampLoad::setFlag(param, val);
	}

}

void fncsLoad::set(const std::string &param, const std::string &val)
{

	if (param == "voltagekey")
	{
		voltageKey = val;
		fncsRegister::instance()->registerPublication(voltageKey, fncsRegister::dataType::fncsComplex);
	}
	else if (param == "loadkey")
	{
		loadKey = val;
		auto Punit = unitConversion(getP(), gridUnits::puMW, gridUnits::MW, systemBasePower);
		auto Qunit = unitConversion(getQ(), gridUnits::puMW, gridUnits::MW, systemBasePower);
		std::string def = std::to_string(Punit) + "+" + std::to_string(Qunit) + "j";
		fncsRegister::instance()->registerSubscription(loadKey, fncsRegister::dataType::fncsComplex, def);

	}
	else if (param == "units")
	{
		inputUnits = gridUnits::getUnits(val);
	}

	else
	{
		//no reason to set the ramps in fncs load so go to gridLoad instead
		gridLoad::set(param, val);
	}

}


void fncsLoad::set(const std::string &param, double val, gridUnits::units_t unitType)
{

	if ((param == "scalefactor") || (param == "scaling"))
	{
		scaleFactor = val;
	}
	else
	{
		gridLoad::set(param, val, unitType);
	}
}