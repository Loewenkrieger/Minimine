static ROW_ATTR void ROW_NAME(const Draw *c,uint16_t *dest,int y,unsigned scale)
{
    int left=c->x0<0?0:c->x0,right=c->x1>SCREEN_W?SCREEN_W:c->x1;
    left+=(int)((scale-(unsigned)left%scale)%scale);
    if(right<=left) return;
    dest+=left/(int)scale;
    if(c->kind==CMD_RECT) {
        for(int x=left;x<right;x+=(int)scale) *dest++=c->arg;
    } else if(c->kind==CMD_SHADE) {
        for(int x=left;x<right;x+=(int)scale,++dest)
            *dest=c->arg==1?(uint16_t)((*dest>>1)&0x7bef):(uint16_t)((*dest>>2)&0x39e7);
    } else {
        Tile tile=(Tile)c->arg;
        bool opaque=tile!=TILE_AIR && tile!=TILE_TRUNK && tile!=TILE_LEAVES &&
            tile!=TILE_TORCH && tile!=TILE_OPEN_BOTTOM && tile!=TILE_OPEN_TOP &&
            tile!=TILE_BED_LEFT && tile!=TILE_BED_RIGHT;
        unsigned index=graphic(tile,(int)c->variant);
        const uint16_t *src=atlas[index]+(y-c->y0)*16;
        uint16_t mask=masks[index][y-c->y0];
        if(opaque && scale==1) {memcpy(dest,src+left-c->x0,(size_t)(right-left)*2);return;}
        for(int x=left;x<right;x+=(int)scale) {
            int px=x-c->x0;
            if(tile==TILE_AIR || tile==TILE_OPEN_BOTTOM || tile==TILE_OPEN_TOP)
                *dest++=tile!=TILE_AIR && px<2?C_RGB(20,13,7):background_pixel(drawing_game,x,y);
            else *dest++=opaque || (mask&(1u<<px))?src[px]:background_pixel(drawing_game,x,y);
        }
    }
}
