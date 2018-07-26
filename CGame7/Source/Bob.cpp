#include "Bob.h"


#define SPEED 1.0f

Bob::Bob()
{
	// we can create with these images but lets store the full set

	char* ImageNames[] = {
		(char*)"Assets/brianWalkR_1.png",
		(char*)"Assets/brianWalkR_2.png",
		(char*)"Assets/brianWalkR_3.png",
		(char*)"Assets/brianWalkR_4.png",
		(char*)"Assets/brianWalkL_1.png",
		(char*)"Assets/brianWalkL_2.png",
		(char*)"Assets/brianWalkL_3.png",
		(char*)"Assets/brianWalkL_4.png",
		(char*)"Assets/brianclimb_1.png",
		(char*)"Assets/brianclimb_2.png",
		(char*)"Assets/brianclimb_3.png",
		(char*)"Assets/brianclimb_4.png",
	};

		// lets turn these all these graphic files into textures, but consider, is it good to have them all as seperate textures?
	for(int i = 0 ; i < 12 ; i++)
	{
		int Grwidth, Grheight, comp;
		unsigned char* data = stbi_load(ImageNames[i], &Grwidth, &Grheight, &comp, 4);      // ask it to load 4 componants since its rgba
		//now its loaded we have to create a texture, which will have a "handle" ID that can be stored, we have added a nice function to make this easy
		// be careful to make sure you load the graphics correctly?

		Images[i] = SimpleObj::TheOGLInstance->CreateTexture2D(Grwidth, Grheight, (char*) data);      //just pass the width and height of the graphic, and where it is located and we can make a texture
		free(data);  // when we load an image, it goes into memory, making it a texture copies it to GPU memory, but we need to remove the CPU memory before to load another or it causes a leak
	}

	BaseAnim = AnimIndex = 0;
	// bob needs an input handler, we defined one in the Bob.h file but its currently an empty pointer
		IH = new Input();
	IH->Init();
	Xpos = 260-(9*8);
	Ypos = 160;
	ScaleX = 32.0;  // this tells us how many pixels to make our texture square, sprite are 32x32
	ScaleY = 32.0;
}

Bob::~Bob()
{
	IH->kQuit = true; // lets stop the keyboard thread
	delete IH;  // and delete theis before we exit
}


// By returning bool as false we indicate to the game the character is dead so make sure bob always returns true
bool Bob::Update(Game* G)
{
	// remember our Ypos needs to be flipped to be able to look through an array since our screen starts at 0,0 in the bottom left, and the array
	// has 0,0 at the top left, since we need it a few times, we created a private variable in the bob class to do this
	BaseAnim = 0;
	TextureID = Images[0];





	if (Climbing == false)
	{
		Yspeed += SPEED;
		if (Yspeed > 9.81f) Yspeed = 9.81f;
		Ypos += Yspeed;
	}
	else
	{
		Ypos += Yspeed;
		Yspeed = 0;
	}
	int YMap = (Ypos + 16) / 16;
	int XMap = (Xpos) / 16;

	int WhatsUnderOurFeet = G->Map2[YMap][XMap];
	int Attrib = G->Attributes[WhatsUnderOurFeet];

	if ((Attrib & SOLID) && Yspeed >= 0 && Climbing == false)
	{
		Ypos = YMap * 16 - 16;
		Yspeed = 0;
	}

		if (IH->TestKey(KEY_RIGHT))
		{
			Climbing = false;
			BaseAnim = 0;
			TextureID = Images[BaseAnim + AnimIndex];
			(AnimIndex += 1) &= 3;

			this->Xpos += SPEED*2;
		}

		if (IH->TestKey(KEY_LEFT))
		{
			Climbing = false;
			BaseAnim = 4;
			TextureID = Images[BaseAnim + AnimIndex];
			(AnimIndex += 1) &= 3;

			this->Xpos -= SPEED*2;
		}

	if (IH->TestKey(KEY_UP))
	{
			// this is the climb
		if(Climbing)
		{
			Yspeed = -SPEED ;
			BaseAnim = 8;
			TextureID = Images[BaseAnim + AnimIndex];
			(AnimIndex += 1) &= 3;
			// we need to test if the climb is over?
			if((Attrib & LADDER) == 0)
					{
						Climbing = false;
					}

		}
		else
		{
			 // now we check if we are on a ladder
			if(TestClimb(G) == true)
			{
				Climbing = true;
			}
			if ((Attrib & SOLID)   && (Yspeed >= 0))
				Yspeed = -SPEED * 8;
		}
	}


	if (IH->TestKey(KEY_DOWN))
	{

		if (TestClimbDown(G))	Climbing = true;


		// check if we are climbing and if so, make sure we use the climbing animations
		// but also if we are currently climbing, make sure we are still on a ladder, and if, not stop climbing and revert to normal walk mode.
			if(Climbing == true)
		{
			int	WhatsUnderOurFeetplus = G->Map2[YMap][XMap];
			int Attrib2 = G->Attributes[WhatsUnderOurFeetplus];

			BaseAnim = 8;
			TextureID= Images[BaseAnim + AnimIndex];
			(AnimIndex -= 1) &= 3;
			this->Ypos += SPEED;
			if ((Attrib2 & LADDER) && (Attrib & LADDER))
			{
				Climbing = false;
				Yspeed = SPEED;
			}
		}
		else
		{
			// so this is I am not climibng, but I am still in the I pressed down condition
		 // maybe we can add a toe test graphic?
		}

	}

	return true;
}



