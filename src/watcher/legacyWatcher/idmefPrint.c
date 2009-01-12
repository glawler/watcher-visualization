/*
parsing_example1.c: prints out contents of parsed IDMEF XML file.
Author: Adam Migus, NAI Labs, (amigus@nai.com)

Copyright (c) 2001 Networks Associates Technology, Inc.
Copyright (C) 2006  Sparta Inc.

This library is released under the GNU GPL and BSD software licenses.
You may choose to use one or the other, BUT NOT BOTH.  The GNU GPL
license is located in the file named COPYING.  The BSD license is located
in the file named COPYING.BSD.  Please contact us if there are any
questions.

$Id: idmefPrint.c,v 1.8 2007/06/27 22:08:47 mheyman Exp $

*/

static const char *rcsid __attribute__ ((unused)) = "$Id: idmefPrint.c,v 1.8 2007/06/27 22:08:47 mheyman Exp $";

/*
This example program attempts to print out all the elements of an
IDMEF message.  As such it doesn't work (it core dumps) on OS's
whose printf doesn't deal well with null pointers such as Solaris.
It does however work on Linux.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <libidmef/idmefxml_global.h>
#include <libidmef/idmefxml_types.h>
#include <libidmef/idmefxml_parse.h>
#include <libidmef/idmefxml.h>

#include "idmefPrint.h"

static void 
print_additionaldata_content(FILE *fil, IDMEFadditionaldata *data)
{
    if (data)
    {
        const char *value = 0;
        char buf[32];
        xmlBufferPtr xmlbuf = xmlBufferCreate();

        switch(data->type) {
          case IDMEF_BOOLEAN:
              value = boolean_as_string(data->data.data_boolean);
              break;
          case IDMEF_BYTE:
              value = data->data.data_byte;
              break;
          case IDMEF_CHARACTER:
              value = data->data.data_character;
              break;
          case IDMEF_DATETIME:
              value = data->data.data_datetime;
              break;
          case IDMEF_INTEGER:
              sprintf(buf, "%d", data->data.data_integer);
              value = buf;
              break;
          case IDMEF_NTPSTAMP:
              value = data->data.data_ntpstamp;
              break;
          case IDMEF_PORTLIST:
              value = data->data.data_portlist;
              break;
          case IDMEF_REAL:
              value = data->data.data_real;
              break;
          case IDMEF_STRING:
              value = data->data.data_string;
              break;
          case IDMEF_XML:
          {
              xmlDocPtr dummyDoc = xmlNewDoc((unsigned char const*)"1.0");
              xmlNodeDump(xmlbuf, dummyDoc, data->data.data_xml, 
                          0 /* indent lvl */,
                          0 /* format */);
              value = (char const*)xmlbuf->content;
              break;
          }
        }
            
        fprintf(fil, "%s", value);

        xmlBufferFree(xmlbuf);
    }

}

