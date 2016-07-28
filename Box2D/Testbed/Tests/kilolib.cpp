// Copyright Simon Jones 2015


#include "kilolib.h"
#include <math.h>

using namespace Kilolib;

int Kilobot::ids = 0;




void Kilobot::make_kilobot(float xp, float yp, float th)
{
	// Create the body
	b2BodyDef   kbdef;
	kbdef.type              = b2_dynamicBody;
	kbdef.position.Set(xp, yp);
	kbdef.angle             = th;
	//kbdef.linearDamping     = kblineardamp;
	//kbdef.angularDamping    = kbangulardamp;
	m_body                  = m_world->CreateBody(&kbdef);

	// Create a reference to here in the physics world
	m_body->SetUserData(this);

	b2CircleShape c;
	b2FixtureDef kfdef;
	// Create the shape
	c.m_radius = settings->kbdia/2.0;
	// Create the fixture for the body
	kfdef.shape             = &c;
	kfdef.density           = settings->kbdensity;
	kfdef.friction          = settings->kbfriction;
	kfdef.restitution       = settings->kbrestitution;
	kfdef.filter.categoryBits   = KILOBOT;
	kfdef.filter.maskBits       = KILOBOT | MESSAGE;
	m_body->CreateFixture(&kfdef);

	// Now do the fixture for the sensor
	c.m_radius              = settings->kbsenserad;
	kfdef.shape             = &c;
	kfdef.isSensor          = true;
	kfdef.density           = 0;
	kfdef.friction          = 0;
	kfdef.restitution       = 0;
	kfdef.filter.categoryBits   = MESSAGE;
	kfdef.filter.maskBits       = KILOBOT;
	m_body->CreateFixture(&kfdef);


}

void Kilobot::check_messages()
{
	// Messages are sent by placing them in a message queue owned by the recipient
	message_t *msg = 0;
	// Now see if its time to try sending a new message
	uint32_t t = pos->GetWorld()->SimTimeNow();
	if (((int)t - (int)last_message) > (message_period * 1e6))
	{
		//if (kilo_uid == 1)
		//    printf("time:%i last:%i period:%f\n", t, last_message, message_period);
		// Time to try and send a message if there is one. First, update the
		// message attempt timestamp with a bit of randomness of 50ms
		// FIXME!! SJ we really don't know how this will affect sim fidelity
		// and perhaps this should be one of the parameters to evolve
		//last_message = t + rand(0, 200000);
		last_message = t;
		// Call the callback function to see if there is anything there
		msg = (*this.*kilo_message_tx)();
		//printf("%s sending at %u\n", pos->Token(), t);


		// Check the set of inrange kilobots and copy the message into the
		// target message queue
		if (msg)
		{
			// Signal successful message transmission. This is no guarantee of
			// reception, it just means (in the hardware) that no collision was
			// detected in the first ~250us
			(*this.*kilo_message_tx_success)();
			int s = inrange_bots.size();
			int r = 0;//rand(0, s);
			for(int i=0; i<s; i++)
			{
				// See if this message actually makes it to the recipient
				if (rand_realrange(0.0,1.0) > settings->kbmsgsuccess)
				{
					//printf("Failed to send message to %i of %i\n",i,s);
					continue;
				}
				int j = (i + r) % s;
				Kilobot *bot = inrange_bots[j];
				// Get a pointer to the recipients message queue
				m_queue_t &meq = bot->message_queue;
				// Calculate distance between centres in mm
				Pose me = pos->GetGlobalPose();
				Pose tx = bot->pos->GetGlobalPose();
				int dist = hypot(me.x - tx.x, me.y - tx.y) * 1000;
				// Copy across the message and the distance
				distance_measurement_t  d;
				d.low_gain = dist;
				// Put the message on the queue
				m_event_t sendmsg;
				sendmsg.m = *msg;
				sendmsg.d = d;
				sendmsg.s = pos->Token();
				meq.push(sendmsg);
				//printf("placing msg from %s in %s dist %d dat %i\n",
				//       pos->Token(),bot->pos->Token(), dist, sendmsg.m.data[0]);
			}
		}
	}

	// Now check if we have messages on our queue and for each one, call the
	// callback function
	m_queue_t &meq = message_queue;
	while (!meq.empty())
	{
		(*this.*kilo_message_rx)(&meq.front().m, &meq.front().d);
		//printf("%s got message from distance %s %i %i\n",
		//       pos->Token(), meq.front().s.c_str(), meq.front().d.low_gain, meq.front().m.data[0]);
		meq.pop();
	}

}

void Kilobot::set_motors(int left_m, int right_m)
{
	current_left_m  = left_m;
	current_right_m = right_m;

}

