/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * Use Google's libkml to out a KML file based upon the current network topology.
 * @author michael.elkins@cobham.com
 */

// libkml
#include "kml/convenience/convenience.h"
#include "kml/engine.h"
#include "kml/dom.h"
#include "kml/base/file.h"

#include "libwatcher/watcherGraph.h"

#include "initConfig.h"
#include "singletonConfig.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

using kmldom::ChangePtr;
using kmldom::CreatePtr;
using kmldom::CoordinatesPtr;
using kmldom::DocumentPtr;
using kmldom::FeaturePtr;
using kmldom::FolderPtr;
using kmldom::IconStyleIconPtr;
using kmldom::IconStylePtr;
using kmldom::KmlFactory;
using kmldom::KmlPtr;
using kmldom::LabelStylePtr;
using kmldom::LineStringPtr;
using kmldom::LineStylePtr;
using kmldom::MultiGeometryPtr;
using kmldom::PlacemarkPtr;
using kmldom::PointPtr;
using kmldom::StylePtr;
using kmldom::UpdatePtr;
using kmlengine::KmlFile;
using kmlengine::KmlFilePtr;

using namespace watcher;

namespace watcher {
    // set from config file
    extern float LayerPadding;
    extern float Lonoff;
    extern float Latoff;
    extern float Altoff;
    extern int SplineSteps;
    extern float IconScale;
    extern std::string IconPath;
}

namespace {

const std::string BASE_ICON_URL = "http://maps.google.com/mapfiles/kml/shapes/";

// template to get the number of items in an array
template <typename T, int N> size_t sizeof_array( T (&)[N] ) { return N; }

struct LayerInfo {
    float zpad; // alt padding value to order layers vertically
    FolderPtr folder; // folder for this layer
    bool visible; // control whether this layer is output to the KML file
    LayerInfo(float zp, FolderPtr p, bool v) : zpad(zp), folder(p), visible(v) {}
    LayerInfo() : zpad(0.0), visible(true) {}  // required for use in std::map
};

typedef std::map<GUILayer, LayerInfo> LayerMap;
typedef LayerMap::iterator LayerMapIterator;

struct DefinedLabelStyle {
    std::string id;
    watcher::Color color;
    DefinedLabelStyle(const watcher::Color& col, const std::string& name) : id(name), color(col) {}
};

typedef boost::shared_ptr<DefinedLabelStyle> DefinedLabelStylePtr;
typedef std::list<DefinedLabelStylePtr> DefinedLabelStyleList;

class Render {
    public:
        Render(const WatcherGraph&);
        KmlPtr kml;
        void start();
    private:
        KmlFactory *kmlFac;
        const WatcherGraph& graph;
        DocumentPtr doc;
        FolderPtr topFolder;
        LayerMap layerMap;
        int label_count;
        DefinedLabelStyleList definedLabelStyles;
        float zpad;

