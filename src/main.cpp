#include <stdio.h>
#include "Logger.h"
#include "GContent.h"
#include "colors.h"
#include "GShaderProgram.h"
#include "GShaderProgramManager.h"
#include "GVertexBufferObject.h"
#include "GPrimativeFactory.h"
#include "StringFormat.h"

#include "GGUI.h"
#include "TD.h"

using util::StringFormat;

using glw::GReturnCode::GLW_SUCCESS;
using glw::GReturnCode::GLW_FAIL;

using glw::engine::GContent;

using glw::engine::glsl::GShaderProgram;
using glw::engine::glsl::GShaderProgramId;
using glw::engine::glsl::GShaderProgramManager;
using glw::engine::glsl::GShaderHandle_T;

using glw::engine::buffers::GVertexBufferObject;
using glw::engine::buffers::GPrimativeFactory;
using glw::engine::buffers::GArrayVertex;
using glw::engine::buffers::GArrayVec3;
using glw::engine::buffers::GArrayVBO;

using glw::gui::GShape;
using glw::gui::GContext;
using glw::gui::GButton;
using glw::gui::GWindow;
using glw::gui::GLabel;
using glw::gui::GTextEdit;
using glw::gui::GSlider;
using glw::gui::GSpinner;
using glw::gui::GDropdown;
using glw::gui::GImageView;
using glw::gui::GProgressBar;
using glw::gui::GCheckBox;
using glw::gui::GDialog;
using glw::gui::GPane;
using glw::gui::GClickable;
using glw::gui::GContextShaderHandle_T;
using glw::gui::GComponent;
using glw::gui::GGroup;
using glw::gui::GContainer;
using glw::meta::GLinker;

//using TD::Credit;
//using TD::Life;
//using TD::Tick;
//using TD::Game;
//using TD::Grid;
//using TD::Map;
//using TD::Tower;
//using TD::Path;
//using TD::Mob;
//using TD::Wave;


namespace
{
  const char* TRG = "MAIN";
  const char* __CLASSNAME__ = "main";
}

#define TOWER_1_RPS 60
#define TOWER_SIZE glm::vec2(50)
#define TOWER_IMAGEFILE "../textures/tower.png"


class Tower : public GImageView
{
public:

  Tower()
    : GImageView(glm::vec2(), TOWER_SIZE, TOWER_IMAGEFILE),
      m_damage(1),
      m_baseCost(100),
      m_rpm(TOWER_1_RPS)
  {}

  Tower(glm::vec2 pos, const char * imagefile, int towerId)
    : GImageView(pos, TOWER_SIZE, imagefile),
      m_damage(1),
      m_baseCost(100),
      m_rpm(TOWER_1_RPS),
      m_towerId(towerId)
  {}

  Tower(glm::vec2 pos, glm::vec2 size, const char * imagefile, int towerId)
    : GImageView(pos, size, imagefile),
      m_damage(1),
      m_baseCost(100),
      m_rpm(TOWER_1_RPS),
      m_towerId(towerId)
  {}

  int getDamage()
  {
    return m_damage;
  }

  int getCost()
  {
    return m_baseCost;
  }

  int getRPM()
  {
    return m_rpm;
  }

  int getTowerId()
  {
    return m_towerId;
  }

  TRIGGERS_DERIVED(
      TOWER, GIMAGEVIEW,
      DEFINE_TRIGGER_NONE):

    private:

    int m_damage;
  int m_baseCost;
  int m_rpm;
  int m_towerId;
};

#define NULL_SHOP_ITEM -1

class TowerContainer : public GContainer, public GLinker
{
public:

  TowerContainer()
    : GContainer()
  {
    m_selectedItemImage = GImageView(glm::vec2(), TOWER_SIZE, "../textures/selected.png");
    addComponent(&m_itemsPane);
    addComponent(&m_selectedItemImage);
  }

  virtual bool checkMouseEvents(int button, int action)
  {
    bool eventHasHappened = GContainer::checkMouseEvents(button, action);

    if (eventHasHappened)
    {
      onPressed();
      onTowerSelected();
    }

    return eventHasHappened;
  }

