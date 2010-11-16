#include <math.h> 
#include <logger.h>
#include <libwatcher/colors.h>
#include <libwatcher/seekWatcherMessage.h>
#include <libwatcher/messageTypesAndVersions.h>

#include "watcherQtGUIConfig.h"
#include "backgroundImage.h"
#include "singletonConfig.h"

using namespace watcher;

INIT_LOGGER(WatcherGUIConfig, "WatcherGUIConfig");

WatcherGUIConfig::WatcherGUIConfig() :
    monochromeMode(false), 
    threeDView(false),
    backgroundImage(false), 
    showGroundGrid(false),
    showWallTimeinStatusString(true),
    showPlaybackTimeInStatusString(true),
    showPlaybackRangeString(true),
    showVerboseStatusString(false),
    showGlobalView(false),
    showBoundingBox(false),
    showDebugInfo(false), 
    showStreamDescription(true),
    autorewind(false), 
    messageStreamFiltering(false),
    maxNodes(0),
    maxLayers(0),
    statusFontPointSize(10),
    statusFontName("Helvetica"),
    hierarchyRingColor(watcher::colors::blue),
    playbackStartTime(event::SeekMessage::eof),  // live mode
    scaleText(20.0), 
    scaleLine(1.0), 
    gpsScale(60000), 
    layerPadding(2.0), 
    antennaRadius(100.0),
    ghostLayerTransparency(0.15)
{
    rgbaBGColors[0]=rgbaBGColors[1]=rgbaBGColors[2]=rgbaBGColors[3]=0.0;
    manetAdjInit.angleX=0.0;
    manetAdjInit.angleY=0.0;
    manetAdjInit.angleZ=0.0;
    manetAdjInit.scaleX=0.035;
    manetAdjInit.scaleY=0.035;
    manetAdjInit.scaleZ=0.03;
    manetAdjInit.shiftX=0.0;
    manetAdjInit.shiftY=0.0;
    manetAdjInit.shiftZ=0.0;
}

WatcherGUIConfig::~WatcherGUIConfig()
{

}

