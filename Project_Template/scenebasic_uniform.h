#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

//#include "helper/skybox.h"
#include <GLFW/glfw3.h>
#include "helper/objmesh.h"
#include "helper/plane.h"
#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/grid.h"
#include "helper/frustum.h"
#include "helper/random.h"
#include "helper/particleutils.h"
#include <glm/glm.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram volumeProg, renderProg, compProg, particProg, flatProg, cellProg;
    GLuint catTex, mossTex, initVel, startTime, particles, nParticles, colorDepthFBO, fsQuad;

    std::unique_ptr<ObjMesh> mesh;
    Plane plane;

    Random rand;

    glm::vec4 lightPos;

    Grid grid;

    glm::vec3 emitterPos, emitterDir;

    float angle, tPrev, rotSpeed, time, particleLifeTime;

    void initBuffers();
    float randFloat();

    void setMatrices(GLSLProgram&);

    void compile();
    
    void setupFBO();
    void drawScene(GLSLProgram&, bool);
    void pass1();
    void pass2();
    void pass3();
    void updateLight();

public:
    SceneBasic_Uniform();
    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
