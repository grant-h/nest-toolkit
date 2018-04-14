class GuiElement
{
public:
  GuiElement();
  ~GuiElement();

  void render();
private:
  Spacing m_padding;
  Spacing m_margin;
};
