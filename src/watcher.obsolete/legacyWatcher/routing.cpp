#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "des.h"
#include "routing.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: routing.cpp,v 1.24 2007/04/23 18:51:07 dkindred Exp $";

/* This is a reactive source routing algorithm not unlike AODV.  The primary goal being
** simple implementation.
**
** If it has a route cached, it uses it
** If it dosn't have a route, it floods a route request.
** If it gets a route request, for someone else it repeats it
** If it gets a route request, for itself, it replies to it.
** If it gets a route reply, and its on the route, it forwards it.
**
** Route requests have a list of forwarding nodes which is appended each time it gets forwarded
** that list becomes the route when the route request arrives at its destination.
**
** The packet fields for source routing are annoying.  So instead, while the entire route is
** known, only the next hop is stored/used.  The entire path will have the next hop, because the
** route reply was forwarded along it, and cached.
**
** So how can the end of the route be corrected, as the destination node moves?  If we generate
** extra hop replies as mobility is noted (or listen to the HELLO packets, and note how the
** destination node moves from 1 hop neighbor to 2 hop neighbor or vice versa, then the end
** of the route can be updated on the fly.  
**
*/

#define ROUTINGTTL 100
#define ROUTEMAXAGE 4000
static void intRouteSend(manetNode *us, packet *p);

static routingNode *routeInsert(manetNode *us, ManetAddr dst, ManetAddr nexthop, int distance);
static void routeDelete(manetNode *us, ManetAddr dst) __attribute__((unused));
static routingNode *routeSearch(manetNode *us, ManetAddr dst);

static routingRequest *routingRequestInsert(routingRequest **list, ManetAddr src, ManetAddr dst, int seqnum, int hopcount);
static void routingRequestDelete(routingRequest **list, ManetAddr src)
  __attribute__((unused));
static routingRequest *routingRequestSearch(routingRequest **list, ManetAddr src,ManetAddr dst);

void routePacketRoute(manetNode *us, packet *p);
void routePacketData(manetNode *us, packet *p);
void routePacketTest(manetNode *us, packet *p);

/* This does the encapsulation thing, and attempts to send the packet
*/
void routeSend(manetNode *us, packet *p)
{
	packet *np;
	packetRoutingData *rpd;

#ifdef DEBUG_ROUTING
	fprintf(stderr,"node %d: ROUTING: sending data from %d to %d\n",us->addr & 0xFF,p->src & 0xFF,p->dst & 0xFF);
#endif

	np=packetCopy(p,sizeof(*rpd));
	rpd=(packetRoutingData*)((char *)np->data+np->len-sizeof(*rpd));

	rpd->origtype=np->type;
	rpd->origsrc=p->src;
	rpd->origdst=p->dst;

	np->type=PACKET_ROUTING_DATA;
	np->src=us->addr;
	np->dst=p->dst;

	/* This is an EVIL hack so we don't count this packet twice (intRouteSend() always calls packetRepeatType(), but we don't want to do that on the first hop, since that transmission has been counted by the caller's packetOriginate() call already.  */
//	us->manet->packetrepeat--;us->manet->packethistogram[rpd->origtype].packetrepeat--;

	intRouteSend(us,np);
}

