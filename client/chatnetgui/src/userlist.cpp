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

#include "userlist.h"
#include <iostream>

using namespace std;

userlist::userlist()
{
    //Create TreeModel
    m_refTreeModel = Gtk::ListStore::create(m_Columns);
    set_model(m_refTreeModel);

// Add TreeView's View columns
    append_column("People",m_Columns.m_col_people);
// Add menus
    Gtk::Menu::MenuList& menulist = m_Menu_Popup.items();
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("_Chat", sigc::mem_fun(*this,&userlist::on_menu_file_popup_chat)));
}

bool userlist::on_button_press_event(GdkEventButton* event)
{
//Call base class, to allow normal handling,
//such as allowing the row to be selected by the right-click:
    bool return_value = TreeView::on_button_press_event(event);
//Then do our custom stuff:
    if( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
    {
        Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();

        if(refSelection)
        {
            Gtk::TreeModel::iterator iter = refSelection->get_selected();

            if(iter)
            {
                m_Menu_Popup.popup(event->button, event->time);
            }
        }

    }
    return return_value;
}

void userlist::add_new_row(const Glib::ustring& name)
{
    Gtk::TreeModel::Row row = (*m_refTreeModel->append());
    row[m_Columns.m_col_people] = name;

    Gtk::TreeModel::Path path = m_refTreeModel->get_path(row);

    /* Scroll to the end of the ScrolledWindow */
    scroll_to_row(path,0.0f);
}

void userlist::remove_row(const Glib::ustring& name)
{
    typedef Gtk::TreeModel::Children type_children; //minimise code length.
    type_children children = m_refTreeModel->children();
    for(type_children::iterator iter = children.begin();
            iter != children.end(); ++iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if(row[m_Columns.m_col_people] == name)
        {
            m_refTreeModel->erase(iter);
            break;
        }
    }
}

signal_chat_type userlist::on_signal_chat() const
{
    return event_signal_chat;
}

void userlist::on_menu_file_popup_chat()
{
    Glib::RefPtr<Gtk::TreeView::Selection> m_refSelection = get_selection();
    Gtk::TreeModel::iterator iter = m_refSelection->get_selected();
    if(iter)
    {
        selected_name = (*iter)[m_Columns.m_col_people];
        event_signal_chat.emit();
    }
}

Glib::ustring userlist::get_selected_name()
{
    return selected_name;
}

void userlist::clear()
{
    m_refTreeModel->clear();
}

Glib::RefPtr<Gtk::ListStore> userlist::get_refTreeModel()
{
    return m_refTreeModel;
}
