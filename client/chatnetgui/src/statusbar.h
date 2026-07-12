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

#ifndef STATUSBAR_H_INCLUDED
#define STATUSBAR_H_INCLUDED
#include <gtkmm.h>
class statusbar : public Gtk::Statusbar
{
public:
    virtual ~statusbar();
    statusbar();
    void set_text(const Glib::ustring& text);
protected:
// child widgets
    Gtk::Label m_Label;
};
#endif // STATUSBAR_H_INCLUDED
