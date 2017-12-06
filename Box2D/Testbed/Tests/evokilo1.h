//----------------------------------------------------------
// Evokilo1 - First experiment in evolved kilobot controller
// (c) Simon Jones 2015
//----------------------------------------------------------

#ifndef EVOKILO1_H
#define EVOKILO1_H

#include "kilolib.h"
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <math.h>

using namespace Kilolib;

// Default values for core definitions
#define SITE_NUM 2
#define MAX_MSG_SIZE 800
#define MIN_DISTANCE 100

#define BELIEF_BYTES 1
#define BELIEF_PRECISION 0


class Kilobee : public Kilobot
{
public:
    // Foraging robot using message system to produce swarm effects
    //
    //
    // Pass the model reference up to the parent constructor
    Kilobee(ModelPosition *_pos, Settings *_settings) :
    Kilobot (_pos, _settings)
    {
        kilo_message_tx         = (message_tx_t)&Kilobee::message_tx_dummy;
        kilo_message_rx         = (message_rx_t)&Kilobee::message_rx_dummy;
        kilo_message_tx_success = (message_tx_success_t)&Kilobee::message_tx_success_dummy;

        setup();
    }

    //------------------------------------------------------------
    // Kilobot user functions
    //------------------------------------------------------------

    void setup();
    void loop();

    // Individual variables for bots
    int updateTicks = 8; // 32 updates per second
    int initialDelay = 0;
    int lastUpdate = -1;
    int messageCount = 0;
    int nestQualities[SITE_NUM] = {7, 9};
    int loopCounter = 0;

    uint8_t beliefs[SITE_NUM];
    int beliefStart = 2;

    // Frank's T-norm:
    // 0.0 = min
    // 1.0 = product
    // +inf = Luk
    //
    double baseParam = 1.0;

    int isMalicious = 0;

    uint8_t messages[MAX_MSG_SIZE][2 + SITE_NUM];
    message_t msg;

    /*
     * Internal structures for 'kilobees'
     */

    struct NestSite
    {
        uint8_t site;
        uint8_t quality;
    };

    struct State
    {
        uint8_t state;
        uint8_t duration;
    };

    // Kilobee state/belief variables

    struct NestSite nest = {0, 0};
    struct State danceState = {0, 0};

    void setDanceState(uint8_t state, uint8_t duration)
    {
        danceState.state = state;
        danceState.duration = duration;
    }

    void setNestSite(uint8_t site, uint8_t quality)
    {
        nest.site = site;
        nest.quality = quality;
    }

    double bytesToDouble(uint8_t *msgData)
    {
        uint32_t reformedBytes = 0;
        int b = 0;
        for (int i = 8 * (BELIEF_BYTES - 1); i >= 0; i -= 8)
        {
            reformedBytes += (*(msgData + b++) << i);
        }

        return (double) reformedBytes / pow(10, BELIEF_PRECISION);
    }

    void doubleToBytes(double belief, uint8_t *convertedBytes)
    {
        uint32_t convertedBelief = (uint32_t) ((belief * pow(10, BELIEF_PRECISION)) + 0.5);
        int b = 0;
        for (int i = 8 * (BELIEF_BYTES - 1); i >= 0; i -= 8)
        {
            *(convertedBytes + b++) = (uint8_t) (convertedBelief >> i) & 0xFF;
        }
    }

    void formConsistentBeliefs(uint8_t *beliefs)
    {
        // Count the number of true propositions;
        int trueCount = 0;
        for (int b = 0; b < SITE_NUM; b++)
        {
            if (beliefs[b] == 2)
            {
                trueCount++;
            }
        }
        // If greater than a single true value, convert to 1/2s.
        if (trueCount > 1)
        {
            for (int b = 0; b < SITE_NUM; b++)
            {
                if (beliefs[b] == 2)
                {
                    beliefs[b] = 1;
                }
            }
        }
        // Count number of true, borderline and false values. If 0, 1 and 0 respectively
        // then convert single borderline to true. If all false, convert to all borderlines
        trueCount = 0;
        int borderlineCount = 0;
        for (int b = 0; b < SITE_NUM; b++)
        {
            if (beliefs[b] == 2)
            {
                trueCount++;
            }
            else if (beliefs[b] == 1)
            {
                borderlineCount++;
            }
        }
        if (trueCount == 0 && borderlineCount == 1)
        {
            for (int b = 0; b < SITE_NUM; b++)
            {
                if (beliefs[b] == 1)
                {
                    beliefs[b] = 2;
                }
            }
        }
        else if (trueCount == 0 && borderlineCount == 0)
        {
            for (int b = 0; b < SITE_NUM; b++)
            {
                beliefs[b] = 1;
            }
        }
    }

