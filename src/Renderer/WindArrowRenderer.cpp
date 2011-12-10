/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "WindArrowRenderer.hpp"
#include "Look/WindArrowLook.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/Layout.hpp"
#include "Screen/TextInBox.hpp"
#include "Math/Angle.hpp"
#include "Math/Screen.hpp"
#include "NMEA/Derived.hpp"
#include "Units/Units.hpp"
#include "SettingsMap.hpp"
#include "Util/Macros.hpp"

#include <tchar.h>
#include <cstdio>

void
WindArrowRenderer::Draw(Canvas &canvas, const Angle screen_angle,
                        const SpeedVector wind, const RasterPoint pos,
                        const PixelRect rc, bool with_tail)
{
  // Draw arrow

  PixelScalar wmag = iround(4 * wind.norm);

  RasterPoint arrow[] = {
      { 0, -20 },
      { -6, (PixelScalar)(-26 - wmag) },
      { 0, (PixelScalar)(-20 - wmag) },
      { 6, (PixelScalar)(-26 - wmag) },
      { 0, -20 },
  };

  // Rotate the arrow (exclude duplicate last point)
  PolygonRotateShift(arrow, ARRAY_SIZE(arrow) - 1,
                     pos.x, pos.y, wind.bearing - screen_angle);

  // Copy first point to last point to close polygon
  arrow[4] = arrow[0];

  canvas.Select(look.hpWind);
  canvas.Select(look.hbWind);
  canvas.polygon(arrow, 5);


  // Draw arrow tail

  if (with_tail) {
    RasterPoint tail[] = {
      { 0, Layout::FastScale(-20) },
      { 0, Layout::FastScale(-26 - min(PixelScalar(20), wmag) * 3) },
    };

    PolygonRotateShift(tail, ARRAY_SIZE(tail),
                       pos.x, pos.y, wind.bearing - screen_angle);

    canvas.Select(look.hpWindTail);
    canvas.line(tail[0], tail[1]);
  }


  // Draw wind speed label

  StaticString<12> buffer;
  buffer.Format(_T("%i"), iround(Units::ToUserWindSpeed(wind.norm)));

  canvas.SetTextColor(COLOR_BLACK);
  canvas.Select(Fonts::map_bold);

  UPixelScalar text_width = canvas.CalcTextSize(buffer).cx / 2;
  UPixelScalar kx = text_width / Layout::FastScale(1) / 2;
  RasterPoint label[] = {
      { (PixelScalar)(8 + kx), -24 },
      { (PixelScalar)(-8 - kx), -24 }
  };
  PolygonRotateShift(label, ARRAY_SIZE(label),
                     pos.x, pos.y, wind.bearing - screen_angle);

  TextInBoxMode style;
  style.align = A_CENTER;
  style.mode = RM_OUTLINED;

  if (label[0].y >= label[1].y)
    TextInBox(canvas, buffer, label[0].x - kx, label[0].y, style, rc);
  else
    TextInBox(canvas, buffer, label[1].x - kx, label[1].y, style, rc);
}

void
WindArrowRenderer::Draw(Canvas &canvas, const Angle screen_angle,
                        const RasterPoint pos, const PixelRect rc,
                        const DerivedInfo &calculated,
                        const SETTINGS_MAP &settings)
{
  if (!calculated.wind_available)
    return;

  // don't bother drawing it if not significant
  if (calculated.wind.norm < fixed_one)
    return;

  WindArrowRenderer::Draw(canvas, screen_angle, calculated.wind, pos, rc,
                          settings.wind_arrow_style == 1);
}
