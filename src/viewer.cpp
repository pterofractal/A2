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
  
	currMode = VIEW_ROTATE;
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
		for (int j = 0; j<3;j++)
			tempPoints[i][j] = pointsOfCube[i][j];
	}
	
	n = DEFAULT_NEAR;
	f = DEFAULT_FAR;
	angle = DEFAULT_FOV;

	lookFrom[0] = 0;
	lookFrom[1] = 0;
	lookFrom[2] = 17;
		
	up[0] = 0;
	up[1] = 1;
	up[2] = 0;
	
	lookAt[0] = 0;
	lookAt[1] = 0;
	lookAt[2] = 1;
	
	set_view();
	
}

Viewer::~Viewer()
{
	delete(pointsOfCube);
	delete(tempPoints);
	delete(walls);
}

void Viewer::invalidate()
{
  // Force a rerender
  Gtk::Allocation allocation = get_allocation();
  get_window()->invalidate_rect( allocation, false);
}

void Viewer::set_perspective(double fov, double aspect, double near, double far)
{
	// Construct the projection matrix
	m_proj[0][0] = 1/(tan(fov / 2));
	m_proj[0][0] /= aspect;
	
	for (int i = 1;i<4;i++)
	{
		m_proj[0][i] = 0;
		m_proj[i][0] = 0;
	}
	
	m_proj[1][1] = 1/(tan(fov/2));
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
	n = DEFAULT_NEAR;
	f = DEFAULT_FAR;
	angle = DEFAULT_FOV;

	m_M = identity;

	// Initialize Modelling Transformation
	for (int i = 0;i<4;i++)
	{
		for (int j = 0;j<4;j++)
		{
			m_T[i][j] = 0;
		}
	}
	
	// Reset position of camera
	lookFrom[0] = 0;
	lookFrom[1] = 0;
	lookFrom[2] = 17;
		
	up[0] = 0;
	up[1] = 1;
	up[2] = 0;
	
	lookAt[0] = 0;
	lookAt[1] = 0;
	lookAt[2] = 1;
	
	// Reinitialize the viewing matrix
	set_view();
	
	// Reset values for walls
	walls[0][0] = 0.95 * get_width();
	walls[0][1] = 0.5 * get_height();
	
	walls[1][0] = 0.05 * get_width();
	walls[1][1] = 0.5 * get_height();
	
	walls[2][0] = 0.5 * get_width();
	walls[2][1] = 0.95 * get_height();
	
	walls[3][0] = 0.5 * get_width();
	walls[3][1] = 0.05 * get_height();
	
	invalidate();
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
	
	// Specify default position of walls
	walls = new Point2D[4];
	walls[0][0] = 0.95 * get_width();
	walls[0][1] = 0.5 * get_height();
	
	walls[1][0] = 0.05 * get_width();
	walls[1][1] = 0.5 * get_height();
	
	walls[2][0] = 0.5 * get_width();
	walls[2][1] = 0.95 * get_height();
	
	walls[3][0] = 0.5 * get_width();
	walls[3][1] = 0.05 * get_height();
	
	gldrawable->gl_end();
}

