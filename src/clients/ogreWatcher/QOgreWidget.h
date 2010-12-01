#ifndef QOGRE_WIDGET_H
#define QOGRE_WIDGET_H

#include "QOgreWindowWidget.h"
#include "QSdkCameraMan.h"

namespace QtOgre 
{

    class QOgreWidget : public QOgreWindowWidget {
        Q_OBJECT
        public:
            //
            // ########## Public functions ##########
            //
            //
            // --- Constructors ---
            //
            QOgreWidget(QWidget* parent);
            ~QOgreWidget(void);

        public slots:

        signals:
            /** 
             * Called when OGRE is finished loading and all ogre components
             * are initalized and ready to go. Derived classes should not 
             * use OGRE pointers until after this signal is given.
             */
            void ogreInitialized(); 

        protected:
            //
            // ########## Protected functions ##########
            //
            void resizeEvent(QResizeEvent *e);
            void paintEvent(QPaintEvent *e);
            void timerEvent(QTimerEvent *e);
            void mousePressEvent(QMouseEvent *e);
            void mouseReleaseEvent(QMouseEvent *e);
            void mouseMoveEvent(QMouseEvent *e);
            void keyPressEvent(QKeyEvent *e); 
            void keyReleaseEvent(QKeyEvent *e); 

            /** 
             * Overload to create your own scene. Base class sets ambient light
             * and creates a single light source, "MainLight". 
             */
            virtual void createScene(void);

            Ogre::SceneNode *m_mainNode;
            Ogre::SceneManager *mSceneMgr;
            Ogre::Camera *mCamera;
            OgreBites::QSdkCameraMan *mCameraMgr; 

        private:
            void setupResources(void);
            void setupScene(void);
            Ogre::Viewport *m_vp;
    };
} // namespace
#endif
