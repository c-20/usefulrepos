#if 0
# //gcc="ftlibtool --mode=link gcc"
gcc=gcc
src=zhutou.c
out=zhutou.e
inc="-I../ft2/freetype2/include"
libs="-lz -lpng16 -lX11 -lm"
objs="common.o strbuf.o output.o mlgetopt.o ftcommon.o libfreetype.so graph.a"
$gcc -o $out $inc $src $objs $libs
exit 0
#else
#include "ftcommon.h"
#include "common.h"
#include "mlgetopt.h"

// comment out once debugged
#include <freetype/internal/ftdebug.h>

#include FT_MODULE_H
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdrv.h>

#include FT_STROKER_H
#include FT_SYNTHESIS_H
#include FT_LCD_FILTER_H
#include FT_DRIVER_H
#include FT_COLOR_H
#include FT_BITMAP_H

//#ifdef _WIN32 //  double-check WIN32 AND _WIN32
#ifdef _WIN32
#define snprintf _snprintf
#endif

static int LCDModes[] = {
  LCD_MODE_MONO, LCD_MODE_AA, LCD_MODE_LIGHT, LCD_MODE_RGB,
  LCD_MODE_BGR,  LCD_MODE_VRGB, LCD_MODE_VBGR			};
#define LCDModesLEN   ((int)(sizeof(LCDModes) / sizeof(int)))

enum { RENDER_MODE_ALL = 0, RENDER_MODE_FANCY, RENDER_MODE_STROKE,
       RENDER_MODE_TEXT, RENDER_MODE_WATERFALL, N_RENDER_MODES	};

#define VDT  void
#define UCT  unsigned char
#define CST  const char *
#define INT  int
#define UNT  unsigned int
#define FPT  double
#define XPT  FT_Fixed
#define YNT  FT_Bool
#define ERT  FT_Error
#define FAT  FT_Face
#define GST  FT_GlyphSlot
#define GLT  FT_Glyph
#define UIT  FT_UInt
#define SVDT static VDT
#define SCST static CST
#define SERT static ERT
#define SINT static INT
// UNT -> UIT ?



// it is expected that DOSTART starts within a condition
// DOUNTIL will end the single-iteration loop and the condition
// this allows break; for early exit with proper { cleanup(); }
#define DOSTART      do {
#define AFTERDO      } while(0); }
#define ELSEDO       else { DOSTART
// NOTE THAT AFTERDO CLOSES THE CONDITION BUT THERE IS NO DIFFERENCE
// BETWEEN THE END OF THE CONDITION BLOCK AND AFTER THE CONDITION BLOCK
// THE BRACES OF AFTERDO ARE OPTIONAL
//FILE *file = fopen(filename, "r");
//if (!file) {
//  // return error message and code
//} ELSEDO
//  // file is open
//  // do things
//  AFTERDO { fclose(file); }
//}


/*
static struct status_ {
  INT update, rendermode;
  CST keys;
  CST dims;
  CST device;
  INT res, ptsize, lcdix;
  FPT xbold, ybold, radius, slant;
  UNT cffhint, type1hint, t1cidhint;
  UNT ttinterpreterversions[3];
  INT numttinterpreterversions;
  INT ttinterpreterversionidx;
  //TFT warping;
  INT fontix, offset, topleft, fails, preload;
  INT lcdfilter;
  UCT lcdfilterweights[5];
  INT lcdfilterweightidx;
} status = {
   1, RENDER_MODE_TEXT, //ALL,         // update, rendermode
  "", DIM, NULL,               // keys, dims, device
  72, 48, 1,                   // res, ptsize, lcdix
  0.04, 0.04, 0.02, 0.22,      // xbold, ybold, radius, slant
  0, 0, 0,                     // cffhint, type1hint, t1cidhint
  { 0 }, 0, 0,                 // [num]ttinterpreterversion[s,idx]
//  0,                           // warping
  0, 0, 0, 0, 0,               // fontix, offset, topleft, numfails, preload
  FT_LCD_FILTER_DEFAULT,       // lcdfilter
  { 0x08, 0x4D, 0x56, 0x4D, 0x08 }, 2 // lcdfilterweight[s,idx]
};
*/

