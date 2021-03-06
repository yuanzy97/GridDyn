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

#ifndef GRIDDYNEXCITER_H_
#define GRIDDYNEXCITER_H_

#include "gridObjects.h"

class gridDynGenerator;
/** @brief the exciter class defines the interface for power grid exciters and implements a very simple version of such a device
*/

const int exciterVoltageInLocation = 0;
const int exciterVsetInLocation = 1;
const int exciterPmechInLocation = 2;
const int exciterOmegaInLocation = 3;

class gridDynExciter : public gridSubModel
{

public:
  enum exciter_flags
  {
    outside_vlim = object_flag3,
    etrigger_high = object_flag4,
  };
protected:
  double Vrmin = -5.1;         //!< [pu] lower voltage limit
  double Vrmax = 6;         //!< [pu] upper voltage limit
  double Vref = 1.0;          //!< [pu] reference voltage for voltage regulator
  double Ka = 10;            //!< [pu] amplifier gain
  double Ta = 0.004;            //!< [s]    amplifier time constant
  double vBias = 0.0;           //!< bias field level for adjusting the field output so the ref can remain at some nominal level
  int limitState = 0;
public:
  /** @brief constructor*/
  gridDynExciter (const std::string &objName = "exciter_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  /** @brief destructor*/
  virtual ~gridDynExciter ()
  {
  }

  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet,  IOdata &inputSet) override;
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
  //handle the rootfinding functions
  virtual void rootTest  (const IOdata &args, const stateData *sD, double root[],  const solverMode &sMode) override;
  virtual void rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck ( const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;

  //virtual void setTime(double time){prevTime=time;};
protected:
  void checkForLimits ();
};


/** @brief Ieee Type 1 exciter
*/
class gridDynExciterIEEEtype1 : public gridDynExciter
{
protected:
  double Ke = 1.0;            // [pu] self-excited field
  double Te = 1.0;            // [s]    exciter time constant
  double Kf = 0.03;            // [pu] stabilizer gain
  double Tf = 1.0;            // [s]    stabilizer time constant
  double Aex = 0.0;           // [pu] parameter saturation function
  double Bex = 0.0;           // [pu] parameter saturation function
public:
  gridDynExciterIEEEtype1  (const std::string &objName = "exciterIEEEtype1_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet,  IOdata &inputSet) override;
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;

  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;
  virtual void residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

  virtual void rootTest  (const IOdata &args, const stateData *sD, double root[],  const solverMode &sMode) override;
  virtual change_code rootCheck ( const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;

};


/** @brief Ieee Type 2 exciter
*/
class gridDynExciterIEEEtype2 : public gridDynExciterIEEEtype1
{
protected:
  double Tf2 = 1.0;           // [s]    stabilizer time constant
public:
  gridDynExciterIEEEtype2 (const std::string &objName = "exciterIEEEtype2_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

  virtual void rootTest  (const IOdata &args, const stateData *sD, double root[],  const solverMode &sMode) override;
  virtual change_code rootCheck ( const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;
  //virtual void setTime(double time){prevTime=time;};


};

/** @brief DC1A exciter
*/
class gridDynExciterDC1A : public gridDynExciterIEEEtype1
{
protected:
  double Tc = 0.0;
  double Tb = 1.0;
public:
  gridDynExciterDC1A (const std::string &objName = "exciterDC1A_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet,  IOdata &inputSet) override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual stringVec localStateNames () const override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

  virtual void rootTest  (const IOdata &args, const stateData *sD, double root[],  const solverMode &sMode) override;
  virtual change_code rootCheck ( const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;
  //virtual void setTime(double time){prevTime=time;};
protected:
  /** @brief the Jacobian entries for the limiter
  @param[in] V the voltage
  @param[in] Vloc the location of the voltage
  @param[in] refloc  the location of the reference
  @param[in] cj  the differental scale variable
  @parma[out] ad the array structure to store the Jacobian data in
  */
  virtual void limitJacobian (double V, int Vloc, int refLoc, double cj, arrayData<double> *ad);

};

/** @brief DC2A exciter
*/
class gridDynExciterDC2A : public gridDynExciterDC1A
{
protected:
public:
  gridDynExciterDC2A (const std::string &objName = "exciterDC2A_#");
  gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[],  const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void rootTest (const IOdata &args, const stateData *sD, double root[],  const solverMode &sMode) override;
  virtual change_code rootCheck ( const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;
protected:
  virtual void limitJacobian (double V, int Vloc, int refLoc, double cj, arrayData<double> *ad) override;
};




#endif //GRIDDYNEXCITER_H_
