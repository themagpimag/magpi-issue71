#include <iostream> // we need this to output to console this file contains cout
#include "SimpleObj.h"

using namespace std;

// we also can draw index values
//if we sent these to the gpu
static const GLfloat QuadVertices[] = 
{  // Square vertices with no texture
	1, 1,0, -1,1,0, 1,-1,0, /*now 2nd tri*/ -1,1,0, -1,-1,0,	1, -1,0,	 
};
	


// as well as 
SimpleObj::SimpleObj() 
{
	Xspeed = 0.0f; // lets make our construc
	Yspeed = 0.0f;
	ScaleX = 16.0; // this tells us how many pixels to make our texture square, we are using 16x16 so 16 give us a one to one ratio, but we can scale up or down
	ScaleY = 16.0; // default to haveing them both have the same value

} 


SimpleObj::~SimpleObj() {} // no code yet in the destructor

/************************************************************************ 
Update has to do some work, in this case we'll print out some info on our position and
move our object in pixel space, and do a few tests to look interesting
************************************************************************/

void SimpleObj::Update()
{		// this is a tile, so it does not really need any code, its set up when it was made

		SimpleObj::Draw();   // just draw at the xy coordinates
	

}


/************************************************************************
The Draw system needs a bit of setting up
The shader will draw what we tell it but we have to send it some
************************************************************************/
void SimpleObj::Draw() 
{

	
/***********************************************************************
This is still a rather hacky way to send position data, but its getting 
better as we pass more reponsibility to the shader
We could improve on this quite a bit if our position data itself was
a simpleVec2, but we are going to do a better system later so lets not expand
too much on this idea beyond the fact we can create new types
************************************************************************/
	
// Lets use a couple of simpleVec2's, its still not an elegant way to do this but it demonstrates some ideas we can use elsewhere

	simpleVec2 testingVec = { Xpos, Ypos };
	simpleVec2 ScreenData = { 512, 384 };	// we only need half the screen size which is currently a fixed amount
	simpleVec2 Scale = { ScaleX, ScaleX };
	
// we have reduced our interaction with the GPU, but we still have to send the variable infomation each time
	
// 2nd, we can now transfer this to the u_position value in the shader which we have a handle for in positionUniformLoc
	glUniform2fv(positionUniformLoc, 1, &testingVec.xValue); //<< notice the format, we are passing 2 float values (2fV) starting with the 1st which is the xValue
// now transfer our screen sizes as well
	glUniform2fv(ScreenCoordLoc, 1, &ScreenData.xValue);   // pass our half screen sizes by pointing to address of the 1st value in the struct
// we also need to transfer scale, this is a simpler format, 1 float, just tell it the value to send
	glUniform2fv(ScaleLoc, 1,&Scale.xValue);

// each instance has its own texture now so lets "bind it" which means we tell the GPU that this is the only texture it should care about for now
// we could use a single texture, not for speed but for clarity	but we will do that later
	glBindTexture(GL_TEXTURE_2D, TextureID);

		
// we already set up the attributes with start, type, size and stride, so they are ready to go, this enable function resets them
// to their starts so we can do the next object
	glEnableVertexAttribArray(positionLoc);   // when we enable it, the shader can now use it and it starts at its base value
	glEnableVertexAttribArray(texCoordLoc);   // when we enable it, the shader can now use it


// now its been set up, tell it to draw 6 vertices which make up a square	
	glDrawArrays(GL_TRIANGLES, 0, 6);

//  this is a useful thing but its rather slow, so now we know things work we've removed it
//  bring it back if you find there might be errors	
//	if (glGetError() != GL_NO_ERROR) printf("OpenGLES2.0 reports an error!");	
	
} 

// this is a different update, in SimpleObj, its not going to do much really
// but the classes that inheret SimpleObj will use their own versions it.

bool SimpleObj::Update(Game* TheGameClass)
{
	// as a
	return true;
}



// let the ball test to see if it hit any bullets

bool SimpleObj::DidIGetHit(SimpleObj* Obj2)
 // a default box check
{
	// simple box check using AABB principles
	// box 1 we have to remember our 16x16 square has its centre in the middle so adjust to the edges
	int x1Min = Xpos - 8;   // we know the size of these so we can use hard numbers
	int x1Max = Xpos + 8;
	int y1Min = Ypos - 8;
	int y1Max = Ypos + 8;
	
	// box 2 
	int x2Min = Obj2->Xpos - (Obj2->width / 2);   // adjust from the centre, we don't know the size of this, but we know its stored use half values to move to the top
	int x2Max = Obj2->Xpos + (Obj2->width / 2);
	int y2Min = Obj2->Ypos - (Obj2->width / 2);
	int y2Max = Obj2->Ypos + (Obj2->width / 2);
	
	// check x 1st as its more likely to fail
	if(x1Max < x2Min || x1Min > x2Max) return false;
	if (y1Max < y2Min || y1Min > y2Max) return false;
	
	return true;
}
