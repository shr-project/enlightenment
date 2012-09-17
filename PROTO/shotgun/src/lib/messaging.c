#include <Ecore.h>
#include "shotgun_private.h"
#include "xml.h"

static void
shotgun_message_free(void *data __UNUSED__, Shotgun_Event_Message *msg)
{
   free(msg->msg);
   eina_stringshare_del(msg->jid);
   free(msg);
}

Shotgun_Event_Message *
shotgun_message_new(Shotgun_Auth *auth)
{
   Shotgun_Event_Message *msg;
   EINA_SAFETY_ON_NULL_RETURN_VAL(auth, NULL);

   msg = calloc(1, sizeof(Shotgun_Event_Message));
   msg->account = auth;
   return msg;
}

void
shotgun_message_feed(Shotgun_Auth *auth, char *data, size_t size)
{
   Shotgun_Event_Message *msg;

   msg = xml_message_read(auth, data, size);
   EINA_SAFETY_ON_NULL_GOTO(msg, error);

   INF("Message from %s: %s", msg->jid, msg->msg);
   ecore_event_add(SHOTGUN_EVENT_MESSAGE, msg, (Ecore_End_Cb)shotgun_message_free, NULL);
   return;
error:
   ERR("wtf");
}

void
shotgun_event_message_free(Shotgun_Event_Message *msg)
{
   if (!msg) return;
   shotgun_message_free(NULL, msg);
}

Eina_Bool
shotgun_message_send(Shotgun_Auth *auth, const char *to, const char *msg, Shotgun_Message_Status status, Eina_Bool xhtml_im)
{
   size_t len;
   char *xml;

   xml = xml_message_write(auth, to, msg, status, &len, xhtml_im);
   shotgun_write(auth->svr, xml, len);
   free(xml);
   return EINA_TRUE;
}

char *
shotgun_htmlize(Eina_Inlist *list, const char *msg)
{
   Eina_Strbuf *buf;
   Shotgun_Custom_Emoticon *emo;
   char *htmlized,
        *tmp;

   buf = eina_strbuf_new();
   if (!buf) return NULL;
   eina_strbuf_append(buf, "<p>");
   eina_strbuf_append(buf, msg);
   eina_strbuf_append(buf, "</p>");

   EINA_INLIST_FOREACH(list, emo)
     {
        tmp = calloc(1, strlen(emo->text) + strlen(emo->cid) + 21);
        sprintf(tmp, "<img alt='%s' src='%s'/>", emo->text, emo->cid);
        eina_strbuf_replace_all(buf, emo->text, tmp);
        free(tmp);
     }

   eina_strbuf_replace_all(buf, "\n", "<br />");

   htmlized = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return htmlized;
}

Eina_Bool
shotgun_hashtml(Eina_Inlist *list, const char *msg)
{
   Shotgun_Custom_Emoticon *emo;
   char *p;

   if (!msg) return EINA_FALSE;

   EINA_INLIST_FOREACH(list, emo)
     {
        p = strstr(msg, emo->text);
        if (p)
        return EINA_TRUE;
     }
   return EINA_FALSE;
}
