//----------------------------------------------------------
// Evokilo1 - First experiment in evolved kilobot controller
// (c) Simon Jones 2015
//----------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <atomic>
#include <algorithm>
#include <iterator>
#include <random>

//#include "stage.hh"
//using namespace Stg;

#include "kilolib.h"
#include "evokilo1.h"

using namespace Kilolib;

/*void Kilobee::setup()
{
    // put your setup code here, will be run once at the beginning
    kilo_message_tx = (message_tx_t)&Kilobee::tx_message;
    kilo_message_rx = (message_rx_t)&Kilobee::rx_message;

	// Construct a valid message
	msg.type    = NORMAL;
	msg.crc     = message_crc(&msg);
	*msg.data 	= (long) 0;

	messageCount = 0;

    opinionIndex = rand_soft() % (sizeof(legalOpinions) / sizeof(legalOpinions[1]));
    for (int i = 0; i < LANG_SIZE; i++)
    {
        opinion[i] = legalOpinions[opinionIndex][i];
    }

    if (opinionIndex == -1)
    {
        setNestSite(-1, 0);
        setDanceState(0, 0);
        // Set dance state in the msg
        msg.data[0] = (uint8_t) 0;
        // Set the opinion index
        msg.data[1] = (uint8_t) -1;
    }
    else
    {
        int siteToVisit = getSiteToVisit(opinion);
        setNestSite(siteToVisit, nestQualities[siteToVisit]);
        setDanceState(1, nestQualities[siteToVisit]);
        // Set dance state in the msg
        msg.data[0] = (uint8_t) 1;
        // Set the opinion index
        msg.data[1] = (uint8_t) opinionIndex;
    }
	msg.crc = message_crc(&msg);
}

void Kilobee::loop()
{
    // put your main code here, will be run repeatedly

    if (kilo_ticks > last_update + 16)
    {
    	if (last_update == 0)
    		probNotDancing = rand_soft() % 2;
        last_update = kilo_ticks;

        // Random movement
        // switch(rand_soft() % 4)
        // {
        //     case(0):
        //         set_motors(0,0);
        //         break;
        //     case(1):
        //         //if (last_output == 0) spinup_motors();
        //         set_motors(kilo_turn_left,0);
        //         break;
        //     case(2):
        //         //if (last_output == 0) spinup_motors();
        //         set_motors(0,kilo_turn_right);
        //         break;
        //     case(3):
        //         //if (last_output == 0) spinup_motors();
        //         set_motors(kilo_straight_left, kilo_straight_right);
        //         break;
        // }


        // Find opinion index
        opinionIndex = getOpinionIndex(opinion);
        //std::cout << opinionIndex << std::endl;
        //std::cout << (int) nest.site << std::endl;

        // Set the opinion index
        msg.data[1] = (uint8_t) opinionIndex;

        if (danceState.state == 1)
        {
            // setNestSite(-1, 0);
            // setDanceState(0, 0);
            // Set dance state in msg to NOT_DANCING
            msg.data[0] = (uint8_t) 1;
        }
        else
        {
            // int siteToVisit = getSiteToVisit(opinion);
            // setNestSite(siteToVisit, nestQualities[siteToVisit]);
            // setDanceState(1, nestQualities[siteToVisit]);
            // Set dance state in msg to DANCING
            msg.data[0] = (uint8_t) 0;
        }
		msg.crc = message_crc(&msg);

		// std::cout << "+Messages received:" << loopCounter << ":" << (int) danceState.state << ":" << messageCount << std::endl;
		// std::cout << "+Opinion index:" << loopCounter << ":" << (int) danceState.state << ":" << (int) opinionIndex << std::endl;
		// std::cout << "+Nest site:" << loopCounter << ":" << (int) danceState.state << ":" << (int) nest.site << std::endl;

		std::cout << "+:" << loopCounter << ":" << (int) danceState.state << ":" << messageCount << ":" << (int) opinionIndex << ":" << (int) nest.site << std::endl;

        if (danceState.state == 1)
        {
            switch ((uint8_t) opinionIndex)
            {
                case 0:
                    set_color(RGB(3, 0, 0));
                    break;
                case 1:
                    set_color(RGB(0, 0, 3));
                    break;
                case 2:
                    set_color(RGB(0, 3, 0));
                    break;
                case 3:
                    set_color(RGB(3, 0, 3));
                    break;
                case 4:
                    set_color(RGB(3, 3, 0));
                    break;
                case 5:
                    set_color(RGB(0, 3, 3));
                    break;
                case 6:
                    set_color(RGB(3, 3, 3));
                    break;
                case (uint8_t) -1:
                    set_color(RGB(0, 0, 0));
                    break;
            }
        }
        else
        {
            // Set colour to black for not dancing
            set_color(RGB(0, 0, 0));
        }

        if (danceState.state == 0)
        {
            //std::cout << "Bee is not dancing..." << std::endl;
            double siteNoise = 0.0;
            if (messageCount > 0)
            {
                //std::cout << "Messages received..." << std::endl;

                int dancingBeeCount = 0;
                for (int i = 0; i < messageCount; i++)
                {
                    // If bee's dance state is true
                    if (messages[i][0] == 1)
                    {
                        dancingBeeCount++;
                    }
                }

                if (dancingBeeCount > 0)
                {
               		//std::cout << "Bees dancing..." << std::endl;
                    int *dancingBees = (int *) malloc(sizeof(int) * dancingBeeCount);
                    int dbIndex = 0;
                    for (int i = 0; i < messageCount; i++)
                    {
                        // If bee's dance state is true
                        if (messages[i][0] == 1)
                        {
                            // Set the dancing bee to its opinion index
                            dancingBees[dbIndex] = messages[i][1];
                            dbIndex++;
                        }
                    }

                    int otherOpinion = dancingBees[rand_soft() % dancingBeeCount];
                    int newOpinion[LANG_SIZE];
                    //std::cout << "Consensus forming..." << std::endl;
                    consensus(opinion, legalOpinions[otherOpinion], newOpinion);

                    #ifndef BOOLEAN
                    for (int i = 0; i < LANG_SIZE; i++)
                    {
                        opinion[i] = newOpinion[i];
                    }

                    #else
                    double opinionToAdopt = (double) rand_soft() / 255;

                    int newOpinionIndex = (opinionToAdopt <= BOOLEAN) ? otherOpinion : opinionIndex;
                    for (int i = 0; i < LANG_SIZE; i++)
                    {
                        opinion[i] = legalOpinions[newOpinionIndex][i];
                    }
                    #endif

                    opinionIndex = getOpinionIndex(opinion);
                    if (opinionIndex != -1)
                    {
                    	int siteToVisit = getSiteToVisit(opinion);
			        	setNestSite(siteToVisit, nestQualities[siteToVisit]);

			        	double noiseVal = get_noise();
			        	siteNoise = round(noiseVal);
			        	//std::cout << "noise value: " << noiseVal << std::endl;
			        	//std::cout << "site noise: " << siteNoise << std::endl;
			        	std::cout << "site quality: " << nestQualities[siteToVisit] + siteNoise << std::endl;
			       		setDanceState(1, nestQualities[siteToVisit] + siteNoise);
                    }
                    else
                    {
                    	setNestSite(-1, 0);
                    }

                    free(dancingBees);
                }
            }
            else
            {
	            probNotDancing = rand_soft() % 2;
		        if (probNotDancing == 0 && nestQualities[nest.site] > 0 && nest.site != (uint8_t) -1)
		        {
		            setDanceState(1, nestQualities[nest.site] + siteNoise);
		        }
		        else
		        {
		        	setDanceState(0, 0);
		        }
            }
        }
        else
        {
        	// Bee is dancing, so  decrement dance duration.
			setDanceState(1, danceState.duration - 1);
			//std::cout << "Dance State: " << danceState.state << " : " << (int) danceState.duration << std::endl;
			if (danceState.duration < 1)
			{
				// Bee no longer dances
				setDanceState(0, 0);
				set_color(RGB(0, 0, 0));
			}
        }

        messageCount = 0;
		loopCounter++;
    }

}*/


