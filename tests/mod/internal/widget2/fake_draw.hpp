struct TestDraw : DrawApi
{
    RDPDrawable gd;
    Font font;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h)
    , font(FIXTURES_PATH "/dejavu-sans-10.fv1")
    {}

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        this->gd.draw(cmd, clip, gly_cache);
    }

    virtual void draw(const RDPBrushCache & cmd)
    {
        this->gd.draw(cmd);
    }

    virtual void draw(const RDPColCache & cmd)
    {
        this->gd.draw(cmd);
    }

    virtual void draw(const RDPGlyphCache & cmd)
    {
        this->gd.draw(cmd);
    }

    virtual void draw(const RDPPolygonSC & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPolygonCB & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPolyline & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void begin_update() {}

    virtual void end_update() {}

    virtual void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
    {
        this->gd.server_draw_text(x, y, text, fgcolor, bgcolor, clip, this->font);
    }

    virtual void text_metrics(const char* text, int& width, int& height)
    {
        height = 0;
        width = 0;
        uint32_t uni[256];
        size_t len_uni = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
        if (len_uni){
            for (size_t index = 0; index < len_uni; index++) {
                FontChar *font_item = this->gd.get_font(this->font, uni[index]);
                // width += font_item->incby;
                width += font_item->width + 2;
                height = std::max(height, font_item->height);
            }
            width -= 2;
        }
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize, true);
        fclose(file);
    }
};

