
/*
 * Copyright (C) Sparta, Inc 2005.
 * 
 * C code to pull the attacker and victim node addresses from
 * the False Neighbor Alert message (in IDMEF-defined XML).
 *
 * parseFNMsg() contains the bulk of the logic.  It takes an
 * xmlDocPtr (define by libxml), validates the message document
 * that xmlDocPtr points to against a DTD (also provided
 * as an argument) and parses out strings containing the node 
 * addresses of the attacker and the victim reported by the 
 * message.
 *
 * parseFNMsgFromFile() reads the message from a file and
 * calls parseFNMsg() to return the same addresses.
 *
 * Both functions return zero for success and non-zero for
 * errors.
 *
 * main() exercises parseFNMsgFromFile() using a file name
 * from the command line and the DTD in ./dmef-message.dtd.
 *
 * The only thing at all special needed to compile is libxml2.
 * This will often do the trick:
 *   cc -D GEN_MAIN -I /usr/include/libxml2 -I /usr/include/c++/3.4.2 -lxml2 -lstdc++ XPathExtractor.cpp
 *
 */

#include "XPathExtractor.h"

#include <string.h>
#include <stdlib.h>

#if !defined(LIBXML_XPATH_ENABLED) 
#error("NO XPATH SUPPORT!");
#endif

