/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
{
    // ------- Dumping bitmap RAW data [0x93e1228]---------
    // cx=16 cy=1
    uint8_t raw0x93e1228[] = {    /* line 0 */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    }; /* 0x93e1228 */
    Bitmap bmp0x93e1228(8, &this->palette332, 16, 1, raw0x93e1228, sizeof(raw0x93e1228), false, false);
    RDPBmpCache cmd(&bmp0x93e1228, 0, 0, &this->front->rdp_layer.client_info);
    this->front->reserve_order(bmp0x93e1228.bmp_size(this->front->rdp_layer.client_info.bpp) + 16);
    cmd.emit(this->front->out_stream);
}
{
    RDPMemBlt cmd(0, Rect(0, 0, 16, 1), 204, 0, 0, 0);
    this->front->mem_blt(cmd, this->clip);
}
/* snapshot */
/* snapshot */
{
    RDPMemBlt cmd(0, Rect(0, 0, 16, 1), 204, 0, 0, 0);
    this->front->mem_blt(cmd, this->clip);
}
{
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), 0x000088);
    this->server_set_clip(Rect(0, 0, 800, 600));
    this->front->opaque_rect(cmd, this->clip);
}
{
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), 0x000088);
    this->server_set_clip(Rect(0, 0, 800, 600));
    this->front->opaque_rect(cmd, this->clip);
}
{
    // ------- Dumping bitmap RAW data [0x93e1280]---------
    // cx=16 cy=1
    uint8_t raw0x93e1280[] = {    /* line 0 */
0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7,
    }; /* 0x93e1280 */
    Bitmap bmp0x93e1280(8, &this->palette332, 16, 1, raw0x93e1280, sizeof(raw0x93e1280));
    RDPBmpCache cmd(&bmp0x93e1280, 0, 1, &this->front->rdp_layer.client_info);
    this->front->reserve_order(bmp0x93e1280.bmp_size(this->front->rdp_layer.client_info.bpp) + 16);
    cmd.emit(this->front->out_stream);
}
{
    RDPMemBlt cmd(0, Rect(0, 0, 16, 1), 204, 0, 0, 1);
    this->front->mem_blt(cmd, this->clip);
}
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
/* snapshot */
