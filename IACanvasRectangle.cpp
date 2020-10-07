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



#include "IACanvasRectangle.h"
#include "IAPort.h"

#include <qpainter.h>

IACanvasRectangle::IACanvasRectangle( IAPort * port, QCanvas * canvas )
	: QCanvasRectangle( canvas )
{	
	this->prt = port;
	setBrush( QBrush(QColor( 235, 245, 255 )) );
	setPen( QPen( QColor( 30, 30, 50 ), 1, DotLine) );

	updatePort();
	show();
}

IAPort * IACanvasRectangle::port()
{
	return this->prt;
}

void IACanvasRectangle::updatePort()
{
	QFont stdFont;
	QFontMetrics fm( stdFont );
	
	invalidate();
	setSize( canvas()->width()- fm.width( "FFFF " ), fm.lineSpacing()*port()->size() );
	move( fm.width( "FFFF " ), fm.lineSpacing()*port()->baseAddress() );
	update();
	
	if( canvas() )
		canvas()->update();

}

void IACanvasRectangle::draw( QPainter & p )
{
	QCanvasRectangle::draw( p );

	// Añadimos a la salida el nombre
	QString output = port()->name();

	// y el modo de acceso
	output += " (";
	if( port()->readable() && port()->writable() )
		output += "L/E";
	else if( port()->readable() )
		output += "L";
	else if( port()->writable() )
		output += "E";
	output += ")";

	p.drawText( x()+5, y(), p.fontMetrics().width(output), p.fontMetrics().lineSpacing(), 0, output );

}

