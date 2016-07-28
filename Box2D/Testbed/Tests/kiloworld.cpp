
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "kiloworld.h"
#include "evokilo1.h"
#include "worldfile.h"

using namespace Kilolib;


void Kiloworld::Step(Settings* settings)
{
    float dt = settings->hz > 0.0f ? 1.0f / settings->hz : float32(0.0f);


    // Jiggle and render if we are actually moving forward in time
    if (!settings->pause || (settings->pause && settings->singleStep))
    {
        // Calculate this way otherwise we accumulate floating point error
        steps++;
        simtime = dt * steps;
        for(int i=0; i<bots.size(); i++)
            bots[i]->update(dt, simtime);
    }
    //run the default physics and rendering
    Test::Step(settings);

    // Show some text in the main screen
    if (settings->time_to_draw)
    {
        render_arena();
        for(int i=0; i<regions.size(); i++)
            regions[i]->render();
        // Render the sensor first, then the kilobot body
        for(int i=0; i<bots.size(); i++)
            bots[i]->rendersensor();
        for(int i=0; i<bots.size(); i++)
            bots[i]->renderbody();

        std::string s = string_format("Time:%8.2f", simtime);
        m_debugDraw.DrawString(5, m_textLine, s.c_str());
        m_textLine += 15;

        // Get total food
        int f = 0;
        for(int i=0; i<bots.size(); i++)
            f += bots[i]->metric();
        s = string_format("Food:%8i", f);
        m_debugDraw.DrawString(5, m_textLine, s.c_str());
        m_textLine += 15;
    }
}

void Kiloworld::Finish(Settings *settings)
{
    printf("Sim about to finish\n");
}

static std::vector<std::string> split(std::string s)
{
    // Quite frankly, this is inpenetrable, found on stackoverflow, but at least it doesn't require boost
    // Apparently an istream iterator of type string regards whitespace as the separator..
    std::vector<std::string> words;
    std::istringstream iss(s);
    std::copy (std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), back_inserter(words));
    return words;
}

void Kiloworld::parse_worldfile(float xoffset, float yoffset)
{
    // Scan the parsed data structure, first for a global defn, then
    // for everything else, since the global settings my impact on the rest
    // eg for choosing a random bias value
    for(int entity=1; entity < wf->GetEntityCount(); entity++)
    {
        const char *typestr = (char*)wf->GetEntityType(entity);
        int entity_parent = wf->GetEntityParent(entity);
        if (entity_parent == 0 && !strcmp(typestr, "global"))
        {
            // simulation parameters may be defined on the command line
            // with the '--params' option, or with:
            //  global ( params [ <val1> <val1> ... <val14> ] )
            // Each val is floating point and has the same meaning as the
            // command line params
            if (wf->PropertyExists(entity, "params"))
            {
                wf->ReadTuple(entity, "params", 0, 15, "fffffffffffffff",
                    &(settings->kbsigma_vbias),
                    &(settings->kbsigma_omegabias),
                    &(settings->kbsigma_vnoise),
                    &(settings->kbsigma_omeganoise),
                    &(settings->kbdia),
                    &(settings->kbdensity),
                    &(settings->kblineardamp),
                    &(settings->kbangulardamp),
                    &(settings->kbfriction),
                    &(settings->kbrestitution),
                    &(settings->kbsenserad),
                    &(settings->kbspeedconst),
                    &(settings->kbwheeloffset),
                    &(settings->kbwheeldist),
                    &(settings->kbmsgsuccess)
                    );
            }
        }
    }
    for(int entity=1; entity < wf->GetEntityCount(); entity++)
    {
        const char *typestr = (char*)wf->GetEntityType(entity);
        int entity_parent = wf->GetEntityParent(entity);
        //printf("%d %d %s\n", entity, wf->GetEntityParent(entity), typestr);
        // We are interested in the entities of type position with parent entity 0
        // These are top level kilobit models within the world, grab the pose
        // data associated with them, and the controller
        if (entity_parent == 0 && !strcmp(typestr, "region"))
        {
            // We define regions which return particular values from environmental
            // sensing
            // Regions can be:
            //  region ( circle [ <xpos> <ypos> <radius> <return> ] )
            //  region ( rectangle [ <xpos> <ypos> <xsize> <ysize> <return> ] )
            if (wf->PropertyExists(entity, "circle"))
            {
                float x, y, r;
                int rt;
                wf->ReadTuple(entity, "circle", 0, 4, "fffi", &x, &y, &r, &rt);
                //printf("circle %f %f %f %i\n", x, y, r, rt);
                regions.push_back((Region*)(new Circle(x, y, r, rt)));
            }
            if (wf->PropertyExists(entity, "rectangle"))
            {
                float x, y, xs, ys;
                int rt;
                wf->ReadTuple(entity, "rectangle", 0, 5, "ffffi", &x, &y, &xs, &ys, &rt);
                //printf("rectangle %f %f %f %f %i\n", x, y, xs, ys, rt);
                regions.push_back((Region*)(new Rectangle(x, y, xs, ys, rt)));
            }
        }
        if (entity_parent == 0 && !strcmp(typestr, "position"))
        {
            double x, y, z, a;
            if (!wf->PropertyExists(entity, "pose"))
            {
                printf("No pose found for kilobot entity %d, exitting\n", entity);
                exit(1);
            }
            wf->ReadTuple(entity, "pose", 0, 4, "llla", &x, &y, &z, &a);
            CProperty *ctrlp =  wf->GetProperty(entity, "ctrl");
            if (!ctrlp)
            {
                printf("No controller token for kilobot entity %d, exitting\n", entity);
                exit(1);
            }
            const char *controller = wf->GetPropertyValue(ctrlp, 0);
            if (!controller)
            {
                printf("No controller string for kilobot entity %d, exitting\n", entity);
                exit(1);
            }
            //printf("Kilobot pose[%f,%f,%f,%f] ctrl[%s]\n", x, y, z, a, controller);

            // Now create the kilobot
            ModelPosition *mod = new ModelPosition;
            mod->pose.x = x + xoffset;
            mod->pose.y = y + yoffset;
            mod->pose.a = a;
            mod->world = m_world;
            mod->kworld = this;
            // tokenize the argument string into words
            std::vector<std::string> words = split(controller);
            //printf("ctrargs is %s %s\n", ctrlarg_words[0].c_str(), ctrlarg_words[1].c_str());
            std::string logfile = "";
            if (ctrlarg_words.size() == 2 && ctrlarg_words[0] == "log")
                logfile = ctrlarg_words[1];

            if (words[0] == "kilobee")
                bots.push_back((Kilobot*)(new Kilobee(mod, settings)));

        }
    }
}

