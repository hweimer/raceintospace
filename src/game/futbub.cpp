/*
    Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/** \file futbub.c Draw future bubbles
 *
 * The code in this file will create
 * the various bubbles in the "future missions" screen.
 *
 * It also draws the splines.
 */

#include "futbub.h"

#include "display/graphics.h"
#include "display/surface.h"

#include "Buzz_inc.h"
#include "draw.h"
#include "game_main.h"
#include "gr.h"
#include "logging.h"
#include "mc.h"
#include "mc2.h"


LOG_DEFAULT_CATEGORY(future)


static const int MAXBUB = 30;

namespace
{
// Stepbub, missStep, and B_Mis are used in drawing mission flight
// paths.
struct StepInfo {
    int16_t x_cor;
    int16_t y_cor;
} StepBub[MAXBUB];


int Bub_Num;
int bubCount;
// SEG determines the number of control points used in creating
// the B-splines for drawing the mission flight path.
// The more control points, the smoother the path should
// appear.
int SEG = 15;
char missStep[1024];


static inline char B_Mis(char x)
{
    return missStep[x] - 0x30;
};
void Bd(int x, int y);
void drawBspline(int segments, char color, ...);
void Draw_IJ(char w);
void Draw_IJV(char w);
void Draw_LowS(char a, char b, char c, char x, char y, char z);
void DrawLunPas(char x, char y, char z, char w);
void DrawLefMoon(char x, char y);
void DrawSTUV(char x, char y, char z, char w);
void Draw_HighS(char x, char y, char z);
void DrawMoon(char x, char y, char z, char w, char j, char k, char l);
void Draw_PQR(void);
void Draw_PST(void);
void Draw_GH(char a, char b);
void DrawZ(void);
void Fly_By(void);
void LefEarth(char a, char b);
void LefGap(char x);
void LefOrb(char a, char b, char c, char d);
void OrbIn(char a, char b, char c);
void OrbMid(char a, char b, char c, char d);
void OrbOut(char a, char b, char c);
void Q_Patch(void);
void RghtMoon(char x, char y);
void S_Patch(char x);
void VenMarMerc(char x);
};  // End of local namespace



/**
 * TODO: Documentation.
 *
 * TODO: This name is surprisingly uninformative for being so long.
 */
void DecreasePathResolution()
{
    if (SEG > 1) {
        SEG--;
    }
}


/**
 * TODO: Documentation
 *
 * \return  the index of the bubble, or -1 if none selected.
 */
char GetBubbleAt(int x, int y)
{
    for (int8_t i = 0; i < bubCount; i++) {
        if (x >= StepBub[i].x_cor && x <= StepBub[i].x_cor + 7 &&
            y >= StepBub[i].y_cor && y <= StepBub[i].y_cor + 7) {
            return i;
        }
    }

    return -1;
}


/**
 * TODO: Documentation.
 *
 * TODO: This name is surprisingly uninformative for being so long.
 */
void IncreasePathResolution()
{
    if (SEG < 500) {
        SEG++;
    }
}


/* Illustrates the mission path on the starfield and loads mission
 * step information.
 *
 * This populates the global variable Mev and the file variable
 * missStep with data about the specified mission.
 *
 * Flight path information is stored in the file missStep.dat.
 * missStep.dat is plain text, with:
 *  -  Mission Number (2 first bytes of each line)
 *  -  A Coded letter, each drawing a different line
 *  -  Numbers following each letter, which are the parameters
 *     of the function
 *  -  Each line must finish with a Z, so the game stops reading
 * Any other char is ignored, but it's easier for a human to read that
 * way.
 *
 * This modifies the global variables Mev via MissionCodes().
 *
 * TODO: This should try to redraw the starfield.
 *
 * \param plr  See MissionCodes().
 * \param val  The mission code.
 * \param pad  the pad (0, 1, or 2) where the mission is being launched.
 */
