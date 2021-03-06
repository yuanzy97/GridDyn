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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "gridRecorder.h"
#include "gridEvent.h"
#include "fileReaders.h"
#include <cstdio>
#include <cmath>

//test case for gridCoreObject object

#define RECORDER_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/recorder_tests/"

BOOST_FIXTURE_TEST_SUITE (recorder_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (ts2_tests)
{
  timeSeries2 ts2;
  timeSeries2 ts3 (1,10);
  std::vector<double> tv (10);
  std::vector<double> val (10);
  ts2.setCols (1);
  int kk;
  double t = 0.0;
  for (kk = 0; kk < 10; ++kk)
    {
      ts2.addData (t, 4.5);
      tv[kk] = t;
      val[kk] = 4.5;
      t = t + 1.0;
    }
  BOOST_CHECK_EQUAL (ts2.count, 10u);
  ts3.addData (tv, val);
  BOOST_CHECK_EQUAL (ts3.count, 10u);

  BOOST_CHECK_SMALL (compare (&ts2,&ts3), 0.0001);

  ts3.setCols (4);
  ts3.addData (val, 1);
  ts3.addData (val, 2);
  ts3.addData (val, 3);
  ts3.data[3][4] = 6.5;

  BOOST_CHECK_CLOSE (compare (&ts2, &ts3, 0, 3), 2.0, 0.0001);

}

BOOST_AUTO_TEST_CASE (file_save_tests)
{
  timeSeries2 ts2;
  ts2.setCols (1);
  int kk;
  int ret;
  double t = 0.0;
  for (kk = 0; kk < 10; ++kk)
    {
      ts2.addData (t, 4.5);
      t = t + 1.0;
    }
  BOOST_CHECK_EQUAL (ts2.count, 10u);
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "ts_test.dat");
  ts2.writeBinaryFile (fname);

  timeSeries2 ts3;

  ret = ts3.loadBinaryFile (fname);
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_EQUAL (ts3.cols, 1u);
  BOOST_CHECK_EQUAL (ts3.count, 10u);
  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
  ret = remove (fname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (file_save_tests2)
{
  timeSeries2 ts2;
  ts2.setCols (4);
  int kk;
  int ret;
  double t = 0.0;
  std::vector<double> vt (4);
  vt[0] = 4.5;
  vt[1] = 5.5;
  vt[2] = 6.5;
  vt[3] = 7.5;
  for (kk = 0; kk < 30; ++kk)
    {
      ts2.addData (t, vt);
      t = t + 1.0;
    }
  BOOST_CHECK_EQUAL (ts2.count, 30u);
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "ts_test2.dat");
  ts2.writeBinaryFile (fname);

  timeSeries2 ts3;

  ret = ts3.loadBinaryFile (fname);
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_EQUAL (ts3.cols, 4u);
  BOOST_CHECK_EQUAL (ts3.count, 30u);
  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);

  ts3.data[3][2] = 7;
  double diff = compare (&ts2, &ts3);
  BOOST_CHECK_CLOSE (diff, 0.5, 0.001);
  BOOST_CHECK_EQUAL (ts3.count, ts3.data[3].size ());
  ret = remove (fname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (recorder_test1)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test.xml");
  gds = (gridDynSimulation *)readSimXMLFile (fname);
  gds->consolePrintLevel = 5;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 1);
  val = gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);
  BOOST_CHECK_EQUAL (val, PARAMETER_FOUND);
  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "loadrec.dat");
  timeSeries2 ts3;
  int ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 31u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

}

BOOST_AUTO_TEST_CASE (recorder_test2)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test2.xml");
  gds = (gridDynSimulation *)readSimXMLFile (fname);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 1);
  val = gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);
  BOOST_CHECK_EQUAL (val, PARAMETER_FOUND);
  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "genrec.dat");
  timeSeries2 ts3;
  int ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 31u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

}

BOOST_AUTO_TEST_CASE (recorder_test3)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test3.xml");
  gds = (gridDynSimulation *)readSimXMLFile (fname);
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 3);
  val = gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);
  BOOST_CHECK_EQUAL (val, PARAMETER_FOUND);
  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "genrec.dat");
  timeSeries2 ts3;
  int ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 121u);
  BOOST_CHECK_EQUAL (ts3.cols, 4u);
  //ret = remove(recname.c_str());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  //ret = remove(recname.c_str());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "loadrec.dat");
  ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 31u);
  BOOST_CHECK_EQUAL (ts3.cols, 1u);
