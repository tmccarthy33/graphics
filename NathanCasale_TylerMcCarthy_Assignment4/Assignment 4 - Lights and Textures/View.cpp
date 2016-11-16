#include "View.h"
#include "PolygonMesh.h"
#include "sgraph/ScenegraphInfo.h"
#include "sgraph/SceneXMLReader.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>
#include <string>
using namespace std;

View::View()
{
  WINDOW_WIDTH = WINDOW_HEIGHT = 0;
  trackballRadius = 300;
  trackballTransform = glm::mat4(1.0);
  proj = glm::mat4(1.0);
  scenegraph = NULL;
  time = 0;
}

View::~View()
{
  if (scenegraph!=NULL)
    delete scenegraph;
}

void View::initScenegraph(util::OpenGLFunctions &gl, const string& filename) throw(runtime_error)
{
  if (scenegraph!=NULL)
    delete scenegraph;

  program.enable(gl);
  sgraph::ScenegraphInfo<VertexAttrib> sinfo;
  sinfo = sgraph::SceneXMLReader::importScenegraph<VertexAttrib>(filename);
  scenegraph = sinfo.scenegraph;

  renderer.setContext(&gl);
  map<string,string> shaderVarsToVertexAttribs;
  shaderVarsToVertexAttribs["vPosition"] = "position";
  shaderVarsToVertexAttribs["vNormal"] = "normal";
  shaderVarsToVertexAttribs["vTexCoord"] = "texcoord";
  renderer.initShaderProgram(program,shaderVarsToVertexAttribs);
  scenegraph->setRenderer<VertexAttrib>(&renderer,sinfo.meshes);
  scenegraph->addTexturesToRenderer(); //Add textures we parsed in Scenegraph to the renderer
  program.disable(gl);

}

void View::init(util::OpenGLFunctions& gl) throw(runtime_error)
{
  //do this if your initialization throws an error (e.g. shader not found,
  //some model not found, etc.
  //  throw runtime_error("Some error happened!");

  //create the shader program
  program.createProgram(gl,string("shaders/phong-multiple.vert"),string("shaders/phong-multiple.frag"));

  //assuming it got created, get all the shader variables that it uses
  //so we can initialize them at some point
  shaderLocations = program.getAllShaderVariables(gl);
}

void View::draw(util::OpenGLFunctions& gl)
{
  time++;
  gl.glClearColor(0,0,0,1);
  gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gl.glEnable(GL_DEPTH_TEST);

  if (scenegraph==NULL)
    return;

  program.enable(gl);

  //Empty our modelview
  while (!modelview.empty())
    modelview.pop();

  //If we are using global camera, set modelview accordingly and take into account
  //the trackball transformation
  if(camera_type == GLOBAL)
  {
      modelview.push(glm::mat4(1.0));
      modelview.top() = modelview.top() *
          glm::lookAt(glm::vec3(0.0f,50.0f,80.0f),
                      glm::vec3(0.0f,50.0f,0.0f),
                      glm::vec3(0.0f,1.0f,0.0f)) *
          trackballTransform;
  }
  else
  {
      modelview.push(glm::mat4(1.0f));
  }


  //Pass the projection matrix to the shader
  gl.glUniformMatrix4fv(shaderLocations.getLocation("projection"),
                        1,
                        false,
                        glm::value_ptr(proj));

  //Animate our two models
  scenegraph->animate(time, 2, "WALKER-");
  scenegraph->animate(time, 3, "EWOK-");

  //Parse for light information as well as the object stationary camera transform
  scenegraph->parseForLights(modelview,camera_transform);

  //If we are using object stationary camera, set the modelview accordingly
  if(camera_type == FPS)
  {
      modelview.top() = modelview.top() *
              //glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, -40.0f)) *
              glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
              camera_transform;
              //* trackballTransform;
  }

  //Enable Textures and tell OpenGL to look for its first
  //indexed texture
  gl.glEnable(GL_TEXTURE_2D);
  gl.glActiveTexture(GL_TEXTURE0);

  //Draw the objects in our scenegraph
  scenegraph->draw(modelview);

  //Flush all draws to the screen
  gl.glFlush();

  program.disable(gl);
}

void View::mousePressed(int x,int y)


{
  mousePos = glm::vec2(x,y);
}

void View::mouseReleased(int x,int y)
{

}

void View::mouseDragged(int x,int y)
{
  glm::vec2 newM = glm::vec2((float)x,(float)y);

  glm::vec2 delta = glm::vec2((float)(newM.x-mousePos.x),(float)(newM.y-mousePos.y));
  mousePos = newM;

  trackballTransform =
      glm::rotate(glm::mat4(1.0),delta.x/trackballRadius,glm::vec3(0.0f,1.0f,0.0f)) *
      glm::rotate(glm::mat4(1.0),delta.y/trackballRadius,glm::vec3(1.0f,0.0f,0.0f)) *
      trackballTransform;
}

void View::reshape(util::OpenGLFunctions& gl,int width,int height)
{
  //record the new width and height
  WINDOW_WIDTH = width;
  WINDOW_HEIGHT = height;

  /*
     * The viewport is the portion of the screen window where the drawing
     * would be placed. We want it to take up the entire area of the window
     * so we set the viewport to be the entire window.
     * Look at documentation of glViewport
     */

  gl.glViewport(0, 0, width, height);

  /*
     * This sets up the part of our virtual world that will be visible in
     * the screen window. Since this program is drawing 2D, the virtual world
     * is 2D. Thus this window can be specified in terms of a rectangle
     * Look at the documentation of glOrtho2D, which glm::ortho implements
     */

  proj = glm::perspective(glm::radians(120.0f),(float)width/height,0.1f,10000.0f);

}

void View::dispose(util::OpenGLFunctions& gl)
{
  //clean up the OpenGL resources used by the object
  scenegraph->dispose();
  renderer.dispose();
  //release the shader resources
  program.releaseShaders(gl);
}

void View::setFPS()
  {
    camera_type = FPS;
  }

void View::setGlobal()
  {
    camera_type = GLOBAL;
  }