void MissionPath(char plr, int val, int pad)
{
    TRACE3("->MissionPath(plr, val %d, pad %d)", val, pad);

    // Clear existing global / file global mission step information.
    // These are cleared by DrawMission, but no point taking chances.
    bubCount = 0;  // set the initial bub_count
    memset(Mev, 0x00, sizeof Mev);
    
    // Deserialize missSteps
    std::vector<std::string> missSteps;
    DESERIALIZE_JSON_FILE(&missSteps, locate_file("missSteps.json", FT_DATA));
    
    // Read mission step data and find correct entry
    int index = 0;
    std::string code;
    while (index < missSteps.size()) {
        code = missSteps[index];
        // Transform the first characters in a number and compare with val
        if ((code[0] - '0') * 10 + (code[1] - '0') == val) { 
            break;
        }
      index++;
    }
    
    strncpy(missStep, code.c_str(), 1024 - 1);
    missStep[1024-1] = '\0';

    for (int n = 2; missStep[n] != 'Z'; n++) {
        switch (missStep[n]) {
        case 'A':
            Draw_IJ(B_Mis(++n));
            break;

        case 'B':
            Draw_IJV(B_Mis(++n));
            break;

        case 'C':
            OrbOut(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3));
            n += 3;
            break;

        case 'D':
            LefEarth(B_Mis(n + 1), B_Mis(n + 2));
            n += 2;
            break;

        case 'E':
            OrbIn(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3));
            n += 3;
            break;

        case 'F':
            OrbMid(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4));
            n += 4;
            break;

        case 'G':
            LefOrb(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4));
            n += 4;
            break;

        case 'H':
            Draw_LowS(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4), B_Mis(n + 5), B_Mis(n + 6));
            n += 6;
            break;

        case 'I':
            Fly_By();
            break;

        case 'J':
            VenMarMerc(B_Mis(++n));
            break;

        case 'K':
            Draw_PQR();
            break;

        case 'L':
            Draw_PST();
            break;

        case 'M':
            Draw_GH(B_Mis(n + 1), B_Mis(n + 2));
            n += 2;
            break;

        case 'N':
            Q_Patch();
            break;

        case 'O':
            RghtMoon(B_Mis(n + 1), B_Mis(n + 2));
            n += 2;
            break;

        case 'P':
            DrawLunPas(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4));
            n += 4;
            break;

        case 'Q':
            DrawLefMoon(B_Mis(n + 1), B_Mis(n + 2));
            n += 2;
            break;

        case 'R':
            DrawSTUV(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4));
            n += 4;
            break;

        case 'S':
            Draw_HighS(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3));
            n += 3;
            break;

        case 'T':
            DrawMoon(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4), B_Mis(n + 5), B_Mis(n + 6), B_Mis(n + 7));
            n += 7;
            break;

        case 'U':
            LefGap(B_Mis(++n));
            break;

        case 'V':
            S_Patch(B_Mis(++n));
            break;

        case 'W':
            DrawZ();
            break;

        default :
            break;
        }
    }

    gr_sync();

    MissionCodes(plr, val, pad);

    TRACE1("<-MissionPath()");
}


/**
 * Check how many steps of the mission flight path have been drawn.
 */
int MissionStepsDrawn()
{
    return bubCount;
}


//----------------------------------------------------------------------
// Local code
//----------------------------------------------------------------------

