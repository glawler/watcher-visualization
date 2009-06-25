/*
 * Copyright 2007 Sparta Inc. 
 *
 * This code will dump the hierarchy in a pile of text such that each
 * leaf node has a line with the following nodes going up to the root
 * (the root node will appear many times).
 */
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>  // for "tie"

#include "apisupport.h" // for "getMilliTime
#include "idsCommunications.h"
#include "graphics.h"
#include "mallocreadline.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: hierarchy2text.cpp,v 1.5 2007/09/05 21:42:14 sherman Exp $";

using boost::graph_traits;
using boost::adjacency_list;
using boost::tie;
using boost::add_edge;
using boost::in_degree;
using std::vector;
using std::for_each;
using std::find_if;

//
// Holds the per-node information. Also tracks the time of last update
// so we can decide when to break out of the select loop.
//
class Node
{
    static void updateFlag(IDSPositionStatus stat, bool &b);
    static void statusUpdate(void *data, ApiStatus *as);
    static void positionUpdate(void *data, IDSPositionType pos,
            IDSPositionStatus stat);

    static void neighborUpdate(void *data, CommunicationsNeighbor *cn);
    public:
    ManetAddr addr;
    int level;
    CommunicationsStatePtr cs; 
    destime lastOpenAttempt;
    bool isRoot;
    bool isRootGroup;
    bool isRegional;
    bool isNeighborhood;
    Node() : 
        addr(0),
        level(0),
        cs(0),
        lastOpenAttempt(0),
        isRoot(false),
        isRootGroup(false),
        isRegional(false),
        isNeighborhood(false)
    {
    }
    void attemptOpen();
};


//
// The edges run from child to parent.
//
// The graph vertices have a "Node", the edges, an int holding the
// number of hops to the parent.
//
typedef adjacency_list<
boost::setS, boost::vecS, boost::bidirectionalS, Node, int > Graph;


destime globalLastUpdate = 0;

static Graph globalGraph;

class MatchAddr
{
    ManetAddr addr;
    public:
    MatchAddr(ManetAddr toMatch) : addr(toMatch) {}
    bool operator()(graph_traits<Graph>::vertex_descriptor const &v)
    {
        return globalGraph[v].addr == addr;
    }
}; // class MatchAddr

static bool find(ManetAddr addr,
        graph_traits<Graph>::vertex_iterator &i_ret)
{
    graph_traits<Graph>::vertex_iterator beg;
    graph_traits<Graph>::vertex_iterator end;
    tie(beg, end) = vertices(globalGraph);
    i_ret = find_if(beg, end, MatchAddr(addr));
    return i_ret != end;
}

//
// Update the given bool based on "stat" and set the last update time if
// an update did occur.
//
void Node::updateFlag(IDSPositionStatus stat, bool &b)
{
    if(stat == IDSPOSITION_INACTIVE)
    {
        if(b)
        {
            globalLastUpdate = getMilliTime();
        }
        b = false;
    }
    else if(stat == IDSPOSITION_ACTIVE)
    {
        if(!b)
        {
            globalLastUpdate = getMilliTime();
        }
        b = true;
    }
} // Node::updateFlag

//
// idsCommunications callback to update the hierarchy level of the
// node. Updates the last update time if the level changes.
//
void Node::statusUpdate(void *data, ApiStatus *as)
{
    Node *n = reinterpret_cast<Node*>(data);
    if(n->level != as->level)
    {
        globalLastUpdate = getMilliTime();
        n->level = as->level;
    }
} // Node::statusUpdate

//
// idsCommunications callback to update the state of the node. Updates
// the last update time if the state changes.
//
void Node::positionUpdate(void *data, IDSPositionType pos, IDSPositionStatus stat)
{
    Node *n = reinterpret_cast<Node*>(data);
    switch(pos)
    {
        case COORDINATOR_ROOT:
            updateFlag(stat, n->isRoot);
            break;
        case COORDINATOR_ROOTGROUP:
            updateFlag(stat, n->isRootGroup);
            break;
        case COORDINATOR_REGIONAL:
            updateFlag(stat, n->isRegional);
            break;
        case COORDINATOR_NEIGHBORHOOD:
            updateFlag(stat, n->isNeighborhood);
            break;
    }
    return;
} // Node::positionUpdate