  virtual void update()
  {
    if (!hasFocusedChild())
    {
      m_selectedItemImage.setVisible(false);
    }
  }

  void addTower(Tower* tower)
  {
    m_itemsPane.addComponent(tower);
  }

  bool checkPlacedTowerBounds(Tower* towerToBePlaced)
  {
    bool canBePlaced = true;

    for (IGComponent* component : m_itemsPane.getGroupComponents())
    {
      if (DCAST_COMPONENT(component, Tower, tower))
      {
        if (tower->isInside(towerToBePlaced))
        {
          canBePlaced = false;
        }
      }
    }

    return canBePlaced;
  }

  void setSelectedTowerSize(glm::vec2 size)
  {
    m_selectedItemImage.setSize(size);
  }

  Tower* getSelectedTower()
  {
    return m_selectedTower;
  }

  TRIGGERS_BASE(
      SHOP,
      DEFINE_TRIGGER(onTowerSelected),
      DEFINE_TRIGGER(onPressed),
      DEFINE_TRIGGER(onUnfocus)):

  trigger_func onTowerSelected()
  {
    for (IGComponent* component : m_itemsPane.getGroupComponents())
    {
      if (DCAST_COMPONENT(component, Tower, tower))
      {
        if (tower->isPressed())
        {
          m_selectedTower = tower;
          break;
        }
      }
    }

    if (NULL != m_selectedTower)
    {
      m_selectedItemImage.setVisible(true);
      m_selectedItemImage.setPos(m_selectedTower->getPos());
      LINKER_CALL(onTowerSelected);
    }
  }

  trigger_func onPressed()
  {
    if (isPressed())
    {
      LINKER_CALL(onPressed);
    }
  }

  virtual trigger_func unfocusComponent()
  {
    LINKER_CALL(onUnfocus);
  }

protected:

  Tower* m_selectedTower = NULL;

  GPane m_itemsPane;
  GImageView m_selectedItemImage;

};

class Shop : public TowerContainer
{
public:

  Shop()
    : TowerContainer()
  {
    Tower* tower1 = new Tower(glm::vec2(), "../textures/tower1.png", 0);
    addTower(tower1);
    Tower* tower2 = new Tower(glm::vec2(), "../textures/tower2.png", 1);
    addTower(tower2);
    Tower* tower3 = new Tower(glm::vec2(), "../textures/tower3.png", 2);
    addTower(tower3);
  }

  virtual void validate()
  {

    for (int ix = 0; ix < m_itemsPane.getGroupComponents().size(); ++ix)
    {
      if(DCAST_COMPONENT(m_itemsPane.getGroupComponents()[ix], Tower, tower))
      {
        tower->setSize(TOWER_SIZE);
        tower->setPos(glm::vec2(tower->getSize().x * ix, 0));
      }
    }

    TowerContainer::validate();
  }

private:

};

class Map : public TowerContainer
{
public:

  Map() : TowerContainer() {}

  void placeTower(Tower* tower)
  {
    addTower(tower);
    tower->init(getContext(), &m_itemsPane);
  }

private:

};

class Game : public GPane
{
public:

  Game()
    : GPane()
  {
    addComponent(&m_shop);
    addComponent(&m_map);
    addComponent(&m_hoverTower);
    m_hoverTower.setEnabled(false);
    m_hoverTower.setFocusable(false);
    LINKER_NEW_LINK(&m_map, Map::T_onPressed, ACTION(*this, &Game::buyTower));
  }

  Game(glm::vec2 pos, glm::vec2 size)
    : GPane(pos, size)
  {
    addComponent(&m_shop);
    addComponent(&m_map);
    addComponent(&m_hoverTower);
    m_hoverTower.setEnabled(false);
    m_hoverTower.setFocusable(false);
    LINKER_NEW_LINK(&m_map, Map::T_onPressed, ACTION(*this, &Game::buyTower));
  }

