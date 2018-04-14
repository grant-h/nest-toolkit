class NestInput
{
public:
  enum input_event_t
  {
    BUTTON_UP,
    BUTTON_DOWN,
    DIAL_LEFT,
    DIAL_RIGHT
  };

  // which type of button event (up or down)
  typedef void (*button_callback_t)(input_event_t);
  // which direction the user turned and what speed
  typedef void (*dial_callback_t)(input_event_t, int);

  NestInput(const char * buttonPath, const char * dialPath);
  ~NestInput();

  void regDialCallback(dial_callback_t callback);
  void regButtonCallback(button_callback_t callback);
  void unregDialCallback();
  void unregButtonCallback();

  // read all input sources with an immediate timeout
  void poll();
private:
  void processButton();
  void processDial();

  button_callback_t m_buttonCallback;
  dial_callback_t m_dialCallback;

  const char * m_dialPath;
  const char * m_buttonPath;
  int m_dialFd;
  int m_buttonFd;

  //const static int m_dialHisteresis;
};