//
// Got a neighbor update callback telling us of an edge to add.
//
// Returns true if an update occured.
//
static bool updateEdgeAdd(
        graph_traits<Graph>::vertex_descriptor parent,
        graph_traits<Graph>::vertex_descriptor child,
        int distance)
{
    bool ret;
    graph_traits<Graph>::edge_descriptor e;
    bool found;
    tie(e, found) = edge(child, parent, globalGraph);
    if(!found)
    {
        add_edge(child, parent, distance, globalGraph);
        ret = true;
    }
    else
    {
        // already exists, see if distance has changed.
        if(globalGraph[e] != distance)
        {
            globalGraph[e] = distance;
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
    return ret;
} // updateEdgeAdd

//
// Got a neighbor update callback telling us of an edge to remove.
//
// Returns true if an update occured.
//
static bool  updateEdgeRemove(
        graph_traits<Graph>::vertex_descriptor parent,
        graph_traits<Graph>::vertex_descriptor child)
{
    bool ret;
    graph_traits<Graph>::edge_descriptor e;
    bool found;
    tie(e, found) = edge(child, parent, globalGraph);
    if(found)
    {
        remove_edge(child, parent, globalGraph);
        ret = true;
    }
    else
    {
        ret = false;
    }
    return ret;
} // updateEdgeRemove

//
// Modifies edges between the node and its neighbors. Updates the last
// update time if updates actually occur.
//
void Node::neighborUpdate(void *data, CommunicationsNeighbor *cn)
{
    Node *n = reinterpret_cast<Node*>(data);
    while(cn)
    {
        graph_traits<Graph>::vertex_iterator i;
        graph_traits<Graph>::vertex_iterator neighbor_i;
        if(find(n->addr, i) && find(cn->addr, neighbor_i))
        {
            switch(cn->state)
            {
                case COMMUNICATIONSNEIGHBOR_ARRIVING:
                case COMMUNICATIONSNEIGHBOR_UPDATING:
                    // update has same symantics as add for our
                    // situation
                    if(cn->type == COMMUNICATIONSNEIGHBOR_PARENT)
                    {
                        if(updateEdgeAdd(*neighbor_i, *i, cn->distance))
                        {
                            globalLastUpdate = getMilliTime();
                        }
                    }
                    else if(cn->type == COMMUNICATIONSNEIGHBOR_CHILD)
                    {
                        if(updateEdgeAdd(*i, *neighbor_i, cn->distance))
                        {
                            globalLastUpdate = getMilliTime();
                        }
                    }
                    break;
                case COMMUNICATIONSNEIGHBOR_DEPARTING:
                    if(cn->type == COMMUNICATIONSNEIGHBOR_PARENT)
                    {
                        if(updateEdgeRemove(*neighbor_i, *i))
                        {
                            globalLastUpdate = getMilliTime();
                        }
                    }
                    else if(cn->type == COMMUNICATIONSNEIGHBOR_CHILD)
                    {
                        if(updateEdgeRemove(*i, *neighbor_i))
                        {
                            globalLastUpdate = getMilliTime();
                        }
                    }
                    break;
            }
        }
        cn = cn->next;
    }
    return;
} // Node::neighborUpdate

//
// Try to open an idsCommunciations channel for the node.
//
void Node::attemptOpen()
{
    if (!cs)
    {
        destime tick = getMilliTime();
        if ((tick - lastOpenAttempt) > 1000)
        {
            lastOpenAttempt = tick;
            if(addr)
            {
                cs = communicationsInit(addr);
                if(cs)
                {
                    communicationsNameSet(cs,"hierarchy2text","");
                    addr = communicationsNodeAddress(cs);
                    level = 0;
                    communicationsStatusRegister(
                            cs, 
                            0,
                            statusUpdate, 
                            this);
                    idsPositionRegister(
                            cs,
                            COORDINATOR_ROOT,
                            IDSPOSITION_INFORM,
                            positionUpdate,
                            this);
                    idsPositionRegister(
                            cs,
                            COORDINATOR_ROOTGROUP,
                            IDSPOSITION_INFORM,
                            positionUpdate,
                            this);
                    idsPositionRegister(
                            cs,
                            COORDINATOR_REGIONAL,
                            IDSPOSITION_INFORM,
                            positionUpdate,
                            this);
                    idsPositionRegister(
                            cs,
                            COORDINATOR_NEIGHBORHOOD,
                            IDSPOSITION_INFORM,
                            positionUpdate,
                            this);
                    communicationsNeighborRegister(
                            cs, 
                            neighborUpdate,
                            this);
                }
                else
                {
                    fprintf(stderr,"failed errno= %d\n",errno);
                }
            }
        }
    }
    return;
} // Node::attemptOpen

//
// Calls the idsCommunications API to get each node's neighbor list and
// connect up the graph.
//
static void loadDataAndEdges()
{
    size_t updateCount = 0;
    graph_traits<Graph>::vertex_iterator i;
    graph_traits<Graph>::vertex_iterator end;
    for(tie(i, end) = vertices(globalGraph); i != end; ++i)
    {
        Node &n = globalGraph[*i];
        if(n.cs)
        {
            CommunicationsNeighbor *cn =
                communicationsNeighborList(n.cs);
            while(cn)
            {
                graph_traits<Graph>::vertex_iterator neighbor_i;
                if(find(cn->addr, neighbor_i))
                {
                    if(cn->type == COMMUNICATIONSNEIGHBOR_PARENT)
                    {
                        if(updateEdgeAdd(*neighbor_i, *i, cn->distance))
                        {
                            ++updateCount;
                        }
                    }
                    else if(cn->type == COMMUNICATIONSNEIGHBOR_CHILD)
                    {
                        add_edge(*neighbor_i, *i, cn->distance, globalGraph);
                        if(updateEdgeAdd(*i, *neighbor_i, cn->distance))
                        {
                            ++updateCount;
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "Failed to find neighbor %d.%d.%d.%d of %d.%d.%d.%d\n",
                            (cn->addr >> 24) & 0xFF,
                            (cn->addr >> 16) & 0xFF,
                            (cn->addr >> 8) & 0xFF,
                            cn->addr & 0xFF,
                            (n.addr >> 24) & 0xFF,
                            (n.addr >> 16) & 0xFF,
                            (n.addr >> 8) & 0xFF,
                            n.addr & 0xFF);
                }
                cn = cn->next;
            }
            IDSPosition *p = idsPositionCurrent(n.cs);
            while(p)
            {
                switch(p->position)
                {
                    case COORDINATOR_ROOT:
                        if(p->status == IDSPOSITION_INACTIVE)
                        {
                            if(n.isRoot)
                            {
                                n.isRoot = false;
                                ++updateCount;
                            }
                        }
                        else if(p->status == IDSPOSITION_ACTIVE)
                        {
                            if(!n.isRoot)
                            {
                                n.isRoot = true;
                                ++updateCount;
                            }
                        }
                        break;
                    case COORDINATOR_ROOTGROUP:
                        if(p->status == IDSPOSITION_INACTIVE)
                        {
                            if(n.isRootGroup)
                            {
                                n.isRootGroup = false;
                                ++updateCount;
                            }
                        }
                        else if(p->status == IDSPOSITION_ACTIVE)
                        {
                            if(!n.isRootGroup)
                            {
                                n.isRootGroup = true;
                                ++updateCount;
                            }
                        }
                        break;
                    case COORDINATOR_REGIONAL:
                        if(p->status == IDSPOSITION_INACTIVE)
                        {
                            if(n.isRegional)
                            {
                                n.isRegional = false;
                                ++updateCount;
                            }
                        }
                        else if(p->status == IDSPOSITION_ACTIVE)
                        {
                            if(!n.isRegional)
                            {
                                n.isRegional = true;
                                ++updateCount;
                            }
                        }
                        break;
                    case COORDINATOR_NEIGHBORHOOD:
                        if(p->status == IDSPOSITION_INACTIVE)
                        {
                            if(n.isNeighborhood)
                            {
                                n.isNeighborhood = false;
                                ++updateCount;
                            }
                        }
                        else if(p->status == IDSPOSITION_ACTIVE)
                        {
                            if(!n.isNeighborhood)
                            {
                                n.isNeighborhood = true;
                                ++updateCount;
                            }
                        }
                        break;
                }
                p = p->next;
            }
        }
    }
    //fprintf(stderr, "loadDataAndEdges got %d update%s\n",
    //        updateCount, updateCount == 1 ? "" : "s");
} // loadDataAndEdges

//
// Returns a vector of the leaves in globalGraph
//
static vector<graph_traits<Graph>::vertex_descriptor> leaves()
{
    vector<graph_traits<Graph>::vertex_descriptor> ret;
    graph_traits<Graph>::vertex_iterator i;
    graph_traits<Graph>::vertex_iterator end;
    for(tie(i, end) = vertices(globalGraph); i != end; ++i)
    {
        if(in_degree(*i, globalGraph) == 0)
        {
            ret.push_back(*i);
        }
    }
    return ret;
} // leaves

//
// Run a select loop until there are no more updates for "msec"
// milliseconds
//
static void getInformationFromNodes(int msec)
{
    destime curtime = getMilliTime();

    while(1)
    {
        int fdcnt = 0;
        fd_set readfds;
        FD_ZERO(&readfds);	
        graph_traits<Graph>::vertex_iterator i;
        graph_traits<Graph>::vertex_iterator end;
        for(tie(i, end) = vertices(globalGraph); i != end; ++i)
        {
            int fd;
            Node &n = globalGraph[*i];
            if(n.cs == 0)     /* If never managed to open  */
            {
                n.attemptOpen();
                fd = communicationsReturnFD(n.cs);
            }
            else 
            {
                fd = communicationsReturnFD(n.cs);
                if((fd < 0) && ((curtime - n.lastOpenAttempt) > 1000))
                {
                    /* we did successfully open, but its not currently open, AND time for retry.   */
                    if(communicationsReadReady(n.cs) == 0)
                    {
                        n.addr = communicationsNodeAddress(n.cs);
                    }
                    n.lastOpenAttempt = curtime;
                    fd = communicationsReturnFD(n.cs);
                }
            }
            if((n.cs) && (fd >= 0))
            {
                FD_SET(fd, &readfds);
                fdcnt = (fd >= fdcnt) ? fd + 1 : fdcnt;
            }
        }

        if (fdcnt > 0)
        {
            struct timeval timeout = { 1, 0 };
            int rc = select(fdcnt, &readfds, NULL, NULL, &timeout);

            curtime = getMilliTime();

            if(rc == 0)
            {
                if(curtime > globalLastUpdate + msec)
                {
                    break;
                }
            }
            else if (rc < 0)
            {
                perror("checkIO");
                break;
            }
            else
            {
                for(tie(i, end) = vertices(globalGraph); i != end; ++i)
                {
                    Node &n = globalGraph[*i];
                    if(n.cs)
                    {
                        int fd = communicationsReturnFD(n.cs);
                        if(fd >= 0)
                        {
                            if(FD_ISSET(fd, &readfds))
                            {
                                communicationsReadReady(n.cs);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // give nodes a chance
            sleep(1);
        }
    }
    loadDataAndEdges();
    return;
} // getInformationFromNodes

static bool initNodes(char const *prog, Config *conf)
{
    bool ret;
    char const *locfile = configSearchStr(conf, "mobilityinitfile");
    if (locfile)
    {
        char fullpath[PATH_MAX];
        if (0 == configGetPathname(conf,
                    locfile,
                    fullpath,
                    sizeof(fullpath)))
        {
            FILE *fp = fopen(fullpath, "r");
            if (fp)
            {
                for(;;)
                {
                    char *line = mallocreadnextnonblank(fp);
                    if(!line)
                    {
                        break;
                    }
                    struct in_addr dst;
                    for(char *iter = line; *iter; ++iter)
                    {
                        if(isspace(*iter))
                        {
                            *iter = 0;
                            break;
                        }
                    }
                    if(inet_pton(AF_INET, line, &dst) > 0)
                    {
                        ManetAddr addr = ntohl(dst.s_addr);
                        graph_traits<Graph>::vertex_iterator dummy;
                        if(!find(addr, dummy))
                        {
                            graph_traits<Graph>::vertex_descriptor v = 
                                add_vertex(globalGraph);
                            globalGraph[v].addr = addr;
                        }
                    }
                    else
                    {
                        fprintf(stderr, 
                                "%s: Invalid address \"%s\".\n",  
                                prog, line);
                    }
                    free(line);
                }
                fclose(fp);
                ret = true;
            }
            else
            {
                fprintf(stderr,
                        "%s: could not open location file \"%s\".  %s(%d)\n",
                        prog, fullpath, strerror(errno), errno);
                ret = false;
            }
        }
        else
        {
            fprintf(stderr,
                    "%s: configGetPathname(%s) failed!\n",
                    prog, locfile);
            ret = false;
        }
    }
    else
    {
        fprintf(stderr,
                "%s: Configuration file does not contain entry for \"mobilityinitfile\"\n",
                prog);
        ret = false;
    }
    return ret;
} // initNodes

//
// Print a node's address and node state information to stdout
//
static void printNodeVerbose(Node const &n)
{
    printf("%d.%d.%d.%d%s%s%s%s%s%s%s%s%s [level %d]", 
            (n.addr >> 24) & 0xFF,
            (n.addr >> 16) & 0xFF,
            (n.addr >> 8) & 0xFF,
            n.addr & 0xFF,
            n.isRoot || n.isRootGroup || n.isRegional || n.isNeighborhood ? " (" : "",
            n.isRoot ? "Root" : "",
            n.isRoot && n.isRootGroup ? " " : "",
            n.isRootGroup ? "RootGroup" : "",
            (n.isRoot || n.isRootGroup) && n.isRegional ? " " : "",
            n.isRegional ? "Regional" : "",
            (n.isRoot || n.isRootGroup || n.isRegional) && n.isNeighborhood ? " " : "",
            n.isNeighborhood ? "Neighborhood" : "",
            n.isRoot || n.isRootGroup || n.isRegional || n.isNeighborhood ? ")" : "",
            n.level);
} // printNodeVerbose
   
//
// Functor to print a line of the graph. The opererator() call expects
// a leaf node.
//
class PrintLine
{
    bool verbose;
    public:
    PrintLine(bool verboseFlag) : verbose(verboseFlag) {}
    void operator()(graph_traits<Graph>::vertex_descriptor const
            &leaf)
    {
        graph_traits<Graph>::vertex_descriptor v = leaf;
        for(;;)
        {
            Node &n = globalGraph[v];
            if(verbose)
            {
                printNodeVerbose(n);
                printf(" ");
            }
            else
            {
                printf("%d.%d.%d.%d ", 
                        (n.addr >> 24) & 0xFF,
                        (n.addr >> 16) & 0xFF,
                        (n.addr >> 8) & 0xFF,
                        n.addr & 0xFF);
            }
            if(n.isRoot)
            {
                break;
            }
            graph_traits<Graph>::out_edge_iterator oei;
            graph_traits<Graph>::out_edge_iterator oeend;
            tie(oei, oeend) = out_edges(v, globalGraph);
            if(oei != oeend)
            {
                v = target(*oei, globalGraph);
                if(verbose)
                {
                    printf("[%d hop%s] ", 
                            globalGraph[*oei],
                            globalGraph[*oei] == 1 ? "" : "s");
                }
                ++oei;
                if(oei != oeend)
                {
                    // can happen when two nodes have the same child due
                    // to mobility and update skew.
                    fprintf(stderr, "%d.%d.%d.%d can't handle multiple parents\n",
                            (globalGraph[v].addr >> 24) & 0xff,
                            (globalGraph[v].addr >> 16) & 0xff,
                            (globalGraph[v].addr >> 8) & 0xff,
                            globalGraph[v].addr & 0xff);
                    break;
                }
            }
            else
            {
                // disconnected node
                break;
            }
        }
        printf("\n");
        return;
    }
}; // class PrintLine

//
// Dump the graph to stdout such that every leaf node is on a line with
// its chain to the root following.
//
static void printGraph(bool verbose)
{
    vector<graph_traits<Graph>::vertex_descriptor> l = leaves();
    for_each(l.begin(), l.end(), PrintLine(verbose));
    return;
} // printGraph

class PrintChildAndParent
{
    bool verbose;
    bool rootgroup;
    public:
    PrintChildAndParent(bool verboseFlag, bool rootgroupFlag) : verbose(verboseFlag), rootgroup(rootgroupFlag) {}
    void operator()(graph_traits<Graph>::edge_descriptor const &e)
    {
        Node &c = globalGraph[source(e, globalGraph)];
        Node &p = globalGraph[target(e, globalGraph)];
        if(verbose)
        {
            printNodeVerbose(c);
            printf(" [%d hop%s] ", 
                    globalGraph[e],
                    globalGraph[e] == 1 ? "" : "s");
            printNodeVerbose(p);
        }
        else
        {
            printf("%d.%d.%d.%d %d.%d.%d.%d", 
                    (c.addr >> 24) & 0xFF,
                    (c.addr >> 16) & 0xFF,
                    (c.addr >> 8) & 0xFF,
                    c.addr & 0xFF,
                    (p.addr >> 24) & 0xFF,
                    (p.addr >> 16) & 0xFF,
                    (p.addr >> 8) & 0xFF,
                    p.addr & 0xFF);
        }
        if(rootgroup && c.isRootGroup)
        {
            printf(" ROOTGROUP");
        }
        printf("\n");
        return;
    }
}; // PrintChildAndParent

//
// Functor to print a node and its parent.
//
class PrintMeAndMyParents
{
    bool verbose;
    bool rootgroup;
    public:
    PrintMeAndMyParents(bool verboseFlag, bool rootgroupFlag) : verbose(verboseFlag), rootgroup(rootgroupFlag) {}
    void operator()(graph_traits<Graph>::vertex_descriptor const &me)
    {
        graph_traits<Graph>::out_edge_iterator beg;
        graph_traits<Graph>::out_edge_iterator end;
        tie(beg, end) = out_edges(me, globalGraph);
        for_each(beg, end, PrintChildAndParent(verbose, rootgroup));
        return;
    }
}; // class PrintMeAndMyParents

//
//
//
static void printGraphPairs(bool verbose, bool rootgroup)
{
    graph_traits<Graph>::vertex_iterator beg;
    graph_traits<Graph>::vertex_iterator end;
    tie(beg, end) = vertices(globalGraph);
    for_each(beg, end, PrintMeAndMyParents(verbose, rootgroup));
    return;
} // printGraphPairs

//
// Don't long anything
//
static void nolog(char const *fmt, ...)
{
    return;
}

//
// Print a small header 
//
static void printHeader()
{
    struct tm tm;
    struct timeval tv;
    char buf[128];
    gettimeofday(&tv, 0);
    gmtime_r(&tv.tv_sec, &tm);
    snprintf(buf, sizeof(buf), "%04d%02d%02d %02d:%02d:%02d.%03ld (%lu)",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            tv.tv_usec / 1000, 
            tv.tv_sec);
    fprintf(stdout, "Hierarchy Relationships at: %s\n", buf); 
}

//
// 
//
int main(int argc, char **argv)
{
    char const *prog = strrchr(argv[0], '/') ?
        strrchr(argv[0], '/') + 1 : argv[0];
    Config *conf;
    int ch;
    bool verbose = false;
    bool pairs = false;
    bool rootgroup = false;
    int pausetime = 0;
    int repeatcount = -1; 

    while((ch=getopt(argc, argv, "vprt:c:"))!=-1)
    {
        switch(ch)
        {
            case 'v':
                verbose = true;
                break;
            case 'p':
                pairs = true;
                break;
            case 'r':
                rootgroup = true;
                pairs = true;
                break;
            case 't':
            case 'c':
                {
                    char *endptr=NULL;
                    if(ch=='t') pausetime = strtol(optarg, &endptr, 10);
                    else repeatcount = strtol(optarg, &endptr, 10); 
                    if(errno==ERANGE || *endptr)
                    {
                        fprintf(stderr, "Unable to parse '%c' argument: %s. It should be an integer of "
                                "reasonable size.\n", ch=='t'?'t':'c', optarg); 
                        exit(1); 
                    }
                    if(ch=='c' && repeatcount<=0)
                    {
                        fprintf(stderr, "Count must be positive.\n"); 
                        exit(1);
                    }
                }
                break;
                
            case '?':
            default:
                fprintf(stderr,
                        "usage: %s [-vpr] configfile\n"
                        "   -p    prints only child-parent pairs\n"
                        "   -r    prints child-parent pairs and whether child is in rootgroup\n"
                        "         (the 'r' option forces the 'p' option)\n"
                        "   -v    prints additional information such as node type and hop count\n"
            			"   -t    display hierarchy information every 't' seconds\n"
                        "   -c    display hierarchy information 'c' times. Only use with 't'\n",    
                        prog);
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    if(!pausetime && repeatcount>0) 
    {
        fprintf(stderr, "The count 'c' option can only be used with the time 't' option.\n"); 
        exit(1); 
    }

    conf = configLoad(argc > 0 ? argv[0] : NULL);
    if(conf)
    {
        communicationsLogDebugDefaultSet(nolog);
        if(initNodes(prog, conf))
        {
            do
            {
                getInformationFromNodes(1000);

                // Don't bother with header if we are only printing
                // once. 
                if(pausetime) printHeader(); 
                if(pairs)
                {
                    printGraphPairs(verbose, rootgroup);
                }
                else
                {
                    printGraph(verbose);
                }
                fflush(stdout); 

            } while(pausetime && 0==sleep(pausetime) && --repeatcount!=0);
          
        }
        else
        {
            fprintf(stderr, "%s: Failed to initialize nodes\n", prog);
        }
    }
    else
    {
        fprintf(stderr,"%s: could not open config file %s!\n", prog, argc > 0 ? argv[0] : "");
        exit(1);
    }
    return 0;
} // main

// vim:expandtab tabstop=4 textwidth=72 shiftwidth=4 shiftround 
