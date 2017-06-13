#ifndef IF_FRAME_H
#define IF_FRAME_H

#include <QOpenGLWidget>
#include <vector>
#include "tensor.h"
#include "thread.h"


namespace e8
{

typedef e8util::vec<4, unsigned char>   pixel;

// Thread-safe frame buffer.
class if_frame
{
public:
        if_frame();
        virtual ~if_frame();

        pixel           operator()(unsigned i, unsigned j) const;
        pixel&          operator()(unsigned i, unsigned j);

        unsigned        width() const;
        unsigned        height() const;

        void            resize(unsigned w, unsigned h);

        virtual void    commit() = 0;

protected:
        struct surface
        {
                surface();
                ~surface();

                pixel*          pixels = nullptr;
                unsigned        w = 0;
                unsigned        h = 0;

                void    resize(unsigned w, unsigned h);
        };

        surface                 front() const;
        surface                 back() const;
        void                    swap();

        e8util::mutex_t         m_mutex;
        surface                 m_surface[2];
        bool                    m_spin = 0;
};

class ram_ogl_frame: public if_frame, public QOpenGLWidget
{
public:
        ram_ogl_frame(QWidget* parent);
        ~ram_ogl_frame();

        void            commit() override;

        void            initializeGL() override;
        void            resizeGL(int w, int h) override;
        void            paintGL() override;

};

}

#endif // IF_FRAME_H