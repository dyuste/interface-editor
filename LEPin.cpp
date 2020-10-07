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



#include "LEPin.h"
#include "LELabel.h"
#include "LEDevice.h"

#include <qpainter.h>
#include "LEWireLine.h"

#define CIRCLE_RADIO 4

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

LEPin::LEPin( QCanvas * canvas, LEItem * parentItem )
	: LEItem( canvas, parentItem )
{
	this->pos = -1.0;
	this->align = Left;
	this->access = Input;
	this->label = new LELabel( canvas, this );
	this->actLevel = HiLevel;

	// Punto de Conexión
	lpCnnct = new LEConnectionPoint( canvas, this );

	// Geometría inicial
	LEItem::setSize( PinLenght, 1 );

}

LEPin::LEPin( Alignment align, AccessMode access, const QString &label, double pos, Level l, QCanvas * canvas, LEItem * parentItem )
	: LEItem( canvas, parentItem )
{ 
	this->pos = ((pos>1.0)?1.0:pos); 
	this->align = align; 
	this->access = access; 
	this->label = new LELabel( label, canvas, this );
	this->actLevel = HiLevel;
	this->actLevel = l;

	lpCnnct = new LEConnectionPoint( canvas, this );
	
	adjustSize();
}

LEPin::~LEPin()
{
	LEItem::hide();
}

int LEPin::rtti() const
{
	return RTTI;
}

void LEPin::show()
{
	LEItem::show();
	if( label != NULL )
		label->show();

	lpCnnct->show();
}

void LEPin::hide()
{
	LEItem::hide();
	if( label != NULL )
		label->hide();

	lpCnnct->hide();
}

QString LEPin::resolvName( char separator ) const
{
	return QString("%1%2%3").arg(parent()->name()).arg(separator).arg(name());
}


//////////////////////////////////////////////////////////////////////
// Conectividad


LEConnectionPoint * LEPin::connectionPoint(){
	return lpCnnct;
}

//////////////////////////////////////////////////////////////////////
// Parámetros del pin
void LEPin::setText( QString &lb )
{
	label->setText( lb );
	
	adjustSize();
}
	
void LEPin::setAccessMode( AccessMode access )
{
	this->access = access;
}

void LEPin::setAlignment( Alignment align )
{
	this->align = align;
	
	// Actualiza las dimensiones para que el dispositivo
	// padre pueda ubicarlo correctamente
	adjustSize();

	// Pedimos al LEDevice padre que nos reubique
	if( parent() )
		if( parent()->rtti() == LEDevice::RTTI )
			((LEDevice*)parent())->placePin( this );
}

void LEPin::setPosition( double pos )
{
	this->pos = ((pos>1.0)?1.0:pos);
	
	// Pedimos al LEDevice padre que nos reubique
	if( parent() )
		if( parent()->rtti() == LEDevice::RTTI )
			((LEDevice*)parent())->placePin( this );
}

void LEPin::setActiveLevel( Level l )
{
	actLevel = l;
}

QString LEPin::text() const
{
	return label->text();
}

LEPin::AccessMode LEPin::accessMode() const
{
	return access;
}

LEPin::Alignment LEPin::alignment() const
{
	return align;
}

double LEPin::position() const
{
	return pos;
}

LEPin::Level LEPin::activeLevel() const
{
	return actLevel;
}

//////////////////////////////////////////////////////////////////////
//	Geometría
void LEPin::move( double x, double y )
{
	invalidate();
	
	LEItem::move( x, y );
	adjustSize();
	
	update();

	// Actualizamos el cable de conexión
	switch( align ){
	case Left:
		lpCnnct->move( this->x()-1, this->y() );
		break;
	case Top:
		lpCnnct->move( this->x(), this->y()+1 );
		break;
	case Right:
		lpCnnct->move( this->x() + width()+1, this->y() );
		break;
	case Bottom:
		lpCnnct->move( this->x(), this->y() + height()+1 );
	}
		
}

void LEPin::moveBy( double x, double y )
{
	invalidate();

	LEItem::moveBy( x, y );
	label->moveBy( x, y );

	update();

	// Actualizamos el cable de conexión
	switch( align ){
	case Left:
		lpCnnct->move( this->x()-1, this->y() );
		break;
	case Top:
		lpCnnct->move( this->x(), this->y()+1 );
		break;
	case Right:
		lpCnnct->move( this->x() + width()+1, this->y() );
		break;
	case Bottom:
		lpCnnct->move( this->x(), this->y() + height()+1 );
	}
}

