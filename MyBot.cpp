#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <cmath>
#include <map>

#include "hlt.hpp"
#include "networking.hpp"

float compute_force (hlt::Site site, float dist, int myArea)
{
	float force;
	float production = (float) site.production;
	float strength = (float) site.strength;
	if (myArea > 16)
		force = production / (dist * dist);
	else
		force = (1.0f / strength) / (dist * dist);
	return force;
}

int main ()
{
	std::ofstream fout;	fout.open ("fout.log");
	srand (time (NULL));
	std::cout.sync_with_stdio (0);

	unsigned char myID;
	hlt::GameMap currMap;
	std::set<hlt::Move> moveList;
	getInit (myID, currMap);
	sendInit ("Since 08/17/1926");

	int clock;
	for (clock = 0; ; clock ++)
	{
		moveList.clear ();
		getFrame (currMap);

		std::set<hlt::Location> boarders;
		std::map<hlt::Move, bool> moveMap;
		int myArea = 0;
		for (unsigned short a = 0; a < currMap.height; a ++)
		{
			for (unsigned short b = 0; b < currMap.width; b ++)
			{
				if (currMap.getSite ({b, a}, STILL).owner == myID)
					myArea ++;
				else
				{
					for (unsigned char i = 1; i < 5; i ++)
						if (currMap.getSite ({b, a}, i).owner == myID)
						{
							boarders.insert ({b, a});
							break;
						}
				}
			}
		}
		for (unsigned short a = 0; a < currMap.height; a++)
		{
			for (unsigned short b = 0; b < currMap.width; b++)
			{
				if (currMap.getSite ({b, a}).owner == myID)
				{
					float force_x = 0.0, force_y = 0.0;
					for (std::set<hlt::Location>::iterator it = boarders.begin (); it != boarders.end (); it ++)
					{
						float  dist = currMap.getDistance ({b, a}, *it);
						float angle = currMap.getAngle    ({b, a}, *it);
						float force = compute_force (currMap.getSite (*it, STILL), dist, myArea);
						force_x += force * cos (angle);
						force_y += force * sin (angle);
					}
					unsigned char direction = STILL;
					if (force_x + force_y > 0 && force_x - force_y > 0) direction = EAST;
					if (force_x + force_y > 0 && force_x - force_y < 0) direction = SOUTH;
					if (force_x + force_y < 0 && force_x - force_y < 0) direction = WEST;
					if (force_x + force_y < 0 && force_x - force_y > 0) direction = NORTH;
					hlt::Site currSite = currMap.getSite ({b, a}, STILL);
					hlt::Site nextSite = currMap.getSite ({b, a}, direction);
					if (myArea <= 16 && currSite.strength <= 8 || myArea > 16 && currSite.strength <= 16) continue; // let small pieces be resting ans growing
					if (nextSite.owner != myID && currSite.strength <= nextSite.strength)
					{
						// change direction to merge and attack
							 if (currMap.getSite ({b, a}, (direction + 0) & 0x3 + 1).owner == myID) direction = (direction + 0) & 0x3 + 1;
						else if (currMap.getSite ({b, a}, (direction + 2) & 0x3 + 1).owner == myID) direction = (direction + 2) & 0x3 + 1;
						else continue; // avoid death move
					}
					hlt::Move revMove = {currMap.getLocation ({b, a}, direction), (unsigned char) ((direction + 1) & 0x3 + 1)};
					if (myArea <= 16 && moveMap[revMove] == 1) continue;
					hlt::Move move = {{b, a}, direction};
					moveList.insert (move);
					moveMap[move] = 1;
				}
			}
		}

		sendFrame (moveList);
	}

	return 0;
}

