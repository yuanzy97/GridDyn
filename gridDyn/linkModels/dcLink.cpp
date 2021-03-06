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

#include "linkModels/dcLink.h"
#include "gridBus.h"
#include "gridArea.h"
#include "vectorOps.hpp"
#include "primary/dcBus.h"
#include "gridCoreTemplates.h"

#include <cmath>
#include <cstring>


using namespace gridUnits;
dcLink::dcLink (const std::string &objName) : gridLink (objName)
{
  opFlags.set (dc_only);
  opFlags.set(network_connected);
}

dcLink::dcLink (double rP, double Lp, const std::string &objName) : gridLink (objName),r (rP),x (Lp)
{
  opFlags.set (dc_only);
  opFlags.set(network_connected);
}

dcLink::~dcLink ()
{
}

gridCoreObject *dcLink::clone (gridCoreObject *obj) const
{
  dcLink *nobj = cloneBase<dcLink, gridLink> (this, obj);
  if (nobj == nullptr)
    {
      return obj;
    }
  nobj->r = r;
  nobj->x = x;

  return nobj;
}


double  dcLink::timestep (double /*ttime*/, const solverMode & /*sMode*/)
{

  if (!enabled)
    {
      return 0;

    }
  updateLocalCache ();

  /*if (scheduled)
  {
  Psched=sched->timestepP(time);
  }*/
  return linkFlows.P1;
}


int dcLink::updateBus (gridBus *bus, index_t busnumber)
{
  if (dynamic_cast<dcBus *> (bus))
    {
      return gridLink::updateBus (bus, busnumber);
    }
  else
    {
      return (OBJECT_NOT_RECOGNIZED);
    }
}

double dcLink::getMaxTransfer () const
{
  if (!isConnected ())
    {
      return 0;
    }
  if (Erating > 0)
    {
      return Erating;
    }
  else if (ratingB > 0)
    {
      return ratingB;
    }
  else if (ratingA > 0)
    {
      return ratingA;
    }
  else
    {
      return ((r > 0) ? (1 / r) : kBigNum);
    }
}
// set properties
int  dcLink::set (const std::string &param,  const std::string &val)
{

  return gridLink::set (param, val);
}

int  dcLink::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  if ((param == "r")||(param == "rdc"))
    {
      r = val;
    }
  else if ((param == "l")||(param == "x")||(param == "ldc"))
    {
      x = val;
      // set line admittance
    }
  else
    {
      out = gridLink::set (param, val, unitType);
    }
  return out;
}

void dcLink::pFlowObjectInitializeA (double time0, unsigned long flags)
{
  gridLink::pFlowObjectInitializeA (time0,flags);
  if (enabled)
    {
      if (opFlags[fixed_target_power])
        {
          fixRealPower (Pset,1);
        }
    }
}

void dcLink::pFlowObjectInitializeB ()
{
  if (enabled)
    {
      updateLocalCache ();
      if (opFlags[fixed_target_power])
        {
          fixRealPower (Pset, 1);
        }
    }

}

void dcLink::dynObjectInitializeA (double /*time0*/, unsigned long /*flags*/)
{
  m_dstate_dt.resize (1);
  m_state.resize (1);
  updateLocalCache ();
  if (x != 0)
    {
      m_state[0] = Idc;
      m_dstate_dt[0] = 0.0;
    }
  else
    {

    }
}


void dcLink::loadSizes (const solverMode &sMode, bool /*dynOnly*/)
{
  auto so = offsets.getOffsets (sMode);
  if (isDynamic (sMode))
    {
      if (x != 0)
        {
          if (!isAlgebraicOnly (sMode))
            {
              so->total.diffSize = 1;
              so->total.jacSize = 3;
            }
          else
            {
              so->total.diffSize = 0;
              so->total.jacSize = 0;
            }
        }
      else
        {
          so->total.diffSize = 0;
          so->total.jacSize = 0;
        }
    }
  else if (r <= 0) //superconducting
    {
      so->total.algSize = 1;
      so->total.jacSize = 2;
    }
  else
    {
      so->total.algSize = 0;
      so->total.jacSize = 0;
      so->total.diffSize = 0;
    }
  so->rjLoaded = true;
  so->stateLoaded = true;
}


void dcLink::ioPartialDerivatives (index_t busId, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
  // check if line is enabled
  updateLocalCache  (sD,sMode);
  if (!(enabled))
    {
      return;
    }

  if ((busId == 2) || (busId == B2->getID ()))
    {
      ad->assignCheckCol (PoutLocation, argLocs[voltageInLocation], -Idc);
    }
  else
    {
      ad->assignCheckCol (PoutLocation, argLocs[voltageInLocation], Idc);
    }
}

