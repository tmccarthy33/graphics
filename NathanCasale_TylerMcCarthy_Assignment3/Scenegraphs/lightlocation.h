#ifndef LIGHTLOCATION_H
#define LIGHTLOCATION_H

class LightLocation
{
public:
  int ambient,diffuse,specular,position;
  LightLocation()
  {
    ambient = diffuse = specular = position = -1;
  }

};

#endif // LIGHTLOCATION_H
