#pragma once

#include "Logger.h"
#include "StringFormat.h"
#include <vector>


using util::StringFormat;


namespace TD
{
  namespace
  {
    const char * TRG = "";
    const char * __CLASSNAME__ = "TD";
  }


  typedef int Life;
  typedef int Tick;
  typedef int Credit;

#define DEAD 0
#define TICKS_PER_SEC 16
#define DEFAULT_DAMAGE 1
#define DEFAULT_HEALTH 1
#define DEFAULT_FIRERATE 100
#define DEFAULT_SPEED 1
#define DEFAULT_COST 1
#define DEFAULT_YIELD 1
#define DEFAULT_TIMEOUT TICKS_PER_SEC * 30
#define DEFAULT_GRIDSIZE 10


  class Mob
  {
  public:

    Mob()
      : m_health(DEFAULT_HEALTH),
        m_speed(DEFAULT_SPEED),
        m_yield(DEFAULT_YIELD)
    {}

    Mob(Life health, Tick speed, Credit yield)
      : m_health(health),
        m_speed(speed),
        m_yield(yield)
    {}

    Life getHealth()
    {
      return m_health;
    }

    Tick getSpeed()
    {
      return m_speed;
    }

    Credit getYield()
    {
      return m_yield;
    }

  private:
    Life m_health;
    Tick m_speed;
    Credit m_yield;
  };


  class Tower
  {
  public:

    Tower()
      : m_damage(DEFAULT_DAMAGE),
        m_fireRate(DEFAULT_FIRERATE),
        m_cost(DEFAULT_COST)
    {}

    Tower(Life damage, Tick fireRate, Credit cost)
      : m_damage(damage),
        m_fireRate(fireRate),
        m_cost(cost)
    {}

    Life getDamage()
    {
      return m_damage;
    }

    Tick getFireRate()
    {
      return m_fireRate;
    }

    Credit getCost()
    {
      return m_cost;
    }

  private:
    Life m_damage;
    Tick m_fireRate;
    Credit m_cost;
  };
  typedef Tower* TowerPtr;


  class Wave
  {
  public:

    Wave()
      : m_mobs(),
        m_timeout(DEFAULT_TIMEOUT),
        m_yield(DEFAULT_YIELD)
    {}

    Wave (Tick timeout, Credit yield)
      : m_mobs(),
        m_timeout(timeout),
        m_yield(yield)
    {}

    Wave (std::vector<Mob>& mobs, Tick timeout, Credit yield)
      : m_mobs(mobs),
        m_timeout(timeout),
        m_yield(yield)
    {}

    Tick getTimeout()
    {
      return m_timeout;
    }

    Credit getYield()
    {
      return m_yield;
    }

    void getMobs(std::vector<Mob>& mobs)
    {
      mobs = m_mobs;
    }

    void addMob(Mob& mob)
    {
      m_mobs.push_back(mob);
    }

  private:

    std::vector<Mob> m_mobs;

    Tick m_timeout;
    Credit m_yield;
  };


  class Grid
  {
  public:

    typedef TowerPtr* TowerRow;
    typedef TowerRow* TowerTable;


    Grid()
      : m_width(DEFAULT_GRIDSIZE),
        m_height(DEFAULT_GRIDSIZE)
    {
      m_towers = new TowerRow[m_width];
      for (int i = 0; i < m_width; ++i)
      {
        m_towers[i] = new TowerPtr[m_height];
      }

      for (int i = 0; i < m_width; ++i)
      {
        for (int j = 0; j < m_height; ++j)
        {
          m_towers[i][j] = NULL;
        }
      }
    }

    Grid(int width, int height)
      : m_width(width),
        m_height(height)
    {
      m_towers = new TowerRow[width];
      for (int i = 0; i < width; ++i)
      {
        m_towers[i] = new TowerPtr[height];
      }

      for (int i = 0; i < width; ++i)
      {
        for (int j = 0; j < height; ++j)
        {
          m_towers[i][j] = NULL;
        }
      }
    }

    bool placeTower(int x, int y, TowerPtr tower)
    {
      bool success = true;

      if (this->isWithin(x, y))
      {
        m_towers[x][y] = tower;
      }
      else
      {
        success = false;
      }

      return success;
    }

    bool clearTower(int x, int y)
    {
      return this->placeTower(x, y, NULL);
    }

    TowerPtr getTower(int x, int y)
    {
      TowerPtr tower = NULL;

      if (this->isWithin(x, y))
      {
        tower = m_towers[x][y];
      }

      return tower;
    }

    bool isWithin(int x, int y)
    {
      return x >= 0 && x < m_width && y >= 0 && y < m_height;
    }

    int getWidth()
    {
      return m_width;
    }

    int getHeight()
    {
      return m_height;
    }

  private:
    int m_width;
    int m_height;
    TowerTable m_towers;
  };


  class Path
  {
  public:

    struct Pathlet
    {
      int x, y;
    };


    Path()
      : m_path()
    {}

    bool verifyPathlet(int x, int y)
    {
      bool success = true;

      Pathlet& pathlet = m_path.back();

      if (std::abs(x - pathlet.x) > 1 || std::abs(y - pathlet.y) > 1)
      {
        success = false;
      }

      return success;
    }

    bool addPathlet(int x, int y)
    {
      bool success = verifyPathlet(x, y);

      if (success)
      {
        Pathlet& pathlet = m_path.back();

        if (pathlet.x != x && pathlet.y != y)
        {
          m_path.push_back({x,y});
        }
      }

      return success;
    }

    void breakPathletAt(int index)
    {
      if (index >= 0 && index < m_path.size())
      {
        while (m_path.size() > index)
        {
          m_path.pop_back();
        }
      }
    }

  private:
    std::vector<Pathlet> m_path;
  };


  class Map
  {
  public:

    Map()
      : m_path(),
        m_grid(),
        m_currentWave(NULL)
    {}

    Map(Path path, Grid grid)
      : m_path(path),
        m_grid(grid),
        m_currentWave(NULL)
    {}

    void setCurrentWave(Wave* currentWave)
    {
      m_currentWave = currentWave;
    }

    Path& getPath()
    {
      return m_path;
    }

    Grid& getGrid()
    {
      return m_grid;
    }

    Wave* getCurrentWave()
    {
      return m_currentWave;
    }

  private:
    Path m_path;
    Grid m_grid;
    Wave* m_currentWave;
  };


  class Game
  {
  public:

    Game()
    {}

    Game(Life lives, Credit money)
      : m_lives(lives),
        m_money(money)
    {}


    Grid& getGrid()
    {
      return m_map.getGrid();
    }

  private:
    Map m_map;

    Life m_lives;
    Credit m_money;
  };


  class TD
  {
  public:
    
    TD();
    
    ~TD();
    
  private:
    
  };
}
