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


// LEWireLine.h: interface for the LEWireLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LEWIRELINE_H_)
#define _LEWIRELINE_H_

#include "LEItem.h"

class LEConnetionPoint;

class LEWireLine : public LEItem  
{
public:
	enum { RTTI = LErttiWireLine };

	LEWireLine( QCanvas * canvas, LEItem * parentItem=0 );
	virtual ~LEWireLine();

	virtual void remove();

	virtual int rtti() const;

	// Devuelve el area y rectangulos envolventes (~bufferPolygon)
	virtual QPointArray areaPoints() const;
	virtual QRect boundingRect() const;

	// Genera un buffer envolvente para el conjunto de lineas
	virtual QPointArray bufferPolygon( double d ) const;

	// Devuelve el índice del vértice más próximo al punto p o -1 si no existe ninguna aproximación viable
	int nearestVertex( const QPoint &p ) const;

	QPoint vertex( int i ) const;
	int vertexCount() const;

	// Edición de vértices
	void normalize();
	void setVertexs( QPointArray & vertexs );
	void moveVertex( int i, int x, int y );
	void moveVertex( int i, QPoint p );
	void insertVertex( int x, int y, int i=-1 );
	void insertVertex( const QPoint &p, int i=-1 );
	void removeVertex( int i );

	// Conectividad:
	void connectLeft( LEConnectionPoint * lpCnnct );
	void connectRight( LEConnectionPoint * lpCnnct );
	LEConnectionPoint * leftConnection();
	LEConnectionPoint * rightConnection();
	bool isLeftConnected() const;
	bool isRightConnected() const;

	virtual void connectionMoved( LEConnectionPoint * lpCnnct );
	virtual void connectionRemoved( LEConnectionPoint * lpCnnct );

	// Dibujado
	virtual void drawShape( QPainter & p );

private:
	QPointArray vertexList;
	LEConnectionPoint *lpLeftCnnct, *lpRightCnnct;

};

#endif