/* Called when we want to send, reattempt to send, or forward an encapsulated packet
** 
** This will either forward the packet if we have a route, or buffer it until we do have a route
*/
static void intRouteSend(manetNode *us, packet *p)
{
	routingNode *re;
	packet *np;
	packetRoutingData *rpd;

	if (p->ttl<=0)
	{
		packetFree(p);
		return;
	}

	rpd=(packetRoutingData*)((char *)p->data+p->len-sizeof(*rpd));

	if ((re=routeSearch(us,rpd->origdst))!=NULL)
	{
		if (re->length<0)
		{
			/* working on route, buffer packet  */

#ifdef DEBUG_ROUTING
			fprintf(stderr,"node %d: no route to %d (but working on it) buffering.\n",us->addr&0xFF,rpd->origdst&0xFF);
#endif

			p->next=us->routing->unsentlist;    /* buffer the packet */
			us->routing->unsentlist=p;
		}
		else
		{
			/* have route: send to next hop  */

			np=packetCopy(p,0);
			rpd=(packetRoutingData*)((char *)np->data+np->len-sizeof(*rpd));
			np->src=us->addr;
			np->dst=re->nexthop;
			np->ttl--;
			np->hopcount++;

			assert(np->type==PACKET_ROUTING_DATA);

#ifdef DEBUG_ROUTING
			fprintf(stderr,"node %d: ROUTING sending packet for %d from %d to %d\n",us->addr&0xFF,rpd->origdst&0xFF,rpd->origsrc&0xFF,np->dst&0xFF);
#endif

			packetSend(us,np, PACKET_REPEAT);
			/* this must not be called on the first hop...  that transmission has already been counted by the caller's packetOriginate.  See evil hack in routeSend()  */
//			packetRepeatType(us,rpd->origtype);

			packetFree(np);
			packetFree(p);     /* matches packetCopy() in routeSend()  */
		}
	}
	else
	{
		/* no route at all, send route request, and buffer packet */

		p->next=us->routing->unsentlist;    /* buffer the packet */
		us->routing->unsentlist=p;

		packet *newp;
		packetRouting *rp;

		routeInsert(us,rpd->origdst,rpd->origdst,-1);    /* insert route-in-progress indication  */

		/* and send a route request */
		newp=packetMalloc(us,sizeof(*rp));
		newp->type=PACKET_ROUTING;
		newp->ttl=ROUTINGTTL;
		newp->dst=NODE_BROADCAST;
		rp=(packetRouting*)newp->data;

		rp->type=ROUTE_RREQUEST;
		rp->numhops=0;
		rp->src=us->addr;
		rp->dst=rpd->origdst;
		rp->seqnum=us->routing->reqseqnum++;

#ifdef DEBUG_ROUTING
		fprintf(stderr,"node %d: no route to %d (requesting it seqnum= %d) buffering.\n",us->addr&0xFF,rpd->origdst&0xFF, rp->seqnum);
#endif

		packetSend(us,newp, PACKET_ORIGIN);
		packetFree(newp);
	}
}

/* walk the list of buffered packets, and send any which we now have routes for
**
** if we still can't send a packet, its just rebuffered.  If we can send it, intRouteSend will do the packetSend and packetFree
*/
static void routeSendCheck(manetNode *us)
{
	packet *p,*np;

	p=us->routing->unsentlist;                 /* take the list...  things will be reinserted by intRouteSend as needed  */
	us->routing->unsentlist=NULL;

	while(p)
	{
		np=p->next;
		intRouteSend(us,p);
		p=np;
	}
}

void routeInit(manetNode *us)
{
	routingState *rt=(routingState*)malloc(sizeof(*rt));

	rt->routelist=NULL;
	rt->unsentlist=NULL;
	rt->reqseqnum=37;
	rt->requestlist=NULL;

	manetPacketHandlerSet(us, PACKET_ROUTING_ROUTE, routePacketRoute);
	manetPacketHandlerSet(us, PACKET_ROUTING_DATA, routePacketData);
	manetPacketHandlerSet(us, PACKET_ROUTING_TEST, routePacketTest);

	us->routing=rt;
}

