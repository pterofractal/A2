#include "viewer.hpp"
#include <iostream>
#include <sstream>
#include <GL/gl.h>
#include <GL/glu.h>
#include "draw.hpp"
#include <math.h>

#define DEFAULT_NEAR 6
#define DEFAULT_FAR 16
#define DEFAULT_FOV 31.6
void Viewer::print (Matrix4x4 mat)
{
	for (int i = 0;i<4;i++)
	{
		for (int j = 0;j<4;j++)
		{	
			std::cout<< mat[i][j] << "\t";
		}
		std::cout << std::endl;
	}
}
void Viewer::print (Point3D pt)
{
	for (int j = 0;j<3;j++)
	{	
		std::cout<< pt[j] << "\t";
	}
	std::cout << std::endl;
}
void Viewer::print (Vector3D vec)
{
	for (int j = 0;j<3;j++)
	{	
		std::cout<< vec[j] << "\t";
	}
	std::cout << std::endl;
}
Viewer::Viewer()
{
  Glib::RefPtr<Gdk::GL::Config> glconfig;

  // Ask for an OpenGL Setup with
  //  - red, green and blue component colour
  //  - a depth buffer to avoid things overlapping wrongly
  //  - double-buffered rendering to avoid tearing/flickering
  glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB |
                                     Gdk::GL::MODE_DEPTH |
                                     Gdk::GL::MODE_DOUBLE);
  if (glconfig == 0) {
    // If we can't get this configuration, die
    std::cerr << "Unable to setup OpenGL Configuration!" << std::endl;
    abort();
  }

  // Accept the configuration
  set_gl_capability(glconfig);

  // Register the fact that we want to receive these events
  add_events(Gdk::BUTTON1_MOTION_MASK    |
             Gdk::BUTTON2_MOTION_MASK    |
             Gdk::BUTTON3_MOTION_MASK    |
             Gdk::BUTTON_PRESS_MASK      | 
             Gdk::BUTTON_RELEASE_MASK    |
             Gdk::VISIBILITY_NOTIFY_MASK);
  
  pointsOfCube = new Point3D[8];
  	for (int i = 0;i<8;i++)
	{
		if (i%2 == 1)
			pointsOfCube[i][0] = -1;
		else
			pointsOfCube[i][0] = 1;		
	}
	for (int i = 0;i<8;i++)
	{
		pointsOfCube[i][1] = 1;
		if ((i > 1 && i < 4) || i > 5)
			pointsOfCube[i][1] = -1;
	}		
	for (int i = 0;i<8;i++)
	{
		pointsOfCube[i][2] = 1;
		if (i>3)
			pointsOfCube[i][2] = -1;
	}
	
	tempPoints = new Point3D[8];
	for (int i = 0;i<8;i++)
	{
		//pointsOfCube[i][3] = 0.0;
		for (int j = 0; j<3;j++)
			tempPoints[i][j] = pointsOfCube[i][j];
	}
	n = DEFAULT_NEAR;
	f = DEFAULT_FAR;
	angle = DEFAULT_FOV;
	
	// Initialize Modelling Matrix
	for (int i = 0;i<4;i++)
	{
		for (int j = 0;j<4;j++)
		{
			if (i == j)
				m_M[i][j] = 1;
			else
				m_M[i][j] = 0;
		}
	}
	
	// Initialize Modelling Transformation
	for (int i = 0;i<4;i++)
	{
		for (int j = 0;j<4;j++)
		{
			m_T[i][j] = 0;
		}
	}
	
	lookFrom[0] = 0;
	lookFrom[1] = 0;
	lookFrom[2] = 0;
		
	up[0] = 0;
	up[1] = 1;
	up[2] = 0;
	
	lookAt[0] = 0;
	lookAt[1] = 0;
	lookAt[2] = 1;
}

Viewer::~Viewer()
{
  // Nothing to do here right now.
}

void Viewer::invalidate()
{
  // Force a rerender
  Gtk::Allocation allocation = get_allocation();
  get_window()->invalidate_rect( allocation, false);
}