void Kiloworld::build_world()
{


    // Read in the worldfile

    printf("%s\n", settings->worldfile.c_str());
    // We need to parse the worldfile, which is in Stage format, but the only
    // things we care about are:
    //  quit_time <number>
    //  define <name> kilobot ( ctrl "<control_string>" )
    //  <name> ( pose [ <x> <y> <dont_care> <angle> ] )
    wf = new Worldfile();
    if (!wf->Load(settings->worldfile))
    {
        exit(1);
    }
    int entity = 0;
    //float quit_time = 0;
    // entity 0 is the world, which all settings are attached to
    settings->quit_time = wf->ReadFloat(entity, "quit_time", settings->quit_time);
    printf("Got quit time %f\n", settings->quit_time);

    if (settings->params != "")
    {
        printf("Got non-default kilobot parameters %s\n", settings->params.c_str());
        std::vector<std::string> params = split(settings->params);
        if ((params.size() != 15) && (params.size() != 4))
        {
            printf("Invalid parameter argument, should be 4 or 15 numbers, exitting..\n");
            exit(1);
        }

        settings->kbsigma_vbias         = std::stof(params[0]);
        settings->kbsigma_omegabias     = std::stof(params[1]);
        settings->kbsigma_vnoise        = std::stof(params[2]);
        settings->kbsigma_omeganoise    = std::stof(params[3]);
        if (params.size() == 15)
        {
            settings->kbdia               = std::stof(params[4]);
            settings->kbdensity           = std::stof(params[5]);
            settings->kblineardamp        = std::stof(params[6]);
            settings->kbangulardamp       = std::stof(params[7]);
            settings->kbfriction          = std::stof(params[8]);
            settings->kbrestitution       = std::stof(params[9]);
            settings->kbsenserad          = std::stof(params[10]);
            settings->kbspeedconst        = std::stof(params[11]);
            settings->kbwheeloffset       = std::stof(params[12]);
            settings->kbwheeldist         = std::stof(params[13]);
            settings->kbmsgsuccess        = std::stof(params[14]);
        }
    }

    // Get any command line args for the controllers
    ctrlarg_words = split(settings->ctrlargs);

    // Make a grid of arenas
    for(int y=0; y<ygrid; y++)
    {
        for(int x=0; x<xgrid; x++)
        {
            // Make arena of fixed lines
            float xp = (xsize + gridmargin) * x;
            float yp = (ysize + gridmargin) * y;
            make_static_box(xsize, ysize, xp, yp);
            parse_worldfile(xp, yp);
        }
    }
    printf("Params are:\n");
    printf("kbsigma_vbias       %f\n",settings->kbsigma_vbias);
    printf("kbsigma_omegabias   %f\n",settings->kbsigma_omegabias);
    printf("kbsigma_vnoise      %f\n",settings->kbsigma_vnoise);
    printf("kbsigma_omeganoise  %f\n",settings->kbsigma_omeganoise);
    printf("kbdia               %f\n",settings->kbdia);
    printf("kbdensity           %f\n",settings->kbdensity);
    printf("kblineardamp        %f\n",settings->kblineardamp);
    printf("kbangulardamp       %f\n",settings->kbangulardamp);
    printf("kbfriction          %f\n",settings->kbfriction);
    printf("kbrestitution       %f\n",settings->kbrestitution);
    printf("kbsenserad          %f\n",settings->kbsenserad);
    printf("kbspeedconst        %f\n",settings->kbspeedconst);
    printf("kbwheeloffset       %f\n",settings->kbwheeloffset);
    printf("kbwheeldist         %f\n",settings->kbwheeldist);
    printf("kbmsgsuccess        %f\n",settings->kbmsgsuccess);

}



