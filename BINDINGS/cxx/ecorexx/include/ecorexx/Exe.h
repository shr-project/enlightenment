#ifndef ECOREXX_EXE_H
#define ECOREXX_EXE_H

/* STD */
#include <map>

/* EFL */
#include <Ecore.h>

/* SIGC */
#include <sigc++/sigc++.h>

namespace Ecorexx {

/**
 * @defgroup Ecorexx_Exe_Group Process Spawning Functions
 *
 * This module is responsible for managing portable processes using Ecore.
 * With this module you're able to spawn processes and you also can pause,
 * quit your spawned processes.
 * An interaction between your process and those spawned is possible
 * using pipes or signals. 
 *
 *
 * @ingroup Ecorexx_Main_Loop_Group
 *
 * @{
 */
class Exe
{
public:
  Exe(const std::string &exe_cmd, const void *data);

  //ecore_exe_pipe_run
  Exe(const std::string &exe_cmd, Ecore_Exe_Flags flags, const void *data);
    
  virtual ~Exe();

  void free();

  static void setRunPriority(int pri);

  static int getRunPriority();

  // TODO
//EAPI void ecore_exe_callback_pre_free_set(Ecore_Exe *exe, Ecore_Exe_Cb func);

  bool send(const void *data, int size);

  void stdinClose();

  void setAutoLimits(int start_bytes, int end_bytes, int start_lines, int end_lines);

  // TODO
//EAPI Ecore_Exe_Event_Data *ecore_exe_event_data_get(Ecore_Exe *exe, Ecore_Exe_Flags flags);

  void freeData(Ecore_Exe_Event_Data *data);

  pid_t getPid();

  void setTag(const std::string &tag);

  std::string getTag();

  std::string getCmd();

  void *getData();

  void *setData(void *data);

  Ecore_Exe_Flags getFlags();

  void pause();

  void cont();

  void interrupt();

  void quit();

  void terminate();

  void kill();

  void signal(int num);

  void hup();

private:  
  Exe(const Exe&); // forbid copy constructor

  Ecore_Exe *mExe;
};

} // end namespace Ecorexx

#endif // ECOREXX_EXE_H