void Viewer::set_perspective(double fov, double aspect,
                             double near, double far)
{
  // Fill me in!
	//std::cout << "FOV: " << fov << ",\tnear: " << near << ",\tfar: " << far << std::endl; 
	m_proj[0][0] = 1/(tan(fov / 2));
	m_proj[0][0] /= aspect;
	
	for (int i = 1;i<4;i++)
	{
		m_proj[0][i] = 0;
		m_proj[i][0] = 0;
	}
	
	m_proj[1][1] = 1/(tan(fov/2));
	//m_proj[2][2] = (near + far ) / near;
	//m_proj[2][3] = -1 * far;
	//m_proj[3][2] = 1 / near;
	m_proj[2][2] = (far + near) / (far - near);
	m_proj[2][3] = (-2 * far * near)/(far-near);
	m_proj[3][2] = 1;
	m_proj[3][3] = 0;
	for (int i = 2; i < 4; i++)
	{
		m_proj[1][i] = 0;
		m_proj[i][1] = 0;
	}
}

void Viewer::reset_view()
{
  // Fill me in!
}

void Viewer::on_realize()
{
  // Do some OpenGL setup.
  // First, let the base class do whatever it needs to
  Gtk::GL::DrawingArea::on_realize();
  
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();
  
  if (!gldrawable)
    return;

  if (!gldrawable->gl_begin(get_gl_context()))
    return;

  gldrawable->gl_end();
}

bool Viewer::on_expose_event(GdkEventExpose* event)
{
	Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();
	
	if (!gldrawable) 
		return false;
	
	if (!gldrawable->gl_begin(get_gl_context()))
		return false;

	double aspectRatio = get_width() / get_height();
	for (int i = 0;i<8;i++)
	{
		for (int j = 0; j<3;j++)
			tempPoints[i][j] = pointsOfCube[i][j];
	}
	
	double width, height;
	width = get_width();
	height = get_height();
	// Here is where your drawing code should go.
	draw_init(get_width(), get_height());
	
	// Init projection matrix
	set_perspective(angle, aspectRatio, n, f);	
	m_T[0][0] = get_width() / 10;
	m_T[1][1] = get_height() / 10;
	m_T[2][2] = 1;//get_width()/4;
	m_T[3][3] = 1;
	m_T[0][3] = get_width() / 2;
	m_T[1][3] = get_height() / 2;
	m_T[2][3] = 1;
	
/*	std::cout << "Transform Matrix " << std::endl;
	print(m_T);
	
	std::cout << "Modelling Matrix " << std::endl;
	print(m_M);
	
	std::cout << "Viewing Matrix " << std::endl;
	print(m_V);
	
	std::cout << "Projection Matrix " << std::endl;
	print(m_proj);*/
	
	for (int i = 0;i<8;i++)
	{	
		tempPoints[i] = m_M * tempPoints[i];
		tempPoints[i] = m_V * tempPoints[i];
		tempPoints[i] = m_proj * tempPoints[i];		
	}
		
	for (int i = 0;i<8;i++)
	{
		tempPoints[i][0] /= tempPoints[i][2];
		tempPoints[i][1] /= tempPoints[i][2];
		tempPoints[i][2] /= tempPoints[i][2];
		tempPoints[i][0] *= n;
		tempPoints[i][1] *= n;
		tempPoints[i][2] *= n;
		tempPoints[i] = m_T * tempPoints[i];
	}
	
	// for (int i = 0;i<8;i++)
	// 	{
	// 		std::cout << "Point " << i << ": ";
	// 		print(tempPoints[i]);
	// 	}
	set_colour(Colour(0.1, 0.1, 0.1));

	/*
	0	1	1	1
	1	-1	1	1
	2	1	-1	1
	3	-1	-1	1
	4	1	1	-1
	5	-1	1	-1
	6	1	-1	-1
	7	-1	-1	-1
	*/

	draw_line(	Point2D(tempPoints[0][0], tempPoints[0][1]),
				Point2D(tempPoints[1][0], tempPoints[1][1]));
	
	draw_line(	Point2D(tempPoints[0][0], tempPoints[0][1]),
				Point2D(tempPoints[2][0], tempPoints[2][1]));
	
	draw_line(	Point2D(tempPoints[1][0], tempPoints[1][1]),
				Point2D(tempPoints[3][0], tempPoints[3][1]));
	
	draw_line(	Point2D(tempPoints[2][0], tempPoints[2][1]),
				Point2D(tempPoints[3][0], tempPoints[3][1]));
	
	set_colour(Colour(1, 0, 0));

	draw_line(	Point2D(tempPoints[4][0], tempPoints[4][1]),
				Point2D(tempPoints[5][0], tempPoints[5][1]));
	
	draw_line(	Point2D(tempPoints[4][0], tempPoints[4][1]),
				Point2D(tempPoints[6][0], tempPoints[6][1]));
	
	draw_line(	Point2D(tempPoints[5][0], tempPoints[5][1]),
				Point2D(tempPoints[7][0], tempPoints[7][1]));
	
	draw_line(	Point2D(tempPoints[6][0], tempPoints[6][1]),
				Point2D(tempPoints[7][0], tempPoints[7][1]));
	
	set_colour(Colour(0.1, 0.1, 1));
	
	for (int i = 0; i < 4;i++)
	{
		draw_line(	Point2D(tempPoints[i][0], tempPoints[i][1]),
					Point2D(tempPoints[i+4][0], tempPoints[i+4][1]));
	}
	
	draw_complete();
			
	// Swap the contents of the front and back buffers so we see what we
	// just drew. This should only be done if double buffering is enabled.
	gldrawable->swap_buffers();
	
	gldrawable->gl_end();
	
	return true;
}