void idmefPrint(FILE *fil, xmlDocPtr messagedoc)
{
	size_t i = 0;
	size_t j = 0;
	size_t k = 0;
	IDMEFmessage *message = 0; 

	message=get_idmef_message_from_doc(messagedoc,1);

	fprintf(fil, "message->version=\'%s\'\n", message->version);

	for(i = 0; i < message->nalerts; i++) {
		fprintf(fil, "message->alerts[%zd]->messageid=\'%s\'\n", i, message->alerts[i]->messageid);
	
		if(message->alerts[i]->analyzer) {
			fprintf(fil, "message->alerts[%zd]->analyzer->analyzerid=\'%s\'\n", i, message->alerts[i]->analyzer->analyzerid);
			fprintf(fil, "message->alerts[%zd]->analyzer->manufacturer=\'%s\'\n", i, message->alerts[i]->analyzer->manufacturer);
			fprintf(fil, "message->alerts[%zd]->analyzer->model=\'%s\'\n", i, message->alerts[i]->analyzer->model);
			fprintf(fil, "message->alerts[%zd]->analyzer->version=\'%s\'\n", i, message->alerts[i]->analyzer->version);
			fprintf(fil, "message->alerts[%zd]->analyzer->cls=\'%s\'\n", i, message->alerts[i]->analyzer->cls);
			fprintf(fil, "message->alerts[%zd]->analyzer->ostype=\'%s\'\n", i, message->alerts[i]->analyzer->ostype);
			fprintf(fil, "message->alerts[%zd]->analyzer->osversion=\'%s\'\n", i, message->alerts[i]->analyzer->osversion);
		
			if(message->alerts[i]->analyzer->node) {
				fprintf(fil, "message->alerts[%zd]->analyzer->node->ident=\'%s\'\n", i, message->alerts[i]->analyzer->node->ident);
				fprintf(fil, "message->alerts[%zd]->analyzer->node->category=\'%s\'(%d)\n", i, node_category_as_string(message->alerts[i]->analyzer->node->category), message->alerts[i]->analyzer->node->category);
				fprintf(fil, "message->alerts[%zd]->analyzer->node->location=\'%s\'\n", i, message->alerts[i]->analyzer->node->location);
				fprintf(fil, "message->alerts[%zd]->analyzer->node->name=\'%s\'\n", i, message->alerts[i]->analyzer->node->name);
				for(j = 0; j < message->alerts[i]->analyzer->node->naddresses; j++) {
					fprintf(fil, "message->alerts[%zd]->analyzer->node->addresses[%zd]->ident=\'%s\'\n", i, j, message->alerts[i]->analyzer->node->addresses[j]->ident);
					fprintf(fil, "message->alerts[%zd]->analyzer->node->addresses[%zd]->category=\'%s\'(%d)\n", i, j, address_category_as_string (message->alerts[i]->analyzer->node->addresses[j]->category), message->alerts[i]->analyzer->node->addresses[j]->category);
					fprintf(fil, "message->alerts[%zd]->analyzer->node->addresses[%zd]->vlan_name=\'%s\'\n", i, j, message->alerts[i]->analyzer->node->addresses[j]->vlan_name);
                    if (message->alerts[i]->analyzer->node->addresses[j]->vlan_num)
                        fprintf(fil, "message->alerts[%zd]->analyzer->node->addresses[%zd]->vlan_num=\'%d\'\n", i, j, *message->alerts[i]->analyzer->node->addresses[j]->vlan_num);
					fprintf(fil, "message->alerts[%zd]->analyzer->node->addresses[%zd]->address=\'%s\'\n", i, j, message->alerts[i]->analyzer->node->addresses[j]->address);
					fprintf(fil, "message->alerts[%zd]->analyzer->node->addresses[%zd]->netmask=\'%s\'\n", i, j, message->alerts[i]->analyzer->node->addresses[j]->netmask);
				}
			}
	
			if(message->alerts[i]->analyzer->process) {
				fprintf(fil, "message->alerts[%zd]->analyzer->process->ident=\'%s\'\n", i, message->alerts[i]->analyzer->process->ident); fprintf(stdout, "message->alerts[%zd]->analyzer->process->name=\'%s\'\n", i, message->alerts[i]->analyzer->process->name);
                if (message->alerts[i]->analyzer->process->pid)
                    fprintf(fil, "message->alerts[%zd]->analyzer->process->pid=\'%d\'\n", i, *message->alerts[i]->analyzer->process->pid);
				fprintf(fil, "message->alerts[%zd]->analyzer->process->path=\'%s\'\n", i, message->alerts[i]->analyzer->process->path);
				for(j = 0; j < message->alerts[i]->analyzer->process->narg; j++) {
					fprintf(fil, "message->alerts[%zd]->analyzer->process->arg[%zd]=\'%s\'\n", i, j, message->alerts[i]->analyzer->process->arg[j]);
				}
				for(j = 0; j < message->alerts[i]->analyzer->process->nenv; j++) {
					fprintf(fil, "message->alerts[%zd]->analyzer->process->env[%zd]=\'%s\'\n", i, j, message->alerts[i]->analyzer->process->env[j]);
				}
			}
		}
	
		if(message->alerts[i]->createtime) {
			fprintf(fil, "message->alerts[%zd]->createtime->ntpstamp=\'%s\'\n", i, message->alerts[i]->createtime->ntpstamp);
			fprintf(fil, "message->alerts[%zd]->createtime->string=\'%s\'\n", i, message->alerts[i]->createtime->string);
			fprintf(fil, "message->alerts[%zd]->createtime->tv=\'%ld, %ld\'\n", i, message->alerts[i]->createtime->tv.tv_sec, message->alerts[i]->createtime->tv.tv_usec);
		}
	
		if(message->alerts[i]->detecttime) {
			fprintf(fil, "message->alerts[%zd]->detecttime->ntpstamp=\'%s\'\n", i, message->alerts[i]->detecttime->ntpstamp);
			fprintf(fil, "message->alerts[%zd]->detecttime->string=\'%s\'\n", i, message->alerts[i]->detecttime->string);
			fprintf(fil, "message->alerts[%zd]->detecttime->tv=\'%ld, %ld\'\n", i, message->alerts[i]->detecttime->tv.tv_sec, message->alerts[i]->detecttime->tv.tv_usec);
		}

		if(message->alerts[i]->analyzertime) {
			fprintf(fil, "message->alerts[%zd]->analyzertime->ntpstamp=\'%s\'\n", i, message->alerts[i]->analyzertime->ntpstamp);
			fprintf(fil, "message->alerts[%zd]->analyzertime->string=\'%s\'\n", i, message->alerts[i]->analyzertime->string);
			fprintf(fil, "message->alerts[%zd]->analyzertime->tv=\'%ld, %ld\'\n", i, message->alerts[i]->analyzertime->tv.tv_sec, message->alerts[i]->analyzertime->tv.tv_usec);
		}

		for(j = 0; j < message->alerts[i]->nsources; j++) {
			fprintf(fil, "message->alerts[%zd]->sources[%zd]->ident=\'%s\'\n", i, j, message->alerts[i]->sources[j]->ident);
			fprintf(fil, "message->alerts[%zd]->sources[%zd]->spoofed=\'%s\'(%d)\n", i, j, spoofed_as_string(message->alerts[i]->sources[j]->spoofed),
			message->alerts[i]->sources[j]->spoofed); fprintf(fil, "message->alerts[%zd]->sources[%zd]->interface=\'%s\'\n", i, j, message->alerts[i]->sources[j]->interface);

			if(message->alerts[i]->sources[j]->node) {
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->ident=\'%s\'\n", i, j, message->alerts[i]->sources[j]->node->ident);
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->category=\'%s\'(%d)\n", i, j, node_category_as_string (message->alerts[i]->sources[j]->node->category), message->alerts[i]->sources[j]->node->category);
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->location=\'%s\'\n", i, j, message->alerts[i]->sources[j]->node->location);
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->name=\'%s\'\n", i, j, message->alerts[i]->sources[j]->node->name);

				for(k = 0; k < message->alerts[i]->sources[j]->node->naddresses; k++) {
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->addresses[%zd]->ident=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->node->addresses[k]->ident);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->addresses[%zd]->category=\'%s\'(%d)\n", i, j, k, address_category_as_string (message->alerts[i]->sources[j]->node->addresses[k]->category), message->alerts[i]->sources[j]->node->addresses[k]->category);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->addresses[%zd]->vlan_name=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->node->addresses[k]->vlan_name);
                    if (message->alerts[i]->sources[j]->node->addresses[k]->vlan_num)
                        fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->addresses[%zd]->vlan_num=\'%d\'\n", i, j, k, *message->alerts[i]->sources[j]->node->addresses[k]->vlan_num);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->addresses[%zd]->address=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->node->addresses[k]->address);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->node->addresses[%zd]->netmask=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->node->addresses[k]->netmask);
				}
			}

			if(message->alerts[i]->sources[j]->user) {
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->user->ident=\'%s\'\n", i, j, message->alerts[i]->sources[j]->user->ident);
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->user->category=\'%s\'(%d)\n", i, j, user_category_as_string (message->alerts[i]->sources[j]->user->category), message->alerts[i]->sources[j]->user->category);
				for(k = 0; k < message->alerts[i]->sources[j]->user->nuserids; k++) {
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->user->userids[%zd]->ident=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->user->userids[k]->ident);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->user->userids[%zd]->type=\'%s\'(%d)\n", i, j, k, userid_type_as_string (message->alerts[i]->sources[j]->user->userids[k]->type), message->alerts[i]->sources[j]->user->userids[k]->type);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->user->userids[%zd]->name=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->user->userids[k]->name);
                    if (message->alerts[i]->sources[j]->user->userids[k]->number)
                        fprintf(fil, "message->alerts[%zd]->sources[%zd]->user->userids[%zd]->number=\'%d\'\n", i, j, k, *message->alerts[i]->sources[j]->user->userids[k]->number);
				}
			}

			if(message->alerts[i]->sources[j]->process) {
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->process->ident=\'%s\'\n", i, j, message->alerts[i]->sources[j]->process->ident);
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->process->name=\'%s\'\n", i, j, message->alerts[i]->sources[j]->process->name);
                if (message->alerts[i]->sources[j]->process->pid)
                    fprintf(fil, "message->alerts[%zd]->sources[%zd]->process->pid=\'%d\'\n", i, j, *message->alerts[i]->sources[j]->process->pid);
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->process->path=\'%s\'\n", i, j, message->alerts[i]->sources[j]->process->path);
				for(k = 0; k < message->alerts[i]->sources[j]->process->narg; k++) {
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->process->arg[%zd]=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->process->arg[k]);
				}
				for(k = 0; k < message->alerts[i]->sources[j]->process->nenv; k++) {
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->process->env[%zd]=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->process->env[k]);
				}
			}

			if(message->alerts[i]->sources[j]->service) {
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->ident=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->ident);
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->name=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->name);
                if (message->alerts[i]->sources[j]->service->port)
                    fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->port=\'%d\'\n", i, j, *message->alerts[i]->sources[j]->service->port);
				fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->portlist=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->portlist);
                if (message->alerts[i]->sources[j]->service->protocol)
                    fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->protocol=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->protocol);

				if(message->alerts[i]->sources[j]->service->webservice) {
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->webservice->url=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->webservice->url);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->webservice->cgi=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->webservice->cgi);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->webservice->http_method=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->webservice->http_method);
					for(k = 0; k < message->alerts[i]->sources[j]->service->webservice->narg; k++) {
						fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->webservice->arg[%zd]=\'%s\'\n", i, j, k, message->alerts[i]->sources[j]->service->webservice->arg[k]);
					}
				}
				if(message->alerts[i]->sources[j]->service->snmpservice) {
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->snmpservice->oid=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->snmpservice->oid);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->snmpservice->community=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->snmpservice->community);
					fprintf(fil, "message->alerts[%zd]->sources[%zd]->service->snmpservice->command=\'%s\'\n", i, j, message->alerts[i]->sources[j]->service->snmpservice->command);
				}
			}
		}
	
		for(j = 0; j < message->alerts[i]->ntargets; j++) {
			fprintf(fil, "message->alerts[%zd]->targets[%zd]->ident=\'%s\'\n", i, j, message->alerts[i]->targets[j]->ident);
			fprintf(fil, "message->alerts[%zd]->targets[%zd]->decoy=\'%s\'(%d)\n", i, j, spoofed_as_string(message->alerts[i]->targets[j]->decoy), message->alerts[i]->targets[j]->decoy);
			fprintf(fil, "message->alerts[%zd]->targets[%zd]->interface=\'%s\'\n", i, j, message->alerts[i]->targets[j]->interface);

			if(message->alerts[i]->targets[j]->node) {
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->ident=\'%s\'\n", i, j, message->alerts[i]->targets[j]->node->ident);
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->category=\'%s\'(%d)\n", i, j, node_category_as_string (message->alerts[i]->targets[j]->node->category), message->alerts[i]->targets[j]->node->category);
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->location=\'%s\'\n", i, j, message->alerts[i]->targets[j]->node->location);
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->name=\'%s\'\n", i, j, message->alerts[i]->targets[j]->node->name);

				for(k = 0; k < message->alerts[i]->targets[j]->node->naddresses; k++) {
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->addresses[%zd]->ident=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->node->addresses[k]->ident);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->addresses[%zd]->category=\'%s\'(%d)\n", i, j, k, address_category_as_string (message->alerts[i]->targets[j]->node->addresses[k]->category), message->alerts[i]->targets[j]->node->addresses[k]->category);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->addresses[%zd]->vlan_name=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->node->addresses[k]->vlan_name);
                    if (message->alerts[i]->targets[j]->node->addresses[k]->vlan_num)
                        fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->addresses[%zd]->vlan_num=\'%d\'\n", i, j, k, *message->alerts[i]->targets[j]->node->addresses[k]->vlan_num);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->addresses[%zd]->address=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->node->addresses[k]->address);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->node->addresses[%zd]->netmask=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->node->addresses[k]->netmask);
				}
			}

			if(message->alerts[i]->targets[j]->user) {
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->user->ident=\'%s\'\n", i, j, message->alerts[i]->targets[j]->user->ident);
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->user->category=\'%s\'(%d)\n", i, j, user_category_as_string (message->alerts[i]->targets[j]->user->category), message->alerts[i]->targets[j]->user->category);
				for(k = 0; k < message->alerts[i]->targets[j]->user->nuserids; k++) {
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->user->userids[%zd]->ident=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->user->userids[k]->ident);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->user->userids[%zd]->type=\'%s\'(%d)\n", i, j, k, userid_type_as_string (message->alerts[i]->targets[j]->user->userids[k]->type), message->alerts[i]->targets[j]->user->userids[k]->type);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->user->userids[%zd]->name=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->user->userids[k]->name);
                    if (message->alerts[i]->targets[j]->user->userids[k]->number)
                        fprintf(fil, "message->alerts[%zd]->targets[%zd]->user->userids[%zd]->number=\'%d\'\n", i, j, k, *message->alerts[i]->targets[j]->user->userids[k]->number);
				}
			}

			if(message->alerts[i]->targets[j]->process) {
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->process->ident=\'%s\'\n", i, j, message->alerts[i]->targets[j]->process->ident);
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->process->name=\'%s\'\n", i, j, message->alerts[i]->targets[j]->process->name);
                if (message->alerts[i]->targets[j]->process->pid)
                    fprintf(fil, "message->alerts[%zd]->targets[%zd]->process->pid=\'%d\'\n", i, j, *message->alerts[i]->targets[j]->process->pid);
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->process->path=\'%s\'\n", i, j, message->alerts[i]->targets[j]->process->path);
				for(k = 0; k < message->alerts[i]->targets[j]->process->narg; k++) {
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->process->arg[%zd]=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->process->arg[k]);
				}
				for(k = 0; k < message->alerts[i]->targets[j]->process->nenv; k++) {
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->process->env[%zd]=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->process->env[k]);
				}
			}

			if(message->alerts[i]->targets[j]->service) {
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->ident=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->ident);
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->name=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->name);
                if (message->alerts[i]->targets[j]->service->port)
                    fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->port=\'%d\'\n", i, j, *message->alerts[i]->targets[j]->service->port);
				fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->portlist=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->portlist);
                if (message->alerts[i]->targets[j]->service->protocol)
                    fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->protocol=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->protocol);

				if(message->alerts[i]->targets[j]->service->webservice) {
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->webservice->url=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->webservice->url);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->webservice->cgi=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->webservice->cgi);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->webservice->http_method=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->webservice->http_method);
					for(k = 0; k < message->alerts[i]->targets[j]->service->webservice->narg; k++) {
						fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->webservice->arg[%zd]=\'%s\'\n", i, j, k, message->alerts[i]->targets[j]->service->webservice->arg[k]);
					}
				}
				if(message->alerts[i]->targets[j]->service->snmpservice) {
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->snmpservice->oid=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->snmpservice->oid);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->snmpservice->community=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->snmpservice->community);
					fprintf(fil, "message->alerts[%zd]->targets[%zd]->service->snmpservice->command=\'%s\'\n", i, j, message->alerts[i]->targets[j]->service->snmpservice->command);
				}
			}
		}
	
        if (message->alerts[i]->classification) {
            fprintf(fil, "message->alerts[%zd]->classification->ident=\'%s\'\n", i, message->alerts[i]->classification->ident);
            fprintf(fil, "message->alerts[%zd]->classification->text=\'%s\'\n", i, message->alerts[i]->classification->text);
            for(j = 0; j < message->alerts[i]->classification->nreferences; j++) {
                fprintf(fil, "message->alerts[%zd]->classification->references[%zd]->origin=\'%s\'(%d)\n", i, j, reference_origin_as_string (message->alerts[i]->classification->references[j]->origin), message->alerts[i]->classification->references[j]->origin);
                fprintf(fil, "message->alerts[%zd]->classification->references[%zd]->name=\'%s\'\n", i, j, message->alerts[i]->classification->references[j]->name);
                fprintf(fil, "message->alerts[%zd]->classification->references[%zd]->url=\'%s\'\n", i, j, message->alerts[i]->classification->references[j]->url);
            }
        }
	
		for(j = 0; j < message->alerts[i]->nadditionaldatas; j++) {
			fprintf(fil, "message->alerts[%zd]->additionaldatas[%zd]->type=\'%s\'(%d)\n", i, j, additionaldata_type_as_string (message->alerts[i]->additionaldatas[j]->type), message->alerts[i]->additionaldatas[j]->type);
			fprintf(fil, "message->alerts[%zd]->additionaldatas[%zd]->meaning=\'%s\'\n", i, j, message->alerts[i]->additionaldatas[j]->meaning);
            fprintf(fil, "message->alerts[%zd]->additionaldatas[%zd]->data=", i, j);
            print_additionaldata_content(fil,message->alerts[i]->additionaldatas[j]);
            fprintf(fil, "\n");
		}
		if(message->alerts[i]->correlationalert) {	
			fprintf(fil, "message->alerts[%zd]->correlationalert->name=\'%s\'\n", i, message->alerts[i]->correlationalert->name);
		
			for(j = 0; j < message->alerts[i]->correlationalert->nalertidents; j++) {
				fprintf(fil, "message->alerts[%zd]->correlationalert->alertidents[%zd]->data=\'%s\'\n", i, j, message->alerts[i]->correlationalert->alertidents[j]->data);
				fprintf(fil, "message->alerts[%zd]->correlationalert->alertidents[%zd]->analyzerid=\'%s\'\n", i, j, message->alerts[i]->correlationalert->alertidents[j]->analyzerid);
			}
		}

		if(message->alerts[i]->toolalert) {	
			fprintf(fil, "message->alerts[%zd]->toolalert->name=\'%s\'\n", i, message->alerts[i]->toolalert->name);
			fprintf(fil, "message->alerts[%zd]->toolalert->command=\'%s\'\n", i, message->alerts[i]->toolalert->command);
			
			for(j = 0; j < message->alerts[i]->toolalert->nalertidents; j++) {
				fprintf(fil, "message->alerts[%zd]->toolalert->alertidents[%zd]->data=\'%s\'\n", i, j, message->alerts[i]->toolalert->alertidents[j]->data);
				fprintf(fil, "message->alerts[%zd]->toolalert->alertidents[%zd]->analyzerid=\'%s\'\n", i, j, message->alerts[i]->toolalert->alertidents[j]->analyzerid);
			}
		}

		if(message->alerts[i]->overflowalert) {
			fprintf(fil, "message->alerts[%zd]->overflowalert->program=\'%s\'\n", i, message->alerts[i]->overflowalert->program);
            if (message->alerts[i]->overflowalert->size)
                fprintf(fil, "message->alerts[%zd]->overflowalert->size=\'%d\'\n", i, *message->alerts[i]->overflowalert->size);
			fprintf(fil, "message->alerts[%zd]->overflowalert->buffer=\'%s\'\n", i, message->alerts[i]->overflowalert->buffer);
		}
	}

	for(i = 0; i < message->nheartbeats; i++) {
		fprintf(fil, "message->heartbeats[%zd]->messageid=\'%s\'\n", i, message->heartbeats[i]->messageid);
	
		if(message->heartbeats[i]->analyzer) {
			fprintf(fil, "message->heartbeats[%zd]->analyzer->analyzerid=\'%s\'\n", i, message->heartbeats[i]->analyzer->analyzerid);
		
			if(message->heartbeats[i]->analyzer->node) {
				fprintf(fil, "message->heartbeats[%zd]->analyzer->node->ident=\'%s\'\n", i, message->heartbeats[i]->analyzer->node->ident);
				fprintf(fil, "message->heartbeats[%zd]->analyzer->node->category=\'%s\'(%d)\n", i, node_category_as_string(message->heartbeats[i]->analyzer->node->category), message->heartbeats[i]->analyzer->node->category);
				fprintf(fil, "message->heartbeats[%zd]->analyzer->node->location=\'%s\'\n", i, message->heartbeats[i]->analyzer->node->location);
				fprintf(fil, "message->heartbeats[%zd]->analyzer->node->name=\'%s\'\n", i, message->heartbeats[i]->analyzer->node->name);
			
				for(j = 0; j < message->heartbeats[i]->analyzer->node->naddresses; j++) {
					fprintf(fil, "message->heartbeats[%zd]->analyzer->node->addresses[%zd]->ident=\'%s\'\n", i, j, message->heartbeats[i]->analyzer->node->addresses[j]->ident);
					fprintf(fil, "message->heartbeats[%zd]->analyzer->node->addresses[%zd]->category=\'%s\'(%d)\n", i, j, address_category_as_string (message->heartbeats[i]->analyzer->node->addresses[j]->category), message->heartbeats[i]->analyzer->node->addresses[j]->category);
					fprintf(fil, "message->heartbeats[%zd]->analyzer->node->addresses[%zd]->vlan_name=\'%s\'\n", i, j, message->heartbeats[i]->analyzer->node->addresses[j]->vlan_name);
                    if (message->heartbeats[i]->analyzer->node->addresses[j]->vlan_num)
                        fprintf(fil, "message->heartbeats[%zd]->analyzer->node->addresses[%zd]->vlan_num=\'%d\'\n", i, j, *message->heartbeats[i]->analyzer->node->addresses[j]->vlan_num);
					fprintf(fil, "message->heartbeats[%zd]->analyzer->node->addresses[%zd]->address=\'%s\'\n", i, j, message->heartbeats[i]->analyzer->node->addresses[j]->address);
					fprintf(fil, "message->heartbeats[%zd]->analyzer->node->addresses[%zd]->netmask=\'%s\'\n", i, j, message->heartbeats[i]->analyzer->node->addresses[j]->netmask);
				}
			}
			
			if(message->heartbeats[i]->analyzer->process) {
				fprintf(fil, "message->heartbeats[%zd]->analyzer->process->ident=\'%s\'\n", i, message->heartbeats[i]->analyzer->process->ident);
				fprintf(fil, "message->heartbeats[%zd]->analyzer->process->name=\'%s\'\n", i, message->heartbeats[i]->analyzer->process->name);
                if (message->heartbeats[i]->analyzer->process->pid)
                    fprintf(fil, "message->heartbeats[%zd]->analyzer->process->pid=\'%d\'\n", i, *message->heartbeats[i]->analyzer->process->pid);
				fprintf(fil, "message->heartbeats[%zd]->analyzer->process->path=\'%s\'\n", i, message->heartbeats[i]->analyzer->process->path);
				for(j = 0; j < message->heartbeats[i]->analyzer->process->narg; j++) {
					fprintf(fil, "message->heartbeats[%zd]->analyzer->process->arg[%zd]=\'%s\'\n", i, j, message->heartbeats[i]->analyzer->process->arg[j]);
				}
				for(j = 0; j < message->heartbeats[i]->analyzer->process->nenv; j++) {
					fprintf(fil, "message->heartbeats[%zd]->analyzer->process->env[%zd]=\'%s\'\n", i, j, message->heartbeats[i]->analyzer->process->env[j]);
				}
			}
		}
	
		if(message->heartbeats[i]->createtime) {
			fprintf(fil, "message->heartbeats[%zd]->createtime->ntpstamp=\'%s\'\n", i, message->heartbeats[i]->createtime->ntpstamp);
			fprintf(fil, "message->heartbeats[%zd]->createtime->string=\'%s\'\n", i, message->heartbeats[i]->createtime->string);
			fprintf(fil, "message->heartbeats[%zd]->createtime->tv=\'%ld, %ld\'\n", i, message->heartbeats[i]->createtime->tv.tv_sec, message->heartbeats[i]->createtime->tv.tv_usec);
		}
	
		if(message->heartbeats[i]->analyzertime) {
			fprintf(fil, "message->heartbeats[%zd]->analyzertime->ntpstamp=\'%s\'\n", i, message->heartbeats[i]->analyzertime->ntpstamp);
			fprintf(fil, "message->heartbeats[%zd]->analyzertime->string=\'%s\'\n", i, message->heartbeats[i]->analyzertime->string);
			fprintf(fil, "message->heartbeats[%zd]->analyzertime->tv=\'%ld, %ld\'\n", i, message->heartbeats[i]->analyzertime->tv.tv_sec, message->heartbeats[i]->analyzertime->tv.tv_usec);
		}
	
		for(j = 0; j < message->heartbeats[i]->nadditionaldatas; j++) {
			fprintf(fil, "message->heartbeats[%zd]->additionaldatas[%zd]->type=\'%s\'(%d)\n", i, j, additionaldata_type_as_string (message->heartbeats[i]->additionaldatas[j]->type), message->heartbeats[i]->additionaldatas[j]->type);
			fprintf(fil, "message->heartbeats[%zd]->additionaldatas[%zd]->meaning=\'%s\'\n", i, j, message->heartbeats[i]->additionaldatas[j]->meaning);
			fprintf(fil, "message->heartbeats[%zd]->additionaldatas[%zd]->data=", i, j);
            print_additionaldata_content(fil,message->heartbeats[i]->additionaldatas[j]);
            fprintf(fil, "\n");
		}
	}
    return;
}