// Anonymous namespace
namespace
{

/**
 * Draws a flight path bubble on the screen.
 *
 * This stores the bubble coordinates in StepBub and increments the
 * Bub_count.
 *
 * \param x x-coord of the upper left corner of the bubble
 * \param y y-coord of the upper left corner of the bubble
 */
void Bd(int x, int y)
{
    int x1, y1, x2, y2;
    x1 = x - 2;
    y1 = y;
    x2 = x - 1;
    y2 = y - 1;
    fill_rectangle(x1, y1, x1 + 8, y1 + 4, 21);
    fill_rectangle(x2, y2, x2 + 6, y2 + 6, 21);
    display::graphics.setForegroundColor(1);
    grMoveTo(x, y + 4);
    /** \note references bubCount to determine the number of the character to draw in the bubble */
    draw_character(65 + bubCount);
    StepBub[bubCount].x_cor = x1;
    StepBub[bubCount].y_cor = y1;
    ++bubCount;
    return;
}


void drawBspline(int segments, char color, ...)
{
    va_list coord;
    int xpoint[20], ypoint[20], i = 1, j, x, y, oldx, oldy, last, yx, yy, yyy = 0;
    float u, nc1, nc2, nc3, nc4;
    int SamCol;

    yy = yx = 0; /* XXX check uninitialized */

    display::graphics.setForegroundColor(color);
    /* initialize the array  */
    memset(xpoint, 0, sizeof(xpoint));
    memset(ypoint, 0, sizeof(ypoint));

    va_start(coord, color);

    for (i = 1; i <= Bub_Num; i++) {
        xpoint[i] = va_arg(coord, int);
        ypoint[i] = va_arg(coord, int);
    }

    va_end(coord);
    xpoint[0] = xpoint[1];
    ypoint[0] = ypoint[1];
    oldx = xpoint[0];
    oldy = ypoint[0];

    for (j = i; j <= i + 1; j++) {
        xpoint[j] = xpoint[j - 1];
        ypoint[j] = ypoint[j - 1];
    }

    last = j;

    for (i = 1; i <= last - 3; i++) {
        for (u = 0; u <= 1; u += 1.0 / segments) {
            nc1 = -(u * u * u / 6) + u * u / 2 - u / 2 + 1.0 / 6;
            nc2 = u * u * u / 2 - u * u + 2.0 / 3;
            nc3 = (-u * u * u + u * u + u) / 2 + 1.0 / 6;
            nc4 = u * u * u / 6;
            x = (nc1 * xpoint[i - 1] + nc2 * xpoint[i] + nc3 * xpoint[i + 1] + nc4 * xpoint[i + 2]);
            y = (nc1 * ypoint[i - 1] + nc2 * ypoint[i] + nc3 * ypoint[i + 1] + nc4 * ypoint[i + 2]);
            SamCol = display::graphics.legacyScreen()->getPixel(x, y);

            if (yyy == 1) {
                display::graphics.legacyScreen()->setPixel(yx, yy, 5);
                yyy = 0;
            }

            if ((SamCol != 1) && (SamCol != 21)) {
                grMoveTo(oldx, oldy);
                grLineTo(x, y);

                if (i < (last - 3)) {
                    display::graphics.legacyScreen()->setPixel(x, y, 40);
                    yx = x;
                    yy = y;
                    yyy = 1;
                }
            }

            oldx = x;
            oldy = y;
        }
    }

    return;
}


void Draw_IJ(char w)
{
    Bd(73, 134); // bubble 'I'
    Bub_Num = 3;
    drawBspline(SEG, 5, 73, 140, 81, 145, 89, 145);

    if (w == 1) {
        Bd(77, 142);    // bubble 'J'
    }

    return;
}

void Draw_GH(char a, char b)
{
    Bd(60, 130);  // bubble 'G'
    Bub_Num = 4;
    drawBspline(SEG, 5, 61, 136, 61, 143, 63, 147, 68, 148);

    if (a == 1) {
        Bd(60, 143);    // bubble 'H'
    }

    Bub_Num = 4;
    drawBspline(SEG, 5, 68, 148, 71, 151, 81, 153, 84, 151);

    if (b == 1) {
        Bd(77, 150);    // bubble 'K'
    }

    return;
}

void Draw_IJV(char w)
{
    Bub_Num = 3;
    drawBspline(SEG, 5, 89, 145, 100, 143, 110, 143);

    if (w == 1) {
        Bd(102, 141);    // bubble 'V'
    }

    return;
}

void OrbIn(char a, char b, char c)
{
    Bub_Num = 4;
    drawBspline(SEG, 5, 110, 143, 116, 141, 125, 134, 118, 126);

    if (a == 1) {
        Bd(122, 128);    // bubble 'Z'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 118, 126, 110, 123, 103, 123);

    if (b == 1) {
        Bd(108, 122);    // bubble 'Y'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 103, 123, 98, 123);

    if (c == 1) {
        Bd(95, 118);    // bubble 'W'
    }

    return;
}

void OrbMid(char a, char b, char c, char d)
{
    Bub_Num = 3;
    drawBspline(SEG, 5, 110, 143, 116, 141, 125, 134);

    if (a == 1) {
        Bd(116, 137);    // bubble 'a'
    }

// grPutPixel(113,141,5);
    Bub_Num = 2;
    drawBspline(SEG, 5, 125, 134, 128, 125);

    if (b == 1) {
        Bd(122, 128);    // bubble 'Z'
    }

// grPutPixel(122,134,5);
    Bub_Num = 2;
    drawBspline(SEG, 5, 128, 125, 123, 115);

    if (c == 1) {
        Bd(122, 118);    // bubble 'd'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 123, 115, 110, 112);

    if (d == 1) {
        Bd(109, 112);    // bubble 'X'
    }

    return;
}

void Q_Patch(void)
{
    Bub_Num = 2;
    drawBspline(SEG, 5, 85, 151, 107, 144);
    return;
}

void OrbOut(char a, char b, char c)
{
    Bub_Num = 3;
    drawBspline(SEG, 5, 110, 143, 120, 140, 125, 137);

    if (a == 1) {
        Bd(116, 137);    // bubble 'a'
    }

    Bub_Num = 5;
    drawBspline(SEG, 5, 125, 137, 132, 134, 141, 125, 141, 117, 134, 115);

    if (b == 1) {
        Bd(139, 115);    // bubble 'e'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 134, 115, 123, 113, 110, 112);

    if (c == 1) {
        Bd(109, 112);    // bubble 'X'
    }

    return;
}

void LefEarth(char a, char b)
{
    Bub_Num = 5;
    drawBspline(SEG, 5, 35, 137, 20, 149, 36, 157, 54, 157, 61, 155);

    if (a == 1) {
        Bd(51, 152);    // bubble 'E'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 61, 155, 80, 153, 85, 151);

    if (b == 1) {
        Bd(77, 150);    // bubble 'K'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 85, 151, 107, 144);
    return;
}


void LefOrb(char a, char b, char c, char d)
{
    /* bubbles A_B_C_D */
    Bub_Num = 2;
    drawBspline(SEG, 5, 35, 137, 25, 146);

    if (a == 1) {
        Bd(27, 137);    // bubble 'b'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 25, 146, 26, 150, 32, 153);

    if (b == 1) {
        Bd(24, 147);    // bubble 'a'
    }

    Bub_Num = 4;
    drawBspline(SEG, 5, 32, 153, 42, 150, 47, 143, 48, 141);

    if (c == 1) {
        Bd(40, 145);    // bubble 'c'
    }

    if (d == 1) {
        Bd(47, 135);    // bubble 'd'
    }

    return;
}

void Fly_By(void)
{
    /* bubbles I_J_F */
    Bub_Num = 8;
    drawBspline(SEG, 5, 89, 145, 97, 147, 108, 145, 122, 139, 130, 130, 130, 120, 121, 116, 107, 115);
    Bub_Num = 6;
    drawBspline(SEG, 5, 36, 137, 20, 149, 36, 157, 41, 159, 46, 162, 69, 164);
    Bd(59, 160); // bubble 'F'
    return;
}

void VenMarMerc(char x)
{
    Bub_Num = 3;
    drawBspline(SEG, 5, 69, 164, 88, 169, 125, 170);
    Bd(115, 166); // bubble 'L'

    if (x == 1) {
        Bub_Num = 4;
        drawBspline(SEG, 5, 125, 170, 146, 174, 172, 174, 195, 163);
        Bd(189, 159); // bubble 'N'
    }

    if (x == 2) {
        Bub_Num = 2;
        drawBspline(SEG, 5, 125, 170, 162, 174);
        Bd(150, 171); // new bubble
        Bub_Num = 3;
        drawBspline(SEG, 5, 162, 174, 190, 176, 207, 162);
        Bd(204, 160); // bubble 'O'
    }

    if (x == 3) {
        Bub_Num = 4;
        drawBspline(SEG, 5, 125, 170, 138, 171, 153, 170, 166, 165);
        Bd(160, 161); // bubble 'M'
    }

    return;
}

void Draw_PQR(void)
{
    Bub_Num = 5;
    drawBspline(SEG, 5, 65, 164, 91, 168, 162, 186, 198, 190, 203, 190);
    Bd(194, 185); // bubble 'P'
    Bub_Num = 3;
    drawBspline(SEG, 5, 203, 190, 235, 187, 249, 179);
    Bd(242, 181); // bubble 'Q'
    Bub_Num = 2;
    drawBspline(SEG, 5, 249, 179, 263, 172);
    Bd(261, 170); // bubble 'R'
    return;
}

void Draw_PST(void)
{
    Bub_Num = 5;
    drawBspline(SEG, 5, 65, 164, 91, 168, 162, 186, 198, 190, 203, 190);
    Bd(194, 185); // bubble 'P'
    Bub_Num = 4;
    drawBspline(SEG, 5, 203, 190, 240, 193, 269, 192, 279, 191);
    Bd(270, 186); // bubble 'S'
    Bub_Num = 2;
    drawBspline(SEG, 5, 279, 191, 303, 190);
    Bd(300, 188); // bubble 'T'
    return;
}

void Draw_LowS(char a, char b, char c, char x, char y, char z)
{
    Bub_Num = 2;
    drawBspline(SEG, 5, 110, 143, 125, 137);

    if (a == 1) {
        Bd(116, 137);    // bubble 'a'
    }

    Bub_Num =  3;
    drawBspline(SEG, 5, 125, 137, 146, 131, 152, 127);

    if (b == 1) {
        Bd(144, 129);    // bubble 'c'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 152, 127, 171, 114);

    if (c == 1) {
        Bd(165, 114);    // bubble 'g'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 171, 114, 181, 108, 185, 103);

    if (x == 1) {
        Bd(178, 105);    // bubble 'h'
    }

    Bub_Num = 6;
    drawBspline(SEG, 5, 185, 103, 194, 95, 208, 82, 219, 74, 226, 68, 234, 65);

    if (y == 1) {
        Bd(224, 65);    // bubbles 'l'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 234, 65, 240, 63, 251, 58);

    if (z == 1) {
        Bd(243, 60);    // bubble 'm'
    }

    return;
}

void Draw_HighS(char x, char y, char z)
{
    Bub_Num = 4;
    drawBspline(SEG, 5, 84, 151, 102, 153, 134, 147, 140, 143);

    if (x == 1) {
        Bd(131, 144);    // bubble 'b'
    }

    Bub_Num = 4;
    drawBspline(SEG, 5, 140, 143, 184, 126, 200, 106, 207, 93);
    Bub_Num = 3;
    drawBspline(SEG, 5, 207, 93, 215, 81, 219, 76);

    if (y == 1) {
        Bd(216, 78);    // bubble 'j'
    }

    Bub_Num = 5;
    drawBspline(SEG, 5, 219, 76, 226, 68, 234, 65, 240, 63, 251, 58);

    if (z == 1) {
        Bd(243, 60);    // bubble 'm'
    }

    return;
}

void RghtMoon(char x, char y)
{
    Bub_Num = 5;
    drawBspline(SEG, 5, 291, 44, 300, 43, 307, 46, 311, 52, 306, 58);

    if (x == 1) {
        Bd(306, 50);    // bubble 'y'
    }

    Bub_Num = 4;
    drawBspline(SEG, 5, 306, 58, 302, 61, 285, 68, 272, 73);

    if (y == 1) {
        Bd(276, 68);    // bubble 's'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 272, 73, 268, 75);
    return;
}

void DrawLunPas(char x, char y, char z, char w)
{
    Bub_Num = 2;
    drawBspline(SEG, 5, 268, 75, 253, 80);

    if (x == 1) {
        Bd(256, 79);    // bubble 'o'
    }

    Bub_Num = 5;
    drawBspline(SEG, 5, 253, 80, 227, 91, 194, 96, 176, 100, 149, 104);

    if (y == 1) {
        Bd(155, 100);    // bubble 'i'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 149, 104, 152, 104, 125, 110);

    if (z == 1) {
        Bd(130, 106);    // bubble 'f'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 125, 110, 110, 112);

    if (w == 1) {
        Bd(109, 112);    // bubble 'X'
    }

    return;
}

void DrawLefMoon(char x, char y)
{
    Bub_Num = 4;
    drawBspline(SEG, 5, 268, 75, 254, 79, 238, 78, 240, 69);

    if (x == 1) {
        Bd(235, 72);    // bubble 'k'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 240, 69, 251, 58);

    if (y == 1) {
        Bd(243, 60);    // bubble 'm'
    }

    return;
}


void DrawMoon(char x, char y, char z, char w, char j, char k, char l)
{
    Bub_Num = 5;
    drawBspline(SEG, 5, 291, 44, 300, 43, 307, 46, 311, 52, 306, 58);

    if (x == 1) {
        Bd(306, 50);    // 'y'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 306, 58, 293, 57, 288, 55);

    if (y == 1) {
        Bd(291, 54);    // 'w'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 288, 55, 280, 53, 278, 47);

    if (z == 1) {
        Bd(279, 51);    // 'x '
    }

    Bub_Num = 4;
    drawBspline(SEG, 5, 278, 47, 282, 54, 276, 43, 274, 44);

    if (w == 1) {
        Bd(280, 42);    // '? '
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 274, 44, 268, 49, 268, 53);

    if (j == 1) {
        Bd(268, 46);    // 'q'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 268, 53, 267, 61);

    if (k == 1) {
        Bd(267, 55);    // 'r'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 267, 61, 256, 68, 246, 73);

    if (l == 1) {
        Bd(257, 60);    // 'p'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 246, 73, 240, 69, 251, 58);
    return;
}




void DrawZ(void)
{
    Bub_Num = 4;
    drawBspline(SEG, 5, 307, 46, 301, 40, 294, 38, 301, 40);
    Bd(290, 37); // little bubble 'z'
    Bub_Num = 7;
    drawBspline(SEG, 5, 301, 40, 307, 46, 308, 55, 302, 61, 282, 71, 272, 73, 268, 75);
    return;
}

void DrawSTUV(char x, char y, char z, char w)
{
    Bub_Num = 4;
    drawBspline(SEG, 5, 291, 44, 300, 43, 307, 46, 311, 52);
    Bub_Num = 4;
    drawBspline(SEG, 5, 311, 52, 313, 58, 312, 63, 306, 70);

    if (x == 1) {
        Bd(303, 64);    // bubble 'v'
    }

    Bub_Num = 2;
    drawBspline(SEG, 5, 306, 70, 297, 76);

    if (y == 1) {
        Bd(300, 72);    // bubble 'u'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 297, 76, 293, 78, 282, 74);

    if (z == 1) {
        Bd(285, 75);    // bubble 't'
    }

    Bub_Num = 3;
    drawBspline(SEG, 5, 282, 74, 281, 72, 268, 75);

    if (w == 1) {
        Bd(276, 68);    // bubble 's'
    }

    return;
}

void LefGap(char x)
{
    Bub_Num = 5;
    drawBspline(SEG, 5, 291, 44, 306, 44, 312, 47, 313, 56, 310, 66);

    if (x == 1) {
        Bd(303, 64);
    }

    return;
}

void S_Patch(char x)
{
    Bub_Num = 7;
    drawBspline(SEG, 5, 291, 44, 306, 44, 312, 47, 313, 52, 301, 55, 293, 59, 280, 54);

    if (x == 1) {
        Bd(306, 50);
        Bd(291, 54);
        Bd(279, 51);
    }

    return;
}

};  // End of Anonymous namespace