#define FONTPATH "./ukai.ttf"

#define ZHU "\xE7\x8C\xAA" // 猪
#define TOU "\xE5\xA4\xB4" // 头


//#define INITIALTEXT \
//  "\xE4\xBD\xA0\xE8\xBF\x98\xE7\x88\xB1\xE6\x88\x91\xE5\x90\x97\xEF\xBC\x9F"

//SCST Text = ZHU TOU ZHU TOU ZHU TOU;
#define INITIALTEXT    ZHU TOU ZHU TOU ZHU TOU


static struct status_ {
  INT update;
  INT fails;
  CST textcontent;
  INT pixelwidth;
  INT pixelheight;
  INT textmarginleftx;
  INT textmargintopy;
  //INT xincrement;
  //INT yincrement;
} status = {
  1   , // update
  0   , // fails
  INITIALTEXT, // textcontent
//  Text, // textcontent
  250 , // pixelwidth
  350 , // pixelheight
  10  , // textmarginleftx
  10    // textmargintopy
};


/*
SVDT Fatal(CST message) {
  FTDemo_Display_Done(display);
  FTDemo_Done(handle);
  PanicZ(message);
}
*/

// - smaller X
// _ smaller Y
// = larger  X
// + larger  Y
// ^- smaller XY
// ^= larger  XY
// ^_ more superscript
// ^+ more subscript