void LEPin::adjustSize() 
{
	int w, h;

	// Ajustamos el tamaño a la etiqueta
	switch( align )
	{
	case Left:
		w=PinLenght;
		h=1;
		label->setDirection( LELabel::TextHorizontal );
		label->move( x() - label->width() + w/2, y()-label->height() - vrMargin );
		break;
	case Right:
		w=PinLenght;
		h=1;
		label->setDirection( LELabel::TextHorizontal );
		label->move( x() + w/2, y() + vrMargin );
		break;
	case Top:
		w=1;
		h=PinLenght;
		label->setDirection( LELabel::TextVertical );
		label->move( x() + hzMargin, y() - label->height() + h/2 );
		break;
	case Bottom:
		w=1;
		h=PinLenght;
		label->setDirection( LELabel::TextVertical );
		label->move( x() - label->width() - hzMargin, y() + h/2 );
	}

	LEItem::setSize( w, h );
}

QPointArray LEPin::areaPoints() const
{
	QPointArray rv;
	int vx = x();
	int vy = y();
	int vw = width();
	int vh = height();

	// Tenemos en cuenta si es precios dibujar la "bola" de nivel bajo
	if( activeLevel() == LowLevel ){
		switch( align )
		{
		case Left:
		case Right:
			vy -= CIRCLE_RADIO;
			vh += CIRCLE_RADIO;	
			break;
		case Top:
		case Bottom:
			vx -= CIRCLE_RADIO;
			vw += CIRCLE_RADIO;
			break;
		}
	}

	// Formamos el polígono a devolver
	rv.putPoints( 0, 5, vx-1,vy-1, vx+vw+1,vy-1, vx+vw+1,vy+vh+1, vx-1,vy+vh+1, vx-1,vy-1 );

	return rv;
}

QRect LEPin::boundingRect() const
{
	int vx = x();
	int vy = y();
	int vw = width();
	int vh = height();

	// Tenemos en cuenta si es precios dibujar la "bola" de nivel bajo
	if( activeLevel() == LowLevel ){
		switch( align )
		{
		case Left:
		case Right:
			vy -= CIRCLE_RADIO;
			vh += CIRCLE_RADIO;	
			break;
		case Top:
		case Bottom:
			vx -= CIRCLE_RADIO;
			vw += CIRCLE_RADIO;
			break;
		}
	}

	// Formamos el rectángulo a devolver
	return QRect( vx-1, vy-1, vw+2, vh+2 );
}

//////////////////////////////////////////////////////////////////////
//	Dibujado
void LEPin::drawShape( QPainter& p )
{
	int left, right, top, bottom, rx, ry;
	left = x();
	top = y();
	switch( align )
	{
	case Left:
		bottom = y();
		if( activeLevel() == LowLevel ){
			right = x()+width()-2*CIRCLE_RADIO;
			rx = x()+width()-CIRCLE_RADIO;
			ry = y();
		}else
			right = x()+width();
		break;

	case Right:
		right = x()+width();
		bottom = y();
		if( activeLevel() == LowLevel ){
			left = x()+2*CIRCLE_RADIO;
			rx = x()+CIRCLE_RADIO;
			ry = y();
		}
		break;

	case Top:
		right = x();
		bottom = y()+height();
		if( activeLevel() == LowLevel ){
			bottom = y()+height()-2*CIRCLE_RADIO;
			rx = x();
			ry = y()+height()-CIRCLE_RADIO;
		}else
			bottom = y()+height();

		break;

	case Bottom:
		right = x();
		bottom = y()+height();
		if( activeLevel() == LowLevel ){
			top = y()+2*CIRCLE_RADIO;
			rx = x()+CIRCLE_RADIO;
			ry = y();
		}
	}

	p.drawLine( left, top, right, bottom );
	if( activeLevel() == LowLevel )
		p.drawArc( rx-CIRCLE_RADIO, ry-CIRCLE_RADIO, 2*CIRCLE_RADIO, 2*CIRCLE_RADIO, 0, 360*16 );


}

