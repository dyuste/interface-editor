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



#if !defined(_LEHANDLE_H_)
#define _LEHANDLE_H_

#include "LEItem.h"

#include "LErtti.h"


class LEHandle : public LEItem
{
public:
	enum {defaultSize=5, RTTI=LErttiHandle};
	enum HandleAlignment {Left=0x01, Right=0x02, Top=0x04, Bottom=0x08, LeftTop=0x05, LeftBottom=0x09, RightTop=0x06, RightBottom=0x0A };

	// Instanciación
	LEHandle( HandleAlignment hAlign, QCanvas * QCanvas, LEItem * parent=0 );
	LEHandle( QCanvas * canvas, LEItem * parent=0 );
	virtual ~LEHandle();

	virtual int rtti() const;
	
	// Geometría
	QPointArray areaPoints() const;
	QRect boundingRect() const;

	// Atributos
	HandleAlignment alignment() const;
	void setAlignment( HandleAlignment hAlign );

protected:
	// Dibujado
	virtual void drawShape( QPainter &p );

private:
	HandleAlignment align;
};

#endif 