        const LayerInfo& get_layer(const GUILayer& name, bool visible);
        void create_layers();
        void output_nodes();
        void output_edges();
        void output_floating_labels();
        void add_label(const GUILayer& layer, const std::string& label, const watcher::Color& color, double lat, double lng, double alt);
        void add_label(LabelDisplayInfoPtr dispInfo, double lat, double lng, double alt);
        void add_labels(const std::list<LabelDisplayInfoPtr>&, double lat, double lng, double alt);
        PointPtr create_point(double lat, double lng, double alt);
        std::string get_label_style(const watcher::Color&);
        std::string get_edge_style(const WatcherGraphEdge& edge, unsigned int edgenum);
};

/*
 * Convert a watcher color to the KML format.
 */
std::string watcher_color_to_kml(const watcher::Color& color)
{
    char buf[sizeof("aabbggrr")];
    sprintf(buf, "%02x%02x%02x%02x", color.a, color.b, color.g, color.r);
    return std::string(buf);
}

Render::Render(const WatcherGraph& g) :
    // libkml boilerplate
    kmlFac(kmldom::KmlFactory::GetFactory()),
    graph(g),
    kml(kmlFac->CreateKml()),
    doc(kmlFac->CreateDocument()),
    topFolder(kmlFac->CreateFolder()),
    label_count(0),
    zpad(0.0)
{
    /*
     * create initial DOM tree.  As nodes appear, they get added to the tree. 
     * As nodes move, we update the position attribute
     * in the DOM, and regenerate the KML file.
     */
    kml->set_feature(doc);
    topFolder->set_name("Watcher");
    doc->add_feature(topFolder);
}

void Render::start()
{
    create_layers();
    output_nodes();
    output_edges();
    output_floating_labels();
}

/*
 * Return the KML folder associated with the given layer name.  Creates the layer if it does not already exist.
 */
const LayerInfo& Render::get_layer(const GUILayer& name, bool visible = true)
{
    LayerMapIterator it = layerMap.find(name);
    if (it == layerMap.end()) {
        /*
         * GE doesn't seem to honor the <visibility> attribute, so avoid
         * creating a folder for this layer if it is not visible.
         */
        FolderPtr layer;
        if (visible) {
            layer = kmlFac->CreateFolder();
            layer->set_name(name);
            //layer->set_visibility(visible);
            topFolder->add_feature(layer);
            if (!layerMap.empty())
                zpad += LayerPadding;
        }
        layerMap[name] = LayerInfo(zpad, layer, visible);
        return layerMap[name];
    } else
        return it->second;
}

/*
 * Pre-create layers that are defined in the configuration file.  This allows
 * the stacking order to be fixed between runs.  This also allows layers to be
 * hidden.
 */
void Render::create_layers()
{
    libconfig::Config& cfg = SingletonConfig::instance();
    SingletonConfig::lock();
    libconfig::Setting &root = cfg.getRoot();
    std::string prop = "layers";
    if (!root.exists(prop))
        root.add(prop, libconfig::Setting::TypeGroup);
    libconfig::Setting &layers = cfg.lookup(prop);

    int layerNum = layers.getLength();
    for (int i = 0; i < layerNum; i++) {
        std::string name(layers[i].getName());
        bool val = true;
        if (!layers.lookupValue(name, val))
            layers.add(name, libconfig::Setting::TypeBoolean) = val;  // Shouldn't happen unless misformed cfg file. 
        get_layer(name, val);
    }

    SingletonConfig::unlock();
}

/*
 * many labels will probably share the same attributes, so we cache
 * them to make the KML file smaller
  */
std::string Render::get_label_style(const watcher::Color& color)
{
    BOOST_FOREACH(DefinedLabelStylePtr lsp, definedLabelStyles) {
        if (color == lsp->color)
            return lsp->id;
    }
    // not found, create a new style

    // no icon for labels
    IconStylePtr iconStyle(kmlFac->CreateIconStyle());
    iconStyle->set_scale(0); // scale to 0 means hide it

    kmldom::LabelStylePtr labelStyle(kmlFac->CreateLabelStyle());
    labelStyle->set_color(watcher_color_to_kml(color));
    //TODO: KML doesn't support background colors for placemark labels

    std::string url("label-style-" + boost::lexical_cast<std::string>(label_count));
    ++label_count;

    StylePtr style(kmlFac->CreateStyle());
    style->set_id(url);
    style->set_iconstyle(iconStyle);
    style->set_labelstyle(labelStyle);

    doc->add_styleselector(style);

    definedLabelStyles.push_back( DefinedLabelStylePtr(new DefinedLabelStyle(color, url)));

    return url;
}

void Render::add_label(const GUILayer& layer, const std::string& label, const watcher::Color& color, double lat, double lng, double alt)
{
    const LayerInfo& li (get_layer(layer));

    if (li.visible) {
        PlacemarkPtr place(kmlFac->CreatePlacemark());
        place->set_name(label);
        place->set_geometry(create_point(lat, lng, alt + li.zpad));
        place->set_styleurl("#" + get_label_style(color));

        li.folder->add_feature(place);
    }
}

void Render::add_label(LabelDisplayInfoPtr dispInfo, double lat, double lng, double alt)
{
    add_label(dispInfo->layer, dispInfo->labelText, dispInfo->foregroundColor, lat, lng, alt);
}

// create a placemark for each label, putting into the appropriate layer
void Render::add_labels(const std::list<LabelDisplayInfoPtr>& labels, double lat, double lng, double alt)
{
    BOOST_FOREACH(LabelDisplayInfoPtr dispInfo, labels) {
        add_label(dispInfo, lat, lng, alt);
    }
}

/*
 * Point's can not be shared because even though they are marked const, libKML does modify them.
 * Therefore, we must duplicate the points for each Placemark
 */
PointPtr Render::create_point(double lat, double lng, double alt)
{
    CoordinatesPtr coords(kmlFac->CreateCoordinates());
    coords->add_latlngalt(lat + Latoff, lng + Lonoff, alt);

    PointPtr point(kmlFac->CreatePoint());
    point->set_coordinates(coords);
    point->set_altitudemode(kmldom::ALTITUDEMODE_RELATIVETOGROUND);     // avoid clamping to ground

    return point;
}

void Render::output_floating_labels()
{
    WatcherGraph::FloatingLabelList::const_iterator b = graph.floatingLabels.begin(); 
    WatcherGraph::FloatingLabelList::const_iterator e = graph.floatingLabels.end(); 
    for ( ; b != e; ++b) {
        add_label((*b)->layer, (*b)->labelText, (*b)->foregroundColor, (*b)->lat, (*b)->lng, (*b)->alt);
    }
}

void Render::output_nodes()
{
    int nodecount = 0;

    // iterate over all nodes in the graph
    WatcherGraph::vertexIterator vi, vend;
    for (tie(vi, vend) = vertices(graph.theGraph); vi != vend; ++vi, ++nodecount) {
        const WatcherGraphNode &node = graph.theGraph[*vi]; 

        const LayerInfo& layer(get_layer(node.displayInfo->layer));

        if (layer.visible) {
            PlacemarkPtr ptr = kmlFac->CreatePlacemark();
            //ptr->set_name(node.displayInfo->get_label()); // textual label, can be html
            //ptr->set_geometry(create_point(node.gpsData->y, node.gpsData->x, node.gpsData->z + layer.zpad));
            ptr->set_geometry(create_point(node.gpsData->y, node.gpsData->x, layer.zpad));
            ptr->set_id(node.nodeId.to_string());
            // target id is required when changing some attribute of a feature already in the dom
            //ptr->set_targetid("node0");

            // create style for the icon
            IconStyleIconPtr icon(kmlFac->CreateIconStyleIcon());
            std::string url(BASE_ICON_URL + IconPath);
            icon->set_href(url.c_str());

            IconStylePtr iconStyle(kmlFac->CreateIconStyle());
            iconStyle->set_icon(icon);
            iconStyle->set_color(watcher_color_to_kml(node.displayInfo->color));
            iconStyle->set_scale(IconScale);

            LabelStylePtr labelStyle(kmlFac->CreateLabelStyle());
            labelStyle->set_scale(0); // hide the label, we use separate placemarks for the labels so they can have their own style

            StylePtr style(kmlFac->CreateStyle());
            style->set_iconstyle(iconStyle);
            style->set_labelstyle(labelStyle);

            std::string styleId("node-style-" + boost::lexical_cast<std::string>(nodecount));
            style->set_id(styleId);

            doc->add_styleselector(style);

            ptr->set_styleurl("#" + styleId);

            layer.folder->add_feature(ptr); // add placemark to DOM

            // add a label for the node separate from its placemark icon
            add_label(node.displayInfo->layer, node.displayInfo->get_label(), node.displayInfo->labelColor, node.gpsData->y, node.gpsData->x, 0);
        }

        /*
         * Create a placemark for each label, putting into the appropriate layer.
         * Each label has an independant layer, so this call occurs outside the
         * check for the node layer's visibility.
         */
        add_labels(node.labels, node.gpsData->y, node.gpsData->x, 0);
    }
}

std::string Render::get_edge_style(const WatcherGraphEdge& edge, unsigned int edgenum)
{
        std::string edgeid = "edge-style-" + boost::lexical_cast<std::string>(edgenum);

        LineStylePtr lineStyle(kmlFac->CreateLineStyle());
        lineStyle->set_color(watcher_color_to_kml(edge.displayInfo->color));
        lineStyle->set_width(edge.displayInfo->width);

        IconStylePtr iconStyle(kmlFac->CreateIconStyle());
        iconStyle->set_scale(0); // scale to 0 means hide it

        StylePtr style ( kmlFac->CreateStyle() );
        style->set_id(edgeid);
        style->set_linestyle(lineStyle);
        style->set_iconstyle(iconStyle);

        kmldom::LabelStylePtr labelStyle(kmlFac->CreateLabelStyle());
        if (edge.displayInfo->label == "none") {
            labelStyle->set_scale(0); // hide the label
        }
        labelStyle->set_color(watcher_color_to_kml(edge.displayInfo->labelColor));
        style->set_labelstyle(labelStyle);

        doc->add_styleselector(style);

        return "#" + edgeid;
}

// Draw a spline between the given points at the given altitude
void drawSpline(const GPSMessagePtr& a, const GPSMessagePtr& b, float alt, CoordinatesPtr& coords)
{
    float xstep = ( b->x - a->x ) / SplineSteps;
    float ystep = ( b->y - a->y ) / SplineSteps;
    float x = a->x;
    float y = a->y;

    for (int i = 0; i < SplineSteps; ++i, x += xstep, y += ystep) {
        coords->add_latlngalt(y + Latoff, x + Lonoff, alt);
    }
}

void Render::output_edges()
{
    WatcherGraph::edgeIterator ei, eend;
    int edgenum = 0;

    for (tie(ei, eend) = edges(graph.theGraph); ei != eend; ++ei, ++edgenum) {
        const WatcherGraphEdge &edge = graph.theGraph[*ei]; 
        const WatcherGraphNode &node1 = graph.theGraph[source(*ei, graph.theGraph)]; 
        const WatcherGraphNode &node2 = graph.theGraph[target(*ei, graph.theGraph)]; 

        const LayerInfo& layer(get_layer(edge.displayInfo->layer));

        if (layer.visible) {
            CoordinatesPtr coords = kmlFac->CreateCoordinates();
            drawSpline(node1.gpsData, node2.gpsData, layer.zpad, coords);

            LineStringPtr lineString = kmlFac->CreateLineString();
            lineString->set_coordinates(coords);
            //lineString->set_altitudemode(kmldom::ALTITUDEMODE_ABSOLUTE);    // avoid clamping points to the ground
            //lineString->set_tessellate(true);
            lineString->set_altitudemode(kmldom::ALTITUDEMODE_RELATIVETOGROUND);    // avoid clamping points to the ground

            // place label at the midpoint on the line between the two nodes
            PointPtr point(create_point((node1.gpsData->y + node2.gpsData->y)/2,(node1.gpsData->x + node2.gpsData->x)/2, layer.zpad ));

            /*
             * Google Earth doesn't allow a label to be attached to something without a Point, so
             * we need to create a container with the LineString and the Point at which to attach
             * the label/icon.
             * TODO: this could be optimized in the case where the label is "none", since the Point can
             * be omitted.
             */
            MultiGeometryPtr multiGeo(kmlFac->CreateMultiGeometry());
            multiGeo->add_geometry(lineString);
            multiGeo->add_geometry(point);

            PlacemarkPtr ptr = kmlFac->CreatePlacemark();
            ptr->set_geometry(multiGeo);
            ptr->set_name(edge.displayInfo->label);
            ptr->set_styleurl(get_edge_style(edge, edgenum));

            layer.folder->add_feature(ptr);
        }

        // add additional placemarks for each label on this edge
        add_labels(edge.labels, (node1.gpsData->y + node2.gpsData->y)/2, (node1.gpsData->x + node2.gpsData->x)/2, 0);
    }
}

} // end namespace

void write_kml(const WatcherGraph& graph, const std::string& outputFile)
{
    Render args(graph);
    args.start();

    kmlbase::File::WriteStringToFile(kmldom::SerializePretty(args.kml), outputFile);
}
