NULL =

INCLUDES = \
	-DPREFIX=\""$(prefix)"\" 				\
	-DSYSCONFDIR=\""$(sysconfdir)"\"			\
	-DLIBDIR=\""$(libdir)"\"				\
	-DDATADIR=\""$(datadir)"\"				\
	-DPKGDATADIR=\""$(datadir)/baobab"\"			\
	-DBAOBAB_PIX_DIR=\""$(datadir)/baobab/pixmaps/"\" 	\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	$(NULL)

bin_PROGRAMS = baobab

baobab_SOURCES = \
	baobab.c \
	baobab.h \
	baobab-cell-renderer-progress.c \
	baobab-cell-renderer-progress.h \
	baobab-ringschart.c \
	baobab-ringschart.h \
	baobab-scan.c \
	baobab-scan.h \
	baobab-export.c \
	baobab-export.h \
	baobab-treeview.c \
	baobab-treeview.h \
	baobab-utils.c \
	baobab-utils.h \
	callbacks.c \
	callbacks.h \
	baobab-prefs.c \
	baobab-prefs.h \
	baobab-remote-connect-dialog.c \
	baobab-remote-connect-dialog.h \
	baobab-chart.c \
	baobab-chart.h \
	baobab-treemap.c \
	baobab-treemap.h

baobab_CFLAGS = 			\
	$(GLIB_CFLAGS)			\
	$(GIO_CFLAGS)			\
	$(GTK_CFLAGS)			\
	$(LIBGTOP_CFLAGS)		\
	$(LIBXML_CFLAGS)		\
	$(NULL)

baobab_LDFLAGS = -export-dynamic
baobab_LDADD = 				\
	-lm				\
	$(GLIB_LIBS)			\
	$(GIO_LIBS)			\
	$(GTK_LIBS)			\
	$(LIBGTOP_LIBS)			\
	$(LIBXML_LIBS)			\
	$(NULL)

