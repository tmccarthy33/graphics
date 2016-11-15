#version 140

struct MaterialProperties
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct LightProperties
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 position;
    vec3 spotdirection;
    float spotangle;
};


in vec3 fNormal;
in vec4 fPosition;
in vec4 fTexCoord;

const int MAXLIGHTS = 10;

uniform MaterialProperties material;
uniform LightProperties light[MAXLIGHTS];
uniform int numLights;

/* texture */
uniform sampler2D image;

out vec4 fColor;

void main()
{
    vec3 lightVec,viewVec,reflectVec;
    vec3 normalView;
    vec3 ambient,diffuse,specular;
    vec3 spotdir;
    float nDotL,rDotV,dDotNegL,cosTheta;
    float in_spotlight;


    fColor = vec4(0,0,0,1);

    for (int i=0;i<numLights;i++)
    {
        if (light[i].position.w!=0)
            lightVec = normalize(light[i].position.xyz - fPosition.xyz);
        else
            lightVec = normalize(-light[i].position.xyz);

        vec3 tNormal = fNormal;
        normalView = normalize(tNormal.xyz);
        nDotL = dot(normalView,lightVec);

        viewVec = -fPosition.xyz;
        viewVec = normalize(viewVec);

        reflectVec = reflect(-lightVec,normalView);
        reflectVec = normalize(reflectVec);

        rDotV = max(dot(reflectVec,viewVec),0.0);

        ambient = material.ambient * light[i].ambient;
        diffuse = material.diffuse * light[i].diffuse * max(nDotL,0);
        if (nDotL>0)
            specular = material.specular * light[i].specular * pow(rDotV,material.shininess);
        else
            specular = vec3(0,0,0); 

        spotdir = normalize(light[i].spotdirection.xyz);
        dDotNegL = dot(spotdir,(-lightVec));
        cosTheta = cos(light[i].spotangle);
        if(dDotNegL >= cosTheta)
            in_spotlight = 1.0f;
        else
            in_spotlight = 0.0f;



        //fColor = fColor + in_spotlight*vec4(ambient+diffuse+specular,1.0) - in_spotlight*vec4(ambient+diffuse+specular,1.0) ;
        //fColor = fColor + vec4(0,0,1,1) - fColor;
        fColor = fColor + vec4(ambient+diffuse+specular,1.0);
        //fColor = fColor + vec4(0.5*(1+dDotNegL), 0.5*(1+dDotNegL),0.5*(1+dDotNegL),1.0);
    }
    fColor = fColor + texture(image,fTexCoord.st);
    //fColor = vec4(fTexCoord.s,fTexCoord.t,0,1);
}
