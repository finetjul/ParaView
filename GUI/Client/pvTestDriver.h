/*=========================================================================

  Program:   Visualization Toolkit
  Module:    pvTestDriver.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME pvTestDriver - A program to run paraview for testing mpi and server modes.
// .DESCRIPTION
// 


#ifndef __pvTestDriver_h
#define __pvTestDriver_h

#include <vtkstd/string>
#include <vtkstd/vector>
#include <kwsys/Process.h>
#include <kwsys/stl/string>
#include <kwsys/stl/vector>

class pvTestDriver 
{
public:
  int Main(int argc, char* argv[]);
  pvTestDriver();
protected:
  void SeparateArguments(const char* str, 
                         vtkstd::vector<vtkstd::string>& flags);
  
  int WaitForAndPrintData(const char* pname, kwsysProcess* process, double timeout,
                          int* foundWaiting, vtkstd::string* output=0 );
  void ReportCommand(const char* const* command, const char* name);
  int ReportStatus(kwsysProcess* process, const char* name);
  int ProcessCommandLine(int argc, char* argv[]);
  void CollectConfiguredOptions();
  void CreateCommandLine(kwsys_stl::vector<const char*>& commandLine,
                         const char* paraviewFlags, 
                         const char* numProc,
                         int argStart=0,
                         int argCount=0,
                         char* argv[]=0);
  
  int StartServer(kwsysProcess* server, const char* name);
  int OutputStringHasError(const char* pname, vtkstd::string& output);
private:
  vtkstd::string ParaView;
  vtkstd::string MPIRun;
  vtkstd::vector<vtkstd::string> MPIFlags;
  vtkstd::vector<vtkstd::string> MPIPostFlags;
  vtkstd::string MPINumProcessFlag;
  vtkstd::string MPIClientNumProcessFlag;
  vtkstd::string MPIServerNumProcessFlag;
  vtkstd::string MPIRenderServerNumProcessFlag;
  int RenderServerNumProcesses;
  double TimeOut;
  int TestRenderServer;
  int TestServer;
  int ArgStart;
  int AllowErrorInOutput;
};

#endif

