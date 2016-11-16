#ifndef LIGHTLOCATION_H
#define LIGHTLOCATION_H

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class LightLocation
{
public:
  int ambient,diffuse,specular,position,spotangle,spotdirection;

  LightLocation()
  {
    ambient = diffuse = specular = position = spotangle = spotdirection = -1;
  }

};

#endif // LIGHTLOCATION_H