bool Bob::TestClimb(Game* G)
{
	int YMap = (Ypos + 16) / 16;
	int XMap = (Xpos - 2) / 16;

	int	WhatsUnderOurFeet = G->Map2[YMap][XMap];
	int Attrib = G->Attributes[WhatsUnderOurFeet];

	if ((Attrib & LADDER) == 0) return false; // test for ladder

	YMap = (Ypos + 16) / 16;
	XMap = (Xpos + 2) / 16;

	WhatsUnderOurFeet =  G->Map2[YMap][XMap];
	Attrib = G->Attributes[WhatsUnderOurFeet];
	return (Attrib & LADDER) == LADDER;
}


bool Bob::TestClimbDown(Game* G)
{
	int YMap = (Ypos + 32 ) / 16;
	int XMap = (Xpos - 2) / 16;

	int	WhatsUnderOurFeet = G->Map2[YMap][XMap];
	int Attrib = G->Attributes[WhatsUnderOurFeet];

	if ((Attrib & LADDER) == 0) return false; // test for ladder

	YMap = (Ypos + 32 ) / 16;
	XMap = (Xpos + 2) / 16;

	WhatsUnderOurFeet =  G->Map2[YMap][XMap];
	Attrib = G->Attributes[WhatsUnderOurFeet];
	return (Attrib & LADDER) == LADDER;
}

typedef enum {
	STANDING = 0,
	WALKING,
	JUMPING,
	CLIMBING,
	FALLING,
	DROWNING
} StateVaues;


void Bob::BobsLogic()
{

	switch (CurrentState) // the Switch asks what is our current state represented as an enumerated value or int
	{
	case	STANDING:
		//Do code for a standing character
		break; // this ends the code
	case	WALKING:
		break; // notice we must end each section with break...if we don't it falls through
	case	JUMPING:
		//Do code for a jumping character
		break;  // this ends the code
	case	CLIMBING:
		//Do code for a climbing character
		break;  // this ends the code
	case	FALLING :
		//Do code for a falling character
		break;  // this ends the code

	case	DROWNING :
		//Do code for a drowingcharacter
		break;  // this ends the code

	default : // this is a special case in case we have not found code for a particular state value
		//do default code, which usually means we have forgotton something
		break;
	} // close the switch


}

// Bob and other sprites are the only ones now who are really doing thier own draw, for now its a copy
// of the older tile system, but as we optimise the code we are probably going to find this is too slow for our needs.
// But don't be in a rush to make changes, we have more important update and logic code to get working before we start to worry about
// the efficiency of this code.
void Bob::Draw()
{

		/**************************************************************************************
		 This is still the same hacky code we use to draw tiles but this time,we will take
		 responsibility for the flip on the y axis, and maybe later we can do a few other things
		***************************************************************************************/

	// by flipping the y axis here, we can now make sure our Xpos and Ypos relate to Map locations
		simpleVec2 testingVec = { Xpos, 768-Ypos };
		simpleVec2 ScreenData = { 512, 384 };  	// we only need half the screen size which is currently a fixed amount
		simpleVec2 Scale = { ScaleX, ScaleX };
			glUseProgram(this->TheOGLInstance->programObject);

// this code is all explained in the SimpleObh version
			glUniform2fv(positionUniformLoc, 1, &testingVec.xValue);
			glUniform2fv(ScreenCoordLoc, 1, &ScreenData.xValue);
			glUniform2fv(ScaleLoc, 1, &Scale.xValue);

	// this in particular is going to change
		glBindTexture(GL_TEXTURE_2D, TextureID);

			// now its been set up, tell it to draw 6 vertices which make up a square
			glDrawArrays(GL_TRIANGLES, 0, 6);




}





