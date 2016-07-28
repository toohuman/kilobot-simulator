// Copyright Simon Jones 2015

#ifndef KILOLIB_H
#define KILOLIB_H

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include "freeglut/freeglut.h"
#endif


#include <random>
#include <queue>
#include <memory>
#include <iostream>
#include <string>
#include <cstdio>


#include "../Framework/Test.h"
#include "../Framework/Render.h"
#include "kiloworld.h"

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f


typedef uint64_t usec_t;


// From http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args){
	size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size);
}


class ModelStigmergy
{
public:
	struct colour_t {
		GLfloat r;
		GLfloat g;
		GLfloat b;
		GLfloat a;
		colour_t(GLfloat _r,GLfloat _g,GLfloat _b, GLfloat _a) : r(_r),g(_g),b(_b),a(_a){}
	};
};

struct Pose
{
	float x;
	float y;
	float a;
};

class Color
{
public:
	float r,g,b,a;

	Color( float r, float g, float b, float a=1.0 ){}

	Color(){}

	// Set fraction in cubehelix palette
	Color(float f)
	{
		int fi = f * 255;
		fi = fi < 0 ? 0 : fi > 255 ? 255 : fi;
		r = col[fi][0];
		g = col[fi][1];
		b = col[fi][2];
	}

	// cube1 palette from
	// https://mycarta.wordpress.com/2013/03/06/perceptual-rainbow-palette-the-goodies/
	float col[256][3] = {
		{0.454901961, 0, 0.505882353},
		{0.462745098, 0, 0.521568627},
		{0.466666667, 0, 0.533333333},
		{0.470588235, 0, 0.545098039},
		{0.474509804, 0, 0.556862745},
		{0.478431373, 0, 0.568627451},
		{0.482352941, 0, 0.580392157},
		{0.48627451, 0, 0.592156863},
		{0.490196078, 0, 0.603921569},
		{0.494117647, 0, 0.615686275},
		{0.498039216, 0.003921569, 0.62745098},
		{0.501960784, 0.007843137, 0.639215686},
		{0.501960784, 0.015686275, 0.650980392},
		{0.505882353, 0.023529412, 0.662745098},
		{0.509803922, 0.031372549, 0.674509804},
		{0.51372549, 0.043137255, 0.690196078},
		{0.51372549, 0.050980392, 0.701960784},
		{0.517647059, 0.062745098, 0.71372549},
		{0.521568627, 0.070588235, 0.725490196},
		{0.521568627, 0.078431373, 0.737254902},
		{0.521568627, 0.090196078, 0.749019608},
		{0.521568627, 0.101960784, 0.760784314},
		{0.521568627, 0.11372549, 0.77254902},
		{0.521568627, 0.125490196, 0.784313725},
		{0.521568627, 0.137254902, 0.796078431},
		{0.521568627, 0.149019608, 0.803921569},
		{0.517647059, 0.160784314, 0.815686275},
		{0.51372549, 0.17254902, 0.823529412},
		{0.51372549, 0.184313725, 0.831372549},
		{0.509803922, 0.196078431, 0.839215686},
		{0.505882353, 0.207843137, 0.847058824},
		{0.501960784, 0.219607843, 0.854901961},
		{0.501960784, 0.231372549, 0.862745098},
		{0.498039216, 0.243137255, 0.870588235},
		{0.494117647, 0.250980392, 0.878431373},
		{0.494117647, 0.258823529, 0.88627451},
		{0.490196078, 0.270588235, 0.894117647},
		{0.48627451, 0.278431373, 0.905882353},
		{0.48627451, 0.28627451, 0.91372549},
		{0.482352941, 0.294117647, 0.921568627},
		{0.474509804, 0.301960784, 0.929411765},
		{0.470588235, 0.309803922, 0.941176471},
		{0.466666667, 0.317647059, 0.949019608},
		{0.462745098, 0.325490196, 0.956862745},
		{0.458823529, 0.333333333, 0.964705882},
		{0.454901961, 0.341176471, 0.968627451},
		{0.450980392, 0.349019608, 0.976470588},
		{0.447058824, 0.356862745, 0.980392157},
		{0.443137255, 0.364705882, 0.988235294},
		{0.439215686, 0.368627451, 0.988235294},
		{0.435294118, 0.376470588, 0.992156863},
		{0.431372549, 0.384313725, 0.992156863},
		{0.42745098, 0.392156863, 0.992156863},
		{0.423529412, 0.4, 0.992156863},
		{0.419607843, 0.407843137, 0.992156863},
		{0.415686275, 0.419607843, 0.988235294},
		{0.411764706, 0.42745098, 0.988235294},
		{0.407843137, 0.435294118, 0.984313725},
		{0.403921569, 0.443137255, 0.984313725},
		{0.4, 0.450980392, 0.980392157},
		{0.4, 0.458823529, 0.976470588},
		{0.396078431, 0.466666667, 0.97254902},
		{0.392156863, 0.474509804, 0.968627451},
		{0.388235294, 0.482352941, 0.968627451},
		{0.384313725, 0.490196078, 0.964705882},
		{0.380392157, 0.494117647, 0.960784314},
		{0.376470588, 0.501960784, 0.956862745},
		{0.37254902, 0.509803922, 0.952941176},
		{0.368627451, 0.517647059, 0.949019608},
		{0.364705882, 0.525490196, 0.945098039},
		{0.360784314, 0.529411765, 0.941176471},
		{0.356862745, 0.537254902, 0.937254902},
		{0.352941176, 0.541176471, 0.933333333},
		{0.349019608, 0.549019608, 0.925490196},
		{0.345098039, 0.556862745, 0.921568627},
		{0.341176471, 0.560784314, 0.917647059},
		{0.337254902, 0.568627451, 0.909803922},
		{0.333333333, 0.57254902, 0.901960784},
		{0.325490196, 0.580392157, 0.898039216},
		{0.321568627, 0.584313725, 0.890196078},
		{0.317647059, 0.592156863, 0.88627451},
		{0.31372549, 0.596078431, 0.878431373},
		{0.309803922, 0.6, 0.870588235},
		{0.305882353, 0.607843137, 0.866666667},
		{0.301960784, 0.611764706, 0.858823529},
		{0.298039216, 0.619607843, 0.850980392},
		{0.290196078, 0.623529412, 0.843137255},
		{0.28627451, 0.631372549, 0.839215686},
		{0.282352941, 0.635294118, 0.831372549},
		{0.274509804, 0.643137255, 0.823529412},
		{0.270588235, 0.647058824, 0.815686275},
		{0.262745098, 0.654901961, 0.807843137},
		{0.258823529, 0.658823529, 0.8},
		{0.250980392, 0.666666667, 0.792156863},
		{0.247058824, 0.670588235, 0.788235294},
		{0.239215686, 0.678431373, 0.780392157},
		{0.235294118, 0.682352941, 0.77254902},
		{0.231372549, 0.68627451, 0.760784314},
		{0.22745098, 0.694117647, 0.752941176},
		{0.223529412, 0.698039216, 0.745098039},
		{0.223529412, 0.701960784, 0.737254902},
		{0.219607843, 0.709803922, 0.729411765},
		{0.219607843, 0.71372549, 0.721568627},
		{0.219607843, 0.717647059, 0.71372549},
		{0.219607843, 0.721568627, 0.705882353},
		{0.223529412, 0.725490196, 0.698039216},
		{0.223529412, 0.733333333, 0.690196078},
		{0.22745098, 0.737254902, 0.682352941},
		{0.22745098, 0.741176471, 0.670588235},
		{0.231372549, 0.745098039, 0.662745098},
		{0.235294118, 0.749019608, 0.654901961},
		{0.239215686, 0.752941176, 0.647058824},
		{0.243137255, 0.756862745, 0.639215686},
		{0.243137255, 0.760784314, 0.62745098},
		{0.247058824, 0.764705882, 0.619607843},
		{0.250980392, 0.768627451, 0.611764706},
		{0.254901961, 0.77254902, 0.603921569},
		{0.258823529, 0.776470588, 0.592156863},
		{0.262745098, 0.780392157, 0.584313725},
		{0.262745098, 0.784313725, 0.576470588},
		{0.266666667, 0.788235294, 0.568627451},
		{0.270588235, 0.792156863, 0.556862745},
		{0.270588235, 0.796078431, 0.549019608},
		{0.274509804, 0.796078431, 0.541176471},
		{0.278431373, 0.8, 0.529411765},
		{0.278431373, 0.803921569, 0.521568627},
		{0.282352941, 0.807843137, 0.51372549},
		{0.28627451, 0.811764706, 0.505882353},
		{0.28627451, 0.815686275, 0.494117647},
		{0.290196078, 0.815686275, 0.48627451},
		{0.294117647, 0.819607843, 0.478431373},
		{0.294117647, 0.823529412, 0.466666667},
		{0.298039216, 0.82745098, 0.458823529},
		{0.301960784, 0.82745098, 0.450980392},
		{0.301960784, 0.831372549, 0.443137255},
		{0.305882353, 0.835294118, 0.435294118},
		{0.309803922, 0.839215686, 0.423529412},
		{0.31372549, 0.843137255, 0.415686275},
		{0.31372549, 0.847058824, 0.407843137},
		{0.317647059, 0.847058824, 0.396078431},
		{0.321568627, 0.850980392, 0.384313725},
		{0.321568627, 0.854901961, 0.376470588},
		{0.325490196, 0.858823529, 0.364705882},
		{0.329411765, 0.862745098, 0.352941176},
		{0.329411765, 0.866666667, 0.341176471},
		{0.333333333, 0.870588235, 0.333333333},
		{0.337254902, 0.870588235, 0.321568627},
		{0.341176471, 0.874509804, 0.31372549},
		{0.341176471, 0.878431373, 0.305882353},
		{0.345098039, 0.882352941, 0.298039216},
		{0.349019608, 0.882352941, 0.294117647},
		{0.352941176, 0.88627451, 0.290196078},
		{0.356862745, 0.890196078, 0.28627451},
		{0.360784314, 0.890196078, 0.28627451},
		{0.368627451, 0.894117647, 0.28627451},
		{0.37254902, 0.898039216, 0.28627451},
		{0.380392157, 0.898039216, 0.28627451},
		{0.388235294, 0.901960784, 0.290196078},
		{0.396078431, 0.901960784, 0.290196078},
		{0.407843137, 0.905882353, 0.290196078},
		{0.415686275, 0.905882353, 0.294117647},
		{0.42745098, 0.909803922, 0.294117647},
		{0.435294118, 0.909803922, 0.298039216},
		{0.447058824, 0.91372549, 0.298039216},
		{0.458823529, 0.91372549, 0.301960784},
		{0.470588235, 0.917647059, 0.305882353},
		{0.478431373, 0.917647059, 0.305882353},
		{0.490196078, 0.917647059, 0.309803922},
		{0.501960784, 0.921568627, 0.309803922},
		{0.509803922, 0.921568627, 0.31372549},
		{0.521568627, 0.921568627, 0.31372549},
		{0.529411765, 0.921568627, 0.31372549},
		{0.537254902, 0.921568627, 0.317647059},
		{0.549019608, 0.921568627, 0.317647059},
		{0.556862745, 0.921568627, 0.321568627},
		{0.568627451, 0.921568627, 0.321568627},
		{0.576470588, 0.921568627, 0.321568627},
		{0.588235294, 0.925490196, 0.325490196},
		{0.596078431, 0.925490196, 0.325490196},
		{0.607843137, 0.925490196, 0.329411765},
		{0.615686275, 0.925490196, 0.329411765},
		{0.62745098, 0.925490196, 0.329411765},
		{0.635294118, 0.925490196, 0.333333333},
		{0.647058824, 0.925490196, 0.333333333},
		{0.654901961, 0.925490196, 0.333333333},
		{0.662745098, 0.925490196, 0.337254902},
		{0.670588235, 0.925490196, 0.337254902},
		{0.678431373, 0.925490196, 0.337254902},
		{0.68627451, 0.925490196, 0.341176471},
		{0.694117647, 0.925490196, 0.341176471},
		{0.705882353, 0.925490196, 0.341176471},
		{0.71372549, 0.925490196, 0.341176471},
		{0.721568627, 0.925490196, 0.345098039},
		{0.725490196, 0.925490196, 0.345098039},
		{0.733333333, 0.925490196, 0.345098039},
		{0.741176471, 0.925490196, 0.345098039},
		{0.749019608, 0.925490196, 0.349019608},
		{0.756862745, 0.925490196, 0.349019608},
		{0.764705882, 0.925490196, 0.349019608},
		{0.768627451, 0.925490196, 0.349019608},
		{0.776470588, 0.925490196, 0.349019608},
		{0.784313725, 0.925490196, 0.349019608},
		{0.788235294, 0.925490196, 0.352941176},
		{0.796078431, 0.925490196, 0.352941176},
		{0.8, 0.925490196, 0.352941176},
		{0.803921569, 0.925490196, 0.352941176},
		{0.811764706, 0.925490196, 0.352941176},
		{0.815686275, 0.921568627, 0.352941176},
		{0.819607843, 0.917647059, 0.356862745},
		{0.823529412, 0.917647059, 0.356862745},
		{0.82745098, 0.91372549, 0.356862745},
		{0.831372549, 0.909803922, 0.356862745},
		{0.835294118, 0.901960784, 0.356862745},
		{0.839215686, 0.898039216, 0.356862745},
		{0.843137255, 0.894117647, 0.356862745},
		{0.847058824, 0.88627451, 0.356862745},
		{0.850980392, 0.882352941, 0.356862745},
		{0.854901961, 0.878431373, 0.360784314},
		{0.858823529, 0.870588235, 0.360784314},
		{0.862745098, 0.866666667, 0.360784314},
		{0.866666667, 0.858823529, 0.360784314},
		{0.870588235, 0.854901961, 0.360784314},
		{0.874509804, 0.850980392, 0.360784314},
		{0.878431373, 0.843137255, 0.360784314},
		{0.88627451, 0.839215686, 0.360784314},
		{0.890196078, 0.835294118, 0.364705882},
		{0.898039216, 0.82745098, 0.364705882},
		{0.901960784, 0.823529412, 0.364705882},
		{0.905882353, 0.815686275, 0.364705882},
		{0.91372549, 0.807843137, 0.364705882},
		{0.917647059, 0.803921569, 0.364705882},
		{0.925490196, 0.796078431, 0.364705882},
		{0.929411765, 0.788235294, 0.368627451},
		{0.933333333, 0.784313725, 0.368627451},
		{0.937254902, 0.776470588, 0.368627451},
		{0.941176471, 0.768627451, 0.368627451},
		{0.945098039, 0.760784314, 0.368627451},
		{0.949019608, 0.752941176, 0.368627451},
		{0.952941176, 0.745098039, 0.368627451},
		{0.952941176, 0.737254902, 0.368627451},
		{0.956862745, 0.729411765, 0.368627451},
		{0.956862745, 0.721568627, 0.368627451},
		{0.960784314, 0.71372549, 0.368627451},
		{0.960784314, 0.705882353, 0.368627451},
		{0.964705882, 0.698039216, 0.368627451},
		{0.964705882, 0.690196078, 0.364705882},
		{0.968627451, 0.678431373, 0.364705882},
		{0.968627451, 0.670588235, 0.364705882},
		{0.97254902, 0.658823529, 0.364705882},
		{0.97254902, 0.650980392, 0.364705882},
		{0.97254902, 0.639215686, 0.360784314},
		{0.976470588, 0.631372549, 0.360784314},
		{0.976470588, 0.619607843, 0.360784314},
		{0.976470588, 0.611764706, 0.360784314},
		{0.976470588, 0.6, 0.356862745},
		{0.976470588, 0.588235294, 0.356862745}};


