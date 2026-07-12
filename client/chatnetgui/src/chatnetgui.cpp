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

#include "chatnetgui.h"
#include <iostream>


/* Responsible for setting up UI and initializing certain variables */
chatnetgui::chatnetgui() :
    m_btn_send("send"),
    m_VBox(5,false),
    m_VBox_top(5,false),
    m_Table(1,2,false)
{
    set_border_width(5);
    set_default_size(640,480);
    set_default_icon_name("chatnetgui");
    // setup MenuBar
    m_refActionGroup = Gtk::ActionGroup::create();
//File Menu
    m_refActionGroup->add(Gtk::Action::create("FileMenu", "File"));
//File Menu sub menus
    m_refActionGroup->add(Gtk::Action::create("FileConnect",Gtk::Stock::CONNECT),sigc::mem_fun(*this, &chatnetgui::on_menu_file_connect));
    m_refActionGroup->add(Gtk::Action::create("FileDisconnect", Gtk::Stock::DISCONNECT), sigc::mem_fun(*this,&chatnetgui::on_menu_file_disconnect));
    m_refActionGroup->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT),
                          sigc::mem_fun(*this, &chatnetgui::on_menu_file_quit));

    m_refActionGroup->add(Gtk::Action::create("HelpMenu", "Help"));
    m_refActionGroup->add(Gtk::Action::create("HelpAbout", Gtk::Stock::ABOUT), sigc::mem_fun(*this, &chatnetgui::on_about));

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);
    add_accel_group(m_refUIManager->get_accel_group());

    // For setting up UI for MenuBars and ToolBars
    Glib::ustring ui_info =
        "<ui>"
        " <menubar name='MenuBar'>"
        " <menu action='FileMenu'>"
        " <menuitem action='FileConnect'/>"
        " <menuitem action='FileDisconnect'/>"
        " <separator/>"
        " <menuitem action='FileQuit'/>"
        "</menu>"
        "<menu action ='HelpMenu'>"
        "<menuitem action ='HelpAbout'/>"
        "</menu>"
        " </menubar>"
        " <toolbar name='ToolBar'>"
        " <toolitem action='FileConnect'/>"
        " <toolitem action='FileDisconnect'/>"
        " <separator/>"
        " <toolitem action='FileQuit'/>"
        " <toolitem action='HelpAbout'/>"
        " </toolbar>"
        "</ui>";
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
        m_refUIManager->add_ui_from_string(ui_info);
    }
    catch(const Glib::Error& ex)
    {
        std::cerr << "building menus failed: " << ex.what();
    }
#else
    std::auto_ptr<Glib::Error> ex;
    m_refUIManager->add_ui_from_string(ui_info, ex);
    if(ex.get())
    {
        std::cerr << "building menus failed: " << ex->what();
    }
#endif //GLIBMM_EXCEPTIONS_ENABLED

