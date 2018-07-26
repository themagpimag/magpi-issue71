// On the surface our code is much the same but we've added a much more complex update system called FastUpdate
// you can set the value in OGL.h to TRUE to see the impressive difference in speed.
// this will help us to demonstrate why code that works fine on the surface can be dramatically improved on by small changes in the way we update.
// However there is an extra level of complexity (which we've actually made more complex by having two systems)
// That can be a little daunting. But run the code with FastUpdate set to FALSE then TRUE, and decide if its worth
// the effort of the extra complexity?
// You will find the slower code a little easier to work with as you test the attributes and movment sytems for climbing
// next time we'll improve and clean up the code to make it a lot more playable.


#include "Game.h"
#include "SimpleObj.h"
#include "Bob.h"

GLint SimpleObj::ProgramObject = 0;
GLint SimpleObj::positionLoc =0;
GLint SimpleObj::texCoordLoc = 0;

// if we want to position it somewhere we need to ask for the handle for the position uniform and other unfirms
GLint SimpleObj::positionUniformLoc = 0;
GLint SimpleObj::samplerLoc = 0;
GLint SimpleObj::ScreenCoordLoc = 0;
GLint SimpleObj::ScaleLoc = 0;
GLuint SimpleObj::VBO = -1;


// we are going to reflect the size of our textures, so we need a square that has size 1x1
// since the distance from the centre to the corners is 0.5x and 0.5y, two tringles will use offsets of 0.5f
static const GLfloat QuadVerticesWithTex[] =
{
	// square vertices with texture coordinates added, so we have 3 floats of vertex,
   // then 2 floats of texture coords (called uv coords), this is called interlacing data
	0.5f,0.5f,0,
	1.0f,0.0f,	// texture coordinate
	-0.5f,0.5f,0,
	0.0f,0.0f,	// texture coordinate
	0.5f,-0.5f,  0,
	1.0f,1.0f, 	// texture coordinate

	//2nd tri
	-0.5f,0.5f,0,
	0.0f,0.0f,	// texture coordinate
	-0.5f,-0.5f,0,
	0.0f,1.0f, 	// texture coordinate
	0.5f,-0.5f,0,
	1.0f,1.0f // texture coordinate
};

// because we defined this as static variables in our header we have to make sure it exists

std::vector<SimpleObj*> Game::MyObjects;

Game::Game()
{
	OGLES.Init(); // make sure we initialise the OGL instance we called OGLES



// this time we will do a few things to set up our GPU to draw faster, the trick is to avoid doing any GPU calls we don't need to
// we can setup a VBO, make it the current active buffer (binding it) and also set up the attribute pointers so that the VBO data
// goes to them, since all current objects use the same quad, we will use a static VBO

	// because we're always using the same shader, we can do these set ups 1 time also, and not each time we draw
	glUseProgram(OGLES.programObject);
	//	 Get the attribute locations, we'll store these in values we can call handles.
	SimpleObj::positionLoc = glGetAttribLocation(OGLES.programObject, "a_position");
	SimpleObj::texCoordLoc = glGetAttribLocation(OGLES.programObject, "a_texCoord");


// put our vertices into our VBO
	glGenBuffers(1, &SimpleObj::VBO);  // first we generate the buffer and store the handle in VBO
	glBindBuffer(GL_ARRAY_BUFFER, SimpleObj::VBO);   //now we mind that, which we can leave as bound since we use the same VBO
	glBufferData(GL_ARRAY_BUFFER, // now we send the vertex data to the GPU one time
		6*5 * sizeof(float),
		QuadVerticesWithTex,
		GL_STATIC_DRAW);



	// this function is able to take an address in CPU as well as an index in the GPU, but when sending an index, we have


};

Game::~Game() {}; // We should close down some things here but can do it later.


