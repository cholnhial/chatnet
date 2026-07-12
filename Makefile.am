bin_PROGRAMS = chatnets chatnetgui

chatnets_SOURCES = \
	server/main.cpp \
	server/chatnet.cpp \
	server/chatnet.h \
	server/common.cpp \
	server/global.h
chatnets_LDADD = $(PTHREAD_LIBS)

chatnetgui_SOURCES = \
	client/chatnetgui/src/main.cpp \
	client/chatnetgui/src/chatnetgui.cpp \
	client/chatnetgui/src/chatnetgui.h \
	client/chatnetgui/src/common.cpp \
	client/chatnetgui/src/connect_dialog.cpp \
	client/chatnetgui/src/connect_dialog.h \
	client/chatnetgui/src/global.h \
	client/chatnetgui/src/logview.cpp \
	client/chatnetgui/src/logview.h \
	client/chatnetgui/src/network.cpp \
	client/chatnetgui/src/network.h \
	client/chatnetgui/src/progress_connect.h \
	client/chatnetgui/src/statusbar.cpp \
	client/chatnetgui/src/statusbar.h \
	client/chatnetgui/src/userlist.cpp \
	client/chatnetgui/src/userlist.h
chatnetgui_CPPFLAGS = $(GTKMM_CFLAGS)
chatnetgui_LDADD = $(GTKMM_LIBS) $(PTHREAD_LIBS)

applicationsdir = $(datadir)/applications
applications_DATA = client/chatnetgui/desktop/chatnetgui.desktop

icon48dir = $(datadir)/icons/hicolor/48x48/apps
icon48_DATA = client/chatnetgui/icons/hicolor/48x48/apps/chatnetgui.png

icon64dir = $(datadir)/icons/hicolor/64x64/apps
icon64_DATA = client/chatnetgui/icons/hicolor/scalable/apps/chatnetgui.png

dist_doc_DATA = README.md docs/pseudo_code docs/chatnet-2026.png

install-data-hook:
	$(GTK_UPDATE_ICON_CACHE) --force --ignore-theme-index \
		$(DESTDIR)$(datadir)/icons/hicolor
	$(UPDATE_DESKTOP_DATABASE) $(DESTDIR)$(applicationsdir)

uninstall-hook:
	$(GTK_UPDATE_ICON_CACHE) --force --ignore-theme-index \
		$(DESTDIR)$(datadir)/icons/hicolor
	$(UPDATE_DESKTOP_DATABASE) $(DESTDIR)$(applicationsdir)
