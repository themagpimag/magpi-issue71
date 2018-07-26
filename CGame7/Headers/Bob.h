#include "SimpleObj.h"
#include "Input.h"
#include "Game.h"
class Bob : public SimpleObj
{
public:
	Bob();
	~Bob();
	
// one noticiable differnce is I am passing a pointer to Game, because Game has useful data that my update might need	
// I could, (and should) only pass the data I want, but its just easier like this for now.	
	bool Update(Game* G);
	
	int	BaseAnim;
	int	AnimIndex;
	
	
private:	
	// no other classes need to have access to these;	
	bool Climbing;
	bool TestClimb(Game*);
	bool TestClimbDown(Game*);
	
	void BobsLogic();
	void Draw();
	int CurrentState;
	int TestYPos;  //
	GLuint Images[12]; // handles for the 12 images we convert to texture (hmmm)
	Input* IH;
};