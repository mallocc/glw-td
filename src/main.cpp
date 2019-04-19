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
using glw::meta::GLinker;

using TD::Credit;
using TD::Life;
using TD::Tick;
using TD::Game;
using TD::Grid;
using TD::Map;
using TD::Tower;
using TD::Path;
using TD::Mob;
using TD::Wave;

class GridComponent;
class ShopComponent;

namespace
{
  const char* TRG = "MAIN";
  const char* __CLASSNAME__ = "main";

  GContent* content;
  GShaderProgramManager shaderProgramManager;
  GShaderProgramId BASIC_PROGRAM;
  GShaderProgramId GUI_PROGRAM;

  GVertexBufferObject vbo;

  GArrayVBO guiVBOs;

  GCamera camera(glm::vec3(0, 0, 5), glm::vec3(), glm::vec3(0,0,-1), glm::vec3(0, 1, 0));

  GContext context;

  GLabel* fpsLabel;

  GPane* pane;

  Grid mapGrid;
  Grid shopGrid;

  GridComponent* map;
  ShopComponent* shop;
}


#define NULL_SELECTED_INDEX -1

struct TableCell
{
  int x, y;
};

class GridComponent : public GClickable
{
public:

  enum CellType
  {
    CT_SELECTED,
    CT_EMPTY,
    CT_TOWER,
    CT_COUNT
  };

  GridComponent()
    : m_grid(NULL),
      m_selectedCell({NULL_SELECTED_INDEX, NULL_SELECTED_INDEX}),
      m_uniqueImages()
  {
    initImageGrid(DEFAULT_GRIDSIZE, DEFAULT_GRIDSIZE);
  }

  GridComponent(glm::vec2 pos, glm::vec2 size, Grid* grid = NULL)
    : GClickable(pos, size),
      m_grid (grid),
      m_selectedCell({NULL_SELECTED_INDEX, NULL_SELECTED_INDEX}),
      m_uniqueImages()
  {
    if (NULL != grid)
    {
      initImageGrid(grid->getWidth(), grid->getHeight());
    }
  }

  virtual void draw(glm::mat4 parentMatrix, glw::engine::glsl::GShaderHandle_T shaderHandle, glw::gui::GContextShaderHandle_T contextHandle)
  {
    if (NULL != m_grid)
    {
      for (int ix = 0; ix < m_grid->getWidth(); ++ix)
      {
        for (int iy = 0; iy < m_grid ->getHeight(); ++iy)
        {
          CellType ct = CT_EMPTY;
          if (NULL != m_grid->getTower(ix,iy))
          {
            ct = CT_TOWER;
          }
          GImageView& image = m_uniqueImages[ct];
          image.setPos(glm::vec2(image.getSize().x * ix,
                                 image.getSize().y * iy));
          image.validate();
          image.draw(parentMatrix * getRelativeModelMatrix(), shaderHandle, contextHandle);
        }
      }
      if (m_grid->isWithin(m_selectedCell.x, m_selectedCell.y))
      {
        GImageView& image = m_uniqueImages[CT_SELECTED];
        image.setPos(glm::vec2(image.getSize().x * m_selectedCell.x,
                               image.getSize().y * m_selectedCell.y));
        image.validate();
        image.draw(parentMatrix * getRelativeModelMatrix(), shaderHandle, contextHandle);
      }
    }
  }

  virtual bool checkKeyEvents(int key, int action)
  {
    return false;
  }

  virtual bool checkMouseEvents(int button, int action)
  {
    bool eventHasHappened = false;

    eventHasHappened |= GClickable::checkMouseEvents(button, action);

    onPressed();

    return eventHasHappened;
  }

  virtual void init(glw::gui::GContext *context, IGComponent *parent)
  {
    setContext(context);
    setParent(parent);
    setId("gridcomponent");
    inheritColorStyle();

    loadImages();
    initImages(context, parent);

  }

  virtual void validate()
  {

  }

  virtual void update()
  {

  }