//Get the menubar and toolbar widgets, and add them to a container widget:
    Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
    Gtk::Widget* pToolbar = m_refUIManager->get_widget("/ToolBar") ;
    p_menu_file_connect = m_refUIManager->get_widget("/MenuBar/FileMenu/FileConnect");
    p_menu_file_disconnect = m_refUIManager->get_widget("/MenuBar/FileMenu/FileDisconnect");
    p_toolbar_file_connect = m_refUIManager->get_widget("/ToolBar/FileConnect");
    p_toolbar_file_disconnect = m_refUIManager->get_widget("/ToolBar/FileDisconnect");
    Gtk::Widget* p_toolbar_quit = m_refUIManager->get_widget("/ToolBar/FileQuit");
    Gtk::Widget* p_toolbar_about = m_refUIManager->get_widget("/ToolBar/HelpAbout");

    // set tooltips
    p_toolbar_file_connect->set_tooltip_text("Connect to chatnet server");
    p_toolbar_file_disconnect->set_tooltip_text("Disconnect from chatnet server");
    p_toolbar_quit->set_tooltip_text("Quit");
    p_toolbar_about->set_tooltip_text("About chatnetgui");

    if(pMenubar)
        m_VBox.pack_start(*pMenubar, Gtk::PACK_SHRINK);
    if(pToolbar)
        m_VBox.pack_start(*pToolbar, Gtk::PACK_SHRINK);

    // set up ScrolledWindow
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_ScrolledWindow1.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_ScrolledWindow1.add(m_userlist);
    m_ScrolledWindow.add(m_logview);
    m_ScrolledWindow1.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    m_ScrolledWindow.set_shadow_type(Gtk::SHADOW_ETCHED_IN);

    // add scrolledwindows to HPane
    m_HPaned.pack1(m_ScrolledWindow);
    m_HPaned.pack2(m_ScrolledWindow1);
    m_VBox.pack_start(m_HPaned,Gtk::PACK_EXPAND_WIDGET);

    // the bottom entry box with the send button
    m_Table.attach(m_Entry,0,1,0,1,Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL);
    m_Table.attach(m_btn_send,1,2,0,1,Gtk::FILL,Gtk::SHRINK);
    m_VBox.pack_start(m_Table,Gtk::PACK_SHRINK);
    m_VBox.pack_start(m_statusbar, Gtk::PACK_SHRINK);

    m_VBox.set_homogeneous(false);
    m_VBox.set_homogeneous(false);
    m_HPaned.set_position(520);
    // add widget to window
    add(m_VBox);


    /* since the user will not be connected straight away, make the send button
     insensitive and others too*/
    m_btn_send.set_sensitive(false);
    m_Entry.set_sensitive(false);
    p_menu_file_disconnect->set_sensitive(false);
    p_toolbar_file_disconnect->set_sensitive(false);
    m_logview.set_sensitive(false);
    m_userlist.set_sensitive(false);

    // connect signals
    m_btn_send.signal_clicked().connect(sigc::mem_fun(*this, &chatnetgui::on_button_send_clicked));
    m_Entry.signal_activate().connect(sigc::mem_fun(*this, &chatnetgui::on_button_send_clicked));

    // for some other signals
    signals_connected = false;

    m_statusbar.set_text("Not connected");
    show_all_children();
}



void chatnetgui::on_menu_file_connect()
{
    bool is_done = false;
    bool end = false;
    do
    {   m_connect_dialog.set_transient_for(*this);
        int result = m_connect_dialog.run();

        switch(result)
        {
        case(Gtk::RESPONSE_CANCEL):
        {
            m_connect_dialog.hide();
            end = true;

            break;
        }
        case(Gtk::RESPONSE_DELETE_EVENT):
        {
            m_connect_dialog.hide();
            end = true;

            break;
        }
        case(Gtk::RESPONSE_OK):
        {

            char buff[16];
            sprintf(buff,"%i", m_connect_dialog.get_port());
//Before starting the thread check if the nickname exist and if legal

            if(net.start(m_connect_dialog.get_host().c_str(),buff,CLIENT) == -1)
            {
                // error
                display_msgbox(*this,"Network error", "Unable to connect", Gtk::MESSAGE_ERROR);
            }
            else
            {
                int ret;
                if((ret = net.check_name(m_connect_dialog.get_nickname())) != 0)
                {

// network error
                    if(ret == -1)
                    {
                        display_msgbox(m_connect_dialog,"Network error", net.get_buffer(),Gtk::MESSAGE_ERROR);
                    }
                    else
                    {
                        display_msgbox(m_connect_dialog,"Illegal name", net.get_buffer(),Gtk::MESSAGE_INFO);
                        close(net.get_sockfd()); // close the connected socket
                    }
                }

                if(ret == 0)
                {
                    is_done = true;
                }

            }
            break;
        }
        default:
        {
// do nothing
        }
        }

        /* If the dialog received a Gtk::RESPONSE_CANCEL or Gtk::RESPONSE_DELETE_EVENT
           then end the loop and dialog will be gone*/
        if(end)
        {
            break;
        }

    }
    while(is_done == false);

// if the users input is valid start the network interface...etc
    if(is_done)
    {
        m_connect_dialog.hide();
// is all done connect to the server...etc
//char buff[16];
        /*sprintf(buff,"%i", m_connect_dialog.get_port());
        net.start(m_connect_dialog.get_host().c_str(),buff,CLIENT);
        net.on_signal_output().connect(sigc::mem_fun(*this,&chatnetgui::on_output));
        */
        // connect some signals
        if(!signals_connected)
        {
            /* make 'm_userlist' and 'm_logview' sensitive */
            m_logview.set_sensitive(true);
            m_userlist.set_sensitive(true);

            net.on_signal_output().connect(sigc::mem_fun(*this,&chatnetgui::on_output));
            net.on_signal_new_user().connect(sigc::mem_fun(*this,&chatnetgui::on_new_user));
            net.on_signal_remove_user().connect(sigc::mem_fun(*this,&chatnetgui::on_remove_user));
            net.on_signal_network_error().connect(sigc::mem_fun(*this, &chatnetgui::on_network_error));
            m_userlist.on_signal_chat().connect(sigc::mem_fun(*this,&chatnetgui::on_chat));
        }
        m_logview.clear();
        thread_server_handler->create(sigc::mem_fun(net,&network::mainloop),false);
//        thread_client_synchronizer->create(sigc::mem_fun(*this,&chatnetgui::client_synchronizer),false);
        /* Now that we connected some widgets available */
        m_btn_send.set_sensitive(true);
        m_Entry.set_sensitive(true);
        p_menu_file_connect->set_sensitive(false);
        p_toolbar_file_connect->set_sensitive(false);
        p_menu_file_disconnect->set_sensitive(true);
        p_toolbar_file_disconnect->set_sensitive(true);
        myname = m_connect_dialog.get_nickname();

        /* set to true so the signals are not reconnected */
        signals_connected = true;
        m_statusbar.set_text("Connected");
    }
}

