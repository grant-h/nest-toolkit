#include "NestInput.h"

#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

NestInput::NestInput(const char * buttonPath, const char * dialPath)
{
  BUG_ON(!buttonPath || !dialPath);

  m_buttonPath = buttonPath;
  m_dialPath = dialPath;
  m_dialCallback = NULL;
  m_buttonCallback = NULL;

  m_dialFd = open(m_dialPath, O_RDONLY);

  if(m_dialFd < 0)
  {
    log_errno("Failed to open dial device %s", m_dialPath);
    die();
  }

  m_buttonFd = open(m_buttonPath, O_RDONLY);

  if(m_buttonFd < 0)
  {
    log_errno("Failed to open button device %s", m_buttonPath);
    die();
  }

  log_info("Opened button and dial input devices");
}

NestInput::~NestInput()
{
  if(m_dialFd >= 0)
    close(m_dialFd);

  if(m_buttonFd >= 0)
    close(m_buttonFd);
}

void NestInput::regDialCallback(dial_callback_t callback)
{
  if(callback)
    m_dialCallback = callback;
}

void NestInput::regButtonCallback(button_callback_t callback)
{
  if(callback)
    m_buttonCallback = callback;
}

void NestInput::unregDialCallback()
{
  m_dialCallback = NULL;
}

void NestInput::unregButtonCallback()
{
  m_buttonCallback = NULL;
}

void NestInput::poll()
{
  fd_set set;

  FD_ZERO(&set);

  FD_SET(m_buttonFd, &set);
  FD_SET(m_dialFd, &set);

  // do some polling
  struct timeval time = {0, 0};

  int nfds = select(max(m_dialFd, m_buttonFd)+1, &set, NULL, NULL, &time);

  // we've got a bite
  if(nfds > 0)
  {
    if(FD_ISSET(m_buttonFd, &set))
      processButton();
    if(FD_ISSET(m_dialFd, &set))
      processDial();

    return nfds;
  }
  else if(nfds < 0)
  {
    log_errno("error while select()'ing in NestInput");
  }

  return 0;
}