  void onPressed()
  {
    if (NULL != m_grid)
    {
      if (this->isPressed())
      {
        glm::vec2 pos = this->getRelativeMousePos();

        float cellWidth = this->getSize().x / m_grid->getWidth();
        float cellHeight = this->getSize().y / m_grid->getHeight();

        int x = pos.x / cellWidth;
        int y = pos.y / cellHeight;

        m_selectedCell = {x, y};
      }
    }
  }


protected:
  Grid* m_grid;

private:
  TableCell m_selectedCell;

  std::map<CellType, GImageView> m_uniqueImages;

  typedef CellType* CellRow;
  typedef CellRow* CellGrid;
  CellGrid m_cellGrid;

  void initImageGrid(int width, int height)
  {
    m_cellGrid = new CellRow[width];
    for (int i = 0; i < width; ++i)
    {
      m_cellGrid[i] = new CellType[height];
    }

    for (int i = 0; i < width; ++i)
    {
      for (int j = 0; j < height; ++j)
      {
        m_cellGrid[i][j] = CT_EMPTY;
      }
    }
  }

  void loadImages()
  {
    if (NULL != m_grid)
    {
      float cellWidth = this->getSize().x / m_grid->getWidth();
      float cellHeight = this->getSize().y / m_grid->getHeight();
      m_uniqueImages[CT_EMPTY] = GImageView(glm::vec2(),
                                            glm::vec2(cellWidth,
                                                      cellHeight),
                                            "../textures/empty.png");
      m_uniqueImages[CT_TOWER] = GImageView(glm::vec2(),
                                            glm::vec2(cellWidth,
                                                      cellHeight),
                                            "../textures/tower.png");
      m_uniqueImages[CT_SELECTED] = GImageView(glm::vec2(),
                                            glm::vec2(cellWidth,
                                                      cellHeight),
                                            "../textures/selected.png");
    }
  }
  void initImages(glw::gui::GContext *context, IGComponent *parent)
  {
    std::map<CellType, GImageView>::iterator itor = m_uniqueImages.begin();

    while(itor != m_uniqueImages.end())
    {
      itor->second.init(context, parent);
      ++itor;
    }
  }
};

class ShopComponent : public GridComponent
{
public:
  ShopComponent()
  {}

  ShopComponent(glm::vec2 pos, glm::vec2 size)
    : GridComponent(pos, size, new Grid(GridComponent::CT_COUNT, 1))
  {
    m_grid->placeTower(0, 0, new Tower());
  }

  virtual void init(glw::gui::GContext *context, IGComponent *parent)
  {
    GridComponent::init(context, parent);
  }
  virtual void draw(glm::mat4 parentMatrix, glw::engine::glsl::GShaderHandle_T shaderHandle, glw::gui::GContextShaderHandle_T contextHandle)
  {
    GridComponent::draw(parentMatrix, shaderHandle, contextHandle);
  }
  virtual bool checkKeyEvents(int key, int action)
  {
    return GridComponent::checkKeyEvents(key, action);
  }
  virtual bool checkMouseEvents(int button, int action)
  {
    return GridComponent::checkMouseEvents(button, action);
  }
  virtual void validate()
  {
    GridComponent::validate();
  }
  virtual void update()
  {
    GridComponent::update();
  }
private:

};

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

//  mapGrid = Grid(10,10);
//  map = new GridComponent(glm::vec2(), glm::vec2(500), &mapGrid);
//  map->setPos((windowSize - map->getSize()) / 2.0f);
//  map->validate();
//  pane->addComponent(map);

  shop = new ShopComponent(glm::vec2(), glm::vec2(300, 100));
  shop->setPos(glm::vec2((windowSize.x - shop->getSize().x)/2.0f, windowSize.y - shop->getSize().y));
  shop->validate();
  pane->addComponent(shop);


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
    case GLFW_KEY_ESCAPE:
      // Create a dialog for exit
      GDialog * dialog = createDialog(context, pane, "Are you sure you want to exit?");
      dialog->addConfirmCallback(ACTION(*content, &GContent::exit));
      dialog->makeUnique("EXIT_DIALOG");
      break;
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
