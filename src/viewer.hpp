#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <gtkmm.h>
#include <gtkglmm.h>
#include "algebra.hpp"

// The "main" OpenGL widget
class Viewer : public Gtk::GL::DrawingArea {
public:
		enum Mode {
			VIEW_ROTATE,
			VIEW_TRANSLATE,
			VIEW_PERSPECTIVE,
			MODEL_ROTATE,
			MODEL_TRANSLATE,
			MODEL_SCALE,
			VIEWPORT
	};
  Viewer();
  virtual ~Viewer();

  // A useful function that forces this widget to rerender. If you
  // want to render a new frame, do not call on_expose_event
  // directly. Instead call this, which will cause an on_expose_event
  // call when the time is right.
  void invalidate();

  // *** Fill in these functions (in viewer.cpp) ***

  // Set the parameters of the current perspective projection using
  // the semantics of gluPerspective().
  void set_perspective(double fov, double aspect,
                       double near, double far);



  // Restore all the transforms and perspective parameters to their
  // original state. Set the viewport to its initial size.
  void reset_view();
	
	
	void set_mode(Mode newMode);

	void set_labels(Gtk::Label *currentModel, Gtk::Label *nearFar);
	void update_labels();
	void set_view();

protected:

  // Events we implement
  // Note that we could use gtkmm's "signals and slots" mechanism
  // instead, but for many classes there's a convenient member
  // function one just needs to define that'll be called with the
  // event.

  // Called when GL is first initialized
  virtual void on_realize();
  // Called when our window needs to be redrawn
  virtual bool on_expose_event(GdkEventExpose* event);
  // Called when the window is resized
  virtual bool on_configure_event(GdkEventConfigure* event);
  // Called when a mouse button is pressed
  virtual bool on_button_press_event(GdkEventButton* event);
  // Called when a mouse button is released
  virtual bool on_button_release_event(GdkEventButton* event);
  // Called when the mouse moves
  virtual bool on_motion_notify_event(GdkEventMotion* event);

private:

  // *** Fill me in ***
  // You will want to declare some more matrices here
	Matrix4x4 m_proj;
	Matrix4x4 m_M, m_T, m_V;
	Matrix4x4 identity;
	
	Vector3D lookAt, up; 
	Vector3D lookFrom;
	
	// Mouse button flags
	bool mb1, mb2, mb3;
	
	Point2D startPos;
	Point3D *pointsOfCube;
	Point3D *tempPoints;
	Point2D *walls;
	
	Gtk::Label *nearFarLabel;
	Gtk::Label *currentModeLabel;
	double angle;
	double n, f;

	struct Line {
		Point2D pt1, pt2;
		double z1, z2;
		bool draw;
	};

	Mode currMode;
  
	void clip_sides(Line *sides);
	void print (Matrix4x4 mat);
	void print (Point3D pt);
	void print (Vector3D vec);
};

#endif