void Game::Update()
{

// make our Bob
	Bob* MyBob = new Bob();
	MyBob->TheOGLInstance = &OGLES;
	MyObjects.push_back(MyBob);  // put our Bob characer into a processing list of characters


	// here's another useful array
	GLint TileIDs[16];  // define an array to hold the id's we get

#if (FastUpdate == TRUE)
// if we are using the FastUpdate we are going to draw the tiles in a totally different way.
// 1 texture
// a GPU based buffer called a VBO with all the positions and graphic of the tiles prepped and ready
// 1 draw call

		int Grwidth, Grheight, comp;
		char* OneTileTexture =  	(char*)"Assets/tileset.png"; // 1st load our texture containing all our tiles
// now lets build our buffer


// make our texture	but just load it into the 0 entry of the array
		unsigned char* data = stbi_load(OneTileTexture, &Grwidth, &Grheight, &comp, 4);     // ask it to load 4 componants since its rgba
		TileIDs[0] = this->OGLES.CreateTexture2D(Grwidth, Grheight, (char*) data);      //just pass the width and height of the graphic, and where it is located and we can make a texture
		free(data);  // when we load an image, it goes into memory, making it a texture copies it to GPU memory, but we need to remove the CPU memory before to load another or it causes a leak

// ok we loaded our texture, now lets create a GPU buffer called a VBO and stoe all the vertices we need to draw the whole map in there.

typedef struct // A very simple structure to hold the data we need to send to the new special shader that uses uv's
	{
		float	PositionX;  // we will load these into the shader
		float	PositionY; // these are basially the relative offsets of the quad
		float	dummyZ;
// now this time we send the screen pixel position this way
		float	sPositionX;   // we will load these into the shader
		float	sPositionY;  // these are basially the relative offsets of the quad

		float	u;		// our shader will
		float	v;



	} TileVectorData;

// now we will build up our tiles in to our array but this time our array is made up of TileData

		std::vector<TileVectorData> NewTiles;
// because we are using a single texture where each tile is a fraction of the whole we need to work out the new UV coordinates for each tile
// each vertix needs a UV coordinate, so thats means each tile has 6 pairs of UV coordinates or a simpleVec2, so 16 tiles, s * 6 vertices

	simpleVec2 UVcoords[16 * 6];

// now the uv which will depend on the tile we want to draw, this is a bit more complex as the old UV coordinates depended on 1 set of data for each time, here the data is going to be different
// our tilemap has 2x8 tiles in each texture, so the step to the next textire is goig to be 1/8 for X and 1/2 for Y
	float stepX = 1.0f / 8.0f;
	float stepY = 1.0f / 2.0f;




// rework the old texture offsets using these new values
GLfloat QuadTexVals [] =  // these are the offset texture coordinates for a single cell, we need to add an offset to then to point to the correct cell for each tile.
	{
		stepX, 0.0f, 	// texture coordinate
		0.0f,0.0f,
		stepX,stepY,

		0.0f,0.0f,
		0.0f,stepY,
		stepX,stepY
	};

// now go through a loop of 2 rows, 8 colums, and 6 vertices (though they are paies so a counter of 12)
	int StoreIndex = 0;
	for (int Y = 0; Y < 2; Y++)
	{
		for (int X = 0; X < 8; X++)
		{
			for (int k = 0; k < 12; k += 2)  // increment in 2's
			{
				float NewU =  QuadTexVals[k] + (stepX*X);
				float NewV =  QuadTexVals[k+ 1]+ (stepY*Y);

				UVcoords[StoreIndex].xValue = NewU; // store then increment the index
				UVcoords[StoreIndex].yValue = NewV;
				StoreIndex++;
			} // end of k loop
		} // end of X loop

	} // end of Y loop

// now we have our UV coordinates stored ready to be used  any time that we want them

// lets make our playfiled the same way as last lesson but this time use a vector to store all the vertex offsets, and UV's.
// Start by scanning through the map, finding out which tile is to be drawn.
// Then work out the position of its 6 vectors and is UV coordinates
// store them all into a nice big array or vector (vector are better for this as they are more dynamic)
// Once we have a buffer transfer it to the GPU memory
// then when its time to write, use the GPU memory buffer
// All this setting up, takes a little bit of time, but its only ever done once...at a point where we're not watchng the screen
// which means the user will never see this slow setup, but will appreciate the faster update.

GLfloat QuadVerticesOnly[] =
{
	0.5f,0.5f,
	-0.5f,0.5f,
	0.5f,-0.5f,

	//2nd tri
	-0.5f,0.5f,
	-0.5f,-0.5f,
	0.5f,-0.5f

};


	for(int Row = 0 ; Row < 40 ; Row++) // this is an outer loop we will use the Row variable to go down the grid
	{
		for (int Column = 0; Column < 64; Column++) // this is an inner loop and will go along the grid
			{
				GLint WhatTile = Map2[Row][Column];  // what tile is it?
					// each tile has 6 vertices**, so we have to provide the data for the quad and the screen position for the shader to place it
					for(int i = 0 ; i < 6 ; i++ )	 // we
					{
						TileVectorData NewTile;  // create a new instance of TileVectorData

// now work out the  position and get the uv for this vertex
						NewTile.PositionX = QuadVerticesOnly[ i*2 ]; // times 2 because we are using pairs of floats
						NewTile.PositionY = QuadVerticesOnly[(i*2) + 1];
						NewTile.dummyZ = 0;   // This is "probably" already 0 but its always best to make sure, as there's no certainty that allocated memory is blank
// send the screen position for the centre of the quad, we are repeating ourseves by sending this 6 times but it allows faster access on the GPU
						NewTile.sPositionX = ((Column * 16) + 8);     // the tiles are 16 pixels apart in X and Y lets move them +8 because the reference is in the centre
						NewTile.sPositionY = (768 - ((Row * 16) + 8));


// next we need the UV coordinate we precalulated for this tile
						NewTile.u = UVcoords[(WhatTile * 6)+ i].xValue; // we don't need to double i, since we are using simpleVec2 which already pairs them
						NewTile.v = UVcoords[(WhatTile * 6)+ i].yValue;

						NewTiles.push_back(NewTile);    // place it in the vector
					} // end of for 1-6 vertice




			} // end of for column
	} // end of for row

// now at the end of all this we have a large vector called newTiles which contains ALL the vertex and UV positions for all the tiles, in one big chunk of memory
// we can choose to use this over and over again, which will be fast, but faster still would be to send this to a GPU buffer called a VBO and use that when we
// draw tiles, making the whole draw process simply a case of setting up a few uniforms and then doing a single draw call.

// setting up a VBO is 3 steps
// 1 ask the GPU to make one for us
	GLuint VBO;
	glGenBuffers(1, &VBO);
// 2 bind it
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
// 3 transfer the data to it
	glBufferData(GL_ARRAY_BUFFER, NewTiles.size()* 7 * sizeof(float),	&NewTiles.at(0), GL_STATIC_DRAW);

	this->OGLES.TileVBO = VBO; // save it for later use
// all done, it seems like a lot of work, but it will be good
// we will see the benefit of this when we do 1 draw call to draw all our objects...But...there is an even FASTER way to draw these tiles.. using indexing, but thats something you can research


#else	// but if we're not using the FastUpdate, we will do it the slow way..and compare
char* TileFiles[] =  // here's a good example of an array, being used to store text, which is actually stored somewhere else in memory so we point * at it!
	{
		(char*)"images/tile0.png",
		(char*)"images/tile1.png",
		(char*)"images/tile2.png",
		(char*)"images/tile3.png",
		(char*)"images/tile4.png",
		(char*)"images/tile5.png",
		(char*)"images/tile6.png",
		(char*)"images/tile7.png",
		(char*)"images/tile8.png",
		(char*)"images/tile9.png",
		(char*)"images/tile10.png",
		(char*)"images/tile11.png",
		(char*)"images/tile12.png",
		(char*)"images/tile13.png",
		(char*)"images/tile14.png",
		(char*)"images/tile15.png",
	};






	// lets turn these all these graphic files into textures
	for(int i = 0 ; i < 16; i++)
	{
		int Grwidth, Grheight, comp;
		unsigned char* data = stbi_load(TileFiles[i], &Grwidth, &Grheight, &comp, 4);    // ask it to load 4 componants since its rgba
		//now its loaded we have to create a texture, which will have a "handle" ID that can be stored, we have added a nice function to make this easy
		TileIDs[i] = this->OGLES.CreateTexture2D(Grwidth, Grwidth, (char*) data);     //just pass the width and height of the graphic, and where it is located and we can make a texture
		free(data); // when we load an image, it goes into memory, making it a texture copies it to GPU memory, but we need to remove the CPU memory before to load another or it causes a leak
	}




	// now make a 2Dgrid, this is just a small test one.




	std::vector<SimpleObj> MyTiles; // we can still create instances of objects in a vector

// lets make our playfiled the same way as last lesson but this time use a vector

	int		index = 0; // we need to keep an index counter
	for(int Row = 0 ; Row < 40 ; Row++) // this is an outer loop we will use the Row variable to go down the grid
	{
		for (int Column = 0; Column < 64; Column++) // this is an inner loop and will go along the grid
		{
			GLint WhatTile = Map2[Row][Column]; // what tile is it?
			if(WhatTile != 0) // llets not make 0 tiles
			{
				SimpleObj Tile;
				MyTiles.push_back(Tile); // place it in the vector
	// use the vector just like an array if you want, or continue to use the name Tile and add the features
				MyTiles[index].m_MyName = "I am a generic tile";  // we can still use a name if we want
				MyTiles[index].TextureID = TileIDs[WhatTile];
				MyTiles[index].Xpos = Column * 16 +8  ;  // the tiles are 16 pixels apart in X and Y lets move them +8 because the reference is in the centre
				MyTiles[index].Ypos = 768 - ((Row * 16) + 8 );  // <<woahhh.. whats this? See below
				MyTiles[index].ProgramObject = OGLES.programObject;
				// now its all set up, just increment the index
				index++;  // we will now set up the next MyTile
			}
		}
	}




	// if we want to position it somewhere we need to ask for the handle for the position uniform and other unfirms
	SimpleObj::positionUniformLoc = glGetUniformLocation(SimpleObj::ProgramObject, "u_position");    	 // << you can see we ask for the handle of the variable in our shader called u_position
	SimpleObj::samplerLoc = glGetUniformLocation(SimpleObj::ProgramObject, "s_texture");
	SimpleObj::ScreenCoordLoc = glGetUniformLocation(SimpleObj::ProgramObject, "u_Screensize");
	SimpleObj::ScaleLoc = glGetUniformLocation(SimpleObj::ProgramObject, "u_Scale");



#endif

/*what was this 768-((Row*16)+8)
 *
 *Its just becuase our Screen system is based on having 0,0 at the bottom left of the screen
 *but the grid is read from the top down, so if we didn't flip it like this we'd draw the grid upside down
 *We'll have to account for that whenever we use a grid.
 *the +8 is because our reference point for the tile is its centre, and we want its edge to move over half the tile width
 *
 **/



	struct timeval t1, t2;
	struct timezone tz;
	float deltatime;
	float totaltime = 0.0f;
	unsigned int frames = 0;
	gettimeofday(&t1, &tz);


/**************************************************************************/
// Here's where the min loop occurs
// now we will do a loop to run 10000 cyles to see the result this
	for (int i = 0; i < 10000; i++)
	{
		gettimeofday(&t2, &tz);
		deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 0.0000001f);
		t1 = t2;

		glClear(GL_COLOR_BUFFER_BIT); // clear the screen

		GLuint stride;	 // both fast and slow updates need this so define it here

// FastUpdate is defined ou rour OGL.h file, which is a file used by several other classes so its status will be known to them
#if (FastUpdate == TRUE)
// so if FastUpdate is set we will compile our new fast update code
	if(glGetError() != GL_NO_ERROR) printf("Clearing draw errors\n");
// we are using a different shader
		glUseProgram(OGLES.FastProgramObject);

		//  we could pre store these, there's no sense in asking the same question every cycle
		GLuint positionLoc = glGetAttribLocation(OGLES.FastProgramObject, "a_position");
		GLuint ScreenPositionLoc = glGetAttribLocation(OGLES.FastProgramObject, "u_position"); // remember strictly speaking we wouldn't call this a u_ for uniform but for clarity we will
		GLuint texCoordLoc = glGetAttribLocation(OGLES.FastProgramObject, "a_texCoord");

// let the shaders know what VBO and texture to use
		glBindBuffer(GL_ARRAY_BUFFER, OGLES.TileVBO);
		glBindTexture(GL_TEXTURE_2D, TileIDs[0]); // we kept the texture in tileID[0]

// set up uniforms as normal, though again these are always the same so this could also be done on an init

		GLuint samplerLoc, ScaleLoc, ScreenDataLoc;
		simpleVec2 ScreenData = { 512, 384 };  	// we only need half the screen size which is currently a fixed amount
		simpleVec2 Scale = { 16, 16 };


		samplerLoc = glGetUniformLocation(OGLES.FastProgramObject, "s_texture");
		ScreenDataLoc = glGetUniformLocation(OGLES.FastProgramObject, "u_Screensize");
		ScaleLoc = glGetUniformLocation(OGLES.FastProgramObject, "u_Scale");


	// now transfer our screen sizes and scale
		glUniform2fv(ScreenDataLoc, 1, &ScreenData.xValue);     // pass our half screen sizes by pointing to address of the 1st value in the struct
		glUniform2fv(ScaleLoc, 1, &Scale.xValue);
		glUniform1i(samplerLoc, 0);

			stride = 7*sizeof(float); // 3 position floats, 2 screen location floats, and 2 uv
// now tell the attributes where to find the vertices, positions and uv data

			glVertexAttribPointer(positionLoc,
			3,	// there are 3 values xyz
			GL_FLOAT, // they a float
			GL_FALSE, // don't need to be normalised
			stride,  // how many floats to the next one
			(const void*)0  // where do they start as an index); // use 3 values, but add stride each time to get to the next
			);

			glVertexAttribPointer(ScreenPositionLoc,
			2,		// there are 3 values xyz
			GL_FLOAT,	 // they a float
			GL_FALSE,	 // don't need to be normalised
			stride,		  // how many floats to the next one
			(const void*)(sizeof(float) * 3)    // where do they start as an index
			);

			// Load the texture coordinate
			glVertexAttribPointer(texCoordLoc,
			2, // there are 2 values u and c
			GL_FLOAT, //they are floats
			GL_FALSE, // we don't need them normalised
			stride,  // whats the stride to the next ones
			(const void*)(sizeof(float) * 5)
			);

		glEnableVertexAttribArray(positionLoc);
		glEnableVertexAttribArray(texCoordLoc);
		glEnableVertexAttribArray(ScreenPositionLoc);

		glDrawArrays(GL_TRIANGLES, 0,40*64*6);

		// now that we're back to working with the old style shader, set it all back as default
		glDisableVertexAttribArray(positionLoc);
		glDisableVertexAttribArray(texCoordLoc);
		glDisableVertexAttribArray(ScreenPositionLoc);


#else
		// and now lets print our tiles which should show our grid,

		for(int Tiles = 0 ; Tiles < MyTiles.size() ; Tiles++) // We may not know how many Tiles we made but the Size of the vector can be asked for
		{
			MyTiles[Tiles].Update(); // not only can we access the data of a class we can call its methods

		}

#endif // (FastUpdate == TRUE)


		glBindBuffer(GL_ARRAY_BUFFER, SimpleObj::VBO);    //now we mind that, which we can leave as bound since we use the same VBO
		glUseProgram(OGLES.programObject);
		SimpleObj::positionUniformLoc = glGetUniformLocation(OGLES.programObject, "u_position");      	 // << you can see we ask for the handle of the variable in our shader called u_position
		SimpleObj::samplerLoc = glGetUniformLocation(OGLES.programObject, "s_texture");
		SimpleObj::ScreenCoordLoc = glGetUniformLocation(OGLES.programObject, "u_Screensize");
		SimpleObj::ScaleLoc = glGetUniformLocation(OGLES.programObject, "u_Scale");


		stride = 5*sizeof(float);

		glVertexAttribPointer(SimpleObj::positionLoc,
			3,	// there are 3 values xyz
			GL_FLOAT, // they a float
			GL_FALSE, // don't need to be normalised
			stride,  // how many floats to the next one
			(const void*)0  // where do they start as an index); // use 3 values, but add stride each time to get to the next
			);

		glVertexAttribPointer(SimpleObj::texCoordLoc,
			2,		// there are 2 values xy
			GL_FLOAT,	 // they a float
			GL_FALSE,	 // don't need to be normalised
			stride,		  // how many floats to the next one
			(const void*)(sizeof(float) * 3)    // where do they start as an index
			);



// but we will still ask it to use the same position and textyre attributes locations
			glEnableVertexAttribArray(SimpleObj::positionLoc);      // when we enable it, the shader can now use it and it starts at its base value
			glEnableVertexAttribArray(SimpleObj::texCoordLoc);      // when we enable it, the shader can now use it


// we can go through a vector the same way we can an array.
		for (int i = 0; i < MyObjects.size(); i++)
		{
			bool IsItAlive =	MyObjects[i]->Update(this);
			MyObjects[i]->Draw();
			if (IsItAlive == false)
			{
				delete MyObjects[i]; // it was new'd into existance so we must delete it
				MyObjects.erase(MyObjects.begin() + i); // then remove it from the vector
			}

		}

// put our image on screen
		eglSwapBuffers(OGLES.state.display, OGLES.state.surface);

	//	let our user know how long that all took
		   totaltime += deltatime;
		frames++;
		if (totaltime >  1.0f)
		{
			printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames / totaltime);
			totaltime -= 1.0f;
			frames = 0;
		}

		}
	return; // we don't really need the return but it helps us to see the end of the method was reached and we can breakpoint it

	{
		// clean the memory out before we escape, we newed a lot of things, lets delete them

		for (int i = 0; i < MyObjects.size(); i++)
			delete MyObjects[i];  // delete the objects
		MyObjects.erase(MyObjects.begin(), MyObjects.end());  // clear the vector


	}


};

