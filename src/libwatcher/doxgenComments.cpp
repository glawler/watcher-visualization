//
// This file gets processed by doxygen and ends up as the main page in index.html.
// Do not put code here, just comments for doxyen
//
/**
 ***************************************************************
 @mainpage

 @section Introduction

 The watcher system is composed of three components: a number of test node daemons, a watcher daemon instance, 

 @section Build
 @subsection Dependencies
 
 The watcher relies on a bunch of stuff, they are here: @ref WatcherDependencies.

 @subsection Instructions

 The watcher system is built using auto-tools. 

 @section Components
 @subsection Test Node Daemons

  @ref sendColorMessage

  @ref sendConnectivityMessage

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

  @ref sendGPSMessage 

 @subpage watcherd "Watcher Daemon"
 @subsection GUIs
 @subsubsection Legacy Watcher
 @subsubsection Watcher3d

 @section About

  Point of contact: Geoff Lawler <geoff.lawler@cobham.com>

 **
 **********************************************************************/

/** @namespace libconfig 
 * The libconfig namespace is the namespace in the third party library libconfig++, which the watcher system uses for reading and writing cfg files 
 */
/** @namespace log4cxx
 * The namespace for the third party library log4cxx which the watcher uses to do all its logging. */

/**
 * @page UsersGuide
 * @section TOC
 *  <h3>Table of Contents</h3>
 * @section Modules
 *  Module Descriptions
 * @section HowTo
 *  How to do stuff
 * @section Glossary
 *  Glossary of Terms
 * @section Index
 */

/** 
 * @page WatcherDependencies
 *
 * @section ThirdParty
 *
 * @li libconfig 1.3.1 
 *
 *  the system uses this for creating, reading, and writing to configuration ("cfg") files. 
 *
 * @li log4cxx 0.10.0, which depends on lib apr 1.3.3 and lib apr util 1.3.4
 *
 * This is used for all logging. 
 *
 * @li boost 1.38
 *
 * C++ library used more many things: serialization, network code, regular expressions, etc. 
 *
 * @li qt 4.3
 *
 * Used in the GUI for window frames, GUI widgets, mouse and keyboard handling, etc. 
 *
 * @li sqlite 3
 *
 * This is the database used in the watcher daemon. 
 *
 * @li qwt-5.1.1
 *
 * A small Qt-based library used to draw the 2d scrolling graphs in the watcher .
 *
 * @section Internal
 *
 * @li libwatcherutil 
 *
 * Holds a few things that most components need, like parsing command line args to get to the cfg file.
 *
 * @li liblogger 
 *
 * A small wrapper around log4cxx to abstract which logging library we use. 
 *
 * @li libwatcher
 *
 * The core of the watcher messaging system, the watcher APIs between test node daemons, a watcher daemon, and the GUI(s)s.
 *
 *
 */
