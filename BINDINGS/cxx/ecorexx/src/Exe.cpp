#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* STL */
#include <string>
#include <cassert>
#include <iostream>

#include "ecorexx/Exe.h"

using namespace std;

namespace Ecorexx {
  
Exe::Exe(const std::string &exe_cmd, const void *data)
{  
  mExe = ecore_exe_run(exe_cmd.c_str(), data);
}

Exe::Exe(const std::string &exe_cmd, Ecore_Exe_Flags flags, const void *data)
{  
  mExe = ecore_exe_pipe_run(exe_cmd.c_str(), flags, data);
}

Exe::~Exe()
{
}

void Exe::free()
{
  ecore_exe_free(mExe);
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
  return ecore_exe_send(mExe, data, size);
}

void Exe::stdinClose()
{
  ecore_exe_close_stdin(mExe);
}

void Exe::setAutoLimits(int start_bytes, int end_bytes, int start_lines, int end_lines)
{
  ecore_exe_auto_limits_set(mExe, start_bytes, end_bytes, start_lines, end_lines);
}

void Exe::freeData(Ecore_Exe_Event_Data *data)
{
  ecore_exe_event_data_free(data);
}

pid_t Exe::getPid()
{
  return ecore_exe_pid_get(mExe);
}

void Exe::setTag(const std::string &tag)
{
  ecore_exe_tag_set(mExe, tag.c_str());
}

std::string Exe::getTag()
{
  return ecore_exe_tag_get(mExe);
}

std::string Exe::getCmd()
{
  return ecore_exe_cmd_get(mExe);
}

void *Exe::getData()
{
  return ecore_exe_data_get(mExe);
}

void *Exe::setData(void *data)
{
  return ecore_exe_data_set(mExe, data);
}

Ecore_Exe_Flags Exe::getFlags()
{
  return ecore_exe_flags_get(mExe);
}

void Exe::pause()
{
  ecore_exe_pause(mExe);
}

void Exe::cont()
{
  ecore_exe_continue(mExe);
}

void Exe::interrupt()
{
  ecore_exe_interrupt(mExe);
}

void Exe::quit()
{
  ecore_exe_quit(mExe);
}

void Exe::terminate()
{
  ecore_exe_terminate(mExe);
}

void Exe::kill()
{
  ecore_exe_kill(mExe);
}

void Exe::signal(int num)
{
  ecore_exe_signal(mExe, num);
}

void Exe::hup()
{
  ecore_exe_hup(mExe);
}

} // end namespace Ecorexx