  virtual void validate()
  {
    GPane::validate();

    glm::vec2 windowSize;
    getContext()->getContent()->getWindowSize(windowSize);

    glm::vec2 itemSize = TOWER_SIZE;

    m_shop.setSize(glm::vec2(windowSize.x, itemSize.y));
    m_shop.setPos(glm::vec2(0, windowSize.y - m_shop.getSize().y));

    m_map.setSize(glm::vec2(windowSize.x, windowSize.y - itemSize.y));
    m_map.setPos(glm::vec2());
  }

  virtual void update()
  {
    if (m_map.isHovering())
    {
      if (NULL != m_shop.getSelectedTower())
      {
        m_hoverTower.setPos(getRelativeMousePos());
        m_hoverTower.validate();
      }
    }
  }

  void setMap(Map& map)
  {
    m_map = map;
    LINKER_NEW_LINK(&m_map, Map::T_onPressed, ACTION(*this, &Game::buyTower));
  }

  void buyTower()
  {
    Tower* selectedTower = m_shop.getSelectedTower();
    Tower* tower = new Tower(getRelativeMousePos(), "../textures/tower.png", -1);
    if (m_map.checkPlacedTowerBounds(tower))
    {
      m_map.placeTower(tower);
    }
    validate();
  }

private:
  Map m_map;
  Shop m_shop;

  Tower m_hoverTower;
};

namespace
{
  GContent* content;
  GShaderProgramManager shaderProgramManager;
  GShaderProgramId BASIC_PROGRAM;
  GShaderProgramId GUI_PROGRAM;

  GVertexBufferObject vbo;

  GArrayVBO guiVBOs;

  GCamera camera(glm::vec3(3, 0, 5), glm::vec3(), glm::vec3(0,0,-1), glm::vec3(0, 1, 0));

  GContext context;

  GLabel* fpsLabel;

  GPane* pane;

  Map map;

  Game game;
}


void handleInput()
{
  if(content->getKeyboard()->isKeyDown(GLFW_KEY_LEFT_SHIFT))
  {
    camera.applyForceUp(content->getMouse()->popScrollDelta().y);
  }
  else
  {
    camera.applyForceForward(content->getMouse()->popScrollDelta().y);
  }

}

GReturnCode loop()
{
  handleInput();

  fpsLabel->setText(StringFormat("%0 FPS").arg(content->getFps()).str());

  context.update();

  // (calculations should be done in a different thread)

  // Update the camera
  camera.update(0.1f, 0.9f);

  // Update the engine with the camera
  content->setCamera(camera);

  // Rotate the sphere
  vbo.m_theta += 0.01f;

  // Clear the scene
  content->clearAll();

  // Set the 3D perspective
  content->load3DPerspective();

  // Load the shader program we want to draw with
  shaderProgramManager.loadProgram(BASIC_PROGRAM);

  // Draw the sphere VBO
  shaderProgramManager.drawVBO(vbo);

  // Clear the scene
  content->clearDepthBuffer();

  // Set the 3D perspective
  content->loadExternalOrtho();

  context.draw();


  return GLW_SUCCESS;
}

GReturnCode initShaderPrograms()
{
  GReturnCode success = GLW_SUCCESS;

  LINFO("Initialising GLSL shader programs...");

  // Add a new program to the manager
  if (GLW_SUCCESS == shaderProgramManager.addNewProgram(
        "../shaders/basic.vert",  // Vertex shader
        "../shaders/basic.frag",  // Fragment shader
        content->getModelMat(),            // Pass the engine's model matrix
        content->getViewMat(),             // Pass the engine's view matrix
        content->getProjMat(),             // Pass the engine's proj. matrix
        BASIC_PROGRAM))                   // Supply the id container
  {
    // Get the shader program we have just created
    GShaderProgram * shaderProgram = shaderProgramManager.getProgram(BASIC_PROGRAM);

    // Check the program has been created
    if (NULL != shaderProgram)
    {
      if(!shaderProgram->isValid())
      {
        success = GLW_FAIL;
        LERROR("BASIC_PROGRAM is not valid");
      }
      else
      {
        shaderProgram->setTexHandle();
      }
    }
    else
    {
      success = GLW_FAIL;
      LERROR("BASIC_PROGRAM is NULL");
    }
  }
  else
  {
    success = GLW_FAIL;
    LERROR("Failed to add BASIC_PROGRAM");
  }

  context.initShaderProgram(content->getModelMat(),
                            content->getViewMat(),
                            content->getProjMat());

  return success;
}