	bool operator!=( const Color& other ) const;
	bool operator==( const Color& other ) const;
	static Color RandomColor();
	void Print( const char* prefix ) const;
	//void GLSet( void ) { glColor4f( r,g,b,a ); }
};





class CtrlArgs
{
public:
	std::string worldfile;
	std::string cmdline;

	CtrlArgs( std::string w, std::string c ) : worldfile(w), cmdline(c) {}
};


class ModelPosition
{
// Fake ModelPosition so that we can use the controller code as unchanged as possible from
// the Stage controller
public:
	ModelPosition() {}

	b2World     *world;
	Pose        pose;
	Color       color;
	Pose GetPose()
	{
		return pose;
	}
	Pose GetGlobalPose()
	{
		return pose;
	}
	struct World
	{
		usec_t simtime;
		usec_t SimTimeNow() {return simtime;}
	} fake_world;
	void SetColor(Color col) {color = col;}
	World *GetWorld() {return &fake_world;}
	std::string token;
	const char *Token() {return token.c_str();}
	// hold ref to kiloworld so we can access
	Kilolib::Kiloworld *kworld;
};


namespace Kilolib
{

	// Forward declarations
	class Kilobot;




	enum entityCategory
	{
		KILOBOT         = 0x0001,
		MESSAGE         = 0x0002,
	};