void dcLink::outputPartialDerivatives (index_t busId, const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
  if (!(enabled))
    {
      return;
    }
  updateLocalCache (sD, sMode);

  double P1V2 = 0.0, P2V1 = 0.0;
  if (!isDynamic (sMode))
    {
      if (r > 0)
        {
          P1V2 = -linkInfo.v1 / r;
          P2V1 = -linkInfo.v1 / r;
        }
      else
        {
        }
    }           // in some other mode
  else
    {

    }
  //int mode = B1->getMode(sMode) * 4 + B2->getMode(sMode);



  //	ad->assign(B1Voffset, B2Voffset, Q1V2);
  //	ad->assign(B2Voffset, B1Voffset, Q2V1);
  if ((busId == 2)||(busId == B2->getID ()))
    {

      if (stateSize (sMode) > 0)
        {
          auto offset = isDynamic (sMode) ? offsets.getDiffOffset (sMode) : offsets.getAlgOffset (sMode);
          ad->assign (PoutLocation, offset, -linkInfo.v2);
        }
      else
        {
          int B1Voffset = B1->getOutputLoc(sMode, voltageInLocation);
          ad->assignCheckCol (PoutLocation, B1Voffset, P2V1);
        }
    }
  else
    {

      if (stateSize (sMode) > 0)
        {
          auto offset = isDynamic (sMode) ? offsets.getDiffOffset (sMode) : offsets.getAlgOffset (sMode);
          ad->assign (PoutLocation, offset, linkInfo.v1);
        }
      else
        {
          int B2Voffset = B2->getOutputLoc(sMode, voltageInLocation);
          ad->assignCheckCol (PoutLocation, B2Voffset, P1V2);
        }
    }

}


void dcLink::jacobianElements (const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {

      int B1Voffset = B1->getOutputLoc(sMode, voltageInLocation);
      int B2Voffset = B2->getOutputLoc(sMode, voltageInLocation);
      updateLocalCache (sD, sMode);
      if (isDynamic (sMode))
        {
          auto offset = offsets.getDiffOffset (sMode);
          ad->assignCheckCol (offset, B1Voffset, 1 / x);
          ad->assignCheckCol (offset, B2Voffset, -1 / x);
          ad->assign (offset, offset, -r / x - sD->cj);
        }
      else
        {
          auto offset = offsets.getAlgOffset (sMode);
          ad->assignCheckCol (offset, B1Voffset, 1);
          ad->assignCheckCol (offset, B2Voffset, -1);
          if (opFlags.test (fixed_target_power))
            {
              ad->assignCheckCol (offset,B1Voffset,-Pset / (linkInfo.v1 * linkInfo.v1));
              ad->assign (offset,offset,-1);
            }
        }
    }
}

void dcLink::residual (const stateData *sD, double resid[], const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      updateLocalCache (sD,sMode);
      if (isDynamic (sMode))
        {
          auto offset = offsets.getDiffOffset (sMode);
          resid[offset] = (linkInfo.v1 - linkInfo.v2 - r * sD->state[offset]) / x - sD->dstate_dt[offset];
        }
      else
        {
          auto offset = offsets.getAlgOffset (sMode);
          if (opFlags.test (fixed_target_power))
            {
              resid[offset] = (linkInfo.v1 - linkInfo.v2) + Pset / linkInfo.v1 - sD->state[offset];
            }
          else
            {
              resid[offset] = linkInfo.v1 - linkInfo.v2;
            }
        }
    }
}

void dcLink::setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      if (isDynamic (sMode))
        {
          auto offset = offsets.getDiffOffset (sMode);
          m_state[0] = state[offset];
          m_dstate_dt[0] = dstate_dt[offset];
          Idc = m_state[0];
        }
      else
        {
          auto offset = offsets.getAlgOffset (sMode);
          Idc = state[offset];
        }

    }
  prevTime = ttime;
}

void dcLink::guess (const double /*ttime*/, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      if (isDynamic (sMode))
        {
          auto offset = offsets.getDiffOffset (sMode);
          state[offset] = m_state[0];
          dstate_dt[offset] = m_dstate_dt[0];
        }
      else
        {
          auto offset = offsets.getAlgOffset (sMode);
          state[offset] = Idc;
        }
    }
}


void dcLink::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  if (stateSize (sMode) > 0)
    {
      std::string prefix2 = prefix + name + ':';
      auto offset = (isDynamic (sMode)) ? offsets.getDiffOffset (sMode) : offsets.getAlgOffset (sMode);
      stNames[offset] = prefix2 + "idc";
    }
}

