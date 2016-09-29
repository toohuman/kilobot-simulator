// Copyright Simon Jones 2015

#ifndef KILOWORLD_H
#define KILOWORLD_H

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include "freeglut/freeglut.h"
#endif

#include "../Framework/Test.h"
#include "../Framework/Render.h"

//#include "kilolib.h"
#include "worldfile.h"
#include <random>


namespace Kilolib
{
    class Kilobot;
    // Subclass the built in contact class
    class KBContactListener : public b2ContactListener
    {
        void BeginContact(b2Contact *contact);
        void EndContact(b2Contact *contact);
    };
    class Region
    {
    public:
        Region(float _x, float _y, int _rt)
        :   x(_x),
            y(_y),
            rt(_rt)
        {}
        virtual void render() = 0;
        virtual int read_region(float xp, float yp) = 0;
    protected:
        float x;
        float y;
        int rt;
    };

    class Circle : public Region
    {
    public:
        Circle(float _x, float _y, float _r, int _rt)
        :   Region(_x, _y, _rt),
            r(_r)
        {}
        void render();
        int read_region(float xp, float yp);
    protected:
        float r;
    };

    class Rectangle : public Region
    {
    public:
        Rectangle(float _x, float _y, float _xs, float _ys, int _rt)
        :   Region(_x, _y, _rt),
            xs(_xs),
            ys(_ys)
        {}
        void render();
        int read_region(float xp, float yp);
    protected:
        float xs;
        float ys;
    };


    class Kiloworld : public Test
    {
    public:
        Kiloworld(Settings *_settings)
        :
            settings(_settings),
            xsize(1.2),
            ysize(1.2),
            xgrid(1),
            ygrid(1),
            gridmargin(0.2),
            simtime(0.0),
            steps(0)
        {
            // Turn off gravity
            m_world->SetGravity(b2Vec2(0,0));

            // Initialise the random number generator
            gen.seed(1);

            // Construct the world
            build_world();

            // Tell the engine that we have a contact callback
            m_world->SetContactListener(&contact_listener);
        }

        void Step(Settings* settings);
        void Finish(Settings* settings);

        void build_world();
        void parse_worldfile(float xoffset, float yoffset);
        void make_static_box(float xsize, float ysize, float xpos, float ypos);
        void render_arena();
        void make_kilobot(float xp, float yp, float th);

        // Function called by the testbench to create the world
        static Test* Create(Settings *settings)
        {
            return new Kiloworld(settings);
        }

        int get_environment(float x, float y);


    private:
        // These are the global settings
        Settings *settings;

        float   xsize;
        float   ysize;
        int     xgrid;
        int     ygrid;
        float   gridmargin;
        float   simtime;
        int     steps;

        std::vector<Kilobot*>   bots;

        // Hold list of regions
        std::vector<Region*>    regions;

        KBContactListener   contact_listener;

        float   rand_intrange(float low, float high)
        {
            std::uniform_real_distribution<float>   dist(low, high);
            return dist(gen);
        }

        // Set up a random number generator
        std::mt19937              gen;

        // arena
        //b2Body *arena;
        std::vector<b2Fixture *> arena_fixture;

        Worldfile *wf;

        std::vector<std::string> ctrlarg_words;

    };


};


#endif