	class Kilobot
	{
	public:
		static int ids;
		Kilobot(ModelPosition *_pos, Settings *_settings)
		:
			pos                 (_pos),
			m_world             (_pos->world),
			settings            (_settings),
			omega_goal          (0.0),
			xdot_goal           (0.0),
			ydot_goal           (0.0),
			ambient             (ModelStigmergy::colour_t(0,0,0,0)),
			led_r               (1.0),
			led_g               (1.0),
			led_b               (1.0),
			msgcolour           (0.5),
			current_left_m      (0),
			current_right_m     (0),
			timer0_freq         (8e6/1024),
			master_tick_period  (32768),
			kilo_tx_period      (3906),
			message_period      (kilo_tx_period/timer0_freq),
			last_message        (0),
			kilo_straight_left  (70),
			kilo_straight_right (70),
			kilo_turn_left      (70),
			kilo_turn_right     (70)
		{
			// Generate a unique ID, pre-increment so that first ID is 1
			kilo_uid = ++ids;
			make_kilobot(pos->GetPose().x, pos->GetPose().y, pos->GetPose().a);

			// Give the robot a name
			pos->token = string_format("kilobot:%d", kilo_uid);

			// Seed the random number generator with the unique ID
			gen.seed(settings->seed + kilo_uid);

			kilo_ticks_real = 0;//rand(0, 100);
			kilo_ticks      = kilo_ticks_real;

			// Give the kilobot its motion biasses
			vbias           = rand_gaussian(settings->kbsigma_vbias);
			omegabias       = rand_gaussian(settings->kbsigma_omegabias);

			// Each kilobot has a slightly different clock frequency,
			// this puts 95% within +- 1%
			clkbias         = 1 + rand_gaussian(0.005);
			// Each clock starts at slightly different time, within 0.2s
			clkoffset       = rand_intrange(0, 200000);

		}
		ModelPosition   *pos;
		b2Body          *m_body;
		b2World         *m_world;
		Settings        *settings;
		usec_t          world_us_simtime;


