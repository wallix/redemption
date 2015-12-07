/*

*/
#ifndef RDPQTDRAWABLE_HPP
#define RDPQTDRAWABLE_HPP

#include <QWidget>
#include <QPicture>


#include "../core/RDP/RDPGraphicDevice.hpp"
 class QLabel;
 class QPainter;

class RDPQtDrawable : public RDPGraphicDevice, public QWidget 
{    
public:
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

    void flush() override;


    RDPQtDrawable(int, int);
    ~RDPQtDrawable();
    
    
private:
    QLabel*   _label;
    QPainter* _p;
    QPicture  _pi;
    
};

#endif