SERT rendertext(grBitmap *sbitmap, FT_Face *ftface) { //, CST srctext) { //RenderStroke(INT numindices, INT offset) {
  // where is error defined?

  CST srctext = status.textcontent;
// actual srcbmp may be a different size !
// each actual glyph may be a different size !
  error = FT_Set_Pixel_Sizes(*ftface, status.pixelwidth, status.pixelheight);
 // WIDTH AND HEIGHT !!!!!
  if (error) { printf("SETPIXELSIZES ERROR\n"); return 3; } // <- unsafe exit.. break?


//  FT_Size size;
//  error = FTDemo_Get_Size(handle, &size);
//  if (error) { printf("GETSIZEFAIL\n"); return error; } // non-existent font size
//  INT xstart = START_X;
//  INT ystart = CEIL(size->metrics.ascender - size->metrics.descender) + START_Y;
//  INT ystep  = CEIL(size->metrics.height) + 4;
  INT xstart = status.textmarginleftx;
  INT ystart = status.textmargintopy;
  INT x = xstart;
  INT y = ystart;
//  FAT face = size->face;
//  GST slot = face->glyph;
//  XPT radius = (XPT)(size->metrics.y_ppem * 64 * status.radius);
//  FT_Stroker_Set(handle->stroker, radius, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
//  YNT havetopleft = 0;
  CST textp = srctext;
  CST textend = textp + strlen(srctext);
  //INT i = offset - 1;
  //while (++i < numindices) {
  INT ch = 0;
  // FT_Uint32
  while (ch >= 0) { // loop until done
    ch = utf8_next(&textp, textend);
    if (ch < 0) { printf("EOF\n"); break; } // don't write EOF
    if (ch == 0) { printf("NUL\n"); break; } // or NUL
    UIT glyphix = FT_Get_Char_Index(*ftface, ch);
    if (glyphix == 0) { printf("MISSINGGLYPH\n"); } // continue anyway..
//    UIT glyphix = FTDemo_Get_Index(handle, (FT_UInt32)ch); // <----- unsigned int?
printf("ch is %d, glyphix is %d\n", ch, glyphix);

//    error = FT_Load_Glyph(face, glyphix, handle->load_flags | FT_LOAD_NO_BITMAP);
    error = FT_Load_Glyph(*ftface, glyphix, FT_LOAD_RENDER);
    if (error) { printf("LOADGLYPHFAIL\n"); }
    ELSEDO
      FT_Bitmap *srcbmp = &(*ftface)->glyph->bitmap;
//      grColor white;
//      memset(&white, 0xFF, sizeof(grColor));
      
//      grBlitGlyphToBitmap(sbitmap, srcbmp, x, y, white);
      printf("srcbmp w: %d h: %d\n", srcbmp->width, srcbmp->rows);
      printf("sbitmap w: %d h: %d\n", sbitmap->width, sbitmap->rows);

      int dy = y;
      while (dy <= y + srcbmp->rows) {
        int dx = x;
        while (dx <= x + srcbmp->width) {
          sbitmap->buffer[dy * sbitmap->pitch + dx] =
            srcbmp->buffer[(dy - y) * srcbmp->pitch + (dx - x)];
          dx++;
        }
        dy++;
      }

      if (x + srcbmp->width >= sbitmap->width) {
        x = xstart;
        y += srcbmp->rows; // height; // MINUS?  // = srcbmp->height; // or rows?
      } else { x += srcbmp->width; }

    AFTERDO { } // FT_Done_Glyph only for FT_Get_Glyph? // free srcbmp ?!?!?!
  }
  printf("DONETEXT\n");
  return 0;
}
//      INT width  srcbmp->width;
//      if (x + srcbmp->width >= sbitmap->width) {
//        x = xstart;
//        y += srcbmp->height; // or rows?
//      } else { x += srcbmp->width; }
//    AFTERDO
//    AFTERDO { FT_Done_Glyph(.....); }
/*
//    if (!error && slot->format == FT_GLYPH_FORMAT_OUTLINE) {
      GLT glyph;
      error = FT_Get_Glyph(slot, &glyph);
      if (error) { printf("GETGLYPHFAIL\n"); status.fails++; break; }
      ELSEDO
      else do {
        grBitmap srcbmp;
        // FTDemo_Index_To_Bitmap .........
        // we have a glyph
        grColor white;
        memset(&white, 0xFF, sizeof(grColor));
        grBlitGlyphToBitmap(sbitmap, srcbmp, x, y, white);
      }
FTDemo_Display->forecolor

//        error = FT_Glyph_Stroke(&glyph, handle->stroker, 1);


*/
/*
        if (error) { printf("GLYPHSTROKEFAIL"); status.fails++; break; }
        INT width = slot->advance.x ? slot->advance.x >> 6 : size->metrics.y_ppem / 2;
        if (x + width >= display->bitmap->width) {
          x = xstart;
          y += ystep;
          if (y > display->bitmap->rows)
            { printf("ENDOFSCREEN"); status.fails++; break; }
        }
        x++; // add an extra pixel of space per glyph
        if (slot->advance.x == 0) {
          // FT_Color
printf("NOADVANCE\n");
//          grFillRect(display->bitmap, x, y - width, width, width, white);
//display->warn_color);
          x += width;
        }
//        grColor white = { 0xFF }; // { 0xFF, 0xFF, 0xFF }; // , 0xFF };
        error = FTDemo_Draw_Glyph(handle, display, glyph, &x, &y);
//        error = FTDemo_Draw_Glyph_Color(handle, display, glyph, &x, &y, white);
        if (error) { printf("DRAWGLYPHFAIL\n"); status.fails++; break; }
        else {
          if (!havetopleft)
            { havetopleft = 1; status.topleft = ch; }
          break; // done
        }
      } while (0);
      FT_Done_Glyph(glyph);
    } else {
      // SLOT FORMAT
      printf("slotformat: %d\n", slot->format);
      printf("error: %d\n", error);
      status.fails++;
      break;
    }
*/

//    error = FTDemo_Draw_Index(handle, display, glyphix, &x, &y);
//    if (error) { printf("DRAWINDEXFAIL\n"); status.fails++; break; }


