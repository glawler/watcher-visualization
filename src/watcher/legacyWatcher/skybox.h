#ifndef THISISASKYBOXYOUBETCHA_H
#define THISISASKYBOXYOUBETCHA_H

#include <GL/gl.h>
#include "logger.h"

namespace watcher
{
    class Skybox
    {
        public:
            static Skybox &getSkybox();
            bool loadSkyboxFiles();
            void drawSkybox(GLfloat camX, GLfloat camy, GLfloat camz); 

        protected:
        private:
            // No copies thanks.
            Skybox();
            ~Skybox();
            Skybox(const Skybox &);
            Skybox &operator=(const Skybox &); 

            GLfloat width;

            unsigned int *textureIds; // array of textures ids.

            bool loadBMPFile(const unsigned int textureId, const char *filename);

            DECLARE_LOGGER();
    }; 


} // end namespace watcher

#endif // THISISASKYBOXYOUBETCHA_H
