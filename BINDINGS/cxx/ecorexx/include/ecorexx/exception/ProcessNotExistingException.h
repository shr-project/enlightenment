#ifndef PROCESS_NOT_EXISTING_EXCEPTION_H
#define PROCESS_NOT_EXISTING_EXCEPTION_H

#include <exception>

#include <Ecore.h>

namespace Ecorexx {

class ProcessNotExistingException : public std::exception
{
public:
  ProcessNotExistingException(pid_t pid) : mPid(pid) {}

  virtual ~ProcessNotExistingException() throw() {}

  const char *what() const throw();

private:
  pid_t mPid;
};

} // end namespace Ecorexx

#endif // PROCESS_NOT_EXISTING_EXCEPTION_H

