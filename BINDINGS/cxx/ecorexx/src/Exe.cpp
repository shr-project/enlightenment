#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* STL */
#include <string>
#include <cassert>
#include <iostream>

#include "ecorexx/Exe.h"
#include "ecorexx/exception/ProcessNotExistingException.h"

using namespace std;

namespace Ecorexx {

static Ecore_Event_Handler *gDelHandler = NULL;
static unsigned int gExeRunning = 0;
    
Exe::Exe(const std::string &exe_cmd, const void *data) :
  mDeathPid(0)
{
  if (gExeRunning == 0)
  {
    gDelHandler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, Exe::delhandler, this);
  }
  ++gExeRunning;
  
  mExe = ecore_exe_run(exe_cmd.c_str(), data); 		
}

Exe::Exe(const std::string &exe_cmd, Ecore_Exe_Flags flags, const void *data) :
  mDeathPid(0)
{  
  if (gExeRunning == 0)
  {
    gDelHandler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, Exe::delhandler, this);
  }
  gExeRunning++;
  
  mExe = ecore_exe_pipe_run(exe_cmd.c_str(), flags, data);
}

Exe::~Exe()
{
  if (gExeRunning == 1)
  {
    ecore_event_handler_del(gDelHandler);
  }
  
  if(mExe)
  {
    ecore_exe_free(mExe);
  }

  --gExeRunning;
}

Eina_Bool Exe::delhandler (void *data, int type, void *event)
{
  Exe *exe = static_cast<Exe*>(data);  
  exe->mExe = NULL;

  Ecore_Exe_Event_Del *delEvent = static_cast<Ecore_Exe_Event_Del*>(event);
  exe->mDeathPid = delEvent->pid;
  exe->signalDelete.emit (delEvent);
  
  return true;
}

void Exe::setRunPriority(int pri)
{
  ecore_exe_run_priority_set(pri);
}

int Exe::getRunPriority()
{
  return ecore_exe_run_priority_get();
}

bool Exe::send(const void *data, int size)
{
  exceptionCheck();
  return ecore_exe_send(mExe, data, size);
}

void Exe::stdinClose()
{
  exceptionCheck();
  ecore_exe_close_stdin(mExe);
}

void Exe::setAutoLimits(int start_bytes, int end_bytes, int start_lines, int end_lines)
{
  exceptionCheck();
  ecore_exe_auto_limits_set(mExe, start_bytes, end_bytes, start_lines, end_lines);
}

void Exe::freeData(Ecore_Exe_Event_Data *data)
{
  ecore_exe_event_data_free(data);
}

pid_t Exe::getPid()
{
  exceptionCheck();
  return ecore_exe_pid_get(mExe);
}

void Exe::setTag(const std::string &tag)
{
  exceptionCheck();
  ecore_exe_tag_set(mExe, tag.c_str());
}

std::string Exe::getTag()
{
  exceptionCheck();
  return ecore_exe_tag_get(mExe);
}

std::string Exe::getCmd()
{
  exceptionCheck();
  return ecore_exe_cmd_get(mExe);
}

void *Exe::getData()
{
  exceptionCheck();
  return ecore_exe_data_get(mExe);
}

void *Exe::setData(void *data)
{
  exceptionCheck();
  return ecore_exe_data_set(mExe, data);
}

Ecore_Exe_Flags Exe::getFlags()
{
  exceptionCheck();
  return ecore_exe_flags_get(mExe);
}

void Exe::pause()
{
  exceptionCheck();
  ecore_exe_pause(mExe);
}

void Exe::cont()
{
  exceptionCheck();
  ecore_exe_continue(mExe);
}

void Exe::interrupt()
{
  exceptionCheck();
  ecore_exe_interrupt(mExe);
}

void Exe::quit()
{
  exceptionCheck();
  ecore_exe_quit(mExe);
}

void Exe::terminate()
{
  exceptionCheck();
  ecore_exe_terminate(mExe);
}

void Exe::kill()
{
  exceptionCheck();
  ecore_exe_kill(mExe);
}

void Exe::signal(int num)
{
  exceptionCheck();
  ecore_exe_signal(mExe, num);
}

void Exe::hup()
{
  exceptionCheck();
  ecore_exe_hup(mExe);
}

void Exe::exceptionCheck()
{
  if (!mExe)
  {
    assert(mDeathPid);
    throw ProcessNotExistingException(mDeathPid);
  }
}

} // end namespace Ecorexx
