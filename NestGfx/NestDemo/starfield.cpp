#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include <math.h>

#include "demos.h"
#include "util.h"

static struct point2 to2D(Gfx * fb, struct point3 pt3)
{
  struct point2 pt;
  float scaleRatio = (float)300/(300+pt3.z);

  pt.x = pt3.x*scaleRatio;
  pt.y = pt3.y*scaleRatio;

  pt.x += fb->getWidth()/2;
  pt.y = fb->getHeight()/2 - pt.y;

  return pt;
}

class Star
{
public:
  Star(Gfx * fb);
  ~Star();

  void draw();
  void tick();
  void tick(int amt);
  void back();
  void tunnel(bool enter);
  bool inTunnel();

  bool isVisible();
  float getRot();
  void setRot(float rot);
  void setRotDestructive(float rot);
private:
  void reset();
  void updatePoint();
  rgb_t randColor();

  Gfx * m_fb;
  bool m_tunnel;
  struct point3 m_pos3;
  struct point3 m_pos3rot;
  struct point2 m_pos;
  float m_rot;
  rgb_t m_colorTo;
};

Star::Star(Gfx * fb)
{
  m_fb = fb;
  clear_obj(&m_pos);
  clear_obj(&m_pos3);
  clear_obj(&m_pos3rot);
  setRot(0);
  m_tunnel = false;

  reset();
}

Star::~Star()
{

}

rgb_t Star::randColor()
{
  rgb_t c;
  c.r = randInt(0, 255);
  c.g = randInt(0, 255);
  c.b = randInt(0, 255);

  return c;
}

void Star::reset()
{
  m_pos3.z = randInt(-300, 500);
  m_pos3.x = randInt(-500, 500);
  m_pos3.y = randInt(-500, 500);
  m_colorTo = randColor();

  updatePoint();
}

float Star::getRot()
{
  return m_rot;
}

void Star::setRot(float rot)
{
  m_rot = rot;
  updatePoint();
}

void Star::setRotDestructive(float rot)
{
  float oldRot = m_rot;

  setRot(rot);
  m_pos3 = m_pos3rot;
  setRot(oldRot);
}

void Star::updatePoint()
{
  if(m_rot != 0.0)
  {
    float rotX = cosf(m_rot);
    float rotZ = sinf(m_rot);

    m_pos3rot.x = m_pos3.x*rotX - (m_pos3.z)*rotZ;
    m_pos3rot.z = m_pos3.x*rotZ + (m_pos3.z)*rotX;
    m_pos3rot.y = m_pos3.y;
  }
  else
  {
    m_pos3rot = m_pos3;
  }

  // what we actually draw to the screen
  m_pos = to2D(m_fb, m_pos3rot);
}

void Star::back()
{
  if(m_tunnel)
  {
    int rad = randInt(100, 500);
    float angle = randFloat(0, 6.28);

    //angle = angle - angle % 5; 
    rad = 80;

    m_pos3.x = rad*cosf(angle);
    m_pos3.y = rad*sinf(angle);
  }
  else
  {
    m_pos3.x = randInt(-500, 500);
    m_pos3.y = randInt(-500, 500);
  }

  m_pos3.z = 500;
  updatePoint();
}

void Star::tunnel(bool enter)
{
  m_tunnel = enter;
}

bool Star::inTunnel()
{
  return m_tunnel;
}

void Star::draw()
{
  int normZ = m_pos3rot.z + 300;

  if(normZ <= 0)
    return;

  int size = max(min(800/normZ, 100), 2);
  rgb_t color = m_fb->lerpRange(m_colorTo, COLOR_BLACK, normZ, 800);

  m_fb->fillRect(m_pos.x, m_pos.y, m_pos.x+size, 
    m_pos.y+size, color);
}

void Star::tick()
{
  m_pos3.z--;
  updatePoint();
}

void Star::tick(int amt)
{
  m_pos3.z -= amt;
  updatePoint();
}

bool Star::isVisible()
{
  return m_pos3.z > -300;
}

static int readTurn(int fd)
{
  if(fd < 0)
  {
    log_error("Bad device");
    return 0;
  }

  //  seconds    u sec   unk  ---   diff
  // 051e 53ba f2b2 0005 0002 0000 0001 0000
  char buf[32];
  int res = read(fd, buf, 32);

  if(res != 32 || res < 0)
  {
    if(errno != EAGAIN)
      log_error("read dev");

    return 0;
  }

  int * diff = reinterpret_cast<int *>(buf + 12);

  return *diff;
}

static int readPress(int fd)
{
  if(fd < 0)
  {
    log_error("Bad device");
    return -1;
  }

  //  seconds    u sec   unk  ---   diff
  // 051e 53ba f2b2 0005 0002 0000 0001 0000
  char buf[32];
  int res = read(fd, buf, 32);

  if(res != 32 || res < 0)
  {
    if(errno != EAGAIN)
      log_error("read dev");

    return -1;
  }

  int * diff = reinterpret_cast<int *>(buf + 12);

  return *diff;
}

void demo_starfield(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  int numStars = 500;
  Star ** stars = new Star*[numStars];

  for(int i = 0; i < numStars; i++)
    stars[i] = new Star(fb);

  int fd = open("/dev/input/event1", O_RDONLY);
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  int fd2 = open("/dev/input/event2", O_RDONLY);
  flags = fcntl(fd2, F_GETFL, 0);
  fcntl(fd2, F_SETFL, flags | O_NONBLOCK);

  Timer ani(2);
  ani.start();

  bool fly = false, stayIn = false;
  bool doTick = false;
  int counter = 0;

  while(1)
  {
    int rot = readTurn(fd);

    if(rot)
    {
      // rotate all points and then make them head towards us
      for(int i = 0; i < numStars; i++)
        stars[i]->setRotDestructive(-0.001*rot);
    }

    int press = readPress(fd2);

    if(press != -1)
    {
      if(press == 1)
      {
        if(fly)
        {
          stayIn = false;
          log_info("leaving tunnel");
        }
        else
        {
          stayIn = true;
          fly = true;
          log_info("enter tunnel");
        }

      }
    }

    bool tunnelDone = true;

    for(int i = 0; i < numStars; i++)
    {
      stars[i]->draw();

      if(!stars[i]->isVisible())
      {
        stars[i]->tunnel(stayIn);
        stars[i]->back();
      }

      if(!doTick)
        continue;

      if(fly)
      {
        stars[i]->tick(5);

        if(stars[i]->inTunnel())
          tunnelDone = false;
      }
      else
      {
        stars[i]->tick();
      }
    }

    if(!stayIn && tunnelDone && doTick)
      fly = false;

    if(ani.tick())
    {
      doTick = true;
      ani.restart();
    }
    else
    {
      doTick = false;
    }

    if(fly)
    {
      counter++;

      if(counter == 4)
      {
        fb->fps();
        fb->flip();
        fb->drawClear(COLOR_BLACK);
        counter = 0;
      }
    }
    else
    {
      fb->fps();
      fb->flip();
      fb->drawClear(COLOR_BLACK);
      counter = 0;
    }

    if(demoTime.tick())
    {
      for(int i = 0; i < numStars; i++)
        delete stars[i];

      delete [] stars;
      return;
    }
  }
}
