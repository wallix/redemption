/*

*/
#ifndef RDPQTDRAWABLE_HPP
#define RDPQTDRAWABLE_HPP

#include <stdint.h>
#include "../core/RDP/RDPGraphicDevice.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "text_metrics.hpp"
#include "mod_api.hpp"
#include "RDPQWidget.hpp"


class RDPQtDrawable :  public RDPGraphicDevice
{    
    
private:
    mod_api*   _mod;
    int        _signalCount;
    int        _maxSignals;
    RDPQWidget _viewer;
    
    
    QColor u32_to_qcolor(uint32_t color){
        uint8_t b(color >> 16);
        uint8_t g(color >> 8);
        uint8_t r(color);
        return {r, g, b};
    }
    
    
public:
    using RDPGraphicDevice::draw;
    void draw(const RDPDestBlt          & cmd, const Rect & clip) override {}
    void draw(const RDPMultiDstBlt      & cmd, const Rect & clip) override {}
    void draw(const RDPPatBlt           & cmd, const Rect & clip) override {}
    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {}
    void draw(const RDPOpaqueRect       & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPMultiOpaqueRect  & cmd, const Rect & clip) override {}
    void draw(const RDPScrBlt           & cmd, const Rect & clip) override {}
    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {}
    void draw(const RDPMemBlt           & cmd, const Rect & clip, const Bitmap & bmp) override {}
    void draw(const RDPMem3Blt          & cmd, const Rect & clip, const Bitmap & bmp) override {}
    void draw(const RDPLineTo           & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
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

    void draw( const RDPBitmapData & bitmap_data, const uint8_t * data, std::size_t size, const Bitmap & bmp) override {}

    void server_set_pointer(const Pointer & cursor) override {}
    
    void server_draw_text(Font const & font, int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip) {}
    void text_metrics(Font const & font, const char * text, int & width, int & height) {}

    void flush() override {
        this->_viewer.flush();
    }

    void reInitView(){
        this->_viewer.reInitView();
    }
    
    
    RDPQtDrawable(int width, int height) : RDPGraphicDevice(), _viewer(width, height) {}
    
    ~RDPQtDrawable() {}
    
};

#endif