/* Note that p will be freed by the caller.  We may look but not touch
*/
void routePacketRoute(manetNode *us, packet *p)
{
	packetRouting *rp=(packetRouting*)p->data;
	packet *replypacket;
	packetRouting *reply;
	int pos;

#ifdef DEBUG_ROUTING
	fprintf(stderr,"node %d: ROUTING got a packet from %d to %d\n",us->addr & 0xFF, p->src & 0xFF , p->dst & 0xFF);
#endif
	/* We got a routing packet directly from p->src.  Which means we know how to get there now
	** (they are local...  :-P)
	*/

	routeInsert(us,p->src,p->src,0);

	switch(rp->type)
	{
		case ROUTE_RREQUEST:
		{
			routingRequest *rr;

			if (p->src==us->addr)     /* its one of our transmissions...  */
				return;
			if (rp->src==us->addr)    /* its one of our queries...  */
				return;

#ifdef DEBUG_ROUTING
			fprintf(stderr,"node %d: ROUTING got a routing request from %d to get to %d  seqnum %d\n",us->addr&0xFF,p->src&0xFF,rp->dst&0xFF,rp->seqnum);
#endif

			rr=routingRequestSearch(&(us->routing->requestlist),rp->src,rp->dst);
			if ((rr) && (rp->seqnum <= rr->seqnum))    /* have we seen this request?  (rexmits get new seqnum)  */
			{
#ifdef DEBUG_ROUTING
				fprintf(stderr,"node %d: ROUTING duplicate request\n",us->addr&0xFF);
#endif
				return;
			}

			routingRequestInsert(&(us->routing->requestlist),rp->src,rp->dst,rp->seqnum,p->hopcount);

			/* Got a request.  So we have a route back along the route the request has taken sofar */

			/* save route to get to rp->src:  via the previous hop.  
			**  we should actually be recording the routes for every node on the route...  
			*/
			if (rp->numhops>0)
			{
				routingNode *re;
				int i;

				re=routeSearch(us,rp->src);
				if ((re==NULL) || ((re!=NULL) && (rp->numhops < re->length)))
				{
#ifdef DEBUG_ROUTING
					fprintf(stderr,"node %d: ROUTING add route to %d via %d  len %d\n",us->addr&0xFF,rp->src&0xFF,rp->hops[rp->numhops-1]&0xFF,rp->numhops);
#endif
					routeInsert(us,rp->src,rp->hops[rp->numhops-1],rp->numhops);
				}

				for(i=0;i<(rp->numhops-2);i++)
				{
					re=routeSearch(us,rp->hops[i]);
					if ((re==NULL) || ((re!=NULL) && (rp->numhops < re->length)))
						routeInsert(us,rp->hops[i],rp->hops[rp->numhops-1],rp->numhops);
				}
			}
			/* If rp->numhops==0, this is the first hop of the route request's path, and we got the route already */

			replypacket=packetCopy(p,0);
			reply=(packetRouting*)replypacket->data;
			replypacket->src=us->addr;

			if (rp->dst==us->addr)     /* its a request for us, make a reply */
			{
#if 0
				if ((re!=NULL) && (re->length<0))
					re=NULL;

				if ((re!=NULL) && (rp->numhops > re->length ))   /* we have a shorter route already.  we're off route.  */
				{
					packetFree(replypacket);
					break;
				}
#endif

				reply->type=ROUTE_RREPLY;
				if (reply->numhops>0)
					replypacket->dst=reply->hops[reply->numhops-1];
				else
					replypacket->dst=reply->src;
				replypacket->hopcount=1;
				replypacket->ttl=ROUTINGTTL;

#ifdef DEBUG_ROUTING
				{
					int i;
					fprintf(stderr,"node %d: ROUTING node %d sending reply for %d -> %d ",us->addr & 0xFF, us->addr&0xFF,rp->src&0xFF,rp->dst&0xFF);
					for(i=0;i<reply->numhops;i++)
						fprintf(stderr," %d ",reply->hops[i]&0xFF);
					fprintf(stderr," to %d\n",replypacket->dst & 0xFF);
				}
#endif

				packetSend(us,replypacket,PACKET_ORIGIN);
			}
			else               /* its not for us...  forward it  */
			{ 
#ifdef DEBUG_ROUTING
				fprintf(stderr,"node %d: ROUTING forwarding routing request\n",us->addr&0xFF);
#endif

				reply->type=ROUTE_RREQUEST;
				reply->hops[reply->numhops++]=us->addr;
				replypacket->dst=NODE_BROADCAST;
				replypacket->hopcount++;
				replypacket->ttl--;
				packetSend(us,replypacket, PACKET_REPEAT);
			}
			packetFree(replypacket);
		}
		break;
		case ROUTE_RREPLY:
			/* got a reply... 
			**  only nodes on the route reply.  and its done in the order of the nodes in the list
			**
			** so we have a route looking forward, and a route looking backward
			** but if this is a hopcount=0, then we've already grabbed the looking backward
			*/

			/* check if this route is the shortest.  We're going to get packets from
			** all kinds of [stupid] routes.
			*/

			if (p->src==us->addr)
				return;

			/* what if its a route reply we wanted?   */
			if (rp->src==us->addr)
			{
#ifdef DEBUG_ROUTING
				fprintf(stderr,"node %d: ROUTING got a ROUTE_RREPLY for our query  NH for %d is %d\n",us->addr&0xFF,rp->dst&0xFF,(rp->numhops>0)?(rp->hops[0]&0xFF):(rp->dst&0xFF));
#endif

				routeInsert(us,rp->dst,(rp->numhops>0)?(rp->hops[0]):(rp->dst),rp->numhops);
				break;
			}

			pos=rp->numhops-p->hopcount;   /* our position on the route  */

#ifdef DEBUG_ROUTING
			fprintf(stderr,"node %d: ROUTING got a ROUTE_RREPLY from %d   nh= %d hc= %d pos= %d  lasthop= %d\n",us->addr&0xFF,p->src&0xFF,rp->numhops,p->hopcount,pos,rp->hops[pos]&0xFF);
#endif

			if ((pos>=0) && (rp->hops[pos]==us->addr))    /* are we actually there? If not, this packet is off route, ignore it */
			{
				/* put route onto routing list */
				if (pos>0)
					routeInsert(us,rp->src,rp->hops[pos-1],pos-1);
				if (pos <( rp->numhops-1))
					routeInsert(us,rp->dst,rp->hops[pos+1],rp->numhops-pos);

				/* repeat packet...  */
				replypacket=packetCopy(p,0);
				reply=(packetRouting*)replypacket->data;
				reply->type=ROUTE_RREPLY;
				replypacket->src=us->addr;
				
				if (pos>0)
					replypacket->dst=reply->hops[pos-1];
				else
					replypacket->dst=rp->src;

#ifdef DEBUG_ROUTING
				fprintf(stderr,"node %d: ROUTING we're last hop, forwarding to %d...  \n",us->addr&0xFF,replypacket->dst&0xFF);
#endif

				replypacket->hopcount++;
				packetSend(us,replypacket, PACKET_REPEAT);
				packetFree(replypacket);
			}

		break;
		default:
			fprintf(stderr,"routePacket: ROUTING unknown routing packet type %d\n",rp->type);
			exit(1);
		break;
	}
	/* does it make any unsent packets sendable?  */
	routeSendCheck(us);
}


