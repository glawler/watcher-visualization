SUBDIRS= \
	routeFeeder \
	commandlineFeeders 

if ENABLE_HIERARCHY_CLIENT
	SUBDIRS += watcherHierarchyClient
endif 

if !ENABLE_TESTNODEONLY
	SUBDIRS+=messageStream2Text connectivity2dot
	if ENABLE_WATCHER3D
		SUBDIRS += watcher3D 
	endif

	if ENABLE_OGREWATCHER
		SUBDIRS += ogreWatcher 
	endif
endif