bool Viewer::on_configure_event(GdkEventConfigure* event)
{
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;
  
  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  gldrawable->gl_end();

  return true;
}

bool Viewer::on_button_press_event(GdkEventButton* event)
{
	//std::cerr << "Stub: Button " << event->button << " pressed" << std::endl;
	startPos[0] = event->x;
	startPos[1] = event->y;
	if (event->button == 1)
		mb1 = true;
	else if (event->button == 2)
		mb2 = true;
	else if (event->button == 3)
		mb3 = true;
	
  	/*if (event->state & GDK_SHIFT_MASK)
  	{
  		f++;
  	}
  	else if (event->state & GDK_CONTROL_MASK)
  	{
  		n++;
  	}
  	else
  	{
  		angle++;
  	}*/
  	invalidate();
  	
  return true;
}

bool Viewer::on_button_release_event(GdkEventButton* event)
{
	if (event->button == 1)
		mb1 = false;
	else if (event->button == 2)
		mb2 = false;
	else if (event->button == 3)
		mb3 = false;
  //std::cerr << "Stub: Button " << event->button << " released" << std::endl;
  return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event)
{
	//std::cerr << "Stub: Motion at " << event->x << ", " << event->y << " mode " << currMode << std::endl;
  	double x2x1 = event->x - startPos[0];
	x2x1 /= 10;
	Matrix4x4 temp;
	std::cerr << "Change in x is " << x2x1 << std::endl;
	for (int i = 0;i<4;i++)
	{
		for (int j=0;j<4;j++)
		{
			if (i == j)
				temp[i][j] = 1;
			else
				temp[i][j] = 0;
		}
	}
	if (currMode == MODEL_TRANSLATE)
	{
		if (mb1)
			temp[0][3] -= x2x1;
		else if (mb2)
			temp[1][3] -= x2x1;
		else if (mb3)
			temp[2][3] -= x2x1;
	}
	else if (currMode == MODEL_SCALE)
	{
		x2x1 *= 10;
		if (x2x1 >= 0 && x2x1 < 1)
			x2x1 = 1.1;
		else if (x2x1 <= 0 && x2x1 > -1)
			x2x1 = 0.5;
		else if (x2x1 < 0)
			x2x1 = -1.0 / x2x1;
		
		std::cerr << "Change in x2x1 after modification:\t" << x2x1 << std::endl;
		if (mb1)
			temp[0][0] *= x2x1;
		else if (mb2)
			temp[1][1] *= x2x1;
		else if (mb3)
			temp[2][2] *= x2x1;
	}
	else if (currMode == MODEL_ROTATE)
	{
		double anglePieces = get_width() / (2.0 * M_PI);
		x2x1 /= anglePieces;
		if (mb1)
		{
			temp[1][2] = -1 * sin(x2x1);
			temp[1][1] = cos(x2x1);
			temp[2][2] = cos(x2x1);
			temp[2][1] = sin(x2x1);
		}
		else if (mb2)
		{
			temp[0][1] = -1 * sin(x2x1);
			temp[0][0] = cos(x2x1);
			temp[1][1] = cos(x2x1);
			temp[1][0] = sin(x2x1);
		}
		else if (mb3)
		{
			temp[2][0] = -1 * sin(x2x1);
			temp[0][0] = cos(x2x1);
			temp[2][2] = cos(x2x1);
			temp[0][2] = sin(x2x1);
		}
	}
	else if (currMode == VIEW_TRANSLATE)
	{
		if (mb1)
			lookFrom[0] -= x2x1;
		else if (mb2)
			lookFrom[1] -= x2x1;
		else if (mb3)
			lookFrom[2] -= x2x1;
			
		//lookFrom.normalize();
		set_view();
		std::cout << "Viewing Matrix: " << std::endl;
		print (m_V);
	}
	else if (currMode == VIEW_ROTATE)
	{
		double anglePieces = get_width() / (2.0 * M_PI);
		x2x1 /= anglePieces;
		if (mb1)
		{
			temp[1][2] = -1 * sin(x2x1);
			temp[1][1] = cos(x2x1);
			temp[2][2] = cos(x2x1);
			temp[2][1] = sin(x2x1);
		}
		else if (mb2)
		{
			temp[0][1] = -1 * sin(x2x1);
			temp[0][0] = cos(x2x1);
			temp[1][1] = cos(x2x1);
			temp[1][0] = sin(x2x1);
		}
		else if (mb3)
		{
			temp[2][0] = -1 * sin(x2x1);
			temp[0][0] = cos(x2x1);
			temp[2][2] = cos(x2x1);
			temp[0][2] = sin(x2x1);
		}
		//lookAt = temp * lookAt;
		//lookAt.normalize();
		
		//temp = temp.invert();
		//temp = temp.transpose();
		up = temp * up;
		
		up.normalize();
		set_view();
		
		for (int i = 0;i<4;i++)
		{
			for (int j = 0;j<4;j++)
			{
				if (i == j)
					temp[i][j] = 1;
				else
					temp[i][j] = 0;
			}
		}		
	}
	else if (VIEW_PERSPECTIVE)
	{
		if (mb1)
			angle += x2x1;
		else if (mb2)
			n += x2x1;
		else if (mb3)
			f += x2x1;
			
		update_labels();
	}
	m_M = m_M * temp;
	
//	std::cout<<"modelling matrix: " << std::endl;
	//print (m_M);
	x2x1 /= 10;
	
	// Store the position of the cursor
	startPos[0] = event->x;
	startPos[1] = event->y;
	invalidate();
	return true;
}

