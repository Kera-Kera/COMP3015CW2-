#include "scenebasic_uniform.h"
#include "helper/texture.h"
#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat4;



bool activeParticles = true;
bool activeCellShade = true;


SceneBasic_Uniform::SceneBasic_Uniform() : rotSpeed(0.1f), tPrev(0), plane(10.0f, 10.0f, 2 , 2, 5.0f, 5.0f), time(0), particleLifeTime(700.5f), nParticles(80000), emitterPos(0, 0.5, 0), emitterDir(1, 2, 0)
{
    mesh = ObjMesh::loadWithAdjacency("media/cat.obj");
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    angle = glm::half_pi<float>();;

    setupFBO();

    renderProg.use();
    renderProg.setUniform("LightIntensity", vec3(1.0f));


    //Handler Buffers
    GLfloat verts[] = { -1.0f,-1.0f,0.0f,1.0f,-1.0f,0.0f,1.0f,1.0f,0.0f,-1.0f,1.0f,0.0f };
    GLuint bufHandle;
    glGenBuffers(1, &bufHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufHandle);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);

    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);

    glBindBuffer(GL_ARRAY_BUFFER, bufHandle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);


    //Textures for Models
    glActiveTexture(GL_TEXTURE2);
    catTex = Texture::loadTexture("media/texture/Cat_diffuse.jpg");
    mossTex = Texture::loadTexture("media/texture/moss.png");

    updateLight();

    renderProg.use();
    renderProg.setUniform("Tex", 2);

    compProg.use();
    compProg.setUniform("DiffSpecTex", 0);


    this->animate(true);

    //Buffer for Particles
    initBuffers();

    //Particle Texture
    glActiveTexture(GL_TEXTURE3);
    Texture::loadTexture("media/texture/bluewater.png");

    //Particle Uniform
    particProg.use();
    particProg.setUniform("ParticleTex", 3);
    particProg.setUniform("ParticleLifeTime", particleLifeTime);
    particProg.setUniform("ParticleSize", 0.02f);
    particProg.setUniform("Gravity", vec3(0.0f, -0.2f, 0.0f));
    particProg.setUniform("EmitterPos", emitterPos);


    //Cellshade Uniforms
    cellProg.use();
    cellProg.setUniform("EdgeWidth", 0.005f);
    cellProg.setUniform("PctExtent", 0.25f);
    cellProg.setUniform("LineColor", vec4(0.05f, 0.0f, 0.05f, 1.0f));
    cellProg.setUniform("LightPosition", vec4(0.0f, 0.0f, 0.0f, 1.0f));
    cellProg.setUniform("LightIntensity", vec3(1.0f, 1.0f, 1.0f));
    cellProg.setUniform("Kd", vec3(0.7f, 0.5f, 0.2f));
    cellProg.setUniform("Ka", vec3(0.2f, 0.2f, 0.2f));

}

void SceneBasic_Uniform::updateLight()
{
    lightPos = vec4(5.0f * vec3(cosf(angle) * 7.5f, 1.5f, sinf(angle) * 7.5), 1.0f);
}

void SceneBasic_Uniform::initBuffers()
{
    glGenBuffers(1, &initVel); //Inital Velocity
    glGenBuffers(1, &startTime); //Start time buffer

    //Allocate space for all buffers
    int size = nParticles * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferData(GL_ARRAY_BUFFER, size * 3, 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

    glm::mat3 emitterBasis = ParticleUtils::makeArbitraryBasis(emitterDir);
    vec3 v(0.0f);

    float velocity, theta, phi;
    std::vector<GLfloat> data(nParticles * 3);

    for (uint32_t i = 0; i < nParticles; i++)
    {
        //pick direction of velocity
        theta = glm::mix(0.0f, glm::pi<float>() / 20.0f, randFloat());
        phi = glm::mix(0.0f, glm::two_pi<float>(), randFloat());

        v.x = sinf(theta) * cosf(phi);
        v.y = cosf(theta);
        v.z = sinf(theta) * sinf(phi);

        velocity = glm::mix(1.25f, 1.5f, randFloat());
        v = glm::normalize(emitterBasis * v) * velocity;

        data[3 * i] = v.x;
        data[3 * i + 1] = v.y;
        data[3 * i + 2] = v.x;
    }

    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3, data.data());

    //Fill the start time buffer
    float rate = particleLifeTime / nParticles;
    for (int i = 0; i < nParticles; i++)
    {
        data[i] = rate * i;
    }

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &particles);
    glBindVertexArray(particles);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

float SceneBasic_Uniform::randFloat()
{
    return rand.nextFloat();
}


