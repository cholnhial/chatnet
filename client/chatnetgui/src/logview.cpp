#include "logview.h"
#include "global.h"

logview::logview()
{
    set_editable(false);
    set_cursor_visible(true);
    set_wrap_mode(Gtk::WRAP_WORD_CHAR);

//  Create TextBuffer
    m_refTextBuffer = Gtk::TextBuffer::create();


// create end mark
    m_end_mark = m_refTextBuffer->create_mark("end_mark", m_refTextBuffer->end(), false);
    set_buffer(m_refTextBuffer);
}

void logview::log(const Glib::ustring& text, const Glib::RefPtr<Gtk::TextTag>& tag, bool scroll)
{
    m_refTextBuffer->insert_with_tag(m_refTextBuffer->end(),text,tag);

    if(scroll)
    {
        scroll_to_mark(m_end_mark, 0.0f);
    }
}

void logview::log(const Glib::ustring& text, bool scroll)
{
    m_refTextBuffer->insert(m_refTextBuffer->end(),text);

    if(scroll)
    {
        scroll_to_mark(m_end_mark,0.0f);
    }
}

void logview::clear()
{
    m_refTextBuffer->set_text("");
}
logview::~logview()
{

}


