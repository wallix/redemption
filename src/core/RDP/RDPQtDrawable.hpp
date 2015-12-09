/*

*/
#ifndef RDPQTDRAWABLE_HPP
#define RDPQTDRAWABLE_HPP

#include <stdint.h>
#include "../core/RDP/RDPGraphicDevice.hpp"


#include <QtGui/QWidget>
#include <QtGui/QPicture>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtCore/QTimer>
#include "mod_api.hpp"

 class QLabel;
 class QPainter;
 class mod_api;

class RDPQtDrawable : public QWidget, public RDPGraphicDevice
{    
    Q_OBJECT
public:
    using RDPGraphicDevice::draw;
    void draw(const RDPDestBlt          & cmd, const Rect & clip) override {}
    void draw(const RDPMultiDstBlt      & cmd, const Rect & clip) override {}
    void draw(const RDPPatBlt           & cmd, const Rect & clip) override {}
    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {}
    void draw(const RDPOpaqueRect       & cmd, const Rect & clip) override;
    void draw(const RDPMultiOpaqueRect  & cmd, const Rect & clip) override {}
    void draw(const RDPScrBlt           & cmd, const Rect & clip) override {}
    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {}
    void draw(const RDPMemBlt           & cmd, const Rect & clip, const Bitmap & bmp) override {}
    void draw(const RDPMem3Blt          & cmd, const Rect & clip, const Bitmap & bmp) override {}
    void draw(const RDPLineTo           & cmd, const Rect & clip) override;
    void draw(const RDPGlyphIndex       & cmd, const Rect & clip, const GlyphCache * gly_cache) override {}
    void draw(const RDPPolygonSC        & cmd, const Rect & clip) override {}
    void draw(const RDPPolygonCB        & cmd, const Rect & clip) override {}
    void draw(const RDPPolyline         & cmd, const Rect & clip) override {}
    void draw(const RDPEllipseSC        & cmd, const Rect & clip) override {}
    void draw(const RDPEllipseCB        & cmd, const Rect & clip) override {}

    void draw(const RDP::FrameMarker & order) override {}

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {}
    void draw(const RDP::RAIL::WindowIcon          & order) override {}
    void draw(const RDP::RAIL::CachedIcon          & order) override {}
    void draw(const RDP::RAIL::DeletedWindow       & order) override {}

    void draw( const RDPBitmapData & bitmap_data, const uint8_t * data, std::size_t size
                    , const Bitmap & bmp) override {}

    void server_set_pointer(const Pointer & cursor) override {}
    
    void server_draw_text(Font const & font, int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip) {}
    void text_metrics(Font const & font, const char * text, int & width, int & height) {}

    void flush() override;
    
    void reInit();
    void stop();
    void setModAndStart(mod_api *, int);

    RDPQtDrawable(int, int);
    ~RDPQtDrawable();
    
    
public Q_SLOTS:
    void update();
    
    
private:
    QLabel*   _label;
    QPainter* _p;
    QPicture* _pi;
    int       _width;
    int       _height;
    QTimer*   _timer;
    mod_api*  _mod;
    int       _signalCount;
    int       _maxSignals;
    
};

#endif