/* Displays a message box and hangs until the close button is clicked */
void chatnetgui::display_msgbox(Gtk::Window& parent,const string& title, const string& msg, Gtk::MessageType msg_type)
{
    Gtk::MessageDialog dialog(parent, title,false,msg_type,Gtk::BUTTONS_CLOSE);
    dialog.set_secondary_text(msg);
    int res = dialog.run();
    if(res == Gtk::RESPONSE_CLOSE)
    {
        dialog.hide();
    }

}

void chatnetgui::on_output()
{

    string tempstr;
    /* This section prevents the '/list' output from invalidating the text
    buffer iterators */
    tempstr = net.get_buffer();

    if(command == "/list")
    {
        str_buffer.append(net.get_buffer());
        if(net.get_buffer() != "*end of list.*\n")
        {
            return;
        }
        else
        {
            Glib::RefPtr<Gtk::TextTag> tag;
            tag = Gtk::TextTag::create();
            tag->property_foreground() = "white";
            tag->property_background() = "darkgreen";
            m_logview.get_buffer()->get_tag_table()->add(tag);
            tag->set_priority(0);
            m_logview.log("USERS\n",tag,false);
            tempstr = str_buffer;
            command = ""; // restore
            str_buffer = "";
        }
    }
    // create tag
    Glib::RefPtr<Gtk::TextTag> tag;
    tag = Gtk::TextTag::create();
    tag->property_foreground() = net.get_text_colour();
    m_logview.get_buffer()->get_tag_table()->add(tag);
    tag->set_priority(0);

    // log the text
    m_logview.log(tempstr, tag);


}

/* Responsible for handling network errors from the
network class */
void chatnetgui::on_network_error()
{
    //create a tag
    Glib::RefPtr<Gtk::TextTag> tag;
    tag = Gtk::TextTag::create();
    tag->property_foreground() = "white";
    tag->property_background() = "red";
    m_logview.get_buffer()->get_tag_table()->add(tag);
    tag->set_priority(0);
    // log the error msg
    m_logview.log(net.get_buffer(),tag);

// make some widgets available
    p_menu_file_connect->set_sensitive(true);
    p_toolbar_file_connect->set_sensitive(true);
    p_menu_file_disconnect->set_sensitive(false);
    p_toolbar_file_disconnect->set_sensitive(false);

//clear entry
    m_Entry.set_text("");

//make some widgets unavailable
    m_Entry.set_sensitive(false);
    m_btn_send.set_sensitive(false);

    m_userlist.clear();
    m_statusbar.push("Not connected");
    net.reset();

}


