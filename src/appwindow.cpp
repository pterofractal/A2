#include "appwindow.hpp"

AppWindow::AppWindow()
{
  set_title("CS488 Assignment Two");

  // A utility class for constructing things that go into menus, which
  // we'll set up next.
  using Gtk::Menu_Helpers::MenuElem;
  using Gtk::Menu_Helpers::RadioMenuElem;
  
	sigc::slot1<void, Viewer::Mode> mode_slot = sigc::mem_fun(m_viewer, &Viewer::set_mode);
	sigc::slot0<void> reset_slot = sigc::mem_fun(m_viewer, &Viewer::reset_view);

  // Set up the application menu
  // The slot we use here just causes AppWindow::hide() on this,
  // which shuts down the application.
	m_menu_app.items().push_back(MenuElem("_Quit", Gtk::AccelKey("q"),
		sigc::mem_fun(*this, &AppWindow::hide)));
	m_menu_app.items().push_back(MenuElem("_Reset", Gtk::AccelKey("a"),	reset_slot ) );
  

// Set up the Mode Menu
	m_mode.items().push_back(RadioMenuElem(m_mode_group, "_View Rotate",  Gtk::AccelKey("o"), sigc::bind( mode_slot, Viewer::VIEW_ROTATE ) ) );
	m_mode.items().push_back(RadioMenuElem(m_mode_group, "_View Translate", Gtk::AccelKey("n"), sigc::bind( mode_slot, Viewer::VIEW_TRANSLATE ) ) );
	m_mode.items().push_back(RadioMenuElem(m_mode_group, "_View Perspective", Gtk::AccelKey("p"), sigc::bind( mode_slot, Viewer::VIEW_PERSPECTIVE ) ) );
	m_mode.items().push_back(RadioMenuElem(m_mode_group, "_Model Rotate", Gtk::AccelKey("r"), sigc::bind( mode_slot, Viewer::MODEL_ROTATE ) ) );
	m_mode.items().push_back(RadioMenuElem(m_mode_group, "_Model Translate", Gtk::AccelKey("t"), sigc::bind( mode_slot, Viewer::MODEL_TRANSLATE ) ) );
	m_mode.items().push_back(RadioMenuElem(m_mode_group, "_Model Scale", Gtk::AccelKey("s"), sigc::bind( mode_slot, Viewer::MODEL_SCALE ) ) );
	m_mode.items().push_back(RadioMenuElem(m_mode_group, "_Viewport", Gtk::AccelKey("v"), sigc::bind( mode_slot, Viewer::VIEWPORT ) ) );

  // Set up the menu bar
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Application", m_menu_app));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Mode", m_mode));
 
  
	// Set up the score label	
	currentModeLabel.set_text("Current Mode:\t Rotate View");
	nearFarLabel.set_text("Near Plane:\t0\tFar Plane:\t0");
	
	m_viewer.set_labels(&currentModeLabel, &nearFarLabel);
	
	// Pack in our widgets
  
  // First add the vertical box as our single "top" widget
  add(m_vbox);

  // Put the menubar on the top, and make it as small as possible
	m_vbox.pack_start(m_menubar, Gtk::PACK_SHRINK);
	m_vbox.pack_start(currentModeLabel, Gtk::PACK_EXPAND_PADDING);
	m_vbox.pack_start(nearFarLabel, Gtk::PACK_EXPAND_PADDING);

  // Put the viewer below the menubar. pack_start "grows" the widget
  // by default, so it'll take up the rest of the window.
  m_viewer.set_size_request(300, 300);
  m_vbox.pack_start(m_viewer);

  show_all();
}