void Kilobee::setup()
{
    // Setup code here; run once at the beginning
    kilo_message_tx = (message_tx_t)&Kilobee::tx_message;
    kilo_message_rx = (message_rx_t)&Kilobee::rx_message;

    // Construct a valid message
    // Format will be:
    // [0] - first 8-bits of ID.
    // [1] - final 8-bits of ID.
    // [2] - dance state; 1 (true) if bee is dancing, 0 (false) if not.
    // [3] - dance site; the site that is currently being danced for.
    // [4] - belief[0]; site value "x" (for 0:x, 1: 1 - x).
    // [5] - belief[1]; optional for 3-sites (language size: 3).
    // [6]
    // [7]
    // [8]
    // [9]
    // msg.type = NORMAL;
    // msg.crc = message_crc(&msg);

    while (1)
    {
        for (int b = 0; b < LANG_SIZE - 1; b++)
        {
            beliefs[b] = rand_hard() / 255;
        }

        uint8_t exitScope = 1;
        double prevBelief = beliefs[0];

        for (int b = 1; b < LANG_SIZE - 1; b++)
        {
           if (prevBelief <= beliefs[b])
           {
                exitScope = 0;
                break;
           }
        }

        if (exitScope == 1)
            break;
    }

    uint8_t siteToVisit = getSiteToVisit(beliefs);
    setNestSite(siteToVisit, nestQualities[siteToVisit]);
    setDanceState(1, nestQualities[siteToVisit]);

    double probNotDancing = rand_hard() / 255;
    if (probNotDancing <= 0.5 && nestQualities[siteToVisit] > 0)
    {
        setDanceState(1, nestQualities[siteToVisit]);
    }
    else
    {
        setDanceState(0, 0);
    }

    // Generate random integers to fill both ID bytes, leading to a 16-bit
    // number with 65,536 possible values: 255 (8-bit) x 255 (8-bit).
    msg.data[0] = rand_hard();
    msg.data[1] = rand_hard();
    // Dance state
    msg.data[3] = danceState.state;
    msg.data[4] = nest.site;

    msg.type = NORMAL;
    msg.crc = message_crc(&msg);

    if (danceState.state == 1)
    {
        set_bot_colour(nest.site);
    }
    else
    {
        // Set colour to black; not dancing
        set_color(RGB(0, 0, 0));
    }
}