//	ret = remove(recname.c_str());

  BOOST_CHECK_EQUAL (ret, 0);

}

//testing the recorder as subobject and recorder found via link
BOOST_AUTO_TEST_CASE (recorder_test4)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test4.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  timeSeries2 ts3;
  int ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeries2 ts2;
  ret = ts2.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts2.count, 61u);
  BOOST_CHECK_EQUAL (ts2.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
}


//testing the vector recorcders of the simulation object
BOOST_AUTO_TEST_CASE (recorder_test5)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test5.xml");
  gds = dynamic_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busVrec.dat");
  timeSeries2 ts3;
  int ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 4u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);
  recname = std::string (RECORDER_TEST_DIRECTORY "linkVrec.dat");

  ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 5u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

}

// testing multiple items in a field
BOOST_AUTO_TEST_CASE (recorder_test6)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test6.xml");
  readerConfig::setPrintMode (0);
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  timeSeries2 ts3;
  int ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeries2 ts2;
  ret = ts2.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts2.count, 61u);
  BOOST_CHECK_EQUAL (ts2.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
}

// testing multiple :: naming for fields
BOOST_AUTO_TEST_CASE (recorder_test7)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test7.xml");
  readerConfig::setPrintMode (0);
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);
  int val=gds->getInt ("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  timeSeries2 ts3;
  int ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeries2 ts2;
  ret = ts2.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts2.count, 61u);
  BOOST_CHECK_EQUAL (ts2.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
}

// testing multiple :: naming for fields and units
BOOST_AUTO_TEST_CASE (recorder_test8)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test8.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  timeSeries2 ts3;
  int ret = ts3.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 3u);
  ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  //check to make sure the conversion is correct
  BOOST_CHECK_SMALL (ts3.data[0][3] * 180 / kPI - ts3.data[2][3],0.0001);


  recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeries2 ts2;
  ret = ts2.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts2.count, 61u);
  BOOST_CHECK_EQUAL (ts2.cols, 3u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
}

//testing gain and bias
BOOST_AUTO_TEST_CASE (recorder_test9)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test9.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);

  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();


  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeries2 ts2;
  int ret = ts2.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts2.count, 21u);
  BOOST_CHECK_EQUAL (ts2.cols, 4u);
  ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_CLOSE (ts2.data[1][2] - 1.0, ts2.data[3][2], 0.0001);


}

//testing multiple grabber calculations
BOOST_AUTO_TEST_CASE (recorder_test10)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test10.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);

  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();


  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeries2 ts2;
  int ret = ts2.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts2.count, 11u);
  BOOST_CHECK_EQUAL (ts2.cols, 3u);
  ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_CLOSE (ts2.data[0][2] - ts2.data[1][2], ts2.data[2][2], 0.0001);
  BOOST_CHECK_CLOSE (ts2.data[0][8] - ts2.data[1][8], ts2.data[2][8], 0.0001);

}

BOOST_AUTO_TEST_CASE (recorder_test11)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test11.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);

  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();


  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeries2 ts2;
  int ret = ts2.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts2.count, 11u);
  BOOST_CHECK_EQUAL (ts2.cols, 4u);
  ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_CLOSE (ts2.data[0][2] - (ts2.data[1][2] - ts2.data[2][2]), ts2.data[3][2], 0.0001);
  BOOST_CHECK_CLOSE (ts2.data[0][8] - (ts2.data[1][8] - ts2.data[2][8]), ts2.data[3][8], 0.0001);

}

//testing function calls
BOOST_AUTO_TEST_CASE (recorder_test12)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test12.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = 0;
  gds->solverSet("dynamic", "printlevel", 0);

  gds->set ("recorddirectory", RECORDER_TEST_DIRECTORY);

  gds->run ();


  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeries2 ts2;
  int ret = ts2.loadBinaryFile (recname);
  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_EQUAL (ts2.count, 11u);
  BOOST_CHECK_EQUAL (ts2.cols, 3u);
  ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_CLOSE (sin (ts2.data[0][2] - ts2.data[1][2]), ts2.data[2][2], 0.0001);
  BOOST_CHECK_CLOSE (sin (ts2.data[0][8] - ts2.data[1][8]), ts2.data[2][8], 0.0001);

}

BOOST_AUTO_TEST_SUITE_END ()
