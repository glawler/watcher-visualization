#ifndef QOGRE_WIDGET_H
#define QOGRE_WIDGET_H

#include "QOgreWindowWidget.h"

// Forward decl of our custom Qt-based camera manager
namespace OgreBites {
    class QSdkCameraMan;
}

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
            void createScene(void);

        private:
            void setupResources(void);
            void setupScene(void);
            //
            // ########## Private variables ##########
            //
            Ogre::SceneNode *m_mainNode;
            Ogre::SceneManager *mSceneMgr;
            Ogre::Camera *mCamera;
            Ogre::Viewport *m_vp;
            OgreBites::QSdkCameraMan *mCameraMgr; 
    };
} // namespace
#endif
