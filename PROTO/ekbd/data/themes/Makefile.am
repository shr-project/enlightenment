AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v -id $(top_srcdir)/data/themes -fd $(top_srcdir)/data/themes

filesdir = $(datadir)/ekbd/themes/
files_DATA = default.edj

EXTRA_DIST = \
base_bg.png \
base_menu_cur.png \
base_tie.png \
big_arrow_up.png \
bt_basew.png \
bt_base2.png \
bt_dis_hilight.png \
bt_glow.png \
bt_hilight.png \
bt_hilightw.png \
bt_shine.png \
default.edc \
inset_sunk.png \
key.png \
key_select.png \
k_grad.png \
k_tabhl.png \
k_tabhm.png \
k_tabhr.png \
k_tabl.png \
k_tabr.png \
k_tpsh.png \
k_wtgr.png \
module_icon.png \
shine_tie.png


default.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/default.edc \
	$(top_builddir)/data/themes/default.edj

clean-local:
	rm -f *.edj

