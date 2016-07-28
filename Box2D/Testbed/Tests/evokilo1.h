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

#define W(i,h,o) (((i)+1)*(h)+(h)*(h)+((h)+1)*(o))
#define WF(i,h,o) (((i)+1)*(h)+((h)+1)*(o))

// Default values for core definitions
#define LANG_SIZE 3
#define POPUL_SIZE 200
#define MIN_DISTANCE = 100;

#define BOOLEAN 1.0;

/*class Kilobee : public Kilobot
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

    //#define BOOLEAN 0.5

    // Default values for core variables
    double min_distance = 1;
    #ifdef BOOLEAN
    int legalOpinions[3][LANG_SIZE] = {
        {2,0,0},
        {0,2,0},
        {0,0,2}
    };
    #else
    int legalOpinions[7][LANG_SIZE] = {
        {2,0,0},
        {0,2,0},
        {0,0,2},
        {1,1,0},
        {1,0,1},
        {0,1,1},
        {1,1,1}
    };
    #endif

    // Individual vars for bots
    int messageCount;
    uint8_t messages[POPUL_SIZE][2];
    message_t msg;
    int opinionIndex;
    int opinion[LANG_SIZE];
    int last_update = 0;
    int loopCounter = 0;
    int probNotDancing = 0;

    struct NestSite
    {
        uint8_t site;
        uint8_t siteQuality;
    };

    struct State
    {
        int state;
        uint8_t duration;
    };

    struct NestSite nest = {0, 0};
    struct State danceState = {0, 0};
    int nestQualities[LANG_SIZE] = {7, 9, 10};

    void setDanceState(int state, uint8_t duration)
    {
        danceState.state = state;
        danceState.duration = duration;
    }

    void setNestSite(uint8_t site, uint8_t quality)
    {
        nest.site = site;
        nest.siteQuality = quality;
    }

    int getOpinionIndex(int *opinion)
    {
        int index = -1;
        // Loop through each opinion
        for (int i = 0; i < sizeof(legalOpinions) / sizeof(legalOpinions[0]); i++)
        {
            int found = 1;
            // Loop through values in opinion
            for (int j = 0; j < LANG_SIZE; j++)
            {
                if (legalOpinions[i][j] != opinion[j])
                {
                    found = 0;
                    break;
                }
            }

            // If found is not true, continue the loop
            if (found != 1)
                continue;
            else
            {
                index = i;
                break;
            }
        }

        return index;
    }

    int getSiteToVisit(int *opinion)
    {
        int siteToVisit = -1;
        int siteChoices[LANG_SIZE] = {-1, -1, -1};
        int siteCount = 0;

        for (int i = 0; i < LANG_SIZE; i++)
        {
            if (opinion[i] == 2)
            {
                siteToVisit = i;
                break;
            }
            else if (opinion[i] == 1)
            {
                siteChoices[siteCount] = i;
                siteCount++;
            }
        }

        if (siteToVisit == -1)
        {
            // Choose a random site to visit, out of all 1/2 values
            siteToVisit = siteChoices[(rand_soft() % siteCount)];
        }

        return siteToVisit;
    }

    void consensus(int *opinion1, int *opinion2, int *newOpinion)
    {
        for (int i = 0; i < LANG_SIZE; i++)
        {
            int sum = opinion1[i] + opinion2[i];
            if (sum <= 1)
                newOpinion[i] = 0;
            else if (sum >= 3)
                newOpinion[i] = 2;
            else if (sum == 2)
                newOpinion[i] = 1;
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

            z1 = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
            z2 = sqrt(-2 * log(u1)) * sin(2 * M_PI * u2);

            generate++;
            return z2;
        }
        else
        {
            generate--;
            return z1;
        }
    }

    message_t *tx_message()
    {
        return &msg;
    }

    void rx_message(message_t *m, distance_measurement_t *d)
    {
        int distance = estimate_distance(d);
        //std::cout << distance << std::endl;
        if (true)//distance < min_distance)
        {
            messages[messageCount][0] = m->data[0];
            messages[messageCount][1] = m->data[1];
            messageCount++;
        }
    }

};*/



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
    int updateTicks = 4;
    int initialDelay = 0;
    int lastUpdate = -1;
    int messageCount = 0;
    int nestQualities[LANG_SIZE] = {7, 9, 10};

    double belief[LANG_SIZE - 1];

    // Frank's T-norm:
    // 0.0 = min
    // 1.0 = product
    // +inf = Luk
    //
    double baseParam = 1.0;


    uint8_t messages[POPUL_SIZE][2];
    message_t msg;

    /*
     * Internal structures for 'kilobees'
     */

    struct NestSite
    {
        uint8_t site;
        uint8_t siteQuality;
    };

    struct State
    {
        uint8_t state;
        uint8_t duration;
    };

    double beliefs[LANG_SIZE];

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
        nest.siteQuality = quality;
    }

    uint8_t getSiteToVisit(double *beliefs)
    {
        int siteToVisit = -1;

        double randomSite = (double) rand_hard() / 255;

        for (int i = 0; i < LANG_SIZE - 1; i++)
        {
            if (randomSite <= beliefs[i])
            {
                siteToVisit = i;
                break;
            }
        }

        if (siteToVisit == -1)
        {
            siteToVisit = LANG_SIZE - 1;
        }

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
            return log(1 + (pow(exp(p), belief1) - 1) * (pow(exp(p), belief2) - 1) / (exp(p) - 1) ) / p;
        }
    }

    void consensus(double *beliefs1, double *beliefs2, double *newBeliefs)
    {
        for (int b = 0; b < LANG_SIZE - 1; b++)
        {
            double numerator = franksTNorm(beliefs1[0], beliefs2[0], baseParam);
            double denominator = 1.0 - beliefs1[0] - beliefs2[0] + (2 * numerator);

            double newValue = 0.0;

            // Undefined behaviour of D-S rule of combination for inconsistent beliefs
            if (denominator == 0.0)
            {
                newValue = 0.5;
            }
            else if (numerator == 0.0)
            {
                newValue = 0.0;
            }
            else
            {
                newValue = (numerator / denominator);
            }

            newBeliefs[b] = newValue;
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
            u1 = (double) rand_hard() / 256;
            u2 = (double) rand_hard() / 256;

            z1 = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
            z2 = sqrt(-2 * log(u1)) * sin(2 * M_PI * u2);

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
        }
    }

    message_t *tx_message()
    {
        return &msg;
    }
    void rx_message(message_t *m, distance_measurement_t *d)
    {
        //int distance = estimate_distance(d);
        //std::cout << distance << std::endl;
        if (1)// distance < min_distance)
        {
            messages[messageCount][0] = m->data[0];
            messages[messageCount][1] = m->data[1];
            messageCount++;
        }
    }

};




#endif

