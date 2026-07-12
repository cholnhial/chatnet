#ifndef LOGVIEW_H_INCLUDED
#define LOGVIEW_H_INCLUDED
#include <gtkmm.h>

class logview : public Gtk::TextView
{
public:
    virtual ~logview();
    logview();
    void log(const Glib::ustring& text, const Glib::RefPtr<Gtk::TextTag>& tag, bool scroll = true);
    void log(const Glib::ustring& text, bool scroll = true);
    void clear();
protected:
    Glib::RefPtr<Gtk::TextBuffer> m_refTextBuffer;
    Glib::RefPtr<Gtk::TextMark> m_end_mark;
};

#endif // LOGVIEW_H_INCLUDED
