/*

*/
#ifndef RDPQTDRAWABLE_HPP
#define RDPQTDRAWABLE_HPP

#include <stdint.h>

#include "../core/RDP/RDPGraphicDevice.hpp"
#include "text_metrics.hpp"
#include "mod_api.hpp"
#include "RDPQWidget.hpp"
#include "bitmap.hpp"



class RDPQtDrawable :  public RDPGraphicDevice
{    
    
private:
    RDPQWidget     _viewer;
    int            _order_bpp;
    
    
public:
    using RDPGraphicDevice::draw;
    
    void draw(const RDPDestBlt          & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPMultiDstBlt      & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPPatBlt           & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPOpaqueRect       & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPMultiOpaqueRect  & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPScrBlt           & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPMemBlt           & cmd, const Rect & clip, const Bitmap & bmp) override {this->_viewer.draw(cmd, clip, bmp);}
    void draw(const RDPMem3Blt          & cmd, const Rect & clip, const Bitmap & bmp) override {this->_viewer.draw(cmd, clip, bmp); }
    void draw(const RDPLineTo           & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPGlyphIndex       & cmd, const Rect & clip, const GlyphCache * gly_cache) override {this->_viewer.draw(cmd, clip, gly_cache);}
    void draw(const RDPPolygonSC        & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPPolygonCB        & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPPolyline         & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPEllipseSC        & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    void draw(const RDPEllipseCB        & cmd, const Rect & clip) override {this->_viewer.draw(cmd, clip);}
    
    void draw(const RDP::FrameMarker & order) override {}

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {}
    void draw(const RDP::RAIL::WindowIcon          & order) override {}
    void draw(const RDP::RAIL::CachedIcon          & order) override {}
    void draw(const RDP::RAIL::DeletedWindow       & order) override {}

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data, std::size_t size, const Bitmap & bmp) override {this->_viewer.draw(bitmap_data, data, size, bmp);}

    void server_set_pointer(const Pointer & cursor) override {}

    void flush() override {this->_viewer.flush();}

    void reInitView() {this->_viewer.reInitView();}
    
    
    RDPQtDrawable(int width, int height, int order_bpp) : RDPGraphicDevice(), _viewer(width, height), _order_bpp(order_bpp) {}
    ~RDPQtDrawable() {}
    
    
    // Unused
    void server_draw_text(Font const & font, int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip) {}
    void text_metrics(Font const & font, const char * text, int & width, int & height) {}
    
};

#endif
