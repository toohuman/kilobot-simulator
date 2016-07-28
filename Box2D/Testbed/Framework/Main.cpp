/*
* Copyright (c) 2006-2007 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "Render.h"
#include "Test.h"
#include "glui/glui.h"

#include <stdio.h>

#include <sys/time.h>


#define STAMP(x) {struct timeval t;gettimeofday(&t,NULL);printf("%s %8lu %s\n",__func__,(unsigned long)t.tv_usec,x);}


namespace
{
	int32 testIndex = 0;
	int32 testSelection = 0;
	int32 testCount = 0;
	TestEntry* entry;
	Test* test;
	Settings settings;
	int32 width = 1280;
	int32 height = 720;
    // Unless this is lower than the actual frame rate, this seems to cause
    // problems, possibly due to running on compositing window managers -
    // it happens on both OSX and Ubuntu with compiz.
    // The sim physics is stepped in the glut idle callback and the display callback
    // but only renders within the display callback. When the framePeriod is less than 17
    // (frame rate of 60Hz), the physics only seems to be stepped really slowly.
    //
    // Hmmm.. if we attempt to render at a faster rate than the actual display, I guess
    // swapBuffers blocks to synchronise with the actual display, meaning very few calls to the 
    // idle callback
    // 
	int32 framePeriod = 20;
	int32 mainWindow;
	float settingsHz = 10.0;
	GLUI *glui;
	float32 viewZoom = 0.05f;
	int tx, ty, tw, th;
	bool rMouseDown;
	b2Vec2 lastp;

    double realtime = 0.0;
    double simtime = 0.0;
}

static void Resize(int32 w, int32 h)
{
	width = w;
	height = h;

	GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	glViewport(tx, ty, tw, th);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float32 ratio = float32(tw) / float32(th);

	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= viewZoom;

	b2Vec2 lower = settings.viewCenter - extents;
	b2Vec2 upper = settings.viewCenter + extents;

	// L/R/B/T
	gluOrtho2D(lower.x, upper.x, lower.y, upper.y);
}

static b2Vec2 ConvertScreenToWorld(int32 x, int32 y)
{
	float32 u = x / float32(tw);
	float32 v = (th - y) / float32(th);

	float32 ratio = float32(tw) / float32(th);
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= viewZoom;

	b2Vec2 lower = settings.viewCenter - extents;
	b2Vec2 upper = settings.viewCenter + extents;

	b2Vec2 p;
	p.x = (1.0f - u) * lower.x + u * upper.x;
	p.y = (1.0f - v) * lower.y + v * upper.y;
	return p;
}

// This is used to control the frame rate (60Hz).
static void Timer(int)
{
	glutSetWindow(mainWindow);
	glutPostRedisplay();
	glutTimerFunc(framePeriod, Timer, 0);
}

void simstep()
{
    static int last_sm = 0;
    int speedmult = 0;
    switch (settings.speed)
    {
        case 1: speedmult = 1; break;
        case 2: speedmult = 2; break;
        case 3: speedmult = 5; break;
        case 4: speedmult = 10; break;
        case 5: speedmult = 20; break;
        case 6: speedmult = 50; break;
        case 7: speedmult = 100; break;
    }
    if (last_sm != speedmult)
        // Fast forward realtime to current simtime if just enabled controlled speed
        realtime = simtime / speedmult;
    last_sm = speedmult;
    
    if (!speedmult || (simtime / speedmult < realtime))
    {
        test->Step(&settings);
        if (!settings.pause)
            simtime += 1.0/settingsHz;
    }
    else
    {
        int p = settings.pause;
        settings.pause = 1;
        test->Step(&settings);
        settings.pause = p;
    }
}

static void SimulationAdvance()
{
    //STAMP("Advance");
    settings.time_to_draw = 0;
    simstep();
    //printf("sim:%f real:%f\n",simtime, realtime);
}

static void SimulationDisplay()
{
    //STAMP("Display");
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
	b2Vec2 oldCenter = settings.viewCenter;
	settings.hz = settingsHz;
    
    settings.time_to_draw = 1;
    simstep();

	test->DrawTitle(entry->name);
    
    if (!settings.pause)
        realtime += 0.001 * framePeriod;

    //printf("sim:%f real:%f\n",simtime, realtime);
	glutSwapBuffers();
    
}

static void Keyboard(unsigned char key, int x, int y)
{
	B2_NOT_USED(x);
	B2_NOT_USED(y);

	switch (key)
	{
	case 27:
#ifndef __APPLE__
		// freeglut specific function
		glutLeaveMainLoop();
#endif
		exit(0);
		break;

		// Press 'z' to zoom out.
	case 'z':
		viewZoom = b2Min(1.1f * viewZoom, 20.0f);
		Resize(width, height);
		break;

		// Press 'x' to zoom in.
	case 'x':
		viewZoom = b2Max(0.9f * viewZoom, 0.02f);
		Resize(width, height);
		break;


		// Press space to launch a bomb.
	case ' ':
		if (test)
		{
			test->LaunchBomb();
		}
		break;
 
	case 'p':
		settings.pause = !settings.pause;
		break;

		// Press [ to prev test.
	case '[':
		--testSelection;
		if (testSelection < 0)
		{
			testSelection = testCount - 1;
		}
		glui->sync_live();
		break;

		// Press ] to next test.
	case ']':
		++testSelection;
		if (testSelection == testCount)
		{
			testSelection = 0;
		}
		glui->sync_live();
		break;
		
	default:
		if (test)
		{
			test->Keyboard(key);
		}
	}
}

static void KeyboardSpecial(int key, int x, int y)
{
	B2_NOT_USED(x);
	B2_NOT_USED(y);

	int mod = glutGetModifiers();

	switch (key)
	{
		// Press left to pan left.
	case GLUT_KEY_LEFT:
		if (mod == GLUT_ACTIVE_CTRL)
		{
			b2Vec2 newOrigin(2.0f, 0.0f);
			test->ShiftOrigin(newOrigin);
		}
		else
		{
			settings.viewCenter.x -= 0.5f;
			Resize(width, height);
		}
		break;

		// Press right to pan right.
	case GLUT_KEY_RIGHT:
		if (mod == GLUT_ACTIVE_CTRL)
		{
			b2Vec2 newOrigin(-2.0f, 0.0f);
			test->ShiftOrigin(newOrigin);
		}
		else
		{
			settings.viewCenter.x += 0.5f;
			Resize(width, height);
		}
		break;

		// Press down to pan down.
	case GLUT_KEY_DOWN:
		if (mod == GLUT_ACTIVE_CTRL)
		{
			b2Vec2 newOrigin(0.0f, 2.0f);
			test->ShiftOrigin(newOrigin);
		}
		else
		{
			settings.viewCenter.y -= 0.5f;
			Resize(width, height);
		}
		break;

		// Press up to pan up.
	case GLUT_KEY_UP:
		if (mod == GLUT_ACTIVE_CTRL)
		{
			b2Vec2 newOrigin(0.0f, -2.0f);
			test->ShiftOrigin(newOrigin);
		}
		else
		{
			settings.viewCenter.y += 0.5f;
			Resize(width, height);
		}
		break;

		// Press home to reset the view.
	case GLUT_KEY_HOME:
		viewZoom = 1.0f;
		settings.viewCenter.Set(0.0f, 20.0f);
		Resize(width, height);
		break;
	}
}

static void KeyboardUp(unsigned char key, int x, int y)
{
	B2_NOT_USED(x);
	B2_NOT_USED(y);

	if (test)
	{
		test->KeyboardUp(key);
	}
}

static void Mouse(int32 button, int32 state, int32 x, int32 y)
{
	// Use the mouse to move things around.
	if (button == GLUT_LEFT_BUTTON)
	{
		int mod = glutGetModifiers();
		b2Vec2 p = ConvertScreenToWorld(x, y);
		if (state == GLUT_DOWN)
		{
			b2Vec2 p = ConvertScreenToWorld(x, y);
			if (mod == GLUT_ACTIVE_SHIFT)
			{
				test->ShiftMouseDown(p);
			}
			else
			{
				test->MouseDown(p);
			}
		}
		
		if (state == GLUT_UP)
		{
			test->MouseUp(p);
		}
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{	
			lastp = ConvertScreenToWorld(x, y);
			rMouseDown = true;
		}

		if (state == GLUT_UP)
		{
			rMouseDown = false;
		}
	}
}

static void MouseMotion(int32 x, int32 y)
{
	b2Vec2 p = ConvertScreenToWorld(x, y);
	test->MouseMove(p);
	
	if (rMouseDown)
	{
		b2Vec2 diff = p - lastp;
		settings.viewCenter.x -= diff.x;
		settings.viewCenter.y -= diff.y;
		Resize(width, height);
		lastp = ConvertScreenToWorld(x, y);
	}
}

#ifdef FREEGLUT
static void MouseWheel(int wheel, int direction, int x, int y)
{
	B2_NOT_USED(wheel);
	B2_NOT_USED(x);
	B2_NOT_USED(y);
	if (direction > 0)
	{
		viewZoom /= 1.1f;
	}
	else
	{
		viewZoom *= 1.1f;
	}
	Resize(width, height);
}
#endif


static void Pause(int)
{
	settings.pause = !settings.pause;
}

static void Exit(int code)
{
	// TODO: freeglut is not building on OSX
#ifdef FREEGLUT
	glutLeaveMainLoop();
#endif
	exit(code);
}

static void SingleStep(int)
{
	settings.pause = 1;
	settings.singleStep = 1;
}

#include <getopt.h>

const char* USAGE = 
  "USAGE:  kilobox [options] <worldfile> \n"
  "Available [options] are:\n"
  "  --clock        : print simulation time peridically on standard output\n"
  "  -c             : equivalent to --clock\n"
  "  --gui          : run without a GUI\n"
  "  -g             : equivalent to --gui\n"
  "  --args \"str\"   : define an argument string to be passed to all controllers\n"
  "  --help         : print this message\n"
  "  -h             : equivalent to --help\n"
  "  -?             : equivalent to --help\n"
  "  --seed <num>   : random seed\n"
  "  --params \"p1 p2 ... p12\" : Set simulator parameters (floating point numbers)"

;

static struct option longopts[] = {
	{ "gui",  optional_argument,   NULL,  'g' },
	{ "clock",  optional_argument,   NULL,  'c' },
	{ "help",  optional_argument,   NULL,  'h' },
	{ "args",  required_argument,   NULL,  'a' },
	{ "seed",  required_argument,   NULL,  's' },
	{ "params",  required_argument,   NULL,  'p' },
	{ NULL, 0, NULL, 0 }
};


void rungui(int argc, char** argv);
void runnogui(int argc, char** argv);


int main(int argc, char** argv)
{

    int ch=0, optindex=0;
    bool showclock = false;
    while ((ch = getopt_long(argc, argv, "cgh?", longopts, &optindex)) != -1)
    {
        switch(ch)
        {
            case 0: // long option given
                printf( "option %s given\n", longopts[optindex].name );
                break;
            case 'a':
                settings.ctrlargs = std::string(optarg);
                break;
            case 'c': 
                settings.show_time = true;
                printf( "[Clock enabled]" );
                break;
            case 'g': 
                settings.usegui = false;
                printf( "[GUI disabled]" );
                break;
            case 'h':  
            case '?':  
                puts( USAGE );
                break;
            case 's':
                settings.seed = strtol(optarg, 0, 10);
                printf("Set random seed to %d\n", settings.seed);
                break;
            case 'p':
            {
                settings.params = std::string(optarg);
                printf("params %s\n", settings.params.c_str());
                break;
            }
            default:
                printf("unhandled option %c\n", ch );
                puts( USAGE );

        }
    }

    printf("%d %d\n", argc, optindex);
    optindex = optind; //points to first non-option
    if(optindex < argc && optindex > 0)
    {      
        settings.worldfile = std::string(argv[optindex]);
    }


    testCount = 1;
    testIndex = 0;
    testSelection = testIndex;
    entry = g_testEntries + testIndex;

    // Construct the world
	test = entry->createFcn(&settings);

	settings.hz = settingsHz;
    if (settings.usegui)
        rungui(argc, argv);
    else
        runnogui(argc, argv);
	return 0;
}

void runnogui(int argc, char** argv)
{
    // Don't start paused when in command line mode
    settings.pause = 0;
    while(settings.elapsed_time < settings.quit_time)
    {
        settings.time_to_draw = 0;
	    test->Step(&settings);
    }
    test->Finish(&settings);
}

void rungui(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	//glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowSize(width, height);
	char title[32];
	sprintf(title, "Box2D Version %d.%d.%d", b2_version.major, b2_version.minor, b2_version.revision);
	mainWindow = glutCreateWindow(title);
	//glutSetOption (GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutIdleFunc(SimulationAdvance);
    glutDisplayFunc(SimulationDisplay);
	GLUI_Master.set_glutReshapeFunc(Resize);  
	GLUI_Master.set_glutKeyboardFunc(Keyboard);
	GLUI_Master.set_glutSpecialFunc(KeyboardSpecial);
	GLUI_Master.set_glutMouseFunc(Mouse);
#ifdef FREEGLUT
	glutMouseWheelFunc(MouseWheel);
#endif
	glutMotionFunc(MouseMotion);

	glutKeyboardUpFunc(KeyboardUp);

	glui = GLUI_Master.create_glui_subwindow( mainWindow, 
		GLUI_SUBWINDOW_RIGHT );

	glui->add_statictext("Tests");
	GLUI_Listbox* testList =
		glui->add_listbox("", &testSelection);

	glui->add_separator();

	GLUI_Spinner* velocityIterationSpinner =
		glui->add_spinner("Vel Iters", GLUI_SPINNER_INT, &settings.velocityIterations);
	velocityIterationSpinner->set_int_limits(1, 500);

	GLUI_Spinner* positionIterationSpinner =
		glui->add_spinner("Pos Iters", GLUI_SPINNER_INT, &settings.positionIterations);
	positionIterationSpinner->set_int_limits(0, 100);

	GLUI_Spinner* hertzSpinner =
		glui->add_spinner("Hertz", GLUI_SPINNER_FLOAT, &settingsHz);

	hertzSpinner->set_float_limits(5.0f, 200.0f);

	glui->add_checkbox("Sleep", &settings.enableSleep);
	glui->add_checkbox("Warm Starting", &settings.enableWarmStarting);
	glui->add_checkbox("Time of Impact", &settings.enableContinuous);
	glui->add_checkbox("Sub-Stepping", &settings.enableSubStepping);

	//glui->add_separator();

	GLUI_Panel* drawPanel =	glui->add_panel("Draw");
	glui->add_checkbox_to_panel(drawPanel, "Shapes", &settings.drawShapes);
	glui->add_checkbox_to_panel(drawPanel, "Joints", &settings.drawJoints);
	glui->add_checkbox_to_panel(drawPanel, "AABBs", &settings.drawAABBs);
	glui->add_checkbox_to_panel(drawPanel, "Contact Points", &settings.drawContactPoints);
	glui->add_checkbox_to_panel(drawPanel, "Contact Normals", &settings.drawContactNormals);
	glui->add_checkbox_to_panel(drawPanel, "Contact Impulses", &settings.drawContactImpulse);
	glui->add_checkbox_to_panel(drawPanel, "Friction Impulses", &settings.drawFrictionImpulse);
	glui->add_checkbox_to_panel(drawPanel, "Center of Masses", &settings.drawCOMs);
	glui->add_checkbox_to_panel(drawPanel, "Statistics", &settings.drawStats);
	glui->add_checkbox_to_panel(drawPanel, "Profile", &settings.drawProfile);

    GLUI_Panel* simPanel =	glui->add_panel("Sim");
    GLUI_RadioGroup *rgroup = glui->add_radiogroup_to_panel(simPanel, &settings.speed);
    glui->add_radiobutton_to_group(rgroup, "Full");
    glui->add_radiobutton_to_group(rgroup, "1x");
    glui->add_radiobutton_to_group(rgroup, "2x");
    glui->add_radiobutton_to_group(rgroup, "5x");
    glui->add_radiobutton_to_group(rgroup, "10x");
    glui->add_radiobutton_to_group(rgroup, "20x");
    glui->add_radiobutton_to_group(rgroup, "50x");
    glui->add_radiobutton_to_group(rgroup, "100x");
	glui->add_checkbox_to_panel(simPanel, "Trails", &settings.enableTrails);


	int32 testCount = 0;
	TestEntry* e = g_testEntries;
	while (e->createFcn)
	{
		testList->add_item(testCount, e->name);
		++testCount;
		++e;
	}

	glui->add_button("Pause", 0, Pause);
	glui->add_button("Single Step", 0, SingleStep);

	glui->add_button("Quit", 0,(GLUI_Update_CB)Exit);
	glui->set_main_gfx_window( mainWindow );

	// Use a timer to control the frame rate.
	glutTimerFunc(framePeriod, Timer, 0);

	glutMainLoop();

}
