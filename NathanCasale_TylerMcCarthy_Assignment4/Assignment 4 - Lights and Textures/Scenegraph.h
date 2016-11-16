#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_

#include "GLScenegraphRenderer.h"
#include "INode.h"
#include "OpenGLFunctions.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "IVertexData.h"
#include "PolygonMesh.h"
#include <string>
#include <map>
#include <iostream>

using namespace std;

namespace sgraph
{

  /**
 * A specific implementation of this scene graph. This implementation is still independent
 * of the rendering technology (i.e. OpenGL)
 * \author Amit Shesh
 */

  class Scenegraph
  {
    /**
     * The root of the scene graph tree
     */
  protected:
    INode *root;



    /**
     * A map to store the (name,node) pairs. A map is chosen for efficient search
     */
    map<string,INode *> nodes;

    map<string,string> textures;

    /**
     * The associated renderer for this scene graph. This must be set before attempting to
     * render the scene graph
     */
    GLScenegraphRenderer *renderer;


  public:
    Scenegraph()
    {
      root = NULL;
    }

    ~Scenegraph()
    {
      dispose();
    }

    void dispose()
    {

      if (root!=NULL)
        {
          delete root;
          root = NULL;
        }
    }

    /**
     * Sets the renderer, and then adds all the meshes to the renderer.
     * This function must be called when the scene graph is complete, otherwise not all of its
     * meshes will be known to the renderer
     * \param renderer The IScenegraphRenderer object that will act as its renderer
     * \throws Exception
     */
    template <class VertexType>
    void setRenderer(GLScenegraphRenderer *renderer,map<string,
                     util::PolygonMesh<VertexType> >& meshes) throw(runtime_error)
    {
      this->renderer = renderer;

      //now add all the meshes
      for (typename map<string,util::PolygonMesh<VertexType> >::iterator it=meshes.begin();
           it!=meshes.end();
           it++)
        {
          this->renderer->addMesh<VertexType>(it->first,it->second);
        }

    }


    /**
     * Set the root of the scenegraph, and then pass a reference to this scene graph object
     * to all its node. This will enable any node to call functions of its associated scene graph
     * \param root
     */

    void makeScenegraph(INode *root)
    {
      this->root = root;
      this->root->setScenegraph(this);

    }

    /**
     * Draw this scene graph. It delegates this operation to the renderer
     * \param modelView
     */
    void draw(stack<glm::mat4>& modelView) {
      if ((root!=NULL) && (renderer!=NULL))
        {
          renderer->draw(root,modelView);
        }
    }


    void animate(float time, int animation, string object)
    {

          glm::mat4 rightgun_rotation = glm::mat4(1.0f);
          glm::mat4 leftgun_rotation = glm::mat4(1.0f);
          glm::mat4 upperbody_rotation = glm::mat4(1.0f);
          glm::mat4 left_thigh_transform = glm::mat4(1.0f);
          glm::mat4 left_shin_transform = glm::mat4(1.0f);
          glm::mat4 right_thigh_transform = glm::mat4(1.0f);
          glm::mat4 right_shin_transform = glm::mat4(1.0f);
          glm::mat4 right_foot_transform = glm::mat4(1.0f);
          glm::mat4 left_foot_transform = glm::mat4(1.0f);

          switch (animation)
          {
          case 1:
                  rightgun_rotation = rightgun_rotation *
                                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,23.0f,0.0f)) *
                                          glm::rotate(glm::mat4(1.0),glm::radians(time),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -23.0f, 0.0f));
                  nodes[object+"-rightgun-transform"]->setAnimationTransform(rightgun_rotation);