bool WatcherGUIConfig::loadConfiguration()
{
    TRACE_ENTER();

    scaleText=20;
    scaleLine=1.0;
    monochromeMode = false;
    threeDView = true;
    backgroundImage = true;
    showGroundGrid=false;
    bool retVal=true;

    //
    // Check configuration for GUI settings.
    //
    libconfig::Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();
    libconfig::Setting &root=cfg.getRoot();

    root.lookupValue("maxNodes", maxNodes);
    root.lookupValue("maxLayers", maxLayers); 
    root.lookupValue("server", serverName); 

    try {
        std::string prop="layers";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);

        libconfig::Setting &layers=cfg.lookup(prop);

        LOG_INFO("Reading layer states from cfg file");
        bool foundPhy=false;
        int layerNum=layers.getLength();
        for (int i=0; i<layerNum; i++)
        {
            std::string name(layers[i].getName());
            LOG_DEBUG("Reading layer menu config for " << name); 
            bool val=true;
            if (!layers.lookupValue(name, val))
                layers.add(name, libconfig::Setting::TypeBoolean)=val;  // Shouldn't happen unless misformed cfg file. 
            activeLayers[name]=val; 
            initialLayers.push_back(InitialLayer(name, val)); 
            if (name==event::PHYSICAL_LAYER) 
                foundPhy=true;
        }
        if (!foundPhy) { 
            activeLayers[event::PHYSICAL_LAYER]=true; 
            initialLayers.push_back(InitialLayer(event::PHYSICAL_LAYER, true)); 
        }

        struct 
        {
            const char *prop; 
            bool def; 
            bool *val; 
        } boolVals[] = 
        {
            { "nodes3d", true, &threeDView },
            { "monochrome", false, &monochromeMode }, 
            { "displayBackgroundImage", true, &backgroundImage },
            { "showGlobalView", false, &showGlobalView },
            { "showBoundingBox", false, &showBoundingBox },
            { "showGroundGrid", false, &showGroundGrid },
            { "showVerboseStatusString", false, &showVerboseStatusString }, 
            { "showWallTime", true, &showWallTimeinStatusString }, 
            { "showPlaybackTime", true, &showPlaybackTimeInStatusString },
            { "showPlaybackRange", true, &showPlaybackRangeString },
            { "showDebugInfo", false, &showDebugInfo },
            { "autorewind", true, &autorewind },
            { "messageStreamFiltering", false, &messageStreamFiltering }
        }; 
        for (size_t i=0; i<sizeof(boolVals)/sizeof(boolVals[0]); i++)
        {
            prop=boolVals[i].prop;
            bool boolVal=boolVals[i].def; 
            if (!root.lookupValue(prop, boolVal))
                root.add(prop, libconfig::Setting::TypeBoolean)=boolVal;
            LOG_DEBUG("Setting " << boolVals[i].prop << " to " << (boolVal?"true":"false")); 
            *boolVals[i].val=boolVal; 
        }

        std::string hrc("blue");
        struct 
        {
            const char *prop; 
            const char *def; 
            std::string *val; 
        } strVals[] = 
        {
            { "statusFontName", "Helvetica", &statusFontName },
            { "hierarchyRingColor", "blue", &hrc } 
        }; 
        for (size_t i=0; i<sizeof(strVals)/sizeof(strVals[0]); i++)
        {
            prop=strVals[i].prop;
            std::string strVal=strVals[i].def; 
            if (!root.lookupValue(prop, strVal))
                root.add(prop, libconfig::Setting::TypeString)=strVal;
            *strVals[i].val=strVal; 
            LOG_DEBUG("Setting " << strVals[i].prop << " to " << strVal);
        }
        hierarchyRingColor.fromString(hrc);

        struct 
        {
            const char *prop; 
            float def; 
            float *val; 
        } floatVals[] = 
        {
            { "scaleText", 1.0, &scaleText }, 
            { "scaleLine", 1.0, &scaleLine }, 
            { "layerPadding", 1.0, &layerPadding }, 
            { "gpsScale", 512.0, &gpsScale }, 
            { "antennaRadius", 200.0, &antennaRadius },
            { "ghostLayerTransparency", 0.15, &ghostLayerTransparency }
        }; 
        for (size_t i=0; i<sizeof(floatVals)/sizeof(floatVals[0]); i++)
        {
            prop=floatVals[i].prop;
            float floatVal=floatVals[i].def;
            if (!root.lookupValue(prop, floatVal))
                root.add(prop, libconfig::Setting::TypeFloat)=floatVals[i].def;
            *floatVals[i].val=floatVal;
            LOG_DEBUG("Setting " << floatVals[i].prop << " to " << *floatVals[i].val); 
        }

        struct 
        {
            const char *prop; 
            int def; 
            int *val; 
        } intVals[] = 
        {
            { "statusFontPointSize", 12, &statusFontPointSize }
        }; 
        for (size_t i=0; i<sizeof(intVals)/sizeof(intVals[0]); i++)
        {
            prop=intVals[i].prop;
            int intVal=intVals[i].def; 
            if (!root.lookupValue(prop, intVal))
                root.add(prop, libconfig::Setting::TypeInt)=intVal;
            *intVals[i].val=intVal; 
            LOG_DEBUG("Setting " << intVals[i].prop << " to " << intVal);
        }

        struct 
        {
            const char *prop; 
            long long int def;
            long long int *val; 
        } longlongIntVals[] = 
        {
            { "playbackStartTime", event::SeekMessage::eof, &playbackStartTime } 
        }; 
        for (size_t i=0; i<sizeof(longlongIntVals)/sizeof(longlongIntVals[0]); i++) {
            prop=longlongIntVals[i].prop;
            long long int intVal=longlongIntVals[i].def; 
            if (!root.lookupValue(prop, intVal))
                root.add(prop, libconfig::Setting::TypeInt64)=intVal;
            *longlongIntVals[i].val=intVal; 
            LOG_DEBUG("Setting " << longlongIntVals[i].prop << " to " << intVal);
        }

        //
        // Load background image settings
        //
        prop="backgroundImage";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &s=cfg.lookup(prop);

        prop="imageFile"; 
        std::string strVal;
        if (!s.lookupValue(prop, strVal) || strVal=="none")   // If we don't have the setting or the it's set to 'do not use bg image'
        {
            LOG_INFO("watcherBackground:imageFile entry not found (or it equals \"none\") in configuration file, "
                    "disabling background image functionality");
            if (strVal.empty())
                s.add(prop, libconfig::Setting::TypeString)="none";
            backgroundImage=false;
        }
        else {
            if (!BackgroundImage::getInstance().loadImageFile(strVal)) {
                LOG_WARN("Error loading background image file, " << strVal); 
            }
        }

        // bg image location and size.
        prop="coordinates";
        float coordVals[5]={0.0, 0.0, 0.0, 0.0, 0.0};
        if (!s.exists(prop))
        {
            s.add(prop, libconfig::Setting::TypeArray);
            for (size_t i=0; i<sizeof(coordVals)/sizeof(coordVals[0]); i++)
                s[prop].add(libconfig::Setting::TypeFloat)=coordVals[i];
        }
        else
        {
            for (size_t i=0; i<sizeof(coordVals)/sizeof(coordVals[0]); i++)
                coordVals[i]=s[prop][i];
            BackgroundImage &bg=BackgroundImage::getInstance();
            bg.setDrawingCoords(coordVals[0], coordVals[1], coordVals[2], coordVals[3], coordVals[4]); 
        }

        //
        // Load viewpoint
        //
        prop="viewPoint";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &vp=cfg.lookup(prop); 

        struct 
        {
            const char *type;
            float *data[3];
        } viewPoints[] =
        {
            { "angle", { &manetAdj.angleX, &manetAdj.angleY, &manetAdj.angleZ }},
            { "scale", { &manetAdj.scaleX, &manetAdj.scaleY, &manetAdj.scaleZ }},
            { "shift", { &manetAdj.shiftX, &manetAdj.shiftY, &manetAdj.shiftZ }}
        };
        for (size_t i=0; i<sizeof(viewPoints)/sizeof(viewPoints[0]);i++)
        {
            if (!vp.exists(viewPoints[i].type))
            {
                vp.add(viewPoints[i].type, libconfig::Setting::TypeArray);
                for (size_t j=0; j<sizeof(viewPoints[i].data)/sizeof(viewPoints[i].data[0]); j++)
                    vp[viewPoints[i].type].add(libconfig::Setting::TypeFloat);
            }
            else
            {
                libconfig::Setting &s=vp[viewPoints[i].type];
                for (int j=0; j<s.getLength(); j++)
                    *viewPoints[i].data[j]=s[j];
            }
        }

        LOG_INFO("Set viewpoint - angle: " << manetAdj.angleX << ", " << manetAdj.angleY << ", " << manetAdj.angleZ);
        LOG_INFO("Set viewpoint - scale: " << manetAdj.scaleX << ", " << manetAdj.scaleY << ", " << manetAdj.scaleZ);
        LOG_INFO("Set viewpoint - shift: " << manetAdj.shiftX << ", " << manetAdj.shiftY << ", " << manetAdj.shiftZ);

        // background color
        prop="backgroundColor";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &bgColSet=cfg.lookup(prop);

        struct 
        {
            const char *name;
            float val;
        } bgColors[] = 
        {
            { "r", 0.0 }, 
            { "g", 0.0 }, 
            { "b", 0.0 }, 
            { "a", 1.0 }
        };
        for (size_t i=0; i<sizeof(bgColors)/sizeof(bgColors[0]);i++)
            if (!bgColSet.lookupValue(bgColors[i].name, bgColors[i].val))
                bgColSet.add(bgColors[i].name, libconfig::Setting::TypeFloat)=bgColors[i].val;

        rgbaBGColors[0]=bgColors[0].val;
        rgbaBGColors[1]=bgColors[1].val;
        rgbaBGColors[2]=bgColors[2].val;
        rgbaBGColors[3]=bgColors[3].val;
    }
    catch (const libconfig::SettingException &e) {
        LOG_ERROR("Error loading configuration at " << e.getPath() << ": " << e.what());
        retVal=false;
    }

    SingletonConfig::unlock();

    TRACE_EXIT();
    return retVal;
}

