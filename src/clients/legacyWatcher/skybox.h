/** 
 * @file skybox.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef THISISASKYBOXYOUBETCHA_H
#define THISISASKYBOXYOUBETCHA_H

#include <GL/gl.h>
#include "logger.h"

namespace watcher
{
    class Skybox
    {
        public:
            static Skybox *getSkybox();

            void drawSkybox(GLfloat camX, GLfloat camy, GLfloat camz); 

        protected:
        private:
            // No copies thanks.
            Skybox();
            ~Skybox();
            Skybox(const Skybox &);
            Skybox &operator=(const Skybox &); 

            GLfloat width;

            DECLARE_LOGGER();
    }; 


} // end namespace watcher

#endif // THISISASKYBOXYOUBETCHA_H
