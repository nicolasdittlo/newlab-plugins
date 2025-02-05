/* Copyright (C) 2025 Nicolas Dittlo <bluelab.plugins@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this software; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "Curve.h"
#include "Axis.h"
#include "SpectrumView.h"

SpectrumView::SpectrumView() {}

SpectrumView::~SpectrumView() {}

void
SpectrumView::setHAxis(Axis *axis)
{
    _hAxis = axis;
}

void
SpectrumView::setVAxis(Axis *axis)
{
    _vAxis = axis;
}

void
SpectrumView::addCurve(Curve *curve)
{
    curve->setViewSize(_width, _height);
    
    _curves.push_back(curve);
}

void
SpectrumView::getViewSize(int *width, int *height)
{
    *width = _width;
    *height = _height;
}

void
SpectrumView::setViewSize(int width, int height)
{
    _width = width;
    _height = height;

    for (int i = 0; i < _curves.size(); i++)
        _curves[i]->setViewSize(_width, _height);
}