/*
    if (!havetopleft) // firstchaaaaaar
      { havetopleft = 1; status.topleft = ch; }
    // INT width = size->metrics.max_advance >> 6;
    //INT width = size->metrics.advance.x >> 6;
    INT width = slot->advance.x >> 6; // <------------
    if (x + width >= display->bitmap->width) {
      // wrap to start of next line ....
      x = xstart;
      y += ystep;
    } else { x += width; }
*/
//  }
//  return 0; // GET INDEX ERROR ?return error;
//}
SINT processevent(grEvent event) {
/*
  grEvent event;
  INT ret = 1;
  if (*status.keys) {
    event.key = grKEY(*status.keys++);
  } else {
    grListenSurface(display->surface, 0, &event);
    if (event.type == gr_event_resize) {
      status.update = 1;
      return ret;
    }
  }
  status.update = 0;
  if (status.rendermode == (INT)(event.key - '1')) {
    return ret; // already in the current mode
  } else if (event.key >= '1' && event.key < '1' + N_RENDER_MODES) {
    status.rendermode = (INT)(event.key - '1');
    event_render_mode_change(0);
    status.update = 1;
    return ret;
  } else if (event.key >= 'A' && event.key < 'A' + LCDModesLEN) {
    INT lcdix = (INT)(event.key - 'A');
    if (status.lcdix == lcdix) { return ret; } // already in the current mode
    handle->lcd_mode = LCDModes[lcdix];
// consider just handle->lcd_mode = LCD_MODE_AA; // 0 == monochrome ?
    return ret;
  } else 
*/
  INT ret = 1;
  if (event.key == grKeyEsc || event.key == grKEY('q')) {
    ret = 0; // exit event processing loop
  } else if (event.key == grKEY('L') || event.key == grKEY('l')) {
    printf("LCDIX!!\n");
//    INT lcdixdelta = (event.key == grKEY('L')) ? 1 : -1;
//    status.lcdix = (status.lcdix + lcdixdelta + LCDModesLEN) % LCDModesLEN;
//    handle->lcd_mode = LCDModes[status.lcdix];
//    FTDemo_Update_Current_Flags(handle);
//    status.update = 1;
  // grKeySpace
  // grKeyBackSpace
  // grKeyTab
  } else if (event.key == grKEY('R') || event.key == grKEY('r')) {
    // event_radius_change
    // event_size_change
  } // else { }
  return ret;
}

static struct setup_ {
  UNT ttiversion;
  INT dispwidth;
  INT dispheight;
  INT dispmode;
//  INT dispres;
  INT dispgreys;
  grSurface *surface;
  grBitmap  *surfacebmp;
} setup = {
  0                  , // ttinterpreterversion
  1000               , // dispwidth
  600                , // dispheight
  gr_pixel_mode_rgb24, // dispdepth
//  72                 , // dispres (ppi? not used?)
  256                , // dispgreys
  NULL                 // surfacebmp
};