		// Functions to maintain the list of other bots we are in range of
		void acquired(Kilobot *r)
		{
			//printf("adding   %d\n", r->kb_id);
			inrange_bots.push_back(r);
		}
		void lost(Kilobot *r)
		{
			//printf("removing %d\n", r->kb_id);
			inrange_bots.erase(std::find(inrange_bots.begin(), inrange_bots.end(), r));
		}

		virtual float metric() {return 0.0; }




		void render();
		void rendersensor();
		void renderbody();
		void update_motion();
		void update(float delta_t, float simtime);

	private:
		void make_kilobot(float xp, float yp, float th);
		void check_messages();

		float   dt;
		double  simtime;


		// Each kilobot has its own random number generator
		std::mt19937  gen;

		// Pointers to all kilobots within message range
		std::vector<Kilobot*>       inrange_bots;

		// Controller velocity goal, set by set_motor() based on two-wheel kinematics
		float                      omega_goal;
		float                      xdot_goal;
		float                      ydot_goal;
		// Pheromone strength
		float                      pheromone;
		ModelStigmergy::colour_t    ambient;

		// Vector of locations to plot trails
		std::vector<Pose>           trail;

		// LED colour
		float led_r, led_g, led_b;
		// Message colour
		Color msgcolour;

		// Current motor values
		int                         current_left_m;
		int                         current_right_m;
		// Motion bias
		float                       vbias;
		float                       omegabias;
		// Clock frequency bias
		float                       clkbias;
		// Clock offset in usec
		usec_t                      clkoffset;