void chatnetgui::on_button_send_clicked()
{
    if(m_Entry.get_text().size() > 0)
    {
// if this client is chating with another user
        if((net.is_chat() == true) && (m_Entry.get_text().operator[](0) != '/'))
        {
            //display his name in front of message
            m_logview.log("<" + myname + "> " + m_Entry.get_text() + "\n");
        }
        else
        {
            command = m_Entry.get_text();
            if(command == "/clear")
            {
                m_logview.clear();
                m_Entry.set_text("");
                return;
            }
            if(command == "/help")
            {
                //create a tag
                Glib::RefPtr<Gtk::TextTag> tag;
                tag = Gtk::TextTag::create();
                tag->property_foreground() = "white";
                tag->property_background() = "darkgreen";
                m_logview.get_buffer()->get_tag_table()->add(tag);
                tag->set_priority(0);
                // log
                m_logview.log("HELP MENU\n",tag);

                m_logview.log(COMMANDS);

                // clear entry
                m_Entry.set_text("");

                return;
            }

        }
        net.input_to_server(m_Entry.get_text());

    }
// clear entry
    m_Entry.set_text("");
}


void chatnetgui::on_menu_file_disconnect()
{
    str_buffer = "/exit";
    net.input_to_server("/exit");

    Glib::RefPtr<Gtk::TextTag> tag;
    tag = Gtk::TextTag::create();
    tag->property_foreground() = "red";
    m_logview.get_buffer()->get_tag_table()->add(tag);
    tag->set_priority(0);
    m_logview.log("Disconnected from chatnet server.\n",tag);

    // clear the user list
    m_userlist.clear();

    // make connection available
    p_menu_file_connect->set_sensitive(true);
    p_toolbar_file_connect->set_sensitive(true);
    p_menu_file_disconnect->set_sensitive(false);
    p_toolbar_file_disconnect->set_sensitive(false);
    m_Entry.set_sensitive(false);
    m_btn_send.set_sensitive(false);

    // set stats
    m_statusbar.push("Not connected");

    str_buffer = "";
    net.reset();
}

void chatnetgui::on_new_user()
{
    m_userlist.add_new_row(net.get_buffer());
}

void chatnetgui::on_remove_user()
{
    m_userlist.remove_row(net.get_buffer());
}

void chatnetgui::on_chat()
{
    str_buffer = "/chat " + m_userlist.get_selected_name();
    net.input_to_server(str_buffer);
    str_buffer = "";
}

void chatnetgui::on_about()
{
    Gtk::AboutDialog dlg;
    dlg.set_name("chatnetgui");
    dlg.set_transient_for(*this);
    dlg.set_version(PACKAGE_VERSION);
    dlg.set_comments("A small chat program");

    dlg.set_copyright(
        "Copyright (C) 2012-2012"
    );

    dlg.set_logo_icon_name("chatnetgui");

    std::deque<Glib::ustring> authors;
    authors.push_back("Developers:");
    authors.push_back("  Chol Nhial <chol.nhial.chol@gmail.com>");
    authors.push_back("");

    dlg.set_authors(authors);

    dlg.set_license(
        "This program is free software; you can redistribute it\n"
        "and/or modify it under the terms of the GNU General Public\n"
        "License as published by the Free Software Foundation; either\n"
        "version 2 of the License, or (at your option) any later\n"
        "version.\n"
        "\n"
        "This program is distributed in the hope that it will be\n"
        "useful, but WITHOUT ANY WARRANTY; without even the implied\n"
        "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR\n"
        "PURPOSE.  See the GNU General Public License for more details."
    );
    dlg.run();
}
