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



#include "LELabel.h"

#include <qfontmetrics.h>
#include <qpainter.h>
#include <qnamespace.h>

#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LELabel::LELabel( QCanvas * canvas, LEItem * parentItem )
	:LEItem( canvas, parentItem )
{
	this->txDirection = TextHorizontal;
	this->autoSize = true;

	setFontColor( QColor(QRgb(0x000000)) );
	setLineColor( QColor(QRgb(0xFFFFFF)) );
	setBackgroundColor( QColor(QRgb(0xFFFFFF)) );
	setLineWidth( 0 );
	this->drawBox = false;

	LEItem::setSize( 2*leftMargin, 2*topMargin );
	
}

LELabel::LELabel( QString label, QCanvas * canvas, LEItem * parentItem ) 
	:LEItem( canvas, parentItem )
{
	this->label = label;
	this->txDirection = TextHorizontal;
	this->autoSize = true;
	
	setFontColor( QColor(QRgb(0x000000)) );
	setLineColor( QColor(QRgb(0xFFFFFF)) );
	setBackgroundColor( QColor(QRgb(0xFFFFFF)) );
	setLineWidth( 0 );
	this->drawBox = false;

	adjustSize();
}

LELabel::~LELabel()
{
	hide();
}

int LELabel::rtti() const
{
	return RTTI;
}

// Texto de la etiqueta
void LELabel::setText( const QString &label ) 
{
	invalidate();
	
	this->label = label;

	// Actualizamos la geometría
	if( autoSize )
		adjustSize();

	update();
}

QString LELabel::text() const 
{
	return (const QString)this->label;
}

// Formato del texto
void LELabel::setFont( const QFont &font ) 
{
	invalidate();

	this->fnt = font;

	// Actualizamos la geometría
	if( autoSize )
		adjustSize();

	update();
}

void LELabel::setFontColor( const QColor &color ) 
{
	this->fntColor = color;
}

QFont LELabel::font() const
{
	return (const QFont) this->fnt;
}

QColor LELabel::fontColor() const
{
	return (const QColor) this->fntColor;
}

// Caja envolvente
void LELabel::setBackgroundColor( const QColor & color )
{
	setBrush( QBrush( color ) );
	drawBox = true;
}

void LELabel::setLineColor( const QColor & color ) 
{
	QPen aux = pen();
	aux.setColor( color );
	setPen( aux );
	drawBox = true;
}

void LELabel::setLineWidth( int width )
{
	QPen aux = pen();
	aux.setWidth( width );
	setPen( aux );
	drawBox = true;
}

QColor LELabel::backgroundColor() const
{
	return (const QColor)brush().color();
}

QColor LELabel::lineColor() const
{
	return (const QColor)pen().color();
}

int LELabel::lineWidth() const
{
	return pen().width();
}

// Geometría externa
int LELabel::minWidth() const
{
	int retval;

	QFontMetrics fMetrics(fnt);
	switch( direction() ){
	case TextHorizontal:
		retval = fMetrics.boundingRect( label ).width();
		break;
	case TextVertical:
		retval = fMetrics.boundingRect( label ).height();
	}
	
	return retval;
}

int LELabel::minHeight() const
{
	int retval;

	QFontMetrics fMetrics(fnt);
	switch( direction() ){
	case TextHorizontal:
		retval = fMetrics.boundingRect( label ).height();
		break;
	case TextVertical:
		retval = fMetrics.boundingRect( label ).width();
	}
	
	return retval;
}

void LELabel::setSize( int w, int h )
{
	// Corrección de geometría (tamaño mínimo)
	if( w < minWidth() ) w=width();
	if( h < minHeight() ) h=height();

	invalidate();

	LEItem::setSize( w, h );
	autoSize = false;

	update();
}

QPointArray LELabel::areaPoints() const
{
	QPointArray rv;

	int vx = x();
	int vy = y();
	rv.putPoints( 0, 5, vx-1,vy-1, vx+width()+1,vy-1, vx+width()+1,vy+height()+1, vx-1,vy+height()+1, vx-1,vy-1 );
	return rv;
}

QRect LELabel::boundingRect() const
{
	return QRect( x()-1, y()-1, width()+2, height()+2 );
}

// Orientación del texto (Vertical || Horizontal)
void LELabel::setDirection( TextDirection d ) 
{
	invalidate();

	this->txDirection = d;
	
	// Actualizamos la geometría
	if( autoSize )
		adjustSize();

	update();
}

LELabel::TextDirection LELabel::direction()  const
{
	return this->txDirection;
}

// Ajuste de geometría (ciñe el tamaño de la etiqueta al texto)
void LELabel::adjustSize()
{
	int w, h;
	QFontMetrics fMetrics( fnt );

	switch( txDirection )
	{
	case TextHorizontal:
		w = 2*leftMargin+fMetrics.boundingRect( label ).width();
		h = 2*topMargin+fMetrics.boundingRect( label ).height();
	break;
	case TextVertical:
		w = 2*topMargin+fMetrics.boundingRect( label ).height();
		h = 2*leftMargin+fMetrics.boundingRect( label ).width();
	break;
	default:
		w = width();
		h = height();
	}

	LEItem::setSize( w, h );
}

void LELabel::drawShape( QPainter& p )
{
	int hSpace, vSpace;
	p.save();

	switch( txDirection )
	{
	case TextHorizontal:
		p.translate( x(), y() );
		hSpace = width();
		vSpace = height();
		break;
	case TextVertical:
		p.translate( x()+width(), y() );
		p.rotate( 90.0 );
		hSpace = height();
		vSpace = width();
		break;
	}

	if( drawBox )
	{
		p.fillRect( 0, 0, hSpace, vSpace, brush() );
		p.setPen( pen() );
		p.drawRect( 0, 0, hSpace, vSpace );
	}
	p.setPen( QPen( fntColor ) );
	p.setFont( fnt );
	p.drawText( leftMargin, topMargin, hSpace-leftMargin, vSpace-topMargin, QObject::AlignCenter, label );

	p.restore();

}
