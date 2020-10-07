//  Interface Editor: a basic circuit editor
//  Copyright (C) 2004  David Yuste Romero (david.yuste@gmail.com)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.



#include "LEHandle.h"

#include <qpainter.h>

//////////////////////////////////////////////////////////////////////
// Constructior/Destructior
//////////////////////////////////////////////////////////////////////

LEHandle::LEHandle( LEHandle::HandleAlignment hAlign, QCanvas * canvas, LEItem * parent )
	: LEItem( canvas, parent )
{
	setSize( defaultSize, defaultSize );
	this->align = hAlign;
}

LEHandle::LEHandle( QCanvas * canvas, LEItem * parent )
	: LEItem( canvas, parent )
{
	// Valor de align arbitrario:
	this->align = LEHandle::LeftTop;
}

LEHandle::~LEHandle()
{
	hide();
}


int LEHandle::rtti() const
{
	return RTTI;
}

//////////////////////////////////////////////////////////////////////
// Geometría
//////////////////////////////////////////////////////////////////////

QPointArray LEHandle::areaPoints() const
{
	QPointArray rv;
	int vx = this->x();
	int vy = this->y();
	rv.putPoints( 0, 5, vx-1,vy-1, vx+width()+1,vy-1, vx+width()+1,vy+height()+1, vx-1,vy+height()+1, vx-1,vy-1 );
	return rv;
}

QRect LEHandle::boundingRect() const
{
	return QRect( this->x()-1, this->y()-1, width()+2, height()+2 );
}

LEHandle::HandleAlignment LEHandle::alignment() const
{
	return align;
}

void LEHandle::setAlignment( LEHandle::HandleAlignment hAlign )
{
	align = hAlign;
}

//////////////////////////////////////////////////////////////////////
// Dibujado
//////////////////////////////////////////////////////////////////////

void LEHandle::drawShape( QPainter &p )
{
	p.fillRect( this->x(), this->y(), width(), height(), QColor( 0,0,0 ) );
}