void routePacketData(manetNode *us, packet *p)
{

#ifdef DEBUG_ROUTING
	fprintf(stderr,"node %d: ROUTING got a packet from %d to %d\n",us->addr & 0xFF, p->src & 0xFF , p->dst & 0xFF);
#endif
	/* We got a routing packet directly from p->src.  Which means we know how to get there now
	** (they are local...  :-P)
	*/

	routeInsert(us,p->src,p->src,0);

#ifdef DEBUG_ROUTING
	fprintf(stderr,"node %d: ROUTING: got a PACKET_ROUTING_DATA packet from %d to %d\n",us->addr&0xFF,p->src&0xFF,p->dst&0xFF);
#endif
	if (p->dst==us->addr)     /* are we next hop?   */
	{
		packetRoutingData *rpd;

		rpd=(packetRoutingData*)((char *)p->data+p->len-sizeof(*rpd));

#ifdef DEBUG_ROUTING
		fprintf(stderr,"node %d: ROUTING: got a PACKET_ROUTING_DATA packet from %d to %d for %d\n",us->addr&0xFF,p->src&0xFF,p->dst&0xFF,rpd->origdst&0xFF);
#endif

		if (rpd->origdst==us->addr)   /* are we final destination?   */
		{
			packet *np;

#ifdef DEBUG_ROUTING
			fprintf(stderr,"node %d: ROUTING got a data packet for us, decapsulating\n",us->addr&0xFF);
#endif

			/* do decapsulation thing */

			np=packetCopy(p,-sizeof(*rpd));
			np->src=rpd->origsrc;
			np->dst=rpd->origdst;
			np->type=rpd->origtype;
			np->hopcount--;   /* TOJ: XXX kludge  */
			packetReReceive(us,np);
			packetFree(np);
		}
		else
		{
#ifdef DEBUG_ROUTING
			fprintf(stderr,"node %d: ROUTING: forwarding...\n",us->addr&0xFF);
#endif
			intRouteSend(us,packetCopy(p,0));     /* forward to next hop  */
		}
	}
#ifdef DEBUG_ROUTING
	else
		fprintf(stderr,"node %d: ROUTING not next hop\n",us->addr&0xFF);
#endif

	/* are we the next hop?  */
	/*	update TTL, dst addr with next hop, and forward it.  */
	/* are we the destination?  */
	/*	decapsulate and rereceive  */


	/* does it make any unsent packets sendable?  */
	routeSendCheck(us);
}

void routePacketTest(manetNode *us, packet *p)
{
#ifdef DEBUG_ROUTING
	fprintf(stderr,"node %d: ROUTING got a packet from %d to %d\n",us->addr & 0xFF, p->src & 0xFF , p->dst & 0xFF);
#endif
	/* We got a routing packet directly from p->src.  Which means we know how to get there now
	** (they are local...  :-P)
	*/

	routeInsert(us,p->src,p->src,0);

#ifdef DEBUG_ROUTING
	fprintf(stderr,"routePacket: ROUTING got test packet\n");
#endif
}