//	if (Attrib && SOLID )
//	{
//		// if there is something solid we can walk
//
//			if(IH->TestKey(KEY_RIGHT))
//		{
//			Climbing = false; // he can't climb if moving left or right, this will let him fall of ladders
//			BaseAnim = 0;
//			this->TextureID = Images[BaseAnim + AnimIndex];
//			(AnimIndex += 1) &= 3;
//
//			this->Xpos += SPEED * 2;
//		}
//		if (IH->TestKey(KEY_LEFT))
//		{
//			Climbing = false;
//			BaseAnim = 4;
//			this->TextureID = Images[BaseAnim + AnimIndex];
//			(AnimIndex += 1) &= 3;
//			this->Xpos -= SPEED * 2;
//		}
//
//	}
//	else
//	{ // so if there's nothing under his feet he needs to fall
//		Yspeed += 1.0f;
//		if (Yspeed >= 9.81f) Yspeed = 9.81f; // put a finite limit on gravit
//		Ypos += Yspeed;
//		// we estabished he had to fall, but how much he falls depends on how long he's been falling
//		// so he could be going into our block, so we need a small check to see if we would keep faling
//		// if we stopped falling we need to push him back a little bit to be ontop of the tile that stopped
//		// him not inside it, so do the test in the new ypos location (Xmap is unchanged)
//				YMap = (Ypos + 17)  / 16;
//		WhatsUnderOurFeet = G->Map2[YMap][XMap];
//		Attrib = G->Attributes[WhatsUnderOurFeet];
//		// ok so we know that we are in the tile, lets work out what the position he should be at.
//		// we know his feet sit on the tile at YMap, and we can get a pixel postion by multiplying that by
//		// the height of the tile (16), then subtract the dept test of 17 to put him at his point above the tile
//				if (Attrib && SOLID)
//		{ // so he's in the solid tile to some degree, so he will stop but not always be on top of the tile, we have to do a small calculation to fix that
//			Ypos = (YMap*16)-17 ; // resposition him after landing
//		}
//
//	}
//
//
//
//	// we can return, we set true for now, but later we might use return false to indicate we died or something else our game needs.
//		return true;
//
//}
//
//bool Bob::TestClimb(Game* G)
//{
//	int YMap = (Ypos + 33) / 16;
//	int XMap = (Xpos + (32 / 2) - 5) / 16;
//
//	int	WhatsUnderOurFeet = G->Map2[YMap][XMap];
//	int Attrib = G->Attributes[WhatsUnderOurFeet];
//
//	if ((Attrib & LADDER) == 0) return false; // test for ladder
//
//	YMap = (Ypos + 33) / 16;
//	XMap = (Xpos + (32/ 2) + 5) / 16;
//
//	WhatsUnderOurFeet = G->Map2[YMap][ XMap];
//	Attrib = G->Attributes[WhatsUnderOurFeet];
//	return (Attrib & LADDER) == LADDER;
//}
//
//
//bool Bob::TestClimbDown(Game* G)
//{
//	int YMap = (Ypos + 33 + 16) / 16;
//	int XMap = (Xpos + (32 / 2) - 5) / 16;
//
//	int	WhatsUnderOurFeet = G->Map2[YMap][XMap];
//	int Attrib = G->Attributes[WhatsUnderOurFeet];
//
//	if ((Attrib & LADDER) == 0) return false; // test for ladder
//
//	YMap = (Ypos + 33 + 16) / 16;
//	XMap = (Xpos + (32 / 2) + 5) / 16;
//
//	WhatsUnderOurFeet = G->Map2[YMap][XMap];
//	Attrib = G->Attributes[WhatsUnderOurFeet];
//	return (Attrib & LADDER) == LADDER;
//}