void  dcLink::updateLocalCache (const stateData *sD, const solverMode &sMode)
{
  if ((linkInfo.seqID == sD->seqID) && (sD->seqID != 0))
    {
      return;
    }

  if (!enabled)
    {
      return;
    }
  std::memset (&linkInfo, 0, sizeof(linkI));


  linkInfo.v1 = B1->getVoltage (sD->state, sMode);
  linkInfo.v2 = B2->getVoltage (sD->state, sMode);
  if (stateSize (sMode) > 0)
    {
      auto offset = (isDynamic (sMode)) ? offsets.getDiffOffset (sMode) : offsets.getAlgOffset (sMode);
      Idc = sD->state[offset];
    }
  else
    {
      if (r > 0)
        {
          Idc = (linkInfo.v1 - linkInfo.v2) / r;

          //	Q2 = P2*sqrt(k3sq2*k3sq2 - gamma*gamma);
        }
      else
        {
          Idc = Pset / linkInfo.v1;
        }
    }
  linkFlows.P1 = linkInfo.v1 * Idc;
  linkFlows.P2 = -linkInfo.v2 * Idc;

}

void  dcLink::updateLocalCache ()
{

  std::memset (&linkInfo, 0, sizeof(linkInfo));

  if (enabled)
    {
      linkInfo.v1 = B1->getVoltage ();
      linkInfo.v2 = B2->getVoltage ();
      linkFlows.P1 = linkInfo.v1 * Idc;
      linkFlows.P2 = -linkInfo.v2 * Idc;
    }
}

int dcLink::fixRealPower (double power,index_t mterminal,index_t fixedTerminal,gridUnits::units_t unitType)
{
  int ret = 0;
  opFlags.set (fixed_target_power);
	if (fixedTerminal==0)
	{
		fixedTerminal = mterminal;
	}
  Pset = unitConversion (power,unitType,puMW,systemBasePower);
  if ((fixedTerminal == 2) || (fixedTerminal == B2->getID ()))
    {
      if (B2->getType () == gridBus::busType::SLK)
        {
          linkInfo.v2 = B2->getVoltage ();
          Idc = power / linkInfo.v2;
          double v1 = linkInfo.v2 - Idc * r;
          B1->setVoltageAngle (v1, 0);
          updateLocalCache ();
          return B1->propogatePower (true);
        }
      else if (B1->getType () == gridBus::busType::SLK)
        {
          linkInfo.v1 = B1->getVoltage ();
          if (r > 0)
            {
              double temp = linkInfo.v1 / r;
              Idc = 0.5 * (-temp + std::sqrt (temp * temp + 4 * power / r));
            }
          else
            {
              Idc = power / linkInfo.v1;
            }
          double v2 = power / Idc;
          B2->setVoltageAngle (v2, 0);
          updateLocalCache ();
          ret = B2->propogatePower (true);
        }
      else
        {
          double v1 = B1->getVoltage ();
          double v2 = B2->getVoltage ();
          double delta = (r > 0) ? (power * r - v2 * v2 + v2 * v1) / (v1 + v2) : (v1 - v2) / 2;
          v1 = v1 - delta;
          v2 = v2 + delta;
          B1->setVoltageAngle (v1, 0);
          B2->setVoltageAngle (v2, 0);
          Idc = (r > 0) ? (v1 - v2) / r : power / v1;
          updateLocalCache ();
          B1->propogatePower (false);
          B2->propogatePower (false);
        }
      Idc = -Idc;
    }
  else
    {
      if (B1->getType () == gridBus::busType::SLK)
        {
          linkInfo.v1 = B1->getVoltage ();
          Idc = power / linkInfo.v1;
          double v2 = linkInfo.v1 - Idc * r;
          B2->setVoltageAngle (v2, 0);
          updateLocalCache ();
          return B2->propogatePower (true);
        }
      else if (B2->getType () == gridBus::busType::SLK)
        {
          linkInfo.v2 = B2->getVoltage ();
          if (r > 0)
            {
              double temp = linkInfo.v2 / r;
              Idc = 0.5 * (-temp + std::sqrt (temp * temp + 4 * power / r));
            }
          else
            {
              Idc = -power / linkInfo.v2;
            }

          double v1 = power / Idc;
          B1->setVoltageAngle (v1, 0);
          updateLocalCache ();
          ret = B1->propogatePower (true);
        }
      else
        {
          double v1 = B1->getVoltage ();
          double v2 = B2->getVoltage ();
          double delta = (r > 0) ? (power * r - v1 * v1 + v2 * v1) / (v1 + v2) : (v2 - v1) / 2;
          v1 = v1 + delta;
          v2 = v2 - delta;
          B1->setVoltageAngle (v1, 0);
          B2->setVoltageAngle (v2, 0);
          Idc = (r > 0) ? (v1 - v2) / r : power / v1;
          updateLocalCache ();
          B1->propogatePower (false);
          B2->propogatePower (false);
        }
    }
  return ret;
}