    uint8_t getSiteToVisit(uint8_t *beliefs)
    {
        int siteToVisit = -1;

        int borderlineProps[SITE_NUM];
        int bCount = 0;

        for (int i = 0; i < SITE_NUM; i++)
            borderlineProps[i] = -1;

        // Count the number of borderline propositions
        for (int b = 0; b < SITE_NUM; b++)
        {
            if (beliefs[b] == 2)
                return (uint8_t) b;
            if (beliefs[b] == 1)
            {
                // Store site value and increase count
                borderlineProps[bCount] = b;
                bCount++;
            }
        }
        // siteToVisit = borderlineProps[rand_soft() % bCount];
        // Instead of returning a random site to signal for, return -1
        // and don't signal, just skip straight to updating.
        siteToVisit = -1;

        return (uint8_t) siteToVisit;
    }

    uint8_t getSiteToVisitMalf(uint8_t *beliefs)
    {
        int siteToVisit = -1;

        int borderlineProps[SITE_NUM];
        int bCount = 0;

        for (int i = 0; i < SITE_NUM; i++)
            borderlineProps[i] = -1;

        // Count the number of borderline propositions
        for (int b = 0; b < SITE_NUM; b++)
        {
            if (beliefs[b] == 2)
                return (uint8_t) b;
            if (beliefs[b] == 1)
            {
                // Store site value and increase count
                borderlineProps[bCount] = b;
                bCount++;
            }
        }
        siteToVisit = borderlineProps[rand_soft() % bCount];

        return (uint8_t) siteToVisit;
    }

    double franksTNorm(double belief1, double belief2, double p)
    {
        if (p == 0)
        {
            return fmin(belief1, belief2);
        }
        else if (p == 1)
        {
            return belief1 * belief2;
        }
        else
        {
            return log(1.0 + (pow(exp(p), belief1) - 1.0) * (pow(exp(p), belief2) - 1.0) / (exp(p) - 1.0) ) / p;
        }
    }

    void consensus(uint8_t *beliefs1, uint8_t *beliefs2)
    {
        for (int b = 0; b < SITE_NUM; b++)
        {
            if (beliefs1[b] == 2)
            {
                if (beliefs2[b] == 0)
                {
                    beliefs1[b] = 1;
                }
            }
            else if (beliefs1[b] == 0)
            {
                if (beliefs2[b] == 2)
                {
                    beliefs1[b] = 1;
                }
            }
            else if (beliefs1[b] == 1)
            {
                if (beliefs2[b] == 2)
                {
                    beliefs1[b] = 2;
                }
                else if (beliefs2[b] == 0)
                {
                    beliefs1[b] = 0;
                }
            }
        }
    }

    int generate = 0;
    double u1 = 0.0;
    double u2 = 0.0;
    double z1 = 0.0;
    double z2 = 0.0;

    double get_noise()
    {
        if (generate == 0)
        {
            u1 = (double) (1 + rand_soft()) / 256;
            u2 = (double) (1 + rand_soft()) / 256;

            z1 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
            z2 = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2);

            z1 = (0 * z1);
            z2 = (0 * z2);

            generate++;
            return z2;
        }
        else
        {
            generate--;
            return z1;
        }
    }

    void set_bot_colour(uint8_t site)
    {
        switch (site)
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
                set_color(RGB(3, 3, 0));
                break;
            case 4:
                set_color(RGB(0, 3, 3));
                break;
        }
    }

    message_t *tx_message()
    {
        return &msg;
    }

    void rx_message(message_t *m, distance_measurement_t *d)
    {
        //int distance = estimate_distance(d);
        if (1)// distance < min_distance)
        {
            // Dance state
            messages[messageCount][0] = m->data[0];
            // Dance site
            messages[messageCount][1] = m->data[1];
            // Beliefs
            for (int b = 0; b < SITE_NUM; b++)
            {
                messages[messageCount][2 + b] = m->data[beliefStart + b];
            }
            messageCount++;
        }
    }

};




#endif