namespace SpartaIssoSrdXmlAPI 
{

using namespace std;

//
// extractSpecification: Multiple xpaths separated by
// "specSeparator".
//
// "specSeparator": separates multiple xpaths in
// "extractSpecification".
//
// extractionSeparator: If the results are returned as a single
// string, will separarate the results of the separate xpaths in
// the extractSpecification if that holds more than one xpath.
//
// nodeValueSeparator: If the results of an xpath query return
// more than one value and the results are being returned in
// a single string, this will separate the individual results in
// the string.
//
// unfoundIndicator: not used.
//
XPathExtractor::XPathExtractor(
        const char *extractSpecification,
        const char *specSeparator,
        const char *extractionSeparator,
        const char *nodeValueSeparator,
        const char *unfoundIndicator)
: 
    extractSpec(extractSpecification),
    specSep(specSeparator),
    extrSep(extractionSeparator),
    nodeValSep(nodeValueSeparator) 
{
}

// Destructor

// virtual 
XPathExtractor::~XPathExtractor()
{
}

//
// Evaluate the given xpath on the given document.
//
xmlXPathObjectPtr XPathExtractor::followPath(const xmlDoc* doc, const xmlChar *xpath) {
    xmlXPathObjectPtr answer = NULL;
    xmlXPathContextPtr context = xmlXPathNewContext(const_cast<xmlDocPtr>(doc));

    if (context != NULL) {
        answer = xmlXPathEvalExpression(xpath, context);
        xmlXPathFreeContext(context);
        if (answer != NULL) {
            if(xmlXPathNodeSetIsEmpty(answer->nodesetval)){
                xmlXPathFreeObject(answer);
                answer = NULL;
            }
        }
    }
    return answer;
}


//
// Return a string containing substrings separated by "nodeValSep". 
// Each substring is the contents of each node matching the given 
// xpath.
//
string* XPathExtractor::extractMatchingNodesFromDoc(
        const string& xpath, const xmlDoc* doc)
{
    xmlChar *contentStr;
    bool foundSomething = false;

    string* answer = new string();
    if (answer == NULL)
        return NULL;
    xmlXPathObjectPtr pObj = followPath(doc, (const xmlChar *)xpath.c_str());
    if (pObj == NULL) {
        delete answer;
        return NULL;
    }
    xmlNodeSetPtr nodes = pObj->nodesetval;
    int nodeCnt = (nodes == 0) ? 0 : nodes->nodeNr;
    for (int ix=0; ix < nodeCnt; ix++) {
        contentStr = xmlNodeGetContent(nodes->nodeTab[ix]);
        if (contentStr) {
            foundSomething = true;
            if (ix > 0)
                answer->append(nodeValSep);
            answer->append((const char *)contentStr);
            xmlFree(contentStr);
        };
    };
    xmlXPathFreeObject(pObj);
    if (foundSomething)
        return answer;

    // else
    delete answer;
    return NULL;
}

//
// Return a vector containing strings holding the contents of each node
// matching the given xpath.
//
vector<shared_ptr<string> > XPathExtractor::extractMatchingNodesVectorFromDoc(
        const string& xpath, const xmlDoc* doc)
{
    vector<shared_ptr<string> > ret;
    xmlChar *contentStr;

    xmlXPathObjectPtr pObj = followPath(doc, (const xmlChar *)xpath.c_str());
    if(pObj)
    {
        xmlNodeSetPtr nodes = pObj->nodesetval;
        int nodeCnt = (nodes == 0) ? 0 : nodes->nodeNr;
        for (int ix=0; ix < nodeCnt; ix++)
        {
            contentStr = xmlNodeGetContent(nodes->nodeTab[ix]);
            if (contentStr)
            {
                ret.push_back(shared_ptr<string>(new string(reinterpret_cast<char const*>(contentStr))));
                xmlFree(contentStr);
            }
        }
        xmlXPathFreeObject(pObj);
    }
    return ret;
}

//
// Like "extractFromDoc(const xmlDoc* doc)" but returns an
// true if data was found and loads the given string with the
// found data.
//
bool XPathExtractor::extractFromDoc(const xmlDoc* doc, string &theString)
{
    string *tmpString = extractFromDoc(doc);
    if(tmpString)
    {
        theString = *tmpString;
        delete tmpString;
        return true;
    }
    return false;
}

//
// Extract info from doc.  Put the results in a vector of
// vectors of strings. The outer vector holds an element for 
// each specification in the extractSpecification. Each of 
// those elements has an element holding the contents of each 
// match.
//
vector<vector<shared_ptr<string> > > XPathExtractor::extractVectorFromDoc(const xmlDoc* doc)
{
    vector<vector<shared_ptr<string> > > ret;
    string::size_type pos, beg, sz;

    for (pos = beg = 0; beg < extractSpec.size() && pos != string::npos; beg += sz + specSep.size())
    {
        pos = extractSpec.find(specSep, beg);
        sz = (pos == string::npos) ? extractSpec.size() : pos - beg;
        ret.push_back(
            extractMatchingNodesVectorFromDoc(extractSpec.substr(beg, sz), doc));
    }
    return ret;
}

// 
// Extract info from doc.  Put the results in a single string.
// The string can hold multiple values separated by
// "nodeValueSeparator" and, if the "extractSpecification" holds
// multiple xpaths, the results of each xpath will be separated
// by "extractionSeparator".
//
// Returns null if no data found.
//
// Caller's responsibility to delete returned value.
//
string* XPathExtractor::extractFromDoc(const xmlDoc* doc)
{
    string::size_type pos, beg, sz;
    string *answer, *extract;

    answer = new string();

    for (pos = beg = 0; beg < extractSpec.size() && pos != string::npos; beg += sz + specSep.size()) {
        pos = extractSpec.find(specSep, beg);
        sz = (pos == string::npos) ? extractSpec.size() : pos - beg;
        extract = extractMatchingNodesFromDoc(extractSpec.substr(beg, sz), doc);
        if (beg > 0) 
            answer->append(extrSep);
        if (extract) {
            answer->append(*extract);
        }
        delete extract;
    };
    return answer;
}

//
// Like "extractFromDoc(const xmlDoc* doc)" but reads a file
// instead of the given document.
//
string* XPathExtractor::extractFromFile (const char *fname)
{
    xmlDocPtr docIn;

    docIn = xmlReadFile(fname, NULL, XML_PARSE_NOBLANKS /* | XML_PARSE_NOERROR | XML_PARSE_NOWARNING */ );
    if (!docIn)
        return NULL;
    else
        return (extractFromDoc(docIn));
};

//
// XPathExtractor(xpath).extractFromDoc(doc, theString);
//
// class static 
bool XPathExtractor::extractFromDocUsing(const xmlDoc* doc, const char* xpath, string &theString)
{
    return XPathExtractor(xpath).extractFromDoc(doc, theString);
}

//
// XPathExtractor(xpath).extractFromDoc(doc);
// Caller's responsibility to delete returned value.
//
// class static 
string* XPathExtractor::extractFromDocUsing(const xmlDoc* doc, const char* xpath)
{
    return XPathExtractor(xpath).extractFromDoc(doc);
}

//
// XPathExtractor(xpath).extractFromFile(fname);
// Caller's responsibility to delete returned value.
//
// class static
string* XPathExtractor::extractFromFileUsing (const char *fname, const char* xpath)
{
    return XPathExtractor(xpath).extractFromFile(fname);
}


} // end namespace

#if defined(GEN_MAIN)

#include <stdio.h>

using namespace std;
using namespace SpartaIssoSrdXmlAPI;

int main(int argc, char **argv) {

    char* defaultPathList = "/*";
    char* pathList;

    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage:  %s file-to-be-extracted [xpath-list]\n", argv[0]);
        exit(2);
    };

    if (argc == 3)
        pathList = argv[2];
    else
        pathList = defaultPathList;

    XPathExtractor snatchit(pathList);

    string* result = snatchit.extractFromFile(argv[1]);
    if (result)
        printf ("Value extracted:  \"%s\"\n", result->c_str());
    else
        printf ("Error encountered.\n");

    delete result;

    exit(0);
};

#endif // GEN_MAIN