void Kilobot::update_motion()
{
	// Speed constant, we know from \cite{rubenstein2012kilobot} that the speed
	// of the kilobot is approximately 0.01ms^-1, and the recommended
	// maximum motor power is ~100, so giving the constant
	const double k  = settings->kbspeedconst;     // Speed constant
	const double o  = settings->kbwheeloffset;     // Offset of turning center from oject centre
	// The angular velocity  of the kilobot is ~45deg/s = 0.8rad/s
	// Only one motor is running during rotation, giving an effective
	// distance between the wheels of 12.5mm
	const double l  = settings->kbwheeldist;    // distance between centres of wheels

	// Generate noise terms for velocity goals
	// Motion noise arises from the behaviour of the vibrating motors. Don't
	// inject noise when the commanded motor values are zero, the kilobots will be
	// subject to jiggling from others but won't have noise at zero motor speed.
	//
	// Likewise, inject motion bias of the individual robot only when moving
	float xdot_noise = 0.0;
	float omega_noise = 0.0;
	float xdot_bias = 0.0;
	float omega_bias = 0.0;
	if (current_left_m || current_right_m)
	{
		xdot_noise  = rand_gaussian(settings->kbsigma_vnoise);
		omega_noise = rand_gaussian(settings->kbsigma_omeganoise);
		xdot_bias   = vbias;
		omega_bias  = omegabias;
	}
	// Omega and xdot are our desired angular and forward linear velocities
	xdot_goal   = ((current_left_m * k + current_right_m * k) / 2) + xdot_noise + xdot_bias;
	omega_goal  = ((current_left_m * k - current_right_m * k) / l) + omega_noise + omega_bias;
	// ydot is the effect of rotation off centre on the centre velocity
	ydot_goal   = o * omega_goal;


}


void Kilobot::update(float delta_t, float simtime)
{
	dt = delta_t;
	world_us_simtime = simtime * 1e6;
	// Update the kilobot tick counter
	//
	// Create an internal local time in us, adjusted to use the offset and bias
	// of this kilobot. This should mean that kilobots drift with respect to each other
	usec_t us_simtime = (simtime * 1e6 + clkoffset) * clkbias;
	kilo_ticks_real = us_simtime / master_tick_period;

	kilo_ticks = kilo_ticks_real;


	// Update our fake ModelPosition that the controller can see
	b2Vec2 p    = m_body->GetPosition();
	pos->pose.x = p.x;
	pos->pose.y = p.y;
	float a     = m_body->GetAngle();
	pos->pose.a = a;
	pos->fake_world.simtime = (usec_t) us_simtime;

	// Log the position if making trails
	if (settings->enableTrails)
	{
		trail.push_back(pos->pose);
	}

	// Handle message system
	check_messages();
	//printf("%s dt:%8f kilo_ticks:%8d\n", __PRETTY_FUNCTION__, dt, kilo_ticks);

	// Run the user code of the controller
	loop();

	// Leave pheromone trace in environment
	// Do the physics, this always happens every tick, regardless of loop schelduling
	// This consists of working out what forces to apply to get our desired goal velocities
	// and then applying them
	//
	// The kilobot physical model assumes that there is a constant velocity dependent drag.
	// This is called damping in the Box2D world. Our goal velocities should thus translate
	// directly into forces and torques via the damping constant
	//
	// Update the goal velocities based on the current motor settings
	// and the noise and bias values
	update_motion();

	// The goal velocities are all in the frame of the kilobot, transform into world frame
	float xd = xdot_goal * cos(a) - ydot_goal * sin(a);
	float yd = xdot_goal * sin(a) + ydot_goal * cos(a);

	float xf        = 0.0;
	float yf        = 0.0;
	float torque    = 0.0;
	b2Vec2 v        = m_body->GetLinearVelocity();
	float omega     = m_body->GetAngularVelocity();
	float m         = m_body->GetMass();
	float i         = m_body->GetInertia();

	// Calculate the forces using damping from current velocity counteracted
	// by the goal velocity
	//xd = yd = omega_goal = 0.0;
	xf      += (-v.x + xd) * settings->kblineardamp * m;
	yf      += (-v.y + yd) * settings->kblineardamp * m;
	torque  += (-omega + omega_goal) * settings->kbangulardamp * i;

	// Apply the force to the kilobot body
	m_body->ApplyForceToCenter(b2Vec2(xf, yf), true);
	m_body->ApplyTorque(torque, true);

	//printf("%3d xd:%10.6f yd:%10.6f ad:%10.6f xf:%10.6f yf:%10.6f tq:%10.6f\n",
	//kilo_uid, xd, yd, omega_goal, xf, yf, torque);
}

void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color, bool outline, bool solid)
{
	const float32 k_segments = 16.0f;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (solid)
		glColor4f(color.r, color.g, color.b, 1.0f);
	else
		glColor4f(color.r, color.g, color.b, 0.1f);
	glBegin(GL_TRIANGLE_FAN);
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertex2f(v.x, v.y);
		theta += k_increment;
	}
	glEnd();
	glDisable(GL_BLEND);

	if (outline)
	{
		theta = 0.0f;
		//glColor4f(color.r, color.g, color.b, 1.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		for (int32 i = 0; i < k_segments; ++i)
		{
			b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
			glVertex2f(v.x, v.y);
			theta += k_increment;
		}
		glEnd();

		b2Vec2 p = center + radius * axis;
		glBegin(GL_LINES);
		glVertex2f(center.x, center.y);
		glVertex2f(p.x, p.y);
		glEnd();
	}
}


