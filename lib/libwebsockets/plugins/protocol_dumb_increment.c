/*
 * ws protocol handler plugin for "dumb increment"
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * The person who associated a work with this deed has dedicated
 * the work to the public domain by waiving all of his or her rights
 * to the work worldwide under copyright law, including all related
 * and neighboring rights, to the extent allowed by law. You can copy,
 * modify, distribute and perform the work, even for commercial purposes,
 * all without asking permission.
 *
 * These test plugins are intended to be adapted for use in your code, which
 * may be proprietary.  So unlike the library itself, they are licensed
 * Public Domain.
 */

#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <string.h>

#define DUMB_PERIOD_US 50000

struct pss__dumb_increment {
		 const char *usrInput;
		char sendServ[40];
		char *serStat;
		char *chkChar;
		char usrTimeInput[255];
		char usrAlarmInput[255];

	int number;
	
	
	
	
};

struct vhd__dumb_increment {
	const unsigned int *options;
};

static int
callback_dumb_increment(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{
	struct pss__dumb_increment *pss = (struct pss__dumb_increment *)user;
	struct vhd__dumb_increment *vhd =
				(struct vhd__dumb_increment *)
				lws_protocol_vh_priv_get(lws_get_vhost(wsi),
						lws_get_protocol(wsi));
	uint8_t buf[LWS_PRE + 20], *p = &buf[LWS_PRE];
	const struct lws_protocol_vhost_options *opt;
	int n, m;

	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
			lws_get_protocol(wsi),
			sizeof(struct vhd__dumb_increment));
		if (!vhd)
			return -1;
		if ((opt = lws_pvo_search(
				(const struct lws_protocol_vhost_options *)in,
				"options")))
			vhd->options = (unsigned int *)opt->value;
		break;

	case LWS_CALLBACK_ESTABLISHED:
		
		pss->number = 0;


		if (!vhd->options || !((*vhd->options) & 1))
			lws_set_timer_usecs(wsi, DUMB_PERIOD_US);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
			n = lws_snprintf((char *)p, sizeof(buf) - LWS_PRE, "%d", pss->number++);
			m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
			
			if(strlen(pss->usrTimeInput) > 0)
			{
				n = lws_snprintf((char *)p, sizeof(buf) - LWS_PRE+1, "<br> T:%s",pss->usrTimeInput);
				m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
				for(int i = 0; atoi(pss->usrTimeInput) > i; i++)
				{
					memset(pss->usrTimeInput, 0, 255);
				}		
		}		
			
			if(strlen(pss->usrAlarmInput)> 0)
			{
				n = lws_snprintf((char *)p, sizeof(buf) - LWS_PRE+1, "<br> A:%s",pss->usrAlarmInput);
				m = lws_write(wsi, p, n,LWS_WRITE_TEXT);
				for(int i = 0; atoi(pss->usrAlarmInput) > i; i++)
				{
				memset(pss->usrAlarmInput, 0, 255);
				}
			}
			
			
				
		if (m < n) {
			lwsl_err("ERROR %d writing to di socket\n", n);
			return -1;
		}

		break;

	case LWS_CALLBACK_RECEIVE:
		if (len < 6)
			break;
		if (strncmp((const char *)in, "reset\n", 6) == 0)
			pss->number = 0;
		if (strncmp((const char *)in, "closeme\n", 8) == 0) {
			lwsl_notice("dumb_inc: closing as requested\n");
			lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY,
					 (unsigned char *)"seeya", 5);
			return -1;
		}
				
		if(strstr((const char*)in, "time") != NULL)
		{
			pss->usrInput = strpbrk((const char*)in , "0123456789");
			if(pss->usrInput != 0)
			{
				strncpy(pss->usrTimeInput, pss->usrInput, 4);	
			}	
		}
			
		if(strstr((const char*)in, "alarm")!= NULL)
		{
			pss->usrInput = strpbrk((const char*)in , "0123456789");
			if(pss->usrInput != 0)
				strncpy(pss->usrAlarmInput, pss->usrInput, 4);	
		}
		
		if(strncmp((const char*) in, "stat", 4) != NULL)
		{
			pss->serStat = "OK"
		
		}
		
		break;

	case LWS_CALLBACK_TIMER:
		if (!vhd->options || !((*vhd->options) & 1)) {
			lws_callback_on_writable_all_protocol_vhost(
				lws_get_vhost(wsi), lws_get_protocol(wsi));
			lws_set_timer_usecs(wsi, DUMB_PERIOD_US);
		}
		break;

	default:
		break;
	}

	return 0;
}

#define LWS_PLUGIN_PROTOCOL_DUMB_INCREMENT \
	{ \
		"dumb-increment-protocol", \
		callback_dumb_increment, \
		sizeof(struct pss__dumb_increment), \
		10, /* rx buf size must be >= permessage-deflate rx size */ \
		0, NULL, 0 \
	}

#if !defined (LWS_PLUGIN_STATIC)
		
static const struct lws_protocols protocols[] = {
	LWS_PLUGIN_PROTOCOL_DUMB_INCREMENT
};

LWS_VISIBLE const lws_plugin_protocol_t dumb_increment = {
	.hdr = {
		"dumb increment",
		"lws_protocol_plugin",
		LWS_PLUGIN_API_MAGIC
	},

	.protocols = protocols,
	.count_protocols = LWS_ARRAY_SIZE(protocols),
	.extensions = NULL,
	.count_extensions = 0,
};

#endif
