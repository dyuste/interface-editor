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



#if !defined(_LEDEVICE_H_)
#define _LEDEVICE_H_

#include "LEItem.h"
#include "LEPin.h"

#include <qpointarray.h>
#include <qptrlist.h>

#include "LMComponent.h"
#include "LErtti.h"

#define PIN_MARGIN_H 5
#define PIN_MARGIN_V 5

class LELabel;

class LEDevice : public LEItem
{
	Q_OBJECT
	Q_PROPERTY(bool externSolving READ isExternSolving )
	Q_PROPERTY(bool transposed READ transposed WRITE setTransposed )
	Q_PROPERTY(HDirection HorizontalDirection READ hDirection WRITE setHDirection )
	Q_PROPERTY(VDirection VerticalDirection READ vDirection WRITE setVDirection )
	Q_ENUMS(HDirection)
	Q_ENUMS(VDirection)

public:
	friend class LEPin;

	enum { RTTI=LErttiDevice, margin=5 };
	
	enum HDirection { RightToLeft, LeftToRight };
	enum VDirection { TopToBottom, BottomToTop };

	LEDevice( QCanvas * canvas, LEItem * parentItem=0 );
	virtual ~LEDevice();

	virtual void setName( const char * name );

	void setComponentReference( const LMComponent * cmp );
	const LMComponent * componentReference() const;
	bool isExternSolving() const;

	////////////////////////////////////////////////////////////////
	//	Comportamiento
	////////////////////////////////////////////////////////////////

	// Identificador  del tipo de objeto
	virtual int rtti() const;
	
	// Ocultaci�n y aparici�n (con aplicaci�n en cascada a subobjetos)
	void show();
	void hide();

	////////////////////////////////////////////////////////////////
	//	Geometr�a
	////////////////////////////////////////////////////////////////
	
	//	Tama�o del objeto
	virtual void setSize ( int w, int h );
	
	// Forma
	virtual QPointArray areaPoints() const;
	virtual QRect boundingRect() const;
	
	// Devuelve un buffer del pol�gono escalado con distancia 'd' (esto es una pol�gono que envuelve al escalado)
	QPointArray bufferPolygon( double d ) const;
	
	// Orientaci�n (Efecto Mirror)
	void setHDirection( const HDirection & direction );
	HDirection hDirection() const;
	void setVDirection( const VDirection & direction );
	VDirection vDirection() const;
	void setTransposed( bool t );
	bool transposed() const;

	// Posici�n del objeto
	void move( double x, double y );
	virtual void moveBy( double x, double y );

	// Establece la forma primaria del objeto
	void setShape( const QPointArray & shape );

	// Devuelve la forma primaria del objeto
	QPointArray & getPrimaryShape();

	// Devuelve la versi�n redimensionada de la forma del objeto
	QPointArray & getScaledShape();

	////////////////////////////////////////////////////////////////
	//	Manipulaci�n de pins
	////////////////////////////////////////////////////////////////
	QPtrList<LEPin> &pinList();
	LEPin * insertPin( LEPin::Alignment align, LEPin::AccessMode access, QString label=QString::null, double position=-1.0, LEPin::Level l = LEPin::HiLevel );
	void removePin( LEPin * pin );

	////////////////////////////////////////////////////////////////
	//	Etiqueta
	////////////////////////////////////////////////////////////////	
	void setLabel( const QString & text );
	void showLabel();
	void hideLabel();
	
protected:

	////////////////////////////////////////////////////////////////
	//	C�lculos geom�tricos
	////////////////////////////////////////////////////////////////

	// Determina la proyecci�n de una recta sobre la figura scaledShape seg�n la orLEntaci�n dada
	bool getLeftProjection	( int* x, int  y );
	bool getRightProjection	( int* x, int  y );
	bool getTopProjection	( int  x, int* y );
	bool getBottomProjection( int  x, int* y );

	// Devuelve una copia de 'shape' a la que se le ha aplicado el efecto espejo
	QPointArray horizontalMirror( const QPointArray & shape ) const;
	QPointArray verticalMirror( const QPointArray & shape ) const;

	// Devuelve una copia de 'shape' a la que se ha aplicado una transposici�n de las coordenadas x-y
	QPointArray transposePolygon( const QPointArray & shape ) const;

	// Calcula la ubicaci�n del pin 'pin' seg�n su orLEntaci�n y posici�n relativa y lo emplaza en ella
	void placePin( LEPin * pin );

	// Dibujado de la forma
	virtual void drawShape( QPainter& p );

	// Pins de conexi�n
	QPtrList<LEPin> lsPin;	

private:
	// Forma principal
	QPointArray mainShape;
	QPointArray scaledShape;

	// Direcci�n
	HDirection hDir;
	VDirection vDir;
	bool transpos;

	// Etiqueta
	LELabel * label;

	// Referencia de Componente
	const LMComponent * cmp;
};

#endif
