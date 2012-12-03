#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int position;
   int ignore_self_links;
   int fill_side;
};

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->position = sos_config->position;
   cfdata->ignore_self_links = sos_config->ignore_self_links;
   cfdata->fill_side = sos_config->fill_side;
}

static void *
_create_data(E_Config_Dialog *cfd __UNUSED__)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd  __UNUSED__,
           E_Config_Dialog_Data *cfdata)
{
   mod->cfd = NULL;
   free(cfdata);
}

static int
_basic_check_changed(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata)
{
#define CHECK(X) \
   if (cfdata->X != sos_config->X) return 1

   CHECK(position);
   CHECK(ignore_self_links);
   CHECK(fill_side);

#undef CHECK
   return 0;
}

static void
_radio_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   if (cfdata->fill_side != sos_config->fill_side)
     e_config_dialog_changed_set(mod->cfd, 1);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *ob, *ol, *ot, *otb, *tab, *ow;
   E_Radio_Group *rg;

   tab = e_widget_table_add(evas, 0);
   evas_object_name_set(tab, "dia_table");

   otb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);

   ///////////////////////////////////////////

   ol = e_widget_list_add(evas, 0, 0);

   ob = e_widget_check_add(evas, D_("Ignore Self Links"), &cfdata->ignore_self_links);
   e_widget_list_object_append(ol, ob, 1, 0, 0.5);

   ob = e_widget_check_add(evas, D_("Fill side"), &cfdata->fill_side);
   e_widget_list_object_append(ol, ob, 1, 0, 0.5);

   e_widget_toolbook_page_append(otb, NULL, D_("General"), ol, 1, 1, 1, 1, 0.5, 0.5);

   ot = e_widget_table_add(evas, 1);
   rg = e_widget_radio_group_new(&cfdata->position);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-left",
                                24, 24, E_GADCON_ORIENT_LEFT, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 0, 2, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-right",
                                24, 24, E_GADCON_ORIENT_RIGHT, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 2, 2, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-top",
                                24, 24, E_GADCON_ORIENT_TOP, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 1, 0, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-bottom",
                                24, 24, E_GADCON_ORIENT_BOTTOM, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 1, 4, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-top-left",
                                24, 24, E_GADCON_ORIENT_CORNER_TL, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 0, 0, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-top-right",
                                24, 24, E_GADCON_ORIENT_CORNER_TR, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 2, 0, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-bottom-left",
                                24, 24, E_GADCON_ORIENT_CORNER_BL, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 0, 4, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-bottom-right",
                                24, 24, E_GADCON_ORIENT_CORNER_BR, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 2, 4, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-left-top",
                                24, 24, E_GADCON_ORIENT_CORNER_LT, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 0, 1, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-right-top",
                                24, 24, E_GADCON_ORIENT_CORNER_RT, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 2, 1, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-left-bottom",
                                24, 24, E_GADCON_ORIENT_CORNER_LB, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 0, 3, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, "preferences-position-right-bottom",
                                24, 24, E_GADCON_ORIENT_CORNER_RB, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 2, 3, 1, 1, 1, 1, 1, 1);
   ow = e_widget_radio_icon_add(evas, NULL, NULL, //FIXME
                                24, 24, E_GADCON_ORIENT_FLOAT, rg);
   e_widget_on_change_hook_set(ow, _radio_cb, cfdata);
   e_widget_table_object_append(ot, ow, 1, 2, 1, 1, 1, 1, 1, 1);

   e_widget_toolbook_page_append(otb, NULL, D_("Position"), ot, 1, 1, 1, 1, 0.5, 0.5);

   e_widget_toolbook_page_show(otb, 0);

   e_widget_table_object_append(tab, otb, 0, 0, 1, 1, 1, 1, 1, 1);
   return tab;
}


static int
_basic_apply_data(E_Config_Dialog *cfd  __UNUSED__,
                  E_Config_Dialog_Data *cfdata)
{
   if ((cfdata->fill_side != sos_config->fill_side) ||
       (cfdata->ignore_self_links != sos_config->ignore_self_links) ||
       (cfdata->position != sos_config->position)
      )
     {
#define SET(X) sos_config->X = cfdata->X
        SET(position);
        SET(fill_side);
        SET(ignore_self_links);
     }
   e_config_save_queue();
   return 1;
}

E_Config_Dialog *
e_int_config_sos(E_Container *con, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (mod->cfd) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->basic.check_changed = _basic_check_changed;

   mod->cfd = cfd = e_config_dialog_new(con, D_("Shotgun: Sawed-Off"),
     "E", "extensions/sawed-off_shotgun", mod->edj, 32, v, mod);
   return cfd;
}
