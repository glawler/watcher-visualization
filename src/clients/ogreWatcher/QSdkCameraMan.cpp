#include "QSdkCameraMan.h"

namespace OgreBites
{
    QSdkCameraMan::QSdkCameraMan(Ogre::Camera* cam) : 
        SdkCameraMan(cam), mPrevPoint(0,0) {
    }
    QSdkCameraMan::~QSdkCameraMan() {
    }

    void QSdkCameraMan::injectKeyDown(const QKeyEvent& evt)
    {
        if (mStyle == CS_FREELOOK)
        {
            if (evt.key() == Qt::Key_W || evt.key() == Qt::Key_Up) mGoingForward = true;
            else if (evt.key() == Qt::Key_S || evt.key() == Qt::Key_Down) mGoingBack = true;
            else if (evt.key() == Qt::Key_A || evt.key() == Qt::Key_Left) mGoingLeft = true;
            else if (evt.key() == Qt::Key_D || evt.key() == Qt::Key_Right) mGoingRight = true;
            else if (evt.key() == Qt::Key_PageUp) mGoingUp = true;
            else if (evt.key() == Qt::Key_PageDown) mGoingDown = true;

            if (evt.modifiers() & Qt::ShiftModifier) mFastMove = true;
        }
    }

    void QSdkCameraMan::injectKeyUp(const QKeyEvent& evt)
    {
        if (mStyle == CS_FREELOOK)
        {
            if (evt.key() == Qt::Key_W || evt.key() == Qt::Key_Up) mGoingForward = false;
            else if (evt.key() == Qt::Key_S || evt.key() == Qt::Key_Down) mGoingBack = false;
            else if (evt.key() == Qt::Key_A || evt.key() == Qt::Key_Left) mGoingLeft = false;
            else if (evt.key() == Qt::Key_D || evt.key() == Qt::Key_Right) mGoingRight = false;
            else if (evt.key() == Qt::Key_PageUp) mGoingUp = false;
            else if (evt.key() == Qt::Key_PageDown) mGoingDown = false;

            if (evt.modifiers() & Qt::ShiftModifier) mFastMove = false;
        }
    }

    void QSdkCameraMan::injectMouseMove(const QMouseEvent& evt)
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

    void QSdkCameraMan::injectMouseDown(const QMouseEvent& evt)
    {
        mPrevPoint=evt.globalPos(); 
        if (mStyle == CS_ORBIT)
        {
            if (evt.button() == Qt::LeftButton) mOrbiting = true;
            else if (evt.button() == Qt::RightButton) mZooming = true;
        }
    }
    void QSdkCameraMan::injectMouseUp(const QMouseEvent& evt)
    {
        if (mStyle == CS_ORBIT)
        {
            if (evt.button() == Qt::LeftButton) mOrbiting = false;
            else if (evt.button() == Qt::RightButton) mZooming = false;
        }
    }
    // GTL - copied this from SdkCameraMan. It has this function, but does
    // not derive from FrameListener so it never gets called. Copied here
    // so it does get called as this class derives from FrameListener
    bool QSdkCameraMan::frameRenderingQueued(const Ogre::FrameEvent& evt)
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

            if (mVelocity != Ogre::Vector3::ZERO) 
                mCamera->move(mVelocity * evt.timeSinceLastFrame);
        }

        return true;
    }
} // namespace
