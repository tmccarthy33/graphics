#ifndef _GLSCENEGRAPHRENDERER_H_
#define _GLSCENEGRAPHRENDERER_H_

#include "INode.h"
#include "OpenGLFunctions.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "Material.h"
#include "TextureImage.h"
#include "ObjectInstance.h"
#include "IVertexData.h"
#include "ShaderLocationsVault.h"
#include <string>
#include <map>
#include <stack>
#include "View.h"
#include "lightlocation.h"
using namespace std;

namespace sgraph
{

/**
 * This is a scene graph renderer implementation that works specifically
 * with the Qt library
 * It mandates OpenGL 3 and above.
 * \author Amit Shesh
 */
class GLScenegraphRenderer
{
    /**
     * The Qt specific rendering context
     */
private:
    util::OpenGLFunctions *glContext;
    /**
     * A table of shader locations and variable names
     */
protected :
    util::ShaderLocationsVault shaderLocations;
    /**
     * A table of shader variables -> vertex attribute names in each mesh
     */
    map<string,string> shaderVarsToVertexAttribs;

    /**
     * A map to store all the textures
     */
    map<string, util::TextureImage *> textures;
    /**
     * A table of renderers for individual meshes
     */
    map<string, util::ObjectInstance *> meshRenderers;

    /**
     * A variable tracking whether shader locations have been set. This must be done before
     * drawing!
     */
    bool shaderLocationsSet;

    /**
      * Keeps track of how many lights we have put into our scene
      */
    int light_count = 0;

public:
    GLScenegraphRenderer()
    {
        shaderLocationsSet = false;
    }

    /**
     * Specifically checks if the passed rendering context is the correct JOGL-specific
     * rendering context
     * \param obj the rendering context (should be OpenGLFunctions)
     */
    void setContext(util::OpenGLFunctions *obj)
    {
        glContext = obj;
    }

    /**
     * Add a mesh to be drawn later.
     * The rendering context should be set before calling this function, as this function needs it
     * This function creates a new sgraph::GLMeshRenderer object for this mesh
     * \param name the name by which this mesh is referred to by the scene graph
     * \param mesh the util::PolygonMesh object that represents this mesh
     */
    template <class K>
    void addMesh(const string& name,
                 util::PolygonMesh<K>& mesh) throw(runtime_error)
    {
        if (!shaderLocationsSet)
            throw runtime_error("Attempting to add mesh before setting shader variables. Call initShaderProgram first");
        if (glContext==NULL)
            throw runtime_error("Attempting to add mesh before setting GL context. Call setContext and pass it a GLAutoDrawable first.");

        //verify that the mesh has all the vertex attributes as specified in the map
        if (mesh.getVertexCount()<=0)
            return;
        K vertexData = mesh.getVertexAttributes()[0];
        for (map<string,string>::iterator it=shaderVarsToVertexAttribs.begin();
             it!=shaderVarsToVertexAttribs.end();it++) {
            if (!vertexData.hasData(it->second))
                throw runtime_error("Mesh does not have vertex attribute "+it->second);
        }
        util::ObjectInstance *mr = new util::ObjectInstance(name);
        mr->initPolygonMesh<K>(*glContext,
                            shaderLocations,
                            shaderVarsToVertexAttribs,
                            mesh);
        this->meshRenderers[name] = mr;
    }

    void addTexture(const string& name,const string& path)
    {
        util::TextureImage *image = NULL;
        try {
            image = new util::TextureImage(path,name);
        } catch (runtime_error e) {
            throw runtime_error("Texture "+path+" cannot be read!");
        }
        textures[name]=image;
    }

    /**
     * Begin rendering of the scene graph from the root
     * \param root
     * \param modelView
     */
    void draw(INode *root, stack<glm::mat4>& modelView)
    {
        root->draw(*this,modelView);
    }

    void parseForLights(INode* root, vector<LightLocation>& light_locs, stack<glm::mat4>& modelview,util::OpenGLFunctions& gl,glm::mat4& camera_transform)
    {
        root->parseForLights(*this,light_locs,modelview,gl,camera_transform);
    }

    void getLights(vector<util::Light>& lights, vector<LightLocation>& light_locs, const glm::mat4& transformation,util::OpenGLFunctions& gl)
    {
        //Set light shader variables here...
        if(lights.size()!=0)
        {
           int numLightsLocation = shaderLocations.getLocation("numLights");
           stringstream name;
           for(int i=0; i<lights.size(); i++)
           {
               LightLocation lL;
               name << "light[" << light_count << "]";

               lL.ambient = shaderLocations.getLocation(name.str() + "" +".ambient");
               lL.diffuse = shaderLocations.getLocation(name.str() + ".diffuse");
               lL.specular = shaderLocations.getLocation(name.str() + ".specular");
               lL.position = shaderLocations.getLocation(name.str() + ".position");
               lL.spotangle = shaderLocations.getLocation(name.str() + ".spotangle");
               lL.spotdirection = shaderLocations.getLocation(name.str() + ".spotdirection");


               light_locs.push_back(lL);

               //Actually set light properties in shader
               glm::vec4 pos = lights[i].getPosition();
               pos = transformation * pos;
               gl.glUniform4fv(lL.position, 1, glm::value_ptr(pos));

               gl.glUniform1i(numLightsLocation,light_count);

               gl.glUniform3fv(lL.ambient, 1, glm::value_ptr(lights[i].getAmbient()));
               gl.glUniform3fv(lL.diffuse, 1, glm::value_ptr(lights[i].getAmbient()));
               gl.glUniform3fv(lL.specular, 1,glm::value_ptr(lights[i].getSpecular()));

               //Set Spot angle and direction
               glm::vec4 spot_dir = lights[i].getSpotDirection();
               spot_dir = transformation * spot_dir;

               gl.glUniform3fv(lL.spotdirection,1,glm::value_ptr(spot_dir));

               float spot_angle = lights[i].getSpotCutoff();
               if(spot_angle == 0)
                   spot_angle = 180.0f;
               gl.glUniform1f(lL.spotangle,spot_angle);


               light_count++;


           }
        }

    }

