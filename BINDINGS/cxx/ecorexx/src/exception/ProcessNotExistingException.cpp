#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* STD */
#include <string>

/* EFL */
#include <Ecore.h>

/* Project */
#include "ecorexx/exception/ProcessNotExistingException.h"
#include "util.h"

using namespace std;

const char *Ecorexx::ProcessNotExistingException::what() const throw()
{
  static string s;
  s = "Process with pid '";
  s += toString<pid_t>(mPid);
  s += "' not longer living!";

  return static_cast <const char *>(s.c_str());
}
