//----------------------------------------------------------
// Evokilo1 - First experiment in evolved kilobot controller
// (c) Michael Crosscombe 2016
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

    /*
     * MALICIOUS AGENT DETERMINATION
     */

    if ((rand_soft() / 255.0) < 0.0)
    {
        std::cout << "Malicious" << std::endl;
        isMalicious = 1;
    }

    ////////////////////////////////

    int zeroCount = SITE_NUM;
    while (zeroCount == SITE_NUM)
    {
        zeroCount = 0;
        for (int b = 0; b < SITE_NUM; b++)
        {
            uint8_t truthValue = (rand_soft() % 2);
            if (truthValue == 1)
                truthValue = 2;

            beliefs[b] = truthValue;

            if (beliefs[b] == 0)
            {
                zeroCount++;
            }
        }
    }
    //b std::cout << (int) beliefs[0] << ";" << (int) beliefs[1] << std::endl;
    formConsistentBeliefs(beliefs);
    // std::cout << (int) beliefs[0] << ";" << (int) beliefs[1] << std::endl;
    // if (beliefs[0] == 2 && beliefs[1] == 2)
    // {
    //     std::cout << "NOT SUCCESSFUL" << std::endl;
    // }

    uint8_t siteToVisit = getSiteToVisit(beliefs);

    if (siteToVisit == (uint8_t) -1)
    {
        setDanceState(0, 0);
    }
    else
    {
        setNestSite(siteToVisit, nestQualities[siteToVisit]);
        int danceDuration = nestQualities[siteToVisit] + round(get_noise());
        if (danceDuration <= 0)
        {
            danceDuration = 1;
        }
        setDanceState(1, danceDuration);
    }

    double probNotDancing = rand_soft() / 255.0;
    if (probNotDancing <= 0.5)
    {
        setDanceState(0, 0);
    }

    // Generate random integers to fill both ID bytes, leading to a 16-bit
    // number with 65,536 possible values: 255 (8-bit) x 255 (8-bit).
    //msg.data[0] = rand_soft();
    //msg.data[1] = rand_soft();
    // Dance state
    msg.data[0] = danceState.state;
    msg.data[1] = nest.site;
    // Beliefs
    for (int b = 0; b < SITE_NUM; b++)
    {
        msg.data[beliefStart + b] = beliefs[b];
    }

    msg.type = NORMAL;
    msg.crc = message_crc(&msg);

    if (isMalicious == 1)
    {
        set_color(RGB(3, 3, 3));
    }
    else if (danceState.state == 1)
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

        // if (loopCounter == 10)
        // {
        //     nestQualities[1] = 1;
        // }

        // Random movement
        /*switch(rand_soft() % 4)
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

        if (isMalicious == 0)
        {
            std::cout << "+:" << (int) loopCounter << ":" << (int) danceState.state << ":" << (int) nest.site << ":";
            int semiColon = 0;
            for (int b = 0; b < SITE_NUM; b++)
            {
                if (!semiColon)
                {
                    semiColon = 1;
                }
                else
                {
                    std::cout << ";";
                }
                std::cout << (int) beliefs[b];
            }
            std::cout << ":" << (int) messageCount << std::endl;
        }

	    // Dance state
	    msg.data[0] = danceState.state;
	    msg.data[1] = nest.site;
	    // Beliefs
        for (int b = 0; b < SITE_NUM; b++)
        {
            msg.data[beliefStart + b] = beliefs[b];
        }

	    msg.type = NORMAL;
	    msg.crc = message_crc(&msg);

        if (danceState.state == 0)
        {
            if (isMalicious == 1)
            {
                int zeroCount = SITE_NUM;
                while (zeroCount == SITE_NUM)
                {
                    zeroCount = 0;
                    for (int b = 0; b < SITE_NUM; b++)
                    {
                        uint8_t truthValue = (rand_soft() % 2);
                        if (truthValue == 1)
                            truthValue = 2;

                        beliefs[b] = truthValue;

                        if (beliefs[b] == 0)
                        {
                            zeroCount++;
                        }
                    }
                }

                formConsistentBeliefs(beliefs);

                uint8_t siteToVisit = getSiteToVisitMal(beliefs);
                setNestSite(siteToVisit, nestQualities[siteToVisit]);
                setDanceState(1, nestQualities[siteToVisit]);
            }
            else if (messageCount > 0)
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
                    uint8_t *dancingBees = (uint8_t *) malloc(sizeof(uint8_t) * (dancingBeeCount * SITE_NUM));
                    int dbIndex = 0;
                    for (int i = 0; i < messageCount; i++)
                    {
                        // If bee's dance state is true
                        if (messages[i][0] == 1)
                        {
                            // Set the dancing bee to its beliefs
                            for (int b = 0; b < SITE_NUM; b++)
                            {
                            	dancingBees[dbIndex + b] = messages[i][2 + b];
                            }

                            dbIndex += SITE_NUM;
                        }
                    }

                    uint8_t *otherBeliefs = &dancingBees[(rand_soft() % dancingBeeCount) * (SITE_NUM)];

                    consensus(beliefs, otherBeliefs);

                    formConsistentBeliefs(beliefs);

                    uint8_t siteToVisit = getSiteToVisit(beliefs);

                    if (siteToVisit == (uint8_t) -1)
                    {
                        setDanceState(0, 0);
                    }
                    else
                    {
                        setNestSite(siteToVisit, nestQualities[siteToVisit]);
                        int danceDuration = nestQualities[siteToVisit] + round(get_noise());
                        if (danceDuration <= 0)
                        {
                            danceDuration = 1;
                        }
                        setDanceState(1, danceDuration);
                    }

                    free(dancingBees);
                }
            }
            else
            {
                uint8_t siteToVisit = getSiteToVisit(beliefs);

                if (siteToVisit == (uint8_t) -1)
                {
                    setDanceState(0, 0);
                }
                else
                {
                    setNestSite(siteToVisit, nestQualities[siteToVisit]);
                    int danceDuration = nestQualities[siteToVisit] + round(get_noise());
                    if (danceDuration <= 0)
                    {
                        danceDuration = 1;
                    }
                    setDanceState(1, danceDuration);
                }
            }

            if (isMalicious == 1)
            {
                set_color(RGB(3, 3, 3));
            }
            else if (danceState.state == 1)
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
            // Random movement
            switch(rand_soft() % 4)
            {
                case(0):
                    spinup_motors();
                    set_motors(0,0);
                    break;
                case(1):
                    //if (last_output == 0) spinup_motors();
                    spinup_motors();
                    set_motors(kilo_turn_left,0); // 70
                    break;
                case(2):
                    //if (last_output == 0) spinup_motors();
                    spinup_motors();
                    set_motors(0,kilo_turn_right); // 70
                    break;
                case(3):
                    //if (last_output == 0) spinup_motors();
                    spinup_motors();
                    set_motors(kilo_straight_left, kilo_straight_right); // 65
                    break;
            }

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
        loopCounter++;
    }
}