int main(int argc, char **argv) {
  FT_Library ftlib;
  if (FT_Init_FreeType(&ftlib) != 0)
    { printf("FREETYPEINITFAIL\n"); return 1; }
  ELSEDO // FreeType is initialised
  FT_Int major, minor, patch;
  FT_Library_Version(ftlib, &major, &minor, &patch);
  FT_Property_Get(ftlib, "truetype", "interpreter-version", &setup.ttiversion);
  printf("FreeType v%d.%d(.%d)\n", major, minor, patch);
  printf("TrueType interpreter v%d\n", setup.ttiversion);

  grInitDevices();
 
  grBitmap scrbuf;
  scrbuf.mode  = setup.dispmode  ;
  scrbuf.width = setup.dispwidth ;
  scrbuf.rows  = setup.dispheight;
  scrbuf.grays = setup.dispgreys ;
 
  setup.surface = grNewSurface(0, &scrbuf);
  if (!setup.surface) { printf("NEWSURFACEFAIL\n"); break; }

  setup.surfacebmp = &setup.surface->bitmap;
  printf("scrbuf&: %12lx, surfacebmp: %12lx\n",
           (unsigned long)(void *)&scrbuf,
           (unsigned long)(void *)setup.surfacebmp);
  printf("default GAMMA: %f\n", GAMMA);
    // grSetTargetGamma(bsurface, GAMMA);
  // expect v35, v38 or v40
//  handle = FTDemo_New(); <------------------------------
//  FT_Property_Get(/*handle->*/ftlib, "cff",   "hinting-engine", &status.cffhint);
//  FT_Property_Get(/*handle->*/ftlib, "type1", "hinting-engine", &status.type1hint);
//  FT_Property_Get(/*handle->*/ftlib, "t1cid", "hinting-engine", &status.t1cidhint);
// expect FT_HINTING_FREETYPE or FT_HINTING_ADOBE

//  render_state_init(state, display, library);
//  if (dispres > 0)
//    { render_state_set_resolution(state, (UNT)dispres); }
  // render_state_set_size
  // render_state_set_files
// later: warping
  // later: FTDemo_Set_Preload(handle, 1); <-------
  CST fontfile = FONTPATH;
  INT faceindex = 0; // index 0 unless multi-face (or -1) ?
  FT_Face fontface;
  error = FT_New_Face(ftlib, fontfile, faceindex, &fontface); // -1 ??
  if (error) { printf("FTNEWFACEFAIL\n"); break; }
  ELSEDO
  // can set size later .....?
/*
  if (argc < 2) {
//    printf("SIZE arg required\n");
    status.ptsize = 32.0 * 64.0;
  } else {
    status.ptsize = (INT)(atof(argv[1]) * 64.0);
  }
*/
// instead of FT_Set_Char_Size(face, bla, (FT_F26Dot6)(PIXELSIZE * 64.0), 0, RESOLUTION); <-----

//  long numfaces = fontface->num_faces;
//  error = FTDemo_Install_Font(handle, fontfile, 0, 0);
//  if (error || handle->num_fonts < 1) {
//    printf("failed to open font\n");
//    return 1;
//  }
//  display = FTDemo_Display_New(status.device, status.dims);
//  if (!display) {
//    printf("failed to create a surface\n");
//    return 2;
//  } else {
//  }
  // grSetTitle(adisplay->surface, "...."); ???????????? same thing same surface

//  grSetTitle(display->surface, "ZHUTOU");
  grSetTitle(setup.surface, "ZHUTOU");
//  FTDemo_Icon(handle, display); // TODO: TRY THIS LATER TO SEE IF IT WORKS
  //FTDemo_Display_Clear(display);
  status.fails = 0;
//  FTDemo_Set_Current_Font(handle, handle->fonts[0]);
//  FTDemo_Set_Current_Charsize(handle, status.ptsize, status.res);
//  FTDemo_Update_Current_Flags(handle);
//  INT loadflags = 
//  loadflags |= FT_LOAD_NO_HINTING;
//  loadflags |= FT_LOAD_FORCE_AUTOHINT;
//  loadflags |= FT_LOAD_NO_BITMAP;
//  loadflags |= FT_LOAD_COLOR;
//  loadflags |= FT_LOAD_TARGET_NORMAL; // or _MONO or _LIGHT or _LCD or _LCD_V
//  loadflags |= FT_LOAD_MONOCHROME if FT_LOAD_TARGET_MONO
  // status.offset = handle->current_font->num_indices;
//  grRefreshSurface(display->surface);
  grEvent grevent;
  //grListenSurface(display->surface, gr_event_key, &dummyevent);
  do {
    if (!status.update) { continue; } // <--- undelayed infinite loop
//    FTDemo_Display_Clear(display);
    grColor white = { 0xAA };
//    grFillRect(display->bitmap, 0, 0, 640, 480, white); //display->warn_color);
    grFillRect(setup.surfacebmp, 0, 0, 640, 420, white); //display->warn_color);
    error = rendertext(setup.surfacebmp, &fontface); // , Text); // add properties parameter
    if (error) { printf("RENDERTEXT ERROR\n"); return 3; } // <- unsafe exit.. break?
    //grRefreshSurface(display->surface);
    grRefreshSurface(setup.surface);
    grListenSurface(setup.surface, 0, &grevent);
//    grListenSurface(display->surface, 0, &grevent);
    if (grevent.type == gr_event_resize) {
      printf("THIS IS A RESIZE\n");
    }
  } while (processevent(grevent));
  if (status.fails > 0) {
    printf("status.fails: %d\n", status.fails);
  }
//  FTDemo_Display_Done(display);
//  FTDemo_Done(handle);

  AFTERDO { FT_Done_Face(fontface);  }
  AFTERDO { FT_Done_FreeType(ftlib); }
  ftlib = NULL;
  exit(0);
  return 0;
}
#endif

    /*
    */
//  }
//  // FT_Free_Glyph to counter FT_Load_Glyph?
//  return error;
//}

//SERT Render_Fancy(





