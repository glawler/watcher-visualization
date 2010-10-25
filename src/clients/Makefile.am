include $(top_srcdir)/Makefile.top

if ENABLE_HIERARCHY_CLIENT
WHC_DIR = watcherHierarchyClient
endif 

if !ENABLE_TESTNODEONLY
FULL_DIRS = messageStream2Text connectivity2dot randomScenario gps2eventdb
endif 

if ENABLE_WATCHER3D
W3D_DIR = watcher3D 
endif

if ENABLE_OGREWATCHER
ORGE_DIR = ogreWatcher 
endif

if ENABLE_EARTHWATCHER
EARTH_DIR=earthWatcher
endif

SUBDIRS=\
	routeFeeder \
	commandlineFeeders \
	$(WHC_DIR) \
	$(FULL_DIRS) \
	$(W3D_DIR) \
	$(ORGE_DIR) \
	$(EARTH_DIR)