void SceneBasic_Uniform::setupFBO()
{
    //Depth Buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    //ambient buffer
    GLuint ambBuf;
    glGenRenderbuffers(1, &ambBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, ambBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);

    

    //diffuse+spec
    glActiveTexture(GL_TEXTURE0);
    GLuint diffSpecTex;
    glGenTextures(1, &diffSpecTex);
    glBindTexture(GL_TEXTURE_2D, diffSpecTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //creating FBO
    glGenFramebuffers(1, &colorDepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ambBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, diffSpecTex, 0);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer is complete.\n");
    }
    else
    {
        printf("Framebuffer is not complete.\n");
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::compile()
{
	try {
        cellProg.compileShader("shader/cellshade.vert");
        cellProg.compileShader("shader/cellshade.frag");
        cellProg.compileShader("shader/cellshade.gs");
        cellProg.link();

		volumeProg.compileShader("shader/basic_uniform.vert");
        volumeProg.compileShader("shader/basic_uniform.frag");
        volumeProg.compileShader("shader/basic_uniform.gs");
        volumeProg.link();

        particProg.compileShader("shader/particle.vert");
        particProg.compileShader("shader/particle.frag");
        particProg.link();

        renderProg.compileShader("shader/shadowvolume-render.vs");
        renderProg.compileShader("shader/shadowvolume-render.fs");
        renderProg.link();

        compProg.compileShader("shader/shadowvolume-comp.vs");
        compProg.compileShader("shader/shadowvolume-comp.fs");
        compProg.link();



	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}




void SceneBasic_Uniform::update( float t )
{
    time = t;

    float deltaT = t - tPrev;

    if (tPrev == 0.0f)
        deltaT = 0.0f;

    tPrev = t;

    angle += (glm::pi<float>() / 8.0f) * deltaT;

    if (angle > glm::two_pi<float>())
        angle -= glm::two_pi<float>();


    if (buttonPress1 == GLFW_PRESS)
        activeCellShade = true;
    if (buttonPress2 == GLFW_PRESS)
        activeCellShade = false;
    if (buttonPress3 == GLFW_PRESS)
        activeParticles = true;
    if (buttonPress4 == GLFW_PRESS)
        activeParticles = false;

    

    updateLight();
}




void SceneBasic_Uniform::render()
{
    

    pass1();
    glFlush();
    pass2();
    glFlush();
    pass3();

}

void SceneBasic_Uniform::pass1()
{
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);

    float c = 0.2f;
   // vec3 cameraPos(c * 1.5f * cos(angle), c * 2.0f,c * 10.5f * sin(angle));
    
    view = glm::lookAt(vec3(0.0f, 0.8f, 3.0f), vec3(0.1f, 0.1f, 0.1f), vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(70.0f), (float)width / height, 0.1f, 100.0f);

    renderProg.use();
    renderProg.setUniform("LightPosition", view * lightPos);


    glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    if (!activeCellShade) {
        cellProg.use();
        model = mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.03f));
        model = glm::rotate(model, glm::radians(-5.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(15.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, vec3(50.0, -30.5f, -50.0f));
        setMatrices(cellProg);
        mesh->render();
    }

    renderProg.use();
    drawScene(renderProg, false);

    
}

//generates Shadows
void SceneBasic_Uniform::pass2()
{
    volumeProg.use();
    volumeProg.setUniform("LightPosition", view * lightPos);

    //copies color buffers from FBO to default FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, colorDepthFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width - 1, height - 1, 0, 0, width - 1, height - 1, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);

    //Disable writing to the color buffer and depth buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    //Rebind to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Set up stencil test so that it will succeed, increments for front faces and decrements for backfaces
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xffff);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

    drawScene(volumeProg, true);

   

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void SceneBasic_Uniform::pass3()
{
    glDisable(GL_DEPTH_TEST);

    //sum the ambient and diffuse + spec when the stencil test completes
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);


    //render only pixels that have stencil value = 0
    glStencilFunc(GL_EQUAL, 0, 0xffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    compProg.use();

    model = mat4(1.0f);
    projection = model;
    view = model;
    setMatrices(compProg);

    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    

    //glFinish();
}

void SceneBasic_Uniform::drawScene(GLSLProgram& prog, bool onlyShadowCasters)
{


    vec3 color;
    if (!onlyShadowCasters)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, catTex);
        color = vec3(1.0f);
        prog.setUniform("Ka", color * 0.1f);
        prog.setUniform("Kd", color);
        prog.setUniform("Ks", vec3(0.9f));
        prog.setUniform("Shininess", 150.0f);
    }


    model = mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.18f));
    model = glm::translate(model, vec3(0.0f, -2.9f, -2.0f));
    setMatrices(prog);
    mesh->render();


    //view = glm::lookAt(vec3(3.0f * cos(angle), 1.5f, 3.0f * sin(angle)), vec3(0.0f, 1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

    //model = mat4(1.0f);



    if (!onlyShadowCasters)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mossTex);
        color = vec3(0.5f);
        prog.setUniform("Kd", color);
        prog.setUniform("Ks", vec3(0.0f));
        prog.setUniform("Ka", vec3(0.1f));
        prog.setUniform("Shininess", 1.0f);
        model = mat4(1.0f);
        setMatrices(prog);
        plane.render();
        //plane1
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -0.2f, 0.0f));
        //model = glm::rotate(model, glm::radians(90.0f), vec3(1, 0, 0));
        //model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
        setMatrices(prog);
        plane.render();
        //plane2
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -1.8f, 0.0f));
        model = glm::rotate(model, glm::radians(45.0f), vec3(1.0f, 0.0f, 0.0f));
        setMatrices(prog);
        plane.render();
        //model = mat4(1.0f);
        //plane1
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-1.8f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-45.0f), vec3(0.0f, 0.0f, 1.0f));
        setMatrices(prog);
        plane.render();
        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.8f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 0.0f, 1.0f));
        setMatrices(prog);
        plane.render();
    }

    //view = glm::lookAt(vec3(3.0f * cos(angle), 1.5f, 3.0f * sin(angle)), vec3(0.0f, 1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

    if (!activeParticles) {
        model = mat4(1.0f);
        glDepthMask(GL_FALSE);
        particProg.use();
        setMatrices(particProg);
        particProg.setUniform("Time", time);
        glBindVertexArray(particles);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
    }

    
}


void SceneBasic_Uniform::setMatrices(GLSLProgram& prog)
{
    mat4 mv = view * model;
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    prog.setUniform("ProjMatrix", projection * mv);
    prog.setUniform("Proj", projection);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    //width = w;
    //height = h;
    float c = 1.5f;

    //projection = glm::ortho(-0.4f * c, 0.4f * c, -0.3f * c, 0.3f * c, 0.1f, 100.0f);

    //projection = glm::perspective(glm::radians(90.0f), (float)w / h, 0.3f, 100.0f);
}
