
===Delta3D/Watcher3D Notes===

===Delta3D Build===

|OpenDIS 2.5                 |{-open-dis 2.5 (src**)-} (disable dtDIS)
|Qt 4.5.0                    |qt 4.5.0 (F10 pkg: qt, qt-devel)
|BOOST Python 1.38.0
|PYTHON 2.5
|CAL3D 0.11.0                |cal3d 0.11.0 (F11 pkg: cal3d, cal3d-devel)
|CPPUNIT 1.12.0              |cppunit 1.12.1 (F11 pkg: cppunit, cppunit-devel)
|CEGUI 0.6.2b                |cegui 0.6.2 (F11 pkg: cegui, cegui-devel)
|Game Networking Engine 0.70 |{-gnelib 0.70 (src*)-} (disable dtNetGM)
|GDAL 1.6.0                  |{-gdal 1.6.0 (pkg)-} (disable dtTerrain until F11 is released****)
|HawkNL 1.68                 |{-hawknl 1.68 (src*)-} (disable dtNet, dtDIS, dtNetGM)
|ALUT 1.1.0                  |{-freealut 1.1.0 (pkg)-} (disable dtAudio)
|OpenAL 1.1 (binaries)       |{-openal 0.0.9 (pkg)-} (disable dtAudio)
|Open Dynamics Engine 0.10.1 |ode 0.10.1 (src, --with-trimesh=opcode --enable-shared**)
|OpenSceneGraph 2.8.0        |OpenSceneGraph 2.8.0 (F11 pkg: OpenSceneGraph, OpenSceneGraph-devel)
|PLIB 1.8.5                  |plib 1.8.5 (F11 pkg: plib, plib-devel)
|XERCES 2.8.0                |xerces-c 2.8.0 (F11 pkg: xerces-c, xerces-c-devel)
|RTI                         |{-CERTI (src)-} (disabled dtHLAGM)
 * - see http://www.delta3d.org/article.php?story=20060821181211777&topic=tutorials
 ** - see http://delta3d.wiki.sourceforge.net/Ubuntu8.04
 *** - see http://delta3d.wiki.sourceforge.net/Ubuntu8.10
 **** - the gdal package has 233 dependencies from F11!

Possible Issues:
I had some problems with OsgPlugins not being loaded due to the filenames being
wrong.  I would also watch for problems with dlopen for other libraries.

===Watcher3D===

The current Watcher3D code is a demo that shows how to create actors and set
their properties.  If you look at the code in LibWatcher3D::OnStartup, this
creates some actor proxies (2 nodes and 1 edge) and sets the node and edge
head/tail positions.  The ground clamping code in the node actor appears to
only work after the terrain is fully loaded, which causes a bug in the demo.
The bug can be fixed by setting the edge tail/head positions at some point
after the nodes are ground clamped.  The node itself could even track the
edges that connect to it and update their tail/head positions when its
position changes.

I borrowed the ground clamping code in NodeActor::DrawNode from one of the
online tutorials, but later found a different version of this code in
TestPlayer::HandleTick in /examples/testGameActorLibrary, which might be worth
looking at.

I recommend using /examples (off of delta3d_REL-2.3.0) and *not* the online
tutorials for example code.  The code in /examples works, the online tutorials
often do not due to being written for older versions.  If we want some nicer
environmental effects, look at /examples/testClouds,
/examples/testGameEnvironment, and /examples/testWeather for example code.

In terms of next steps, we need to add some network code to receive watcherd
messages and then create nodes and/or set properties.  I would consider adding
this code as a GMComponent, since this is how Delta3D's network components are
done.

===Delta3D Gotchas===

The biggest "gotcha" working with Delta3D is file/path problems.  I had
several problems with files (models, textures, etc.) not being in the right
places.  The same applies to game libraries being loaded at runtime, as things
like actors, actor registries, and components are done as libraries.  If for
some reason you don't get the error messages, these things can be tough to
debug!
