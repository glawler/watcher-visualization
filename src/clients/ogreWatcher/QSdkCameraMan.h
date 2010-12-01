/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2009 Torus Knot Software Ltd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
 */
/*
 * Modified by Geoff Lawler, SPARTA Inc. to take Qt input events instead of 
 * OIS ones.
 */
#ifndef __QSdkCameraMan_H__
#define __QSdkCameraMan_H__

#include <Ogre.h>
#include <OIS.h>
#include <OGRE/SdkCameraMan.h>
#include <QKeyEvent>
#include <limits>

namespace OgreBites
{
	class QSdkCameraMan : public OgreBites::SdkCameraMan, public Ogre::FrameListener
    {
    public:
		QSdkCameraMan(Ogre::Camera* cam);
		virtual ~QSdkCameraMan(); 

		virtual void injectKeyDown(const QKeyEvent& evt);
		virtual void injectKeyUp(const QKeyEvent& evt);
		virtual void injectMouseMove(const QMouseEvent& evt);
		virtual void injectMouseDown(const QMouseEvent& evt);
		virtual void injectMouseUp(const QMouseEvent& evt);
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    protected:
        // QMouseEvent does not have a relative movement, so we keep 
        // track of it by hand by subtracting the current point from the 
        // previous point. 
        QPoint mPrevPoint;
    };
}

#endif