GReturnCode initVBOs()
{
  GReturnCode success = GLW_SUCCESS;

  LINFO("Initialising VBOs...");

  // Create array containers
  GArrayVertex o;
  GArrayVec3 v, c, n, t;
  GArrayVec2 uv;

  LINFO("Generating Sphere...");

  // Generate an array of vec3s for a sphere
  GPrimativeFactory::sphere(v, 12, 12);

  // Create the texture coordinates for a sphere
  GPrimativeFactory::sphereicalUVs(uv, v);

  // Pack the vec3s into a vertex array
  GPrimativeFactory::packObject(o, v, uv);

  // Create a new VBO with our new vertex array
  vbo = GVertexBufferObject(
        o,                      // Supply the vertex array
        glm::vec3(),            // World position
        glm::vec3(0, 1, 0),     // Rotation axis
        glm::radians(0.0f),     // Rotation angle
        glm::vec3(1, 0, 0),     // Pre-rotation axis
        glm::radians(90.0f),    // Pre-rotation angle
        glm::vec3(1),
        "../textures/mars.jpg");          // Scale vector

  // Get the content window size
  glm::vec2 windowSize;
  content->getWindowSize(windowSize);

  // Context setup
  context.setContent(content);
  context.setColorStyle({glw::WHITE_A, glw::BLACK_A, glw::SKY_BLUE_A});

  // Create pane for all components
  pane = new GPane(glm::vec2(), windowSize);
  context.addComponent(pane);

  // Add a label
  fpsLabel = glw::gui::createLabel("fps", glm::vec2(), 20, glw::BLACK_A);
  pane->addComponent(fpsLabel);

  game.setSize(windowSize);
//  game.setMap(map);
  pane->addComponent(&game);

  // Initialise the context
  context.init();

  // Validate the context
  context.validate();

  return success;
}

GReturnCode init()
{
  GReturnCode success = GLW_SUCCESS;

  // SHADER PROGRAM SETUP //
  success = initShaderPrograms();

  // MESH SETUP //
  if (GLW_SUCCESS == success)
  {
    success = initVBOs();
  }

  return success;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  context.checkMouseEvents(button, action);

  if (action == GLFW_PRESS)
  {
    switch (button)
    {
      case GLFW_MOUSE_BUTTON_RIGHT:

        break;
    }
  }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  context.checkKeyEvents(key, action);

  if (action == GLFW_PRESS || action == GLFW_REPEAT)
  {
    switch (key)
    {
      case GLFW_KEY_W:
      {
        context.printComponentTree(0, "");
        break;
      }
      case GLFW_KEY_ESCAPE:
      {
        // Create a dialog for exit
        GDialog * dialog = createDialog(context, pane, "Are you sure you want to exit?");
        dialog->addConfirmCallback(ACTION(*content, &GContent::exit));
        dialog->makeUnique("EXIT_DIALOG");
        break;
      }
    }
  }
}

int main()
{
  LSTARTLOGGER("../logs/GLW");

  LINFO("Program started.");

  // Get instance pointer
  content = GContent::getInstancePtr();

  // Set the clear colour of the scene
  content->setClearColor(glw::GREY_A / 2.0f);
  // Set the window size
  content->setWindowSize(glm::vec2(1280,720));
  // Set the callbacks for the engine, and run
  content->run(loop, init, key_callback, mouse_button_callback);

  LINFO("Program exit.");

  LENDLOGGER();

  return 0;
}