void Kilobot::render()
{
	b2Vec2 mypos = m_body->GetPosition();
	//float a = m_body->GetAngle();
	const b2Transform &xf = m_body->GetTransform();
	//printf("##id:%5d x:%8.4f y:%8.4f\n", kilo_uid, mypos.x, mypos.y);


	DrawSolidCircle(mypos, settings->kbsenserad, b2Mul(xf, b2Vec2(1.0f, 0.0f)),
					b2Color(msgcolour.r, msgcolour.g, msgcolour.b), false, false);
	DrawSolidCircle(mypos, settings->kbdia/2, b2Mul(xf, b2Vec2(1.0f, 0.0f)), b2Color(led_r, led_g, led_b), true, true);


	//glColor3f(1,1,1);//white
	//glBegin(GL_LINES);
	//for (int i = 0; i < inrange_bots.size(); i++) {
	//    b2Vec2 theirpos = inrange_bots[i]->m_body->GetPosition();
	//    //printf("  id:%5d x:%8.4f y:%8.4f\n", inrange_bots[i]->kb_id, theirpos.x, theirpos.y);
	//    glVertex2f(mypos.x, mypos.y);
	//    glVertex2f(theirpos.x, theirpos.y);
	//}
	//glEnd();

	if (settings->enableTrails && (trail.size() > 1))
	{
		glColor3f(0,0,0);
		glBegin(GL_LINES);
		for(int i=0; i<trail.size()-1; i++)
		{
			glVertex2f(trail[i].x, trail[i].y);
			glVertex2f(trail[i+1].x, trail[i+1].y);
		}
		glEnd();
	}
}

void Kilobot::rendersensor()
{
	b2Vec2 mypos = m_body->GetPosition();
	//float a = m_body->GetAngle();
	const b2Transform &xf = m_body->GetTransform();
	//printf("##id:%5d x:%8.4f y:%8.4f\n", kilo_uid, mypos.x, mypos.y);

	DrawSolidCircle(mypos, settings->kbsenserad, b2Mul(xf, b2Vec2(1.0f, 0.0f)),
					b2Color(msgcolour.r, msgcolour.g, msgcolour.b), false, false);

}

void Kilobot::renderbody()
{
	b2Vec2 mypos = m_body->GetPosition();
	//float a = m_body->GetAngle();
	const b2Transform &xf = m_body->GetTransform();
	//printf("##id:%5d x:%8.4f y:%8.4f\n", kilo_uid, mypos.x, mypos.y);

	DrawSolidCircle(mypos, settings->kbdia/2, b2Mul(xf, b2Vec2(1.0f, 0.0f)), b2Color(led_r, led_g, led_b), true, true);


	//glColor3f(1,1,1);//white
	//glBegin(GL_LINES);
	//for (int i = 0; i < inrange_bots.size(); i++) {
	//    b2Vec2 theirpos = inrange_bots[i]->m_body->GetPosition();
	//    //printf("  id:%5d x:%8.4f y:%8.4f\n", inrange_bots[i]->kb_id, theirpos.x, theirpos.y);
	//    glVertex2f(mypos.x, mypos.y);
	//    glVertex2f(theirpos.x, theirpos.y);
	//}
	//glEnd();

	if (settings->enableTrails && (trail.size() > 1))
	{
		glColor3f(0,0,0);
		glBegin(GL_LINES);
		for(int i=0; i<trail.size()-1; i++)
		{
			glVertex2f(trail[i].x, trail[i].y);
			glVertex2f(trail[i+1].x, trail[i+1].y);
		}
		glEnd();
	}
}

bool get_contact(b2Contact *contact, Kilobot *&sender, Kilobot *&receiver)
{
	b2Fixture *fa = contact->GetFixtureA();
	b2Fixture *fb = contact->GetFixtureB();
	bool sa = fa->IsSensor();
	bool sb = fb->IsSensor();
	if (!(sa ^ sb))
		return false;
	Kilobot *ka = static_cast<Kilobot*>(fa->GetBody()->GetUserData());
	Kilobot *kb = static_cast<Kilobot*>(fb->GetBody()->GetUserData());
	if (!ka || !kb)
		// If the user data is null, the objct is not a kilobot
		return false;
	if (sa)
	{
		sender      = ka;
		receiver    = kb;
	}
	else
	{
		sender      = kb;
		receiver    = ka;
	}
	return true;
}

void KBContactListener::BeginContact(b2Contact *contact)
{
	Kilobot *sender;
	Kilobot *receiver;
	if (get_contact(contact, sender, receiver))
		sender->acquired(receiver);
}

void KBContactListener::EndContact(b2Contact *contact)
{
	Kilobot *sender;
	Kilobot *receiver;
	if (get_contact(contact, sender, receiver))
		sender->lost(receiver);
}

