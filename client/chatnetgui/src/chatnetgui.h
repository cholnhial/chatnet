/* Copyright (C) 2012 Chol Nhial <chol.nhial.chol@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CHATNETGUI_H_INCLUDED
#define CHATNETGUI_H_INCLUDED
#include <gtkmm.h>
#include "userlist.h"
#include "statusbar.h"
#include "connect_dialog.h"
#include "logview.h"
#include "network.h"

/* Represenst chatnet GUI */
class chatnetgui : public Gtk::Window
{   private:
    connect_dialog m_connect_dialog;
    network net;
    string myname;
    string command;
    bool signals_connected;
    string str_buffer;
    Gtk::Widget* p_menu_file_connect;
    Gtk::Widget* p_menu_file_disconnect;
    Gtk::Widget* p_toolbar_file_connect;
    Gtk::Widget* p_toolbar_file_disconnect;
    public:
    chatnetgui();
    virtual ~chatnetgui() {}
    void display_msgbox(Gtk::Window& parent, const string& title, const string& msg,Gtk::MessageType msg_type);
    protected:
    // signal handlers
   void on_menu_file_connect();
   void on_menu_file_disconnect();
   void on_menu_file_quit() { hide(); }
   void on_output();
   void on_button_send_clicked();
   void on_network_error();
   void on_new_user();
   void on_remove_user();
   void on_chat();
   void on_about();
   // child widgets
   Gtk::Button m_btn_send;
   Gtk::Entry m_Entry;
   Gtk::MenuBar m_MenuBar;
   Gtk::VBox m_VBox;
   Gtk::VBox m_VBox_top;
   Gtk::Table m_Table;
   Gtk::HPaned m_HPaned;
   Gtk::ScrolledWindow m_ScrolledWindow;
   Gtk::ScrolledWindow m_ScrolledWindow1;
   statusbar m_statusbar;
   userlist m_userlist;
   Glib::RefPtr<Gtk::UIManager> m_refUIManager;
   Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
   logview m_logview;
};

#endif // CHATNETGUI_H_INCLUDED
