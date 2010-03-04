// this space intentionally left blank
#ifndef XPATHEXTRACTOR_H
#define XPATHEXTRACTOR_H
/*
 * Copyright (C) Sparta, Inc 2005.
 *
 * This class extracts values from XML documents given an XPath.
 *
 */

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxml/xpath.h>

namespace SpartaIssoSrdXmlAPI
{

    using namespace std;
    using namespace boost;

    class XPathExtractor
    {
    public:
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
        XPathExtractor(
                const char *extractSpecification, 
                const char *specSeparator = ";", 
                const char *extractionSeparator = ";", 
                const char *nodeValueSeparator = "&", 
                const char *unfoundIndicator = "*");
        virtual ~XPathExtractor();

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
	    string* extractFromDoc(const xmlDoc* doc);
        //
        // Extract info from doc.  Put the results in a vector of
        // vectors of strings. The outer vector holds an element for 
        // each specification in the extractSpecification. Each of 
        // those elements has an element holding the contents of each 
        // match.
        //
        vector<vector<shared_ptr<string> > > extractVectorFromDoc(const xmlDoc* doc);
        //
        // Like "extractFromDoc(const xmlDoc* doc)" but returns an
        // true if data was found and loads the given string with the
        // found data.
        //
	    bool extractFromDoc(const xmlDoc* doci, string &theString);
        //
        // Like "extractFromDoc(const xmlDoc* doc)" but reads a file
        // instead of the given document.
        //
	    string* extractFromFile (const char *fname);
        
	    //
	    // class static functions
	    //

        //
        // XPathExtractor(xpath).extractFromDoc(doc, theString);
        //
	    static bool extractFromDocUsing(const xmlDoc* doc, const char* xpath, string &theString);

        //
        // XPathExtractor(xpath).extractFromDoc(doc);
	    // Caller's responsibility to delete returned value.
        //
	    static string* extractFromDocUsing(const xmlDoc* doc, const char* xpath);

        //
        // XPathExtractor(xpath).extractFromFile(fname);
	    // Caller's responsibility to delete returned value.
        //
        static string* extractFromFileUsing (const char *fname, const char* xpath);
    
    protected:
    
    private:
	    xmlXPathObjectPtr followPath(const xmlDoc* doc, const xmlChar *xpath);
	    string* extractMatchingNodesFromDoc(const string& xpath, const xmlDoc* doc);
        vector<shared_ptr<string> > extractMatchingNodesVectorFromDoc(const string& xpath, const xmlDoc* doc);

        // contains a series of xpaths separated by 
        // delimiting strings that will be used to
		// extract the data indicated by the xpaths.
	    string extractSpec;

        // a string that separates the xpaths in the
		// extraction specification string.
	    string specSep;

        // separates the values extracted for each xpath
        // in the extraction specification string.
	    string extrSep;

        // separates the values extracted for each node
		// when the xpath indicates more than one node.
    	string nodeValSep;

    }; // end of class.

}  // end of namespace;

#endif /* XPATHEXTRACTOR_H */
