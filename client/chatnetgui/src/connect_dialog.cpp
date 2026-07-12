#include "connect_dialog.h"

connect_dialog::connect_dialog() :
    m_Label_host("Host: ", Gtk::ALIGN_LEFT),
    m_Label_nickname("Nickname: ", Gtk::ALIGN_LEFT),
    m_Label_port("Port: ", Gtk::ALIGN_LEFT),
    m_VBox(5,false),
    m_Table(3,2)
{
    m_Entry_host.set_activates_default(true);
    m_Entry_nickname.set_activates_default(true);
    m_port.set_activates_default(true);
    set_default_response(Gtk::RESPONSE_OK);
    set_title("Connect");
// setup SpinButton
    m_port.set_increments(1,256);
    m_port.set_range(0,65535);
    m_port.set_value(7999);
    m_port.set_wrap();
// Add widgets to table
    m_Table.attach(m_Label_nickname,0,1,0,1,Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
    m_Table.attach(m_Entry_nickname,1,2,0,1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    m_Table.attach(m_Label_host,0,1,1,2, Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
    m_Table.attach(m_Entry_host,1,2,1,2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    m_Table.attach(m_Label_port,0,1,2,3, Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
    m_Table.attach(m_port,1,2,2,3, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);

    m_Image.set_from_icon_name("server", Gtk::ICON_SIZE_DIALOG);

// Add widgets to VBox
    get_vbox()->pack_start(m_Image,Gtk::PACK_SHRINK);
    get_vbox()->pack_start(m_Table);
    Gtk::Button  *m_btn_cancel = add_button(Gtk::Stock::CANCEL,Gtk::RESPONSE_CANCEL);
    Gtk::Button *m_btn_connect = add_button(Gtk::Stock::CONNECT,Gtk::RESPONSE_OK);
    set_default(*m_btn_connect);
    m_btn_cancel->grab_focus();
    show_all_children();
}