	protected:
		int   rand_intrange(int low, int high)
		{
			std::uniform_int_distribution<>   dist(low, high);
			int r = dist(gen);
			//printf("int   %10s %10i\n",pos->Token(), r);
			return r;
		}
		float rand_gaussian(float sigma)
		{
			std::normal_distribution<float> dist(0.0, sigma);
			float r = dist(gen);
			//printf("float %10s %10f %10f\n",pos->Token(), sigma, r);
			return r;
		}
		float rand_realrange(float low, float high)
		{
			std::uniform_real_distribution<>   dist(low, high);
			float r = dist(gen);
			//printf("int   %10s %10i\n",pos->Token(), r);
			return r;
		}

		//------------------------------------------------------------
		// Kilobot API
		//------------------------------------------------------------

		// These methods are declared abstract virtual and must be defined
		// in the KBController class
		virtual void setup()    = 0;
		virtual void loop()     = 0;

		// This method will be called at the end of the simulation, overload
		// it to eg output stats
		virtual void finish() {
			//printf("%s\n",__PRETTY_FUNCTION__);
		}

		// Message container type
		typedef struct {
			uint8_t data[9]; ///< message payload.
			uint8_t type;    ///< message type.
			uint16_t crc;    ///< message crc.
		} message_t;

		// Distance measurement
		typedef struct {
			int16_t low_gain;
			int16_t high_gain;
		} distance_measurement_t;

