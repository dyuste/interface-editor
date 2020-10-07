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



#if !defined(_LEITEM_H_)
#define _LEITEM_H_


#include <qcanvas.h>
#include <qobject.h>
#include <qptrlist.h>

#include "LErtti.h"

#define ITEM_NAME_SEPARATOR '.'

class LEConnectionPoint;
class LEItem;

typedef QPtrList<LEItem> LEItemList;

/////////////////////////////////////////////////////////////////////////////////////
//	LEItem
//
// Clase abstracta pura base para todo objeto de InterfaceEditor.
// Establece:
//		* Manipulación geométrica básica (tamaño y posición [por QCanvasItem])
//		* Relación de parentesco entre objetos
//		* Conectividad básica
//
class LEItem : public QObject, public QCanvasPolygonalItem
{
	Q_OBJECT
	Q_PROPERTY( bool resizable READ isResizable WRITE setResizable )

public:
	enum { RTTI = LErttiItem };

//////////////////////////////////////////////////////////////////////
// Instanciación
//////////////////////////////////////////////////////////////////////

	LEItem( QCanvas * canvas, LEItem * parentItem=0 );
	virtual ~LEItem();
	virtual void setName( const char * name );

	// Lás subclases que requieran procesos especiales al destruirse
	// deben indicarlo aquí
	virtual void remove();
	virtual int rtti() const;


	
//////////////////////////////////////////////////////////////////////
// Características de objeto
//////////////////////////////////////////////////////////////////////
	virtual bool setProperty( const char * name, const QVariant & value );

//////////////////////////////////////////////////////////////////////
// Aspecto
//////////////////////////////////////////////////////////////////////
	virtual void show();
	virtual void hide();
	
	// Métodos a implementar por todas las clases derivadas
	virtual void setResizable( bool rs );
	virtual bool isResizable() const;

	virtual void setSize( int w, int h );
	virtual int width() const;
	virtual int height() const;

//////////////////////////////////////////////////////////////////////
// Parentesco
//////////////////////////////////////////////////////////////////////
	virtual void setParent( LEItem * item );
	virtual LEItem * parent();
	virtual const LEItem * parent() const;

	virtual void addChild( LEItem * item );
	virtual void removeChild( LEItem * item );
	virtual LEItemList & childs();

	virtual QString resolvName( char separator=ITEM_NAME_SEPARATOR ) const;

//////////////////////////////////////////////////////////////////////
// Conectividad
//////////////////////////////////////////////////////////////////////
	
	// A reimplementar si la subclase usa puntos de conexión
	virtual void connectionMoved( LEConnectionPoint * connection );
	virtual void connectionRemoved( LEConnectionPoint * connection );

private:
	LEItem * parentItem;
	LEItemList childList;
	bool resizable;
	int w, h;
};

#endif