                  leftgun_rotation = leftgun_rotation *
                                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,23.0f,0.0f)) *
                                          glm::rotate(glm::mat4(1.0),glm::radians(time),glm::vec3(-1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -23.0f, 0.0f));
                  nodes[object+"-leftgun-transform"]->setAnimationTransform(leftgun_rotation);

                  //Animation to make walker crouch
                  right_thigh_transform = right_thigh_transform *
                                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,70.0f,-5.0f)) *
                                          glm::rotate(glm::mat4(1.0f), glm::radians(40*sin(time/50)),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,-70.0f,5.0f));
                  nodes[object+"-rightleg"]->setAnimationTransform(right_thigh_transform);

                  left_thigh_transform = left_thigh_transform *
                                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,70.0f,-5.0f)) *
                                          glm::rotate(glm::mat4(1.0f), glm::radians(40*sin(time/50)),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,-70.0f,5.0f));
                  nodes[object+"-leftleg"]->setAnimationTransform(left_thigh_transform);

                  right_shin_transform = right_shin_transform *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,52.0f,-20.0f)) *
                                          glm::rotate(glm::mat4(1.0f),glm::radians(-60*sin(time/50)),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,-52.0f,20.0f));
                  nodes[object+"-rightlowerleg"]->setAnimationTransform(right_shin_transform);

                  left_shin_transform = left_shin_transform *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,52.0f,-20.0f)) *
                                          glm::rotate(glm::mat4(1.0f),glm::radians(-60*sin(time/50)),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,-52.0f,20.0f));
                  nodes[object+"-leftlowerleg"]->setAnimationTransform(left_shin_transform);

                  float foot_angle;
                  if(-10*sin(time/50) >= 0)
                  {
                      foot_angle = 0;
                  }
                  else
                      foot_angle = -10*sin(time/50);

                  right_foot_transform = right_foot_transform *
                                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,21.0f,-2.0f)) *
                                          glm::rotate(glm::mat4(1.0f),glm::radians(foot_angle),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,-21.0f,2.0f));
                  nodes[object+"-rightfoot"]->setAnimationTransform(right_foot_transform);

                  left_foot_transform = left_foot_transform *
                                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,21.0f,-2.0f)) *
                                          glm::rotate(glm::mat4(1.0f),glm::radians(foot_angle),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,-21.0f,2.0f));
                  nodes[object+"-leftfoot"]->setAnimationTransform(left_foot_transform);
                      break;
          case 2:
                  upperbody_rotation = upperbody_rotation *
                                          glm::rotate(glm::mat4(1.0),glm::radians(80*sin(time/50)),glm::vec3(0.0f,1.0f,0.0f));
                  nodes[object+"-torso-rotation"]->setAnimationTransform(upperbody_rotation);
                  left_thigh_transform = left_thigh_transform *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 70.0f, -5.0f)) *
                                          glm::rotate(glm::mat4(1.0),glm::radians(40*sin(time/50)),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -70.0f, 5.0f));
                  nodes[object+"-leftleg"]->setAnimationTransform(left_thigh_transform);
                  left_shin_transform = left_shin_transform *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 52.0f, -20.0f)) *
                                          glm::rotate(glm::mat4(1.0f),glm::radians(80*sin(time/50+150)),glm::vec3(-1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -52.0f, 20.0f));
                  nodes[object+"-leftlowerleg"]->setAnimationTransform(left_shin_transform);
                  right_thigh_transform = right_thigh_transform *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 70.0f, -5.0f)) *
                                          glm::rotate(glm::mat4(1.0),glm::radians(40*sin(time/50+3.14159f)),glm::vec3(1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -70.0f, 5.0f));
                  nodes[object+"-rightleg"]->setAnimationTransform(right_thigh_transform);
                  right_shin_transform = right_shin_transform *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 52.0f, -20.0f)) *
                                          glm::rotate(glm::mat4(1.0f),glm::radians(80*sin(time/50+150+3.14159f)),glm::vec3(-1.0f,0.0f,0.0f)) *
                                          glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -52.0f, 20.0f));
                  nodes[object+"-rightlowerleg"]->setAnimationTransform(right_shin_transform);
                  break;
          case 3:
                  break;
          default:
                  break;
          }
    }

    void addNode(const string& name, INode *node) {
      nodes[name]=node;
    }


    INode *getRoot()
    {
      return root;
    }



    map<string, INode *> getNodes()
    {
      return nodes;
    }

    void addTexture(const string& name, const string& path)
    {
      textures[name] = path;
    }
  };
}
#endif
