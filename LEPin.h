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


// LEPin.h: interface for the LEPin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LEPIN_H_)
#define _LEPIN_H_

#include "LEItem.h"

#include <qpointarray.h>
#include "LEConnectionPoint.h"

class LEDevice;
class LELabel;

class LEPin : public LEItem
{
	Q_OBJECT
	Q_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
	Q_PROPERTY( AccessMode accessMode READ accessMode WRITE setAccessMode )
	Q_PROPERTY( double position READ position WRITE setPosition )
	Q_PROPERTY( QString text READ text WRITE setText )
	Q_PROPERTY( Level activeLevel READ activeLevel WRITE setActiveLevel )
	Q_ENUMS( Alignment )
	Q_ENUMS( AccessMode )
	Q_ENUMS( Level )

public:
	enum { PinLenght = 20, hzMargin=1, vrMargin=1, FontSize=9, RTTI=LErttiPin };

	enum Alignment { Left, Right, Top, Bottom };
	enum AccessMode { Input, Output, InputOutput};
	enum Level { LowLevel, HiLevel };
	
	LEPin( QCanvas * canvas=0, LEItem * parentItem=0 );
	LEPin( Alignment align, AccessMode access, const QString &label = QString::null, double pos= -1.0, Level l=HiLevel, QCanvas * canvas=0, LEItem * parentItem=0 );
	virtual ~LEPin();

	virtual int rtti() const;
	virtual QString resolvName( char separator = '_' ) const;

	void show();
	void hide();

	// Propiedades
	void setText( QString &lb );
	void setAccessMode( AccessMode am );
	void setAlignment( Alignment al );
	void setPosition( double pos );
	void setActiveLevel( Level l );
	Level activeLevel() const;
	
	QString text() const;
	AccessMode accessMode() const;
	Alignment alignment() const;
	double position() const;

	// Miembros heredados: Polygonal Item
	virtual void move( double x, double y );
	virtual void moveBy( double x, double y );
	virtual QPointArray areaPoints() const;
	virtual QRect boundingRect() const;

	// Punto de conexión
	LEConnectionPoint * connectionPoint();

protected:
	virtual void drawShape( QPainter& p );
	virtual void adjustSize();

private:
	LELabel * label;

	AccessMode access;
	Alignment align;
	Level actLevel;
	double pos;

	LEConnectionPoint * lpCnnct;
};

#endif