    void resetLightCount()
    {
        light_count = 0;
    }

    void dispose()
    {
        for (map<string,util::ObjectInstance *>::iterator it=meshRenderers.begin();
             it!=meshRenderers.end();it++)
          {
            it->second->cleanup(*glContext);
          }
    }
    /**
     * Draws a specific mesh.
     * If the mesh has been added to this renderer, it delegates to its correspond mesh renderer
     * This function first passes the material to the shader. Currently it uses the shader variable
     * "vColor" and passes it the ambient part of the material. When lighting is enabled, this method must
     * be overriden to set the ambient, diffuse, specular, shininess etc. values to the shader
     * \param name
     * \param material
     * \param transformation
     */
    void drawMesh(const string& name,
                  const util::Material& material,
                  const string& textureName,
                  const glm::mat4& transformation)
    {
        if (meshRenderers.count(name)==1)
        {

            int loc = shaderLocations.getLocation("modelview");
            if (loc<0)
                throw runtime_error("No shader variable for \" modelview \"");

            glContext->glUniformMatrix4fv(loc,
                                  1,
                                  false,glm::value_ptr(transformation));

            meshRenderers[name]->draw(*glContext);

            //Pass Material Properties to Shader
            loc = shaderLocations.getLocation("material.ambient");
            if(loc < 0)
                throw runtime_error("No shader varialbe for \" material.ambient \"");
            glContext->glUniform3fv(loc,
                                    1,
                                    glm::value_ptr(material.getAmbient()));

            loc = shaderLocations.getLocation("material.diffuse");
            if(loc < 0)
                throw runtime_error("No shader varialbe for \" material.diffuse \"");
            glContext->glUniform3fv(loc,
                                    1,
                                    glm::value_ptr(material.getDiffuse()));
            loc = shaderLocations.getLocation("material.specular");
            if(loc < 0)
                throw runtime_error("No shader varialbe for \" material.specular \"");
            glContext->glUniform3fv(loc,
                                    1,
                                    glm::value_ptr(material.getSpecular()));
            loc = shaderLocations.getLocation("material.shininess");
            if(loc < 0)
                throw runtime_error("No shader varialbe for \" material.shininess \"");
            glContext->glUniform1f(loc,
                                    material.getShininess());

            /*loc = shaderLocations.getLocation("image");
            if(loc < 0)
                throw(runtime_error("No shader variable for \"image\""));


            QOpenGLTexture * tex = textures[textureName]->getTexture();

            tex->bind();

            glContext->glUniform1i(loc,0);*/
        }
    }

    /*void drawMesh(const string& name,
                  const util::Material& material,
                  const util::Light& light,
                  const string& textureName,
                  const glm::mat4& transformation)
    {
        //Pass Modelview to shader
        int loc = shaderLocations.getLocation("modelview");
        if(loc < 0)
            throw runtime_error("No shader varialbe for \" modelview\"");
        glContext->glUniformMatrix4fv(loc,
                                      1,
                                      false,
                                      glm::value_ptr(transformation));

        //Pass Normal Matrix to shader
        loc = shaderLocations.getLocation("normalmatrix");
        if(loc < 0)
            throw runtime_error("No shader varialbe for \" normalmatrix\"");
        glm::mat4 normal_mat = glm::inverse(glm::transpose(transformation));
        glContext->glUniformMatrix4fv(loc,
                                      1,
                                      false,
                                      glm::value_ptr(normal_mat));

        //Pass Texture variables to Shader
        //Probably going to need to expand upon this/add texture to parameters
        loc = shaderLocations.getLocation("texturematrix");
        if(loc < 0)
            throw runtime_error("No shader varialbe for \" texturematrix \"");
        glContext->glUniform1i(loc,
                               0);

        //textures
        util::TextureImage *textureImage;

        textureImage = textures[textureName];

        QOpenGLTexture * tex = textureImage->getTexture();

        tex->setWrapMode(QOpenGLTexture::Repeat);
        tex->setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Nearest);

        tex->bind();







    }*/



    /**
     * Queries the shader program for all variables and locations, and adds them to itself
     * \param shaderProgram
     */
    void initShaderProgram(util::ShaderProgram& shaderProgram,
                           map<string,string>& shaderVarsToVertexAttribs)
    {
        if (glContext==NULL)
          throw runtime_error("No context set");

        shaderLocations = shaderProgram.getAllShaderVariables(*glContext);
        this->shaderVarsToVertexAttribs = shaderVarsToVertexAttribs;
        shaderLocationsSet = true;

    }

    int getShaderLocation(const string& name)
    {
        return shaderLocations.getLocation(name);
    }
};
}
#endif