void Kiloworld::make_static_box(float xs, float ys, float xp, float yp)
{
    // Create the body
    b2BodyDef   arenadef;
    b2Body      *arena = m_world->CreateBody(&arenadef);

    // Create the shape of the fixture
    b2Vec2 vs[4];
    vs[0].Set(xp-xs/2, yp-ys/2);
    vs[1].Set(xp+xs/2, yp-ys/2);
    vs[2].Set(xp+xs/2, yp+ys/2);
    vs[3].Set(xp-xs/2, yp+ys/2);
    b2ChainShape perimeter;
    perimeter.CreateLoop(vs, 4);

    // Create the fixture attached to the arena body
    // The fixture is created directly from the shape because
    // we are not altering the default properties of the created
    // fixture
    arena_fixture.push_back(arena->CreateFixture(&perimeter, 0));
}


void Kiloworld::render_arena()
{
    for(int i=0; i<arena_fixture.size(); i++)
    {
        b2ChainShape *chain = (b2ChainShape*)arena_fixture[i]->GetShape();
        int32 count = chain->m_count;
        const b2Vec2* vertices = chain->m_vertices;

        b2Vec2 v1 = vertices[0];

        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        for (int32 i = 1; i < count; ++i)
        {
            b2Vec2 v2 = vertices[i];
            glVertex2f(v1.x, v1.y);
            glVertex2f(v2.x, v2.y);
            //m_debugDraw.DrawSegment(v1, v2, color);
            //m_debugDraw.DrawCircle(v1, 0.05f, color);
            v1 = v2;
        }
        glEnd();
    }
    glEnd();
}


void Circle::render()
{
    glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    switch(rt)
    {
        case(0): glColor4f(0.9, 0.9, 0.9, 1.0);break;
        case(1): glColor4f(0.0, 1.0, 0.0, 0.2);break;
        case(2): glColor4f(1.0, 0.0, 1.0, 0.2);break;
        case(3): glColor4f(0.0, 1.0, 1.0, 0.2);break;
    }
    glBegin(GL_TRIANGLE_FAN);
    for(int i=0; i<32; i++)
    {
        float th = i * 2.0 * M_PI / 32;
        float xp = x + r * cos(th);
        float yp = y + r * sin(th);
        glVertex2f(xp, yp);
    }
    glEnd();
    glDisable(GL_BLEND);
}

void Rectangle::render()
{
    glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    switch(rt)
    {
        case(0): glColor4f(0.9, 0.9, 0.9, 1.0);break;
        case(1): glColor4f(0.0, 1.0, 0.0, 0.2);break;
        case(2): glColor4f(1.0, 0.0, 1.0, 0.2);break;
        case(3): glColor4f(0.0, 1.0, 1.0, 0.2);break;
    }
    glBegin(GL_QUADS);
    glVertex2f(x - xs/2, y - ys/2);
    glVertex2f(x - xs/2, y + ys/2);
    glVertex2f(x + xs/2, y + ys/2);
    glVertex2f(x + xs/2, y - ys/2);
    glEnd();
    glDisable(GL_BLEND);
}


int Kiloworld::get_environment(float x, float y)
{
    // Ask all regions if we are in their area. Positive if yes,
    // first to respond is used
    for(int i=regions.size()-1; i>=0; i--)
    {
        int r = regions[i]->read_region(x, y);
        if (r >= 0)
            return r;
    }
    return 0;
}


int Circle::read_region(float xp, float yp)
{
    //printf("Checking circle at %f %f %f %i\n", x, y, r, rt);
    if (sqrt((xp - x) * (xp - x) + (yp - y) * (yp - y)) < r)
        return rt;
    return -1;
}

int Rectangle::read_region(float xp, float yp)
{
    //printf("Checking rectangle at %f %f %f %f%i\n", x, y, xs, ys, rt);
    if (    (xp > (x - xs/2))
        &&  (xp < (x + xs/2))
        &&  (yp > (y - ys/2))
        &&  (yp < (y + ys/2)))
        return rt;
    return -1;
}