void routeTest(manetNode *us, void *data)
{
	packet *p;
	ManetAddr dst=6;

        timerSet(us,routeTest,1000,NULL);

#ifdef DEBUG_ROUTING
	fprintf(stderr,"node %d: ROUTING %d sending data to %d\n",us->addr&0xFF,us->addr&0xFF,dst&0xFF);
#endif
	p=packetMalloc(us,10);
	p->src=us->addr;
	p->dst=dst;
	p->type=PACKET_ROUTING_TEST;
	p->ttl=255;

	routeSend(us,p);
	packetFree(p);
}

/* Search routing table to see if we have a current route to dst.
**  Note the hack that in-progress routes are stored with a negative distance.
**  They still have a normal expire time though, so if the route request gets dropped,
**  the inprogress times out, and we'll rexmit the request.
*/
static routingNode *routeSearch(manetNode *us, ManetAddr dst)
{
	routingNode *rn;

	rn=us->routing->routelist;

	while(rn)
	{
		if (rn->dst==dst)
		{
			if (rn->expiretime < us->manet->curtime)
				return NULL;
			else
				return rn;
		}
		rn=rn->next;
	}
	return NULL;
}

static void routeDelete(manetNode *us, ManetAddr dst)
{
	routingNode *rn,*q;

	rn=us->routing->routelist;
	q=NULL;

	while(rn)
	{
		if (rn->dst==dst)
			break;
		q=rn;
		rn=rn->next;
	}
	if (rn==NULL)               /* not found...  */
		return;

	if (q==NULL)
		us->routing->routelist=us->routing->routelist->next;
	else
		q->next=rn->next;

	free(rn);
}

static routingNode *routeInsert(manetNode *us, ManetAddr dst, ManetAddr nexthop, int distance)
{
	routingNode *rn;

	rn=us->routing->routelist;

	while(rn)
	{
		if (rn->dst==dst)
			break;
		rn=rn->next;
	}

	if (rn==NULL)
	{
		rn=(routingNode*)malloc(sizeof(*rn));
		rn->next=us->routing->routelist;
		us->routing->routelist=rn;
		rn->dst=dst;
	}
	rn->nexthop=nexthop;
	rn->length=distance;
	rn->expiretime=us->manet->curtime+ROUTEMAXAGE;     /* make this a function of the distance?   */

	return rn;
}


static routingRequest *routingRequestSearch(routingRequest **list, ManetAddr src, ManetAddr dst)
{
	routingRequest *p;

	p=*list;

	while(p)
	{
		if ((p->src==src) && (p->dst==dst))
			return p;
		p=p->next;
	}
	return NULL;
}

static void routingRequestDelete(routingRequest **list, ManetAddr src)
{
	routingRequest *p,*q;

	p=*list;
	q=NULL;

	while(p)
	{
		if (p->src==src)
			break;
		q=p;
		p=p->next;
	}
	if (p==NULL)               /* not found...  */
		return;

	if (q==NULL)
		(*list)=(*list)->next;
	else
		q->next=p->next;

	free(p);
}

static routingRequest *routingRequestInsert(routingRequest **list, ManetAddr src, ManetAddr dst, int seqnum, int hopcount)
{
	routingRequest *p;

	p=routingRequestSearch(list,src,dst);                /* otherwise insert/replace  */
	if (p==NULL)
	{
		p=(routingRequest*)malloc(sizeof(*p));
		p->next=(*list);
		*list=p;
	}
	p->src=src;
	p->dst=dst;
	p->seqnum=seqnum;
	p->hopcount=hopcount;

	return p;
}

void routeDumpBuffered(manetNode *us,FILE *fd)
{
	packet *p,*np;
	packetRoutingData *rpd;

	p=us->routing->unsentlist;                 /* take the list...  things will be reinserted by intRouteSend as needed  */
	us->routing->unsentlist=NULL;

	if (p==NULL)
	{
		fprintf(fd,"node %d: ROUTING: no buffered pkts\n",us->addr & 0xFF);
	}

	while(p)
	{
		np=p->next;

		rpd=(packetRoutingData*)((char *)p->data+p->len-sizeof(*rpd));

		fprintf(fd,"node %d: ROUTING: still buffering packet type %x src= %d dst= %d\n",us->addr & 0xFF, rpd->origtype,rpd->origsrc,rpd->origdst);

		p=np;
	}
}