		// Message queue - not actually part of the API
		struct m_event_t
		{
			message_t               m;
			distance_measurement_t  d;
			std::string             s;
		};
		typedef std::queue<m_event_t>   m_queue_t;
		m_queue_t                       message_queue;

		// Message stuff
		typedef enum {
			NORMAL = 0,
			GPS,
			BOOT = 0x80,
			BOOTPGM_PAGE,
			BOOTPGM_SIZE,
			RESET,
			SLEEP,
			WAKEUP,
			CHARGE,
			VOLTAGE,
			RUN,
			READUID,
			CALIB,
		} message_type_t;

		// Declared above so available for message event queue
//        // Message container type
//        typedef struct {
//            uint8_t data[9]; ///< message payload.
//            uint8_t type;    ///< message type.
//            uint16_t crc;    ///< message crc.
//        } message_t;
//        // Distance measurement
//        typedef struct {
//            int16_t low_gain;
//            int16_t high_gain;
//        } distance_measurement_t;

		// 16 bit CRC function from Atmel
		uint16_t crc_ccitt_update (uint16_t crc, uint8_t data)
		{
			data ^= crc&0xff;
			data ^= data << 4;

			return ((((uint16_t)data << 8) | ((crc>>8)&0xff)) ^ (uint8_t)(data >> 4)
					^ ((uint16_t)data << 3));
		}
		uint16_t message_crc(const message_t *msg) {return 0;}


		distance_measurement_t  dist;
		message_t               msg;

		// Message system callback types
		typedef void        (Kilobot::*message_rx_t) (message_t *, distance_measurement_t *d);
		typedef message_t  *(Kilobot::*message_tx_t)(void);
		typedef void        (Kilobot::*message_tx_success_t)(void);
		// Dummy callbacks
		void                message_rx_dummy(message_t *m, distance_measurement_t *d) { }
		message_t          *message_tx_dummy() { return NULL; }
		void                message_tx_success_dummy() {}

