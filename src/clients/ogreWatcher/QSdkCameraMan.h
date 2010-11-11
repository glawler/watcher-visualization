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

#include "Ogre.h"
#include <limits>

namespace QOgreBites
{
	enum CameraStyle   // enumerator values for different styles of camera movement
	{
		CS_FREELOOK,
		CS_ORBIT,
		CS_MANUAL
	};

	/*=============================================================================
	| Utility class for controlling the camera in samples.
	=============================================================================*/
	class QSdkCameraMan : public Ogre::FrameListener
    {
    public:
		QSdkCameraMan(Ogre::Camera* cam)
		: mCamera(0)
		, mTarget(0)
		, mOrbiting(false)
		, mZooming(false)
		, mTopSpeed(150)
		, mVelocity(Ogre::Vector3::ZERO)
		, mGoingForward(false)
		, mGoingBack(false)
		, mGoingLeft(false)
		, mGoingRight(false)
		, mGoingUp(false)
		, mGoingDown(false)
		, mFastMove(false)
        , mPrevPoint(0,0) 
		{
			setCamera(cam);
			setStyle(CS_FREELOOK);
		}

		virtual ~QSdkCameraMan() {}

		/*-----------------------------------------------------------------------------
		| Swaps the camera on our camera man for another camera.
		-----------------------------------------------------------------------------*/
		virtual void setCamera(Ogre::Camera* cam)
		{
			mCamera = cam;
		}

		virtual Ogre::Camera* getCamera()
		{
			return mCamera;
		}

		/*-----------------------------------------------------------------------------
		| Sets the target we will revolve around. Only applies for orbit style.
		-----------------------------------------------------------------------------*/
		virtual void setTarget(Ogre::SceneNode* target)
		{
			if (target != mTarget)
			{
				mTarget = target;
				if(target)
				{
					setYawPitchDist(Ogre::Degree(0), Ogre::Degree(15), 150);
					mCamera->setAutoTracking(true, mTarget);
				}
				else
				{
					mCamera->setAutoTracking(false);
				}

			}


		}

		virtual Ogre::SceneNode* getTarget()
		{
			return mTarget;
		}

		/*-----------------------------------------------------------------------------
		| Sets the spatial offset from the target. Only applies for orbit style.
		-----------------------------------------------------------------------------*/
		virtual void setYawPitchDist(Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist)
		{
			mCamera->setPosition(mTarget->_getDerivedPosition());
			mCamera->setOrientation(mTarget->_getDerivedOrientation());
			mCamera->yaw(yaw);
			mCamera->pitch(-pitch);
			mCamera->moveRelative(Ogre::Vector3(0, 0, dist));
		}

		/*-----------------------------------------------------------------------------
		| Sets the camera's top speed. Only applies for free-look style.
		-----------------------------------------------------------------------------*/
		virtual void setTopSpeed(Ogre::Real topSpeed)
		{
			mTopSpeed = topSpeed;
		}

		virtual Ogre::Real getTopSpeed()
		{
			return mTopSpeed;
		}

		/*-----------------------------------------------------------------------------
		| Sets the movement style of our camera man.
		-----------------------------------------------------------------------------*/
		virtual void setStyle(CameraStyle style)
		{
			if (mStyle != CS_ORBIT && style == CS_ORBIT)
			{
				setTarget(mTarget ? mTarget : mCamera->getSceneManager()->getRootSceneNode());
				mCamera->setFixedYawAxis(true);
				manualStop();
				setYawPitchDist(Ogre::Degree(0), Ogre::Degree(15), 150);

			}
			else if (mStyle != CS_FREELOOK && style == CS_FREELOOK)
			{
				mCamera->setAutoTracking(false);
				mCamera->setFixedYawAxis(true);
			}
			else if (mStyle != CS_MANUAL && style == CS_MANUAL)
			{
				mCamera->setAutoTracking(false);
				manualStop();
			}
			mStyle = style;

		}

		virtual CameraStyle getStyle()
		{
			return mStyle;
		}

		/*-----------------------------------------------------------------------------
		| Manually stops the camera when in free-look mode.
		-----------------------------------------------------------------------------*/
		virtual void manualStop()
		{
			if (mStyle == CS_FREELOOK)
			{
				mGoingForward = false;
				mGoingBack = false;
				mGoingLeft = false;
				mGoingRight = false;
				mGoingUp = false;
				mGoingDown = false;
				mVelocity = Ogre::Vector3::ZERO;
			}
		}

		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			if (mStyle == CS_FREELOOK)
			{
				// build our acceleration vector based on keyboard input composite
				Ogre::Vector3 accel = Ogre::Vector3::ZERO;
				if (mGoingForward) accel += mCamera->getDirection();
				if (mGoingBack) accel -= mCamera->getDirection();
				if (mGoingRight) accel += mCamera->getRight();
				if (mGoingLeft) accel -= mCamera->getRight();
				if (mGoingUp) accel += mCamera->getUp();
				if (mGoingDown) accel -= mCamera->getUp();

				// if accelerating, try to reach top speed in a certain time
				Ogre::Real topSpeed = mFastMove ? mTopSpeed * 20 : mTopSpeed;
				if (accel.squaredLength() != 0)
				{
					accel.normalise();
					mVelocity += accel * topSpeed * evt.timeSinceLastFrame * 10;
				}
				// if not accelerating, try to stop in a certain time
				else mVelocity -= mVelocity * evt.timeSinceLastFrame * 10;

				Ogre::Real tooSmall = std::numeric_limits<Ogre::Real>::epsilon();

				// keep camera velocity below top speed and above epsilon
				if (mVelocity.squaredLength() > topSpeed * topSpeed)
				{
					mVelocity.normalise();
					mVelocity *= topSpeed;
				}
				else if (mVelocity.squaredLength() < tooSmall * tooSmall)
					mVelocity = Ogre::Vector3::ZERO;

				if (mVelocity != Ogre::Vector3::ZERO) mCamera->move(mVelocity * evt.timeSinceLastFrame);
			}

