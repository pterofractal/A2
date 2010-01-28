#include "viewer.hpp"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include "draw.hpp"
#include <math.h>

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
	for (int j = 0;j<4;j++)
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
	n = 6;
	f = 16;
	angle = 31.6;
	
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
	std::cout << "FOV: " << fov << ",\tnear: " << near << ",\tfar: " << far << std::endl; 
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
	m_T[0][3] = -1 * get_width() / 2;
	m_T[1][3] = -1 * get_height() / 2;
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
		tempPoints[i] = m_T * tempPoints[i];
		tempPoints[i] = m_M * tempPoints[i];
		tempPoints[i] = m_V * tempPoints[i];
		tempPoints[i] = m_proj * tempPoints[i];		
	}
		
	for (int i = 0;i<8;i++)
	{
		tempPoints[i][0] /= tempPoints[i][2];
		tempPoints[i][1] /= tempPoints[i][2];
		tempPoints[i][2] /= tempPoints[i][2];
	}
	
	for (int i = 0;i<8;i++)
	{
		std::cout << "Point " << i << ": ";
		//print(tempPoints[i]);
	}
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
	
	draw_line(	Point2D(tempPoints[4][0], tempPoints[4][1]),
				Point2D(tempPoints[5][0], tempPoints[5][1]));
	
	draw_line(	Point2D(tempPoints[4][0], tempPoints[4][1]),
				Point2D(tempPoints[6][0], tempPoints[6][1]));
	
	draw_line(	Point2D(tempPoints[5][0], tempPoints[5][1]),
				Point2D(tempPoints[7][0], tempPoints[7][1]));
	
	draw_line(	Point2D(tempPoints[6][0], tempPoints[6][1]),
				Point2D(tempPoints[7][0], tempPoints[7][1]));
	
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
	std::cerr << "Stub: Button " << event->button << " pressed" << std::endl;
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
  std::cerr << "Stub: Button " << event->button << " released" << std::endl;
  return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event)
{
	std::cerr << "Stub: Motion at " << event->x << ", " << event->y << " mode " << currMode << std::endl;
  	double x2x1 = event->x - startPos[0];
	x2x1 /= 10;
	Matrix4x4 temp;
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
		if (mb1)
			temp[0][0] *= x2x1;
		else if (mb2)
			temp[1][1] *= x2x1;
		else if (mb3)
			temp[2][2] *= x2x1;
	}
	else if (currMode == MODEL_ROTATE)
	{
		if (mb1)
		{
			Vector3D b(m_M[0][0], 0, m_M[2][0]);
			Vector3D xAxis(m_M[0][0], m_M[1][0], m_M[2][0]);
			//double phi = (b.dot(xAxis)) / (b.length() * xAxis.length());
			double cosPhi = (m_M[0][0]) / b.length();
			double sinPhi = (m_M[2][0]) / b.length();
			Matrix4x4 RyNegPhi;
			for (int i = 0;i<4;i++)
			{
				for (int j = 0;j<4;j++)
				{
					RyNegPhi[i][j] = 0;
				}
			}
			RyNegPhi[0][0] = cosPhi;
			RyNegPhi[0][2] = -1 * sinPhi;
			RyNegPhi[1][1] = 1;
			RyNegPhi[2][0] = sinPhi;
			RyNegPhi[2][2] = cosPhi;
			RyNegPhi[3][3] = 1;
			
			//Vector3D c = RyNegPhi * xAxis;
			Vector3D c(b.length(), xAxis[1], 0);
			
			double cosPsi = b.length();
			double sinPsi = xAxis[1];
			
			Matrix4x4 RzNegPsi;
			for (int i = 0;i<4;i++)
			{
				for (int j = 0;j<4;j++)
				{
					RzNegPsi[i][j] = 0;
				}
			}
			RzNegPsi[0][0] = cosPsi;
			RzNegPsi[0][1] = -1 * sinPsi;
			RzNegPsi[1][0] = sinPsi;
			RzNegPsi[1][1] = cosPsi;
			RzNegPsi[2][2] = 1;
			RzNegPsi[3][3] = 1;
			
			Matrix4x4 RxTheta;
			x2x1 *= 10;
			double anglePieces = get_width() / 360;
			x2x1 /= anglePieces;
			std::cout << "angle" << x2x1 << "\twidth" << get_width() << std::endl;
			double cosTheta = cos(x2x1);
			double sinTheta = sin(x2x1);
			for (int i = 0;i<4;i++)
			{
				for (int j = 0;j<4;j++)
				{
					RzNegPsi[i][j] = 0;
				}
			}
			RxTheta[0][0] = 1;
			RxTheta[1][1] = cosTheta;
			RxTheta[1][2] = -1 * sinTheta;
			RxTheta[2][1] = sinTheta;
			RxTheta[2][2] = cosTheta;
			RxTheta[3][3] = 1;
			
			Matrix4x4 RzPsi, RyPhi;
			RyPhi = RyNegPhi;
			RzPsi = RzPsi;
			
			RyPhi[2][0] *= -1;
			RyPhi[0][2] *= -1;
			
			RzPsi[0][1] *= -1;
			RzPsi[1][0] *= -1;
			
			
			print (RyNegPhi);
			print (RzNegPsi);
			print (RxTheta);
			Matrix4x4 tempRot = RyPhi * RzPsi * RxTheta * RzNegPsi * RyNegPhi;
			std::cout << "Temp rotation: " << std::endl;
			print(tempRot);
			
		}
	}
	m_M = m_M * temp;
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
}
