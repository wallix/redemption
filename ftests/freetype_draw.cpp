#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>

using namespace std;

#include <ft2build.h>
#include FT_FREETYPE_H

void draw_bitmap(FT_Bitmap*  bitmap)
{
    FT_Int  x_max = bitmap->width;
    FT_Int  y_max = bitmap->rows;

    for ( FT_Int x = 0; x < x_max; x++)
    {
        for ( FT_Int y = 0; y < y_max; y++ )
        {
            unsigned c = bitmap->buffer[y * bitmap->width + x];
            cout << (c == 0 ? ' ' : c < 128 ? '+' : '#');
        }
        cout << "\n";
    }
    cout << "\n";
}


int
main( int     argc,
      char**  argv )
{
    cout.sync_with_stdio(false);

    FT_Library    library;
    FT_Face       face;

    FT_GlyphSlot  slot;
    FT_Error      error;

    char*         filename;
    char*         text;

    int           n, num_chars;


    if ( argc != 4)
    {
        fprintf ( stderr, "usage: %s font sample-text size\n", argv[0] );
        exit( 1 );
    }

    filename      = argv[1];                           /* first argument     */
    text          = argv[2];                           /* second argument    */
    num_chars     = strlen( text );

    error = FT_Init_FreeType( &library );              /* initialize library */
    /* error handling omitted */

    error = FT_New_Face( library, filename, 0, &face );/* create face object */
    /* error handling omitted */

    /* use 50pt at 100dpi */
    error = FT_Set_Char_Size( face, 50 * 64, 0,
                                100, 0 );                /* set character size */
    /* error handling omitted */

    slot = face->glyph;

    for ( n = 0; n < num_chars; n++ )
    {
        /* set transformation */
        if (FT_Set_Pixel_Sizes(face, atoi(argv[3]), 0))
            cerr << "error\n";

        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
        if ( error )
            continue;                 /* ignore errors */

        /* now, draw to our target surface (convert position) */
        draw_bitmap(&slot->bitmap);
    }

    FT_Done_Face    ( face );
    FT_Done_FreeType( library );

    return 0;
}