			return true;
		}

		/*-----------------------------------------------------------------------------
		| Processes key presses for free-look style movement.
		-----------------------------------------------------------------------------*/
		virtual void injectKeyDown(const QKeyEvent& evt)
		{
            if (evt.key()==Qt::Key_Space) { 
                if (mStyle==CS_FREELOOK)
                    setStyle(CS_ORBIT); 
                else if (mStyle==CS_ORBIT)
                    setStyle(CS_MANUAL);
                else 
                    setStyle(CS_FREELOOK); 
            }
			if (mStyle == CS_FREELOOK)
			{
				if (evt.key() == Qt::Key_W || evt.key() == Qt::Key_Up) mGoingForward = true;
				else if (evt.key() == Qt::Key_S || evt.key() == Qt::Key_Down) mGoingBack = true;
				else if (evt.key() == Qt::Key_A || evt.key() == Qt::Key_Left) mGoingLeft = true;
				else if (evt.key() == Qt::Key_D || evt.key() == Qt::Key_Right) mGoingRight = true;
				else if (evt.key() == Qt::Key_PageUp) mGoingUp = true;
				else if (evt.key() == Qt::Key_PageDown) mGoingDown = true;
				else if (evt.key() == Qt::Key_Shift) mFastMove = true;
			}
		}

		/*-----------------------------------------------------------------------------
		| Processes key releases for free-look style movement.
		-----------------------------------------------------------------------------*/
		virtual void injectKeyUp(const QKeyEvent& evt)
		{
			if (mStyle == CS_FREELOOK)
			{
				if (evt.key() == Qt::Key_W || evt.key() == Qt::Key_Up) mGoingForward = false;
				else if (evt.key() == Qt::Key_S || evt.key() == Qt::Key_Down) mGoingBack = false;
				else if (evt.key() == Qt::Key_A || evt.key() == Qt::Key_Left) mGoingLeft = false;
				else if (evt.key() == Qt::Key_D || evt.key() == Qt::Key_Right) mGoingRight = false;
				else if (evt.key() == Qt::Key_PageUp) mGoingUp = false;
				else if (evt.key() == Qt::Key_PageDown) mGoingDown = false;
				else if (evt.key() == Qt::Key_Shift) mFastMove = false;
			}
		}

		/*-----------------------------------------------------------------------------
		| Processes mouse movement differently for each style.
		-----------------------------------------------------------------------------*/
		virtual void injectMouseMove(const QMouseEvent& evt)
		{
            QPoint relPos=evt.globalPos()-mPrevPoint;
			if (mStyle == CS_ORBIT)
			{
				Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();

				if (mOrbiting)   // yaw around the target, and pitch locally
				{
					mCamera->setPosition(mTarget->_getDerivedPosition());
					mCamera->yaw(Ogre::Degree(-relPos.x() * 0.25f));
					mCamera->pitch(Ogre::Degree(-relPos.y() * 0.25f));
					mCamera->moveRelative(Ogre::Vector3(0, 0, dist));

					// don't let the camera go over the top or around the bottom of the target
				}
				else if (mZooming)  // move the camera toward or away from the target
				{
					// the further the camera is, the faster it moves
					mCamera->moveRelative(Ogre::Vector3(0, 0, relPos.y() * 0.004f * dist));
				}
                // No z information inQMouseEvent
				// else if (relPos.z() != 0)  // move the camera toward or away from the target
				// {
				// 	// the further the camera is, the faster it moves
				// 	mCamera->moveRelative(Ogre::Vector3(0, 0, -relPos.z() * 0.0008f * dist));
				// }
			}
			else if (mStyle == CS_FREELOOK)
			{
				mCamera->yaw(Ogre::Degree(relPos.x() * 0.15f));
				mCamera->pitch(Ogre::Degree(relPos.y() * 0.15f));
			}
            mPrevPoint=evt.globalPos(); 
		}

		/*-----------------------------------------------------------------------------
		| Processes mouse presses. Only applies for orbit style.
		| Left button is for orbiting, and right button is for zooming.
		-----------------------------------------------------------------------------*/
		virtual void injectMouseDown(const QMouseEvent& evt)
		{
            mPrevPoint=evt.globalPos(); 
			if (mStyle == CS_ORBIT)
			{
				if (evt.button() == Qt::LeftButton) mOrbiting = true;
				else if (evt.button() == Qt::RightButton) mZooming = true;
			}
		}
		/*-----------------------------------------------------------------------------
		| Processes mouse releases. Only applies for orbit style.
		| Left button is for orbiting, and right button is for zooming.
		-----------------------------------------------------------------------------*/
		virtual void injectMouseUp(const QMouseEvent& evt)
		{
			if (mStyle == CS_ORBIT)
			{
				if (evt.button() == Qt::LeftButton) mOrbiting = false;
				else if (evt.button() == Qt::RightButton) mZooming = false;
			}
		}

    protected:

		Ogre::Camera* mCamera;
		CameraStyle mStyle;
		Ogre::SceneNode* mTarget;
		bool mOrbiting;
		bool mZooming;
		Ogre::Real mTopSpeed;
		Ogre::Vector3 mVelocity;
		bool mGoingForward;
		bool mGoingBack;
		bool mGoingLeft;
		bool mGoingRight;
		bool mGoingUp;
		bool mGoingDown;
		bool mFastMove;
        QPoint mPrevPoint;
    };
}

#endif
