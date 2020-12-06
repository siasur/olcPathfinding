#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Pathfinding.cpp"

int main()
{
	Pathfinding demo;
	if (demo.Construct(512, 300, 2, 2))
		demo.Start();
	return 0;
}