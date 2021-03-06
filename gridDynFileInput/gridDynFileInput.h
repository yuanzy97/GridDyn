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

#ifndef GRIDDYNINPUT_H_
#define GRIDDYNINPUT_H_

#include "readerInfo.h"

class gridEvent;
class gridRecorder;
class gridCoreObject;
class gridDynSimulation;
//!< typedef for convenience
typedef std::vector<std::string> stringVec;

#define READER_VERBOSE_PRINT 3
#define READER_NORMAL_PRINT 2
#define READER_SUMMARY_PRINT 1
#define READER_NO_PRINT 0

#define READER_WARN_ALL 2
#define READER_WARN_IMPORTANT 1
#define READER_WARN_NONE 0

namespace readerConfig {
extern int printMode;
extern int warnMode;
extern int warnCount;
void setPrintMode (int val);
void setWarnMode (int val);
void setPrintMode (const std::string &val);
void setWarnMode (int val);
void setWarnMode (const std::string &val);

void setDefaultMatchType (const std::string &matchType);

/** @brief enumeration describing how the matching should be done
*/
enum class match_type
{
  strict_case_match,              //!< match only the cases given
  capital_case_match,              //!< match where the first letter can be either case, or all lower case, or all capitals
  any_case_match,              //!< match where any letter can be any case

};

const double PI = 3.141592653589793;
extern match_type defMatchType;
}

/** @brief defined flags for the readerInfo*/
enum readerFlags
{
  ignore_step_up_transformer = 1, //!< ignore any step up transformer definitions
};

std::shared_ptr<gridDynSimulation> readXML (const std::string &filename, readerInfo *ri = nullptr);

gridDynSimulation * readSimXMLFile (const std::string &filename, readerInfo *ri = nullptr);

std::uint32_t addflags (std::uint32_t iflags, const std::string &flags);

void loadFile (gridCoreObject *parentObject, const std::string &filename, readerInfo *ri = nullptr, std::string ext = "");

void loadCDF (gridCoreObject *parentObject,const std::string &filename, const basicReaderInfo &bri = defInfo);

void loadPSP (gridCoreObject *parentObject, const std::string &filename, const basicReaderInfo &bri = defInfo);
void loadPTI (gridCoreObject *parentObject, const std::string &filename, const basicReaderInfo &bri = defInfo);

void loadRAW (gridCoreObject *parentObject, const std::string &filename, const basicReaderInfo &bri = defInfo);

void loadDYR (gridCoreObject *parentObject, const std::string &filename, const basicReaderInfo &bri = defInfo);
void loadEPC (gridCoreObject *parentObject, const std::string &filename, const basicReaderInfo &bri = defInfo);

//wrapper function to detect m file format for matpower or PSAT
void loadMFile (gridCoreObject *parentObject, const std::string &filename, const basicReaderInfo &bri = defInfo);

void loadCSV (gridCoreObject *parentObject, const std::string &filename, readerInfo *ri, const std::string &oname = "");

void objectParameterSet (const std::string &label, gridCoreObject *obj, gridParameter &param);

void addToParent (gridCoreObject *objectToAdd, gridCoreObject *parentObject);

#endif