void Kilobee::loop()
{
    if (kilo_ticks > lastUpdate + updateTicks)
    {
        lastUpdate = kilo_ticks;

        // Random movement
        /*switch(rand_hard() % 4)
        {
            case(0):
                set_motors(0,0);
                break;
            case(1):
                //if (last_output == 0) spinup_motors();
                set_motors(kilo_turn_left,0); // 70
                break;
            case(2):
                //if (last_output == 0) spinup_motors();
                set_motors(0,kilo_turn_right); // 70
                break;
            case(3):
                //if (last_output == 0) spinup_motors();
                set_motors(kilo_straight_left, kilo_straight_right); // 65
                break;
        }*/


        if (danceState.state == 0)
        {
            if (messageCount > 0)
            {
                int dancingBeeCount = 0;
                for (int i = 0; i < messageCount; i++)
                {
                   // If bee's dance state is true
                   if (messages[i][0] == 1)
                   {
                        dancingBeeCount++;
                   }
                }

                if (dancingBeeCount > 0)
                {
                    double *dancingBees = (double *) malloc(sizeof(double) * (dancingBeeCount * LANG_SIZE - 1));
                    int dbIndex = 0;
                    for (int i = 0; i < messageCount; i++)
                    {
                        // If bee's dance state is true
                        if (messages[i][0] == 1)
                        {
                            // Set the dancing bee to its opinion index
                            dancingBees[dbIndex] = messages[i][1];
                            dbIndex += LANG_SIZE - 1;
                        }
                    }

                    double *otherBeliefs = &dancingBees[(rand_hard() % dancingBeeCount) * (LANG_SIZE - 1)];
                    double newBeliefs[LANG_SIZE - 1];

                    consensus(beliefs, otherBeliefs, newBeliefs);

                    for (int i = 0; i < LANG_SIZE; i++)
                    {
                        beliefs[i] = newBeliefs[i];
                    }

                    uint8_t siteToVisit = getSiteToVisit(beliefs);
                    setNestSite(siteToVisit, nestQualities[siteToVisit]);
                    setDanceState(1, nestQualities[siteToVisit]);

                    free(dancingBees);
                }
            }

            double probNotDancing = rand_hard() / 255;
            if (probNotDancing <= 0.5 && nestQualities[nest.site] > 0)
            {
                setDanceState(1, nestQualities[nest.site]);
            }
            else
            {
                setDanceState(0, 0);
            }

            if (danceState.state == 1)
            {
                set_bot_colour(nest.site);
            }
            else
            {
                // Set colour to black; not dancing
                set_color(RGB(0, 0, 0));
            }
        }

        else
        {
            // Bee is dancing, so decrement dance duration.
            setDanceState(1, danceState.duration - 1);

            if (danceState.duration < 1)
            {
                // Bee no longer dances
                setDanceState(0, 0);
                set_color(RGB(0, 0, 0));
            }
        }

        messageCount = 0;
    }

}