		// Master tick period in usec, this is what Timer 0 is set to interrupt at
		double              timer0_freq;//         = 8e6/1024;
		const double        master_tick_period;//  = 32768;
		// Number of kilobot ticks since reset. Each tick is approximately every 30 ms,
		// so since the simulation tick defaults to 100ms, it will appear to increase in
		// chunks. This may affect the behaviour of some software..
		uint32_t            kilo_ticks;
		float               kilo_ticks_real;
		const uint32_t      kilo_tx_period;//      = 3906;
		const double        message_period;//      = kilo_tx_period / timer0_freq;
		uint32_t            last_message;
		// Unique ID for this kilobot
		uint16_t            kilo_uid;



		// Calibrated values for the motors
		int kilo_straight_left;//    = 50;
		int kilo_straight_right;//   = 50;
		int kilo_turn_left;//        = 40;
		int kilo_turn_right;//       = 40;

		// Message system callbacks
		message_rx_t            kilo_message_rx;
		message_tx_t            kilo_message_tx;
		message_tx_success_t    kilo_message_tx_success;

		//-------------------Not part of kilobot API------------------------------
		// Access to the message rx callback for senders
		//void                call_rx_callback(message_t *m, distance_measurement_t *d)
		//{
		//    (kilo_message_rx)(m,d);
		//}
		//------------------------------------------------------------------------

		// Received message and distance
		message_t               rx_msg;
		distance_measurement_t  rx_dist;

		uint8_t estimate_distance(const distance_measurement_t *dist)
		{
			return dist->low_gain;
		}

		uint8_t rand_hard()             {return rand_intrange(0,255);}
		uint8_t rand_soft()             {return rand_intrange(0,255);}

		void    rand_seed(uint16_t s)    {
			gen.seed(s);
		}

		int16_t get_voltage()           {return 0;}
		int16_t get_temperature()       {return 0;}

		typedef ModelStigmergy::colour_t colour_t;
		//-------------------------------------------------
		int16_t get_environment()
		{
			// THIS IS NOT PART OF THE STANDARD API!!
			// To be implemented using some sort of modulation
			// scheme on the ambient light sensing
			//
			// Now actually implemented based on the modulation pattern
			// of the DLP projector

			int env = pos->kworld->get_environment(pos->pose.x, pos->pose.y);
			//printf("x:%10f y:%10f rt:%2i\n",pos->pose.x, pos->pose.y, env);
			return env;
		}
		void set_color_msg(float c)
		{
			msgcolour = Color(c);
		}
		//-------------------------------------------------


		void set_motors(int left_m, int right_m);


// We are not going to support delay, it needs nasty hacks like coroutines
// and the restriction the lack of it places on coding the kilobot is minimal.
// delay() is horrible anyway
//        void delay(int delay)
//        {
//            // Delay for approximately delay milliseconds
//            // Since the tick defaults to 100ms, this is necessarily very approximate.
//            // We ensure that there is at least one ticks delay, this means that code that
//            // does things like flash the led briefly does still make a visible difference
//            // on the GUI
//            delay_ticks = int(delay * 0.001 / dt);
//            if (delay_ticks < 1) delay_ticks = 1;
//            while (delay_ticks > 0)
//            {
//                delay_ticks--;
//                //printf("About to detach.. delay_ticks %d\n", delay_ticks);
//                Detach();
//            }
//            //printf("Resuming after delay..\n");
//        }

		void spinup_motors() {}

// Macro for easy use of set_color
#define RGB(r,g,b) (r&3)|(((g&3)<<2))|((b&3)<<4)

		void set_color(uint8_t rgb)
		{
			led_r = (rgb&0x3)/3.0;
			led_g = ((rgb>>2)&0x3)/3.0;
			led_b = ((rgb>>4)&0x3)/3.0;
		}
		void set_colorf(float c)
		{
			Color col(c);
			led_r = col.r;
			led_g = col.g;
			led_b = col.b;
		}

	};
};



#endif
