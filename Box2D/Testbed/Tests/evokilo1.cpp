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

    // for (int b = 0; b < SITE_NUM; b++)
    // {
    //     beliefs[b] = 0;
    // }

    // beliefs[rand_soft() % SITE_NUM] = 1;

    for (int b = 0; b < PREF_LIMIT; b++)
    {
        beliefs[b] = 0;
    }

    for (int b = 0; b < PREF_LIMIT; b++)
    {
        int duplicateChoice = 1;
        while (duplicateChoice)
        {
            beliefs[b] = rand_soft() % SITE_NUM;
            duplicateChoice = 0;
            for (int c = 0; c < b; c++)
            {
                if (beliefs[c] == beliefs[b])
                {
                    duplicateChoice = 1;
                    break;
                }
            }
        }
    }

    uint8_t siteToVisit = getSiteToVisit(beliefs);
    setNestSite(siteToVisit, nestQualities[siteToVisit]);
    int danceDuration = nestQualities[siteToVisit] + round(get_noise());
    if (danceDuration <= 0)
    {
        danceDuration = 1;
    }
    setDanceState(1, danceDuration);

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
    for (int b = 0; b < PREF_LIMIT; b++)
    {
        msg.data[beliefStart + b] = beliefs[b];
    }

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

        std::cout << "+:" << (int) loopCounter << ":" << (int) danceState.state << ":" << (int) nest.site << ":";
        int semiColon = 0;
        for (int s = 0; s < SITE_NUM; s++)
        {
            if (!semiColon)
            {
                semiColon = 1;
            }
            else
            {
                std::cout << ";";
            }
            double normBelief = 0;
            double normaliser = (PREF_LIMIT * (PREF_LIMIT + 1)) / 2;
            for (int b = 0; b < PREF_LIMIT; b++)
            {
                if (beliefs[b] == s)
                {
                    normBelief = (b + 1) / normaliser;
                    break;
                }
            }
            std::cout << (float) normBelief;
        }
        std::cout << ":" << (int) messageCount << std::endl;

	    // Dance state
	    msg.data[0] = danceState.state;
	    msg.data[1] = nest.site;
        // Beliefs
        for (int b = 0; b < PREF_LIMIT; b++)
        {
            msg.data[beliefStart + b] = beliefs[b];
        }

	    msg.type = NORMAL;
	    msg.crc = message_crc(&msg);

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
                    uint8_t *dancingBees = (uint8_t *) malloc(sizeof(uint8_t) * (dancingBeeCount * PREF_LIMIT));
                    int dbIndex = 0;
                    for (int i = 0; i < messageCount; i++)
                    {
                        // If bee's dance state is true
                        if (messages[i][0] == 1)
                        {
                            // Set the dancing bee to its beliefs
                            for (int b = 0; b < PREF_LIMIT; b++)
                            {
                                dancingBees[dbIndex + b] = messages[i][2 + b];
                            }

                            dbIndex += PREF_LIMIT;
                        }
                    }

                    //uint8_t *otherBeliefs = &dancingBees[(rand_soft() % dancingBeeCount) * (PREF_LIMIT)];

                    /*
                     * Loop through dancing bees and tally votes for each site
                     */
                    int siteVotes[SITE_NUM];
                    for (int s = 0; s < SITE_NUM; s++)
                    {
                        siteVotes[s] = 0;
                    }
                    dbIndex = 0;
                    for (int i = 0; i < dancingBeeCount; i++)
                    {
                        // Sum up votes for each site
                        for (int b = 0; b < PREF_LIMIT; b++)
                        {
                            // Single-vote
                            siteVotes[dancingBees[dbIndex + b]] += 1;
                            // Borda counts
                            // siteVotes[dancingBees[dbIndex + b]] += b + 1;
                        }

                        dbIndex += PREF_LIMIT;
                    }

                    /*******************/

                    for (int i = PREF_LIMIT - 1; i >= 0; i--)
                    {
                        int site = getHighestVoteIndex(siteVotes);
                        beliefs[i] = site;
                        siteVotes[site] = 0;
                    }

                    uint8_t siteToVisit = getSiteToVisit(beliefs);
                    setNestSite(siteToVisit, nestQualities[siteToVisit]);
                    int danceDuration = nestQualities[siteToVisit] + round(get_noise());
                    if (danceDuration <= 0)
                    {
                        danceDuration = 1;
                    }
                    setDanceState(1, danceDuration);

                    free(dancingBees);
                }
            }
            else
            {
                uint8_t siteToVisit = getSiteToVisit(beliefs);
                setNestSite(siteToVisit, nestQualities[siteToVisit]);
                int danceDuration = nestQualities[siteToVisit] + round(get_noise());
                if (danceDuration <= 0)
                {
                    danceDuration = 1;
                }
                setDanceState(1, danceDuration);
            }

            // double probNotDancing = rand_soft() / 255.0;
            // if (probNotDancing <= 0.5)
            // {
            //     setDanceState(0, 0);
            // }

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
            // Random movement
            switch(rand_soft() % 4)
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
