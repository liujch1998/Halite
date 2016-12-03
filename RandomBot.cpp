#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <cmath>

#include "hlt.hpp"
#include "networking.hpp"

int main ()
{
	std::ofstream log;
	log.open ("log.txt");

    srand(time(NULL));
    std::cout.sync_with_stdio(0);
    std::set<hlt::Move> moves; // output: moves

    unsigned char myID;
    hlt::GameMap presentMap; // input: presentMap
    getInit(myID, presentMap);
    sendInit("Since 08/17/1926");

	std::set<hlt::Location> boarders;
    while(true)
	{
        moves.clear();
        getFrame(presentMap);

		boarders.clear ();
		for (unsigned short a = 0; a < presentMap.height; a ++)
		{
			for (unsigned short b = 0; b < presentMap.width; b ++)
			{
				if (presentMap.getSite ({b, a}, STILL).owner == myID) continue;
				bool isBoarder = 0;
				for (unsigned char i = 1; i < 5; i ++)
					if (presentMap.getSite ({b, a}, i).owner == myID)
					{ isBoarder = 1; break; }
				if (isBoarder)
					boarders.insert ({b, a});
			}
		}
		for (unsigned short a = 0; a < presentMap.height; a++)
		{
			for (unsigned short b = 0; b < presentMap.width; b++)
			{
				if (presentMap.getSite ({b, a}).owner == myID)
				{
					float force_x = 0.0, force_y = 0.0;
					for (std::set<hlt::Location>::iterator it = boarders.begin (); it != boarders.end (); it ++)
					{
						float  dist = presentMap.getDistance ({b, a}, *it);
					//	log << "dist is " << dist << std::endl;
						float angle = presentMap.getAngle    ({b, a}, *it);
					//	float force = /*(float) presentMap.getSite (*it, STILL).production*/ 1.0 /*/ (float) presentMap.getSite (*it, STILL).production*/ / (dist * dist);
					//	float force = /*(float) presentMap.getSite (*it, STILL).production*/ 1.0 / (float) presentMap.getSite (*it, STILL).production / (dist * dist);
						float force = (float) presentMap.getSite (*it, STILL).production /* (float) presentMap.getSite (*it, STILL).production*/ / (dist * dist);
						force_x += force * cos (angle);
						force_y += force * sin (angle);
					}
				//	log << force_x << ' ' << force_y << std::endl;
					if (presentMap.getSite ({b, a}, STILL).strength < 16) continue;
					unsigned char direction = STILL;
					if (force_x + force_y > 0 && force_x - force_y > 0) direction = EAST;
					if (force_x + force_y > 0 && force_x - force_y < 0) direction = SOUTH;
					if (force_x + force_y < 0 && force_x - force_y < 0) direction = WEST;
					if (force_x + force_y < 0 && force_x - force_y > 0) direction = NORTH;
					if (presentMap.getSite ({b, a}, direction).owner != myID && presentMap.getSite ({b, a}, direction).strength >= presentMap.getSite ({b, a}).strength) continue;

					moves.insert ({{b, a}, direction});
				}
			}
		}

        sendFrame(moves);
    }

    return 0;
}
