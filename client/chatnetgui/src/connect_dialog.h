#ifndef CONNECT_DIALOG_H_INCLUDED
#define CONNECT_DIALOG_H_INCLUDED

#include <gtkmm.h>

class connect_dialog : public Gtk::Dialog
{
public:
connect_dialog();
Glib::ustring get_nickname() { return m_Entry_nickname.get_text();}
Glib::ustring get_host() { return m_Entry_host.get_text(); }
int get_port() { return m_port.get_value_as_int();}
virtual ~connect_dialog() {}
protected:
// child widgets
Gtk::Label m_Label_host;
Gtk::Entry m_Entry_host;
Gtk::Label m_Label_nickname;
Gtk::Entry m_Entry_nickname;
Gtk::Label m_Label_port;
Gtk::SpinButton  m_port;
Gtk::VBox m_VBox;
Gtk::Table m_Table;
Gtk::Image m_Image;
};

#endif // CONNECT_DIALOG_H_INCLUDED
