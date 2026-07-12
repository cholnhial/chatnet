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

#ifndef USERLIST_H_
#define USERLIST_H_

#include <gtkmm.h>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>

typedef sigc::signal<void> signal_chat_type;

/* This class is responsible for keeping a list of connected users
and also emiting signals if the user clicks a certain action, chat...etc.*/
class userlist : public Gtk::TreeView
{
private:
    Glib::ustring selected_name;
public:
    userlist();
    virtual ~userlist() {}
    Glib::RefPtr<Gtk::ListStore> get_refTreeModel();
    void add_new_row(const Glib::ustring& name);
    void remove_row(const Glib::ustring& name);
    void clear();
    Glib::ustring get_selected_name();
    signal_chat_type on_signal_chat() const;
protected:
// signal handlers
    virtual bool on_button_press_event(GdkEventButton * ev);
    virtual void on_menu_file_popup_chat();


    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        {
            add(m_col_people);
        }
        Gtk::TreeModelColumn <Glib::ustring> m_col_people;
    };
public:
    ModelColumns m_Columns;
protected:
// The TreeModel
public:
    Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
protected:
    signal_chat_type event_signal_chat;
    Gtk::Menu m_Menu_Popup;
};
#endif

