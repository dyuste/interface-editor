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



#if !defined(_LECONNECTIONPOINT_H_)
#define _LECONNECTIONPOINT_H_

#include "LEItem.h"


////////////////////////////////////////////////////////////////////////////////
//	LEConnectionPoint
//
//	Implementa un punto de conexión de Items.
//
//	Un punto de conexión establece la conexión entre n Items (n>=2)
//	siendo esta conexión asimétrica.
//
//	LEConnectionPoint guarda referencias hacia los LEItem's conectados
//	e invoca sus manejadores connectionMoved(), connectionRemoved() (...)
//	cuando es preciso.
//
////////////////////////////////////////////////////////////////////////////////
typedef QPtrList<LEItem> ConnectionList;

class LEConnectionPoint : public LEItem
{
public:
	enum { RTTI = LErttiConnectionPoint, MinWidth = 7, MinHeight=7, borderColor=0x9095DE };

	LEConnectionPoint(QCanvas * canvas=0, LEItem * parentItem=0 );
	LEConnectionPoint(int maxConnections, QCanvas * canvas=0, LEItem * parentItem=0 );

	virtual ~LEConnectionPoint();
	virtual void remove();

	virtual int rtti() const;

	////////////////////////////////////////////////////////////////////////////////
	// Geometría
	////////////////////////////////////////////////////////////////////////////////
	virtual QPointArray areaPoints() const;

	////////////////////////////////////////////////////////////////////////////////
	// Funcionalidad
	////////////////////////////////////////////////////////////////////////////////
	int maxConnections();
	void setMaxConnections( int max );

	ConnectionList& connectionList();
	bool setConnection( LEItem * item );
	void removeConnection( LEItem * item );
	void connectionRemoved( LEConnectionPoint * connection );

	
	////////////////////////////////////////////////////////////////////////////////
	// Movimiento: usa el estandar de LEItem
	////////////////////////////////////////////////////////////////////////////////
	void move( int x, int y );
	void moveBy( int x, int y );

	////////////////////////////////////////////////////////////////////////////////
	// Aspecto
	////////////////////////////////////////////////////////////////////////////////
	void setDrawSquare( bool val );
	bool drawSquare() const;

protected:
	void drawShape( QPainter &painter );

private:
	int maxCnct;
	bool drwSqr;
	ConnectionList connections;
	
};

#endif