bool WatcherGUIConfig::saveConfiguration()
{
    TRACE_ENTER();
    LOG_DEBUG("Got close event, saving modified configuration"); 

    libconfig::Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock(); 
    libconfig::Setting &root=cfg.getRoot();

    try {

        struct 
        {
            const char *prop;
            bool boolVal;
        } boolConfigs[] =
        {
            { "nodes3d",        threeDView },
            { "monochrome",     monochromeMode },
            { "displayBackgroundImage", backgroundImage },
            { "showGlobalView", showGlobalView },
            { "showBoundingBox", showBoundingBox },
            { "showGroundGrid", showGroundGrid },
            { "showVerboseStatusString", showVerboseStatusString }, 
            { "showWallTime", showWallTimeinStatusString }, 
            { "showPlaybackTime", showPlaybackTimeInStatusString },
            { "showPlaybackRange", showPlaybackRangeString },
            { "showDebugInfo", showDebugInfo },
            { "autorewind", autorewind },
            { "messageStreamFiltering", messageStreamFiltering }
        };

        for (size_t i = 0; i < sizeof(boolConfigs)/sizeof(boolConfigs[0]); i++) {
            if (!root.exists(boolConfigs[i].prop))
                root.add(boolConfigs[i].prop, libconfig::Setting::TypeBoolean)=boolConfigs[i].boolVal;
            else
                root[boolConfigs[i].prop]=boolConfigs[i].boolVal;
        }

        struct 
        {
            const char *prop; 
            float *val; 
        } floatVals[] = 
        {
            { "scaleText", &scaleText }, 
            { "scaleLine", &scaleLine }, 
            { "layerPadding", &layerPadding }, 
            { "gpsScale", &gpsScale }, 
            { "antennaRadius", &antennaRadius },
            { "ghostLayerTransparency", &ghostLayerTransparency }
        }; 
        for (size_t i=0; i<sizeof(floatVals)/sizeof(floatVals[0]); i++) {
            if (!root.exists(floatVals[i].prop))
                root.add(floatVals[i].prop, libconfig::Setting::TypeFloat)=*floatVals[i].val;
            else
                root[floatVals[i].prop]=*floatVals[i].val;
        }

        std::string hrc(hierarchyRingColor.toString());
        struct 
        {
            const char *prop; 
            std::string *val; 
        } strVals[] = 
        {
            { "statusFontName", &statusFontName },
            { "hierarchyRingColor", &hrc }
        }; 
        for (size_t i=0; i<sizeof(strVals)/sizeof(strVals[0]); i++) {
            if (!root.exists(strVals[i].prop))  
                root.add(strVals[i].prop, libconfig::Setting::TypeString)=*strVals[i].val;
            else
                root[strVals[i].prop]=*strVals[i].val;
        }

        struct 
        {
            const char *prop; 
            int *val; 
        } intVals[] = 
        {
            { "statusFontPointSize", &statusFontPointSize },
            { "maxNodes", (int*)&maxNodes },
            { "maxLayers", (int*)&maxLayers }
        }; 
        for (size_t i=0; i<sizeof(intVals)/sizeof(intVals[0]); i++) {
            if (!root.exists(intVals[i].prop))
                root.add(intVals[i].prop, libconfig::Setting::TypeInt)=*intVals[i].val;
            else
                root[intVals[i].prop]=*intVals[i].val;
        }

        struct 
        {
            const char *prop; 
            long long int *val; 
        } longlongIntVals[] = 
        {
            { "playbackStartTime", &playbackStartTime } 
        }; 
        for (size_t i=0; i<sizeof(longlongIntVals)/sizeof(longlongIntVals[0]); i++) {
            if (!root.exists(longlongIntVals[i].prop)) 
                root.add(longlongIntVals[i].prop, libconfig::Setting::TypeInt64)=(int)(*longlongIntVals[i].val); 
            else
                root[longlongIntVals[i].prop]=*longlongIntVals[i].val;
        }

        std::string prop="viewPoint";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &vp=cfg.lookup(prop); 

        struct 
        {
            const char *type;
            float *data[3];
        } viewPoints[] =
        {
            { "angle", { &manetAdj.angleX, &manetAdj.angleY, &manetAdj.angleZ }},
            { "scale", { &manetAdj.scaleX, &manetAdj.scaleY, &manetAdj.scaleZ }},
            { "shift", { &manetAdj.shiftX, &manetAdj.shiftY, &manetAdj.shiftZ }}
        };
        for (size_t i=0; i<sizeof(viewPoints)/sizeof(viewPoints[0]);i++) {
            if (!vp.exists(viewPoints[i].type)) {
                vp.add(viewPoints[i].type, libconfig::Setting::TypeArray);
                for (size_t j=0; j<sizeof(viewPoints[i].data)/sizeof(viewPoints[i].data[0]); j++)
                    vp[viewPoints[i].type].add(libconfig::Setting::TypeFloat);
            }
        }
        root["viewPoint"]["angle"][0]=std::fpclassify(manetAdj.angleX)==FP_NAN ? 0.0 : manetAdj.angleX;
        root["viewPoint"]["angle"][1]=std::fpclassify(manetAdj.angleY)==FP_NAN ? 0.0 : manetAdj.angleY;
        root["viewPoint"]["angle"][2]=std::fpclassify(manetAdj.angleZ)==FP_NAN ? 0.0 : manetAdj.angleZ;
        root["viewPoint"]["scale"][0]=std::fpclassify(manetAdj.scaleX)==FP_NAN ? 0.0 : manetAdj.scaleX;
        root["viewPoint"]["scale"][1]=std::fpclassify(manetAdj.scaleY)==FP_NAN ? 0.0 : manetAdj.scaleY;
        root["viewPoint"]["scale"][2]=std::fpclassify(manetAdj.scaleZ)==FP_NAN ? 0.0 : manetAdj.scaleZ;
        root["viewPoint"]["shift"][0]=std::fpclassify(manetAdj.shiftX)==FP_NAN ? 0.0 : manetAdj.shiftX;
        root["viewPoint"]["shift"][1]=std::fpclassify(manetAdj.shiftY)==FP_NAN ? 0.0 : manetAdj.shiftY;
        root["viewPoint"]["shift"][2]=std::fpclassify(manetAdj.shiftZ)==FP_NAN ? 0.0 : manetAdj.shiftZ;

        BackgroundImage &bg=BackgroundImage::getInstance();
        float bgfloatVals[5];
        bg.getDrawingCoords(bgfloatVals[0], bgfloatVals[1], bgfloatVals[2], bgfloatVals[3], bgfloatVals[4]); 

        prop="backgroundImage";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &bgset=cfg.lookup(prop);

        prop="imageFile"; 
        if (!bgset.exists(prop))
            bgset.add(prop, libconfig::Setting::TypeString);
        std::string imageFile=bg.getImageFile();
        if (!imageFile.empty() || imageFile=="none") 
            root["backgroundImage"]["imageFile"]=bg.getImageFile();
        else 
            root["backgroundImage"]["imageFile"]="none";

        prop="coordinates";
        if (!bgset.exists(prop)) { 
            bgset.add(prop, libconfig::Setting::TypeArray);
            // for (size_t i=0; i<sizeof(bgfloatVals)/sizeof(bgfloatVals[0]); i++)
            //     bgset[prop].add(libconfig::Setting::TypeFloat);                 // I dislike libconfig++
        }

        root["backgroundImage"]["coordinates"][0]=bgfloatVals[0];
        root["backgroundImage"]["coordinates"][1]=bgfloatVals[1];
        root["backgroundImage"]["coordinates"][2]=bgfloatVals[2];
        root["backgroundImage"]["coordinates"][3]=bgfloatVals[3];
        root["backgroundImage"]["coordinates"][4]=bgfloatVals[4];

        prop="showGroundGrid";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeBoolean);
        root[prop]=showGroundGrid;

        prop="backgroundColor";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &bgColSet=cfg.lookup(prop);

        struct 
        {
            const char *name;
            float val;
        } bgColors[] = 
        {
            { "r", 0.0 }, 
            { "g", 0.0 }, 
            { "b", 0.0 }, 
            { "a", 255.0 }
        };
        for (size_t i=0; i<sizeof(bgColors)/sizeof(bgColors[0]);i++)
            if (!bgColSet.exists(bgColors[i].name))
                bgColSet.add(bgColors[i].name, libconfig::Setting::TypeFloat)=bgColors[i].val;

        root["backgroundColor"]["r"]=rgbaBGColors[0];
        root["backgroundColor"]["g"]=rgbaBGColors[1];
        root["backgroundColor"]["b"]=rgbaBGColors[2];
        root["backgroundColor"]["a"]=rgbaBGColors[3];

        SingletonConfig::unlock();
    }
    catch (const libconfig::SettingException &e) {
        LOG_ERROR("Error saving configuration at " << e.getPath() << ": " << e.what() << "  " << __FILE__ << ":" << __LINE__); 
    }

    TRACE_EXIT();
    return true;
}
