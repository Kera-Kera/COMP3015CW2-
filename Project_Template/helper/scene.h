#pragma once

#include <glm/glm.hpp>

class Scene
{
protected:
	glm::mat4 model, view, projection;

public:
    int width;
    int height;
    int buttonPress1;
    int buttonPress2;
    int buttonPress3;
    int buttonPress4;

	Scene() : m_animate(true), width(800), height(600) { }
	virtual ~Scene() {}

	void setDimensions( int w, int h ) {
	    width = w;
	    height = h;
	}
	
    /**
      Load textures, initialize shaders, etc.
      */
    virtual void initScene() = 0;

    /**
      This is called prior to every frame.  Use this
      to update your animation.
      */
    virtual void update( float t ) = 0;

    /**
      Draw your scene.
      */
    virtual void render() = 0;

    /**
      Called when screen is resized
      */
    virtual void resize(int, int) = 0;

    void getButtonPress(int value, int value2, int value3, int value4) { buttonPress1 = value; buttonPress2 = value2; buttonPress3 = value3; buttonPress4 = value4;}

    
    void animate( bool value ) { m_animate = value; }
    bool animating() { return m_animate; }
    
protected:
	bool m_animate;
};