bool Viewer::on_expose_event(GdkEventExpose* event)
{
	Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();
	
	if (!gldrawable) 
		return false;
	
	if (!gldrawable->gl_begin(get_gl_context()))
		return false;
	
	double width = get_width();
	double height = get_height();	
	double aspectRatio = width / height;
	
	// Array of doubles used to store z-values BEFORE projection
	double preProjZ[8];
	
	// Final Scaling matrix
	m_T[0][0] = width / 2;
	m_T[1][1] = height / 2;
	m_T[2][2] = 1;
	m_T[3][3] = 1;
	m_T[0][3] = width / 2;
	m_T[1][3] = height / 2;
	m_T[2][3] = 1;
	
	// Store the points of the cube into a temp array
	for (int i = 0;i<8;i++)
	{
		for (int j = 0; j<3;j++)
			tempPoints[i][j] = pointsOfCube[i][j];
	}
	
	// Here is where your drawing code should go.
	draw_init(width, height);
	
	// Init projection matrix
	set_perspective(angle, aspectRatio, n, f);	
	
	// Apply each transformation to the temp points
	for (int i = 0;i<8;i++)
	{	
		tempPoints[i] = m_M * tempPoints[i];
		tempPoints[i] = m_V * tempPoints[i];
		
		// Store the z values
		preProjZ[i] = tempPoints[i][2];
		
		tempPoints[i] = m_proj * tempPoints[i];		
	}
	
	for (int i = 0;i<8;i++)
	{	
		// Normalize the x and y coordinates	
		tempPoints[i][0] /= preProjZ[i];
		tempPoints[i][1] /= preProjZ[i];		
		
		// Scale each point
		tempPoints[i] = m_T * tempPoints[i];
	}
	
	// Define sides of the cube	
	Line sides[12];
	
	sides[0].pt1 = Point2D(tempPoints[0][0], tempPoints[0][1]);
	sides[0].pt2 = Point2D(tempPoints[1][0], tempPoints[1][1]);

	sides[1].pt1 = Point2D(tempPoints[0][0], tempPoints[0][1]);
	sides[1].pt2 = Point2D(tempPoints[2][0], tempPoints[2][1]);
	
	sides[2].pt1 = Point2D(tempPoints[1][0], tempPoints[1][1]);
	sides[2].pt2 = Point2D(tempPoints[3][0], tempPoints[3][1]);
	
	sides[3].pt1 = Point2D(tempPoints[2][0], tempPoints[2][1]);
	sides[3].pt2 = Point2D(tempPoints[3][0], tempPoints[3][1]);
	
	sides[4].pt1 = Point2D(tempPoints[4][0], tempPoints[4][1]);
	sides[4].pt2 = Point2D(tempPoints[5][0], tempPoints[5][1]);
	
	sides[5].pt1 = Point2D(tempPoints[4][0], tempPoints[4][1]);
	sides[5].pt2 = Point2D(tempPoints[6][0], tempPoints[6][1]);
	
	sides[6].pt1 = Point2D(tempPoints[5][0], tempPoints[5][1]);
	sides[6].pt2 = Point2D(tempPoints[7][0], tempPoints[7][1]);
	
	sides[7].pt1 = Point2D(tempPoints[6][0], tempPoints[6][1]);
	sides[7].pt2 = Point2D(tempPoints[7][0], tempPoints[7][1]);
	
	// Store the z values into each side
	sides[0].z1 = tempPoints[0][2];
	sides[1].z1 = tempPoints[0][2];
	sides[2].z1 = tempPoints[1][2];
	sides[3].z1 = tempPoints[2][2];
	sides[4].z1 = tempPoints[4][2];
	sides[5].z1 = tempPoints[4][2];
	sides[6].z1 = tempPoints[5][2];
	sides[7].z1 = tempPoints[6][2];
	
	sides[0].z2 = tempPoints[1][2];
	sides[1].z2 = tempPoints[2][2];
	sides[2].z2 = tempPoints[3][2];
	sides[3].z2 = tempPoints[3][2];
	sides[4].z2 = tempPoints[5][2];
	sides[5].z2 = tempPoints[6][2];
	sides[6].z2 = tempPoints[7][2];
	sides[7].z2 = tempPoints[7][2];
	
	for (int i = 0; i < 4;i++)
	{
		sides[i+8].pt1 = Point2D(tempPoints[i][0], tempPoints[i][1]);
		sides[i+8].pt2 = Point2D(tempPoints[i+4][0], tempPoints[i+4][1]);
		
		sides[i+8].z1 = tempPoints[i][2];
		sides[i+8].z2 = tempPoints[i+4][2];
	}

	// Assume each side can be drawn
	for (int i = 0;i<12;i++)
		sides[i].draw = true;
	
	// Clip points to the viewport
	clip_sides(sides);
	
	// Draw the cube
	for (int i = 0;i<4;i++)
	{
		// Back face
		set_colour(Colour(0.1, 0.1, 0.1));
		if (sides[i].draw)
			draw_line (sides[i].pt1, sides[i].pt2);
		
		// Front face
		set_colour(Colour(1, 0, 0));
		if (sides[i+4].draw)
			draw_line (sides[i+4].pt1, sides[i+4].pt2);
		
		// Side faces
		set_colour(Colour(0.1, 0.1, 1));
		if (sides[i+8].draw)
			draw_line (sides[i+8].pt1, sides[i+8].pt2);
	}
	
	// Draw viewport
	set_colour(Colour(0, 0.5, 1));
	draw_line(	Point2D(0.05 * width, 0.05 * height), Point2D(0.05 * width, 0.95 * height) );
	draw_line(	Point2D(0.05 * width, 0.05 * height), Point2D(0.95 * width, 0.05 * height) );
	draw_line(	Point2D(0.95 * width, 0.05 * height), Point2D(0.95 * width, 0.95 * height) );
	draw_line(	Point2D(0.05 * width, 0.95 * height), Point2D(0.95 * width, 0.95 * height) );
	
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
	startPos[0] = event->x;
	startPos[1] = event->y;
	if (event->button == 1)
		mb1 = true;
	else if (event->button == 2)
		mb2 = true;
	else if (event->button == 3)
		mb3 = true;


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
  	
	return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event)
{
	Matrix4x4 temp;
	// Change in x
	double x2x1 = event->x - startPos[0];
	
	// Scale it down a bit
	x2x1 /= 10;
	
	if (currMode == MODEL_TRANSLATE)
	{		
		int i = 0;
		if (mb1)
			i = 0;
		else if (mb2)
			i = 1;
		else if (mb3)
			i = 2;
			
		temp[i][3] -= x2x1;
	}
	else if (currMode == MODEL_SCALE)
	{
		x2x1 *= 5;
		if (x2x1 >= 0 && x2x1 < 1)
			x2x1 = 1.1;
		else if (x2x1 <= 0 && x2x1 > -1)
			x2x1 = 0.5;
		else if (x2x1 < 0)
			x2x1 = -1.0 / x2x1;
		
		int i = 0;
		if (mb1)
			i = 0;
		else if (mb2)
			i = 1;
		else if (mb3)
			i = 2;
		
		m_M[i][i] *= x2x1;
	}
	else if (currMode == MODEL_ROTATE)
	{
		double anglePieces = get_width() / (2.0 * M_PI);
		x2x1 /= anglePieces;
		// Construct appropriate rotation matrix
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
			
		// Update the viewing matrix
		set_view();
	}
	else if (currMode == VIEW_ROTATE)
	{
		double anglePieces = get_width() / (2.0 * M_PI);
		x2x1 /= anglePieces;
		// Construct appropriate rotation matrix
		if (mb1)
		{
			temp[1][2] = -1 * sin(x2x1);
			temp[1][1] = cos(x2x1);
			temp[2][2] = cos(x2x1);
			temp[2][1] = sin(x2x1);
			lookAt[0] += x2x1;
		}
		else if (mb2)
		{
			temp[0][1] = -1 * sin(x2x1);
			temp[0][0] = cos(x2x1);
			temp[1][1] = cos(x2x1);
			temp[1][0] = sin(x2x1);
			lookAt[1] += x2x1;
		}
		else if (mb3)
		{
			temp[2][0] = -1 * sin(x2x1);
			temp[0][0] = cos(x2x1);
			temp[2][2] = cos(x2x1);
			temp[0][2] = sin(x2x1);
			lookAt[2] += x2x1;
		}

		// Apply transformation
		m_V = temp.invert() * m_V;
		
		// Set temp matrix back to identity
		temp = identity;
	}
	else if (VIEW_PERSPECTIVE)
	{
		if (mb1)
			angle += x2x1;
		else if (mb2)
			n += x2x1;
		else if (mb3)
			f += x2x1;
		
		// Make sure the field of view angle is in [5, 160]
		if (angle < 5)
			angle = 5;
		else if (angle > 160)
			angle = 160;
		
		// Update on screen labels
		update_labels();
	}
	
	// Apply the modelling matrix transformation
	m_M = m_M * temp;
	
	// Store the position of the cursor
	startPos[0] = event->x;
	startPos[1] = event->y;
	
	// Force render
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
	// Create view matrix based on lookAt, lookFrom and up
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

void Viewer::clip_sides(Line *sides)
{	
	for (int i = 0; i < 12; i++)
	{
		for (int j = 0;j<4;j++)
		{
			double wecA, wecB;
			// If we are dealing with the right and left walls determine difference in X
			if (j < 2)
			{
				wecA = ((sides[i].pt1)[0] - walls[j][0]);
				wecB = ((sides[i].pt2)[0] - walls[j][0]);	
			}
			// If we are dealing with the top and botom walls determine difference in X
			else
			{
				wecA = ((sides[i].pt1)[1] - walls[j][1]);
				wecB = ((sides[i].pt2)[1] - walls[j][1]);
			}
			
			// If we are the right or top wall multiply by -1 to represent the normal
			if (j%2 == 0)
			{
				wecA *= -1;
				wecB *= -1;
			}

			if (wecA < 0 && wecB < 0)
			{
				sides[i].draw = false;
				break;
			}

			if (wecA >= 0 && wecB >= 0)
				continue;

			double t = wecA / (wecA - wecB);
			if (wecA < 0)
			{
				(sides[i].pt1)[0] = (sides[i].pt1)[0] + t * ((sides[i].pt2)[0] - (sides[i].pt1)[0]);
				(sides[i].pt1)[1] = (sides[i].pt1)[1] + t * ((sides[i].pt2)[1] - (sides[i].pt1)[1]);
			}
			else
			{
				(sides[i].pt2)[0] = (sides[i].pt1)[0] + t * ((sides[i].pt2)[0] - (sides[i].pt1)[0]);
				(sides[i].pt2)[1] = (sides[i].pt1)[1] + t * ((sides[i].pt2)[1] - (sides[i].pt1)[1]);
			}
		}
	}
	
	// Near and far plane clipping
	for (int i = 0; i<12;i++)
	{
		for (int j = 0;j<2;j++)
		{
			double pointOnPlane = n;
			
			if (j == 1)
				pointOnPlane = f;
				
			double wecA = (sides[i].z1 - pointOnPlane);
			double wecB = (sides[i].z2 - pointOnPlane);
			
			if (wecA < 0 && wecB < 0)
				sides[i].draw = false;


			if (wecA >= 0 && wecB >= 0)
				continue;
			
			double t = wecA / (wecA - wecB);
			if (wecA < 0)
			{
				(sides[i].pt1)[0] = (sides[i].pt1)[0] + t * ((sides[i].pt2)[0] - (sides[i].pt1)[0]);
				(sides[i].pt1)[1] = (sides[i].pt1)[1] + t * ((sides[i].pt2)[1] - (sides[i].pt1)[1]);
			}
			else
			{
				(sides[i].pt2)[0] = (sides[i].pt1)[0] + t * ((sides[i].pt2)[0] - (sides[i].pt1)[0]);
				(sides[i].pt2)[1] = (sides[i].pt1)[1] + t * ((sides[i].pt2)[1] - (sides[i].pt1)[1]);
			}
			
		}
	}
}