void Viewer::set_mode(Mode newMode)
{
	currMode = newMode;
	std::string str;
	switch (newMode)
	{
		case MODEL_ROTATE:
			str = " Rotate Model";
			break;
		case MODEL_TRANSLATE:
			str = " Translate Model";
			break;
		case MODEL_SCALE:
			str = " Scale Model";
			break;
		case VIEW_TRANSLATE:
			str = " Translate View";
			break;        
		case VIEW_ROTATE:  
			str = " Rotate View";
			break;         
		case VIEW_PERSPECTIVE:
			str = " Change View Perspective";
			break;
		case VIEWPORT:
			str = "Change Viewport";
			break;
	}
	currentModeLabel->set_text("Current Mode:\t" + str);
}


void Viewer::set_labels(Gtk::Label *currentModel, Gtk::Label *nearFar)
{
	currentModeLabel = currentModel;
	nearFarLabel = nearFar;
	
	update_labels();
}

void Viewer::update_labels()
{
	// String streams used to print score and lines cleared	
	std::stringstream ss, ss2;
	
	// Update the score
	ss << n;
	ss2 << f;
	nearFarLabel->set_text("Near Plane:\t" + ss.str() + "\tFar Plane:\t" + ss2.str());
}
void Viewer::set_view()
{
	Vector3D vX, vY, vZ;
	vZ = lookAt - lookFrom;
	vZ.normalize();
	
	vX = up.cross(vZ);
	vX.normalize();
	
	vY = vZ.cross(vX);
	vY.normalize();
	
	for (int i = 0;i<3;i++)
	{
		m_V[i][0] = vX[i];
		m_V[i][1] = vY[i];
		m_V[i][2] = vZ[i];
		m_V[i][3] = lookFrom[i];
		m_V[3][i] = 0;
	}
}