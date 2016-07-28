/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
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

#ifndef TEST_H
#define TEST_H

#include <Box2D/Box2D.h>
#include "Render.h"

#include <stdlib.h>
#include <string>

class Test;
struct Settings;

typedef Test* TestCreateFcn(Settings *settings);

#define	RAND_LIMIT	32767
#define DRAW_STRING_NEW_LINE 25

/// Random number in range [-1,1]
inline float32 RandomFloat()
{
	float32 r = (float32)(rand() & (RAND_LIMIT));
	r /= RAND_LIMIT;
	r = 2.0f * r - 1.0f;
	return r;
}

/// Random floating point number in range [lo, hi]
inline float32 RandomFloat(float32 lo, float32 hi)
{
	float32 r = (float32)(rand() & (RAND_LIMIT));
	r /= RAND_LIMIT;
	r = (hi - lo) * r + lo;
	return r;
}

/// Test settings. Some can be controlled in the GUI.
struct Settings
{
	Settings()
	{
		viewCenter.Set(0.0f, 0.0f);
		hz                  = 60.0f;
		velocityIterations  = 8;
		positionIterations  = 3;
		drawShapes          = 0;
		drawJoints          = 0;
		drawAABBs           = 0;
		drawContactPoints   = 0;
		drawContactNormals  = 0;
		drawContactImpulse  = 0;
		drawFrictionImpulse = 0;
		drawCOMs            = 0;
		drawStats           = 0;
		drawProfile         = 0;
		enableWarmStarting  = 1;
		enableContinuous    = 1;
		enableSubStepping   = 0;
		enableSleep         = 1;
        enableTrails        = 0;
        speed               = 0;
		pause               = 1;
		singleStep          = 0;
        time_to_draw        = 1;
        ctrlargs            = std::string("");
        worldfile           = std::string("");
        params              = std::string("");
        usegui              = true;
        quit_time           = 300;
        elapsed_time        = 0;
        show_time           = false;
        seed                = 1;
        //-----------------------
        // Specific kilobot settings
        kbsigma_vbias       = 0.00125;
        kbsigma_omegabias   = 0.06;
        kbsigma_vnoise      = 0.0001;
        kbsigma_omeganoise  = 0.0001;
        kbdia               = 0.032;
        kbdensity           = 20.0;
        kblineardamp        = 5.0;
        kbangulardamp       = 5.0;
        kbfriction          = 0.8;
        kbrestitution       = 0.1;
        kbsenserad          = 0.1;
        kbspeedconst        = 0.000113;
        kbwheeloffset       = 0.005;
        kbwheeldist         = 0.015;
        kbmsgsuccess        = 0.95;
        //
        kbnestfoodsep       = 0.2;
	}

	b2Vec2 viewCenter;
	float32 hz;
	int32 velocityIterations;
	int32 positionIterations;
	int32 drawShapes;
	int32 drawJoints;
	int32 drawAABBs;
	int32 drawContactPoints;
	int32 drawContactNormals;
	int32 drawContactImpulse;
	int32 drawFrictionImpulse;
	int32 drawCOMs;
	int32 drawStats;
	int32 drawProfile;
	int32 enableWarmStarting;
	int32 enableContinuous;
	int32 enableSubStepping;
	int32 enableSleep;
    int32 enableTrails;
    int32 speed;
	int32 pause;
	int32 singleStep;
    int32 time_to_draw;
    int32 seed;
    std::string ctrlargs;
    std::string worldfile;
    std::string params;
    bool usegui;
    float quit_time;
    float elapsed_time;
    bool show_time;
    // kilobot sim parameters
    float   kbsigma_vbias;
    float   kbsigma_omegabias;
    float   kbsigma_vnoise;
    float   kbsigma_omeganoise;
    float   kbdia;
    float   kbdensity;
    float   kblineardamp;
    float   kbangulardamp;
    float   kbfriction;
    float   kbrestitution;
    float   kbsenserad;

    float   kbspeedconst;
    float   kbwheeloffset;
    float   kbwheeldist;
    float   kbmsgsuccess;
    float   kbnestfoodsep;


};

struct TestEntry
{
	const char *name;
	TestCreateFcn *createFcn;
};

extern TestEntry g_testEntries[];
// This is called when a joint in the world is implicitly destroyed
// because an attached body is destroyed. This gives us a chance to
// nullify the mouse joint.
class DestructionListener : public b2DestructionListener
{
public:
	void SayGoodbye(b2Fixture* fixture) { B2_NOT_USED(fixture); }
	void SayGoodbye(b2Joint* joint);

	Test* test;
};

const int32 k_maxContactPoints = 2048;

struct ContactPoint
{
	b2Fixture* fixtureA;
	b2Fixture* fixtureB;
	b2Vec2 normal;
	b2Vec2 position;
	b2PointState state;
	float32 normalImpulse;
	float32 tangentImpulse;
	float32 separation;
};

class Test : public b2ContactListener
{
public:

	Test();
	virtual ~Test();

    void DrawTitle(const char *string);
	virtual void Step(Settings* settings);
	virtual void Finish(Settings* settings);
	virtual void Keyboard(unsigned char key) { B2_NOT_USED(key); }
	virtual void KeyboardUp(unsigned char key) { B2_NOT_USED(key); }
	void ShiftMouseDown(const b2Vec2& p);
	virtual void MouseDown(const b2Vec2& p);
	virtual void MouseUp(const b2Vec2& p);
	void MouseMove(const b2Vec2& p);
	void LaunchBomb();
	void LaunchBomb(const b2Vec2& position, const b2Vec2& velocity);

	void SpawnBomb(const b2Vec2& worldPt);
	void CompleteBombSpawn(const b2Vec2& p);

	// Let derived tests know that a joint was destroyed.
	virtual void JointDestroyed(b2Joint* joint) { B2_NOT_USED(joint); }

	// Callbacks for derived classes.
	virtual void BeginContact(b2Contact* contact) { B2_NOT_USED(contact); }
	virtual void EndContact(b2Contact* contact) { B2_NOT_USED(contact); }
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		B2_NOT_USED(contact);
		B2_NOT_USED(impulse);
	}

	void ShiftOrigin(const b2Vec2& newOrigin);

protected:
	friend class DestructionListener;
	friend class BoundaryListener;
	friend class ContactListener;

	b2Body* m_groundBody;
	b2AABB m_worldAABB;
	ContactPoint m_points[k_maxContactPoints];
	int32 m_pointCount;
	DestructionListener m_destructionListener;
	DebugDraw m_debugDraw;
	int32 m_textLine;
	b2World* m_world;
	b2Body* m_bomb;
	b2MouseJoint* m_mouseJoint;
	b2Vec2 m_bombSpawnPoint;
	bool m_bombSpawning;
	b2Vec2 m_mouseWorld;
	int32 m_stepCount;

	b2Profile m_maxProfile;
	b2Profile m_totalProfile;
};

#endif
