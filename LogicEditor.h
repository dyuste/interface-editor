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


// LogicEditor.h: interface for the LogicEditor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGICEDITOR_H_)
#define _LOGICEDITOR_H_

#include <qcanvas.h>

#include "LEHandle.h"
#include "LEDevice.h"
#include "LEWireLine.h"
#include "HDLGenerator.h"

class LMComponent;
class LEItem;
class LELabel;
class LEConnectionPoint;
class QDomElement;
class QAction;

#include <qdict.h>
typedef QDict<LEDevice> DeviceMap;
typedef QDictIterator<LEDevice> DeviceMapIterator;
typedef QDict<LEWireLine> WireLineMap;
typedef QDictIterator<LEWireLine> WireLineMapIterator;

class LogicEditor : public QCanvasView
{
Q_OBJECT

public:
	virtual ~LogicEditor();
	LogicEditor( QCanvas *canvas, QWidget *parent=0, const char *name=0 );
	LogicEditor( QWidget *parent=0, const char *name=0 );
	void setCanvas( QCanvas * canvas );

//////////////////////////////////////////////////////////////////////
// Manipulación de instancias de objetos
//////////////////////////////////////////////////////////////////////
	LEWireLine * createWireLine( bool usingIGU = true );
	LEWireLine * createWireLine( LEConnectionPoint *cp1, LEConnectionPoint *cp2, float breakPoint=0.5 );
	LEDevice * createDevice( const LMComponent * cmp, bool usingIGU = true );
	LELabel * createLabel( const QString & label = QString::null );

	LEItem * findItem( const QString & itemName, bool mustSolve = false );	
	
	//////////////////////////////////////////////////////////////////////
	// Gestión de dispositivos duplicados
	//////////////////////////////////////////////////////////////////////
	
	// Duplicación de dispositivos
	LEDevice * duplicateDevice( LEDevice * dev, bool usingIGU = true );
 
	// Iterador de nombres de dispositivos duplicados
	QString firstDupName( const QString &patternName ) const;
	QString nextDupName( const QString &patternName ) const;
	
 // static QString normalizeDupName( const QString & name );
 // static int extractDupNameIndex( const QString &name );
	QString findFreeDupName( const QString & patternName );

	//////////////////////////////////////////////////////////////////////
	// Gestión de nombres
	//////////////////////////////////////////////////////////////////////

	// Intenta cambiar el nombre del item itemName por newItemName.
	// Si newItemName ya existe devuelve false
	bool tryUpdateItemName( const QString & itemName, const QString & newItemName ); 
	
	// Registra el item 'item' con el nombre itemName.
	// Si itemName ya existe devuelve false
	bool registerItem( LEItem * item, const QString& itemName );

	void setActiveItem( LEItem * item );
	LEItem * activeItem();

//////////////////////////////////////////////////////////////////////
// Load y Store de modelos
//////////////////////////////////////////////////////////////////////
	virtual bool load( QIODevice * in );
	virtual bool save( QIODevice * out );
	virtual bool parseDevice( QDomElement & element );
	virtual bool parseWireLine( QDomElement & element );
	static QPoint parsePoint( QString string );
	static QPointArray parsePointArray( QString string );


public slots:
//////////////////////////////////////////////////////////////////////
// Acciones y menú emergente
//////////////////////////////////////////////////////////////////////
	void onActionDelete();
	void onActionDuplicate();
	void zoomIn();
	void zoomOut();
	void zoomFixed( double f );

signals:
	void changed();
	void itemSelected( QObject * item );
	void connected( LEItem* item, LEConnectionPoint *cp );
	void disconnected( LEItem*item, LEConnectionPoint *cp );
	void pendingItemCanceled( LEItem *item, bool destroyed );
	void pendingItemPlaced( LEItem *item );

protected:

//////////////////////////////////////////////////////////////////////
// Control de eventos
//////////////////////////////////////////////////////////////////////
	void contentsMousePressEvent( QMouseEvent *event );
	void contentsMouseMoveEvent( QMouseEvent *event );

	// Eventos para Items
	void mouseOverEvent( LEItem * item, const QPoint &pos );
	void mouseOutEvent( LEItem * item, const QPoint &pos );
	
//////////////////////////////////////////////////////////////////////
// Acciones y menú emergente
//////////////////////////////////////////////////////////////////////
	virtual void contextMenuEvent(QContextMenuEvent *event );

private:
	void createActions();

	QAction *actDelete, *actDuplicate;

//////////////////////////////////////////////////////////////////////
// Control de eventos
//////////////////////////////////////////////////////////////////////

	// Lista de objetos que han recibido un mouseOverEvent
	// y todavía no han sido notificados del mouseOutEvent
	QCanvasItemList lastMouseOverItems;
	
//////////////////////////////////////////////////////////////////////
// Creación de Objetos
//////////////////////////////////////////////////////////////////////
	virtual void pendingItemCancel( LEItem * item );
	virtual void pendingItemPreview( LEItem * item, const QPoint &pos );
	virtual LEItem * pendingItemPlace( LEItem * item, const QPoint &pos );
	
//////////////////////////////////////////////////////////////////////
// Manipulación de objetos
//////////////////////////////////////////////////////////////////////
	void trySelectItem( const QPoint& pos );
	QPoint resizeItem( LEItem * item, QPoint source, QPoint target, LEHandle::HandleAlignment rzSide );
	void moveItem( LEItem * item, QPoint source, QPoint target );
	void moveWireLineVertex( LEWireLine * wlItem, QPoint target );
	
	// Elimina toda referencia al elemento item y lo marca para ser destruido
	void purgeItem( LEItem *item );

//////////////////////////////////////////////////////////////////////
// Variables de estado
//////////////////////////////////////////////////////////////////////
	// Objeto activo (selección actual)
	LEItem * actItem;
	// Objeto pendiente de ser instanciado (creado)
	LEItem * pendingItem;
	// Vértice de WireLine activo (usado para el "redibujado" de lineas)
	int vertexActive;
	// Última posición efectiva (desplazamientos/redimensionados relativos)
	QPoint lastPos;
	// Último connectionPoint modificado (tras mouseMoveEvent)
	LEConnectionPoint * lastCnnctPoint;

// Agarradores (selección y redimensionado)
	// Agarrador activo
	LEHandle * hndlActive;
	// Agarradores del objeto seleccionado
	LEHandle *hndlLeftTop, *hndlLeftBottom, *hndlRightTop, *hndlRightBottom;		

// Mapas de Objetos
	DeviceMap deviceNames;
	WireLineMap wireLineNames;

//////////////////////////////////////////////////////////////////////
// Visualización
//////////////////////////////////////////////////////////////////////
	double zoomFactor;
	void applyZoom();


public:
//////////////////////////////////////////////////////////////////////
// Métodos estáticos
//////////////////////////////////////////////////////////////////////

	// Gestión de nombres de dispositivos duplicados
	// Normaliza un nombre de duplicado eliminando el índice: name(i)->name
	static QString normalizeDupName( const QString & name )
	{
		QString retval;
		int i;
		for( i=0; i<name.length(); i++ )
			if( name[i] == '(' ){
				i--;
				break;
			}

		if( i >= 0 )
			retval = name.left( i+1 );

		return retval;
	}

	static int extractDupNameIndex( const QString &name )
	{
		int start=0, end=0;
		
		// Delimita el campo numérico y chequea la sintaxis:
		// un único carácter '('.
		// un único carácter ')' siempre después de ')'
		// entre '(' y ')' sólo carácteres numéricos
		for( int i=0; i<name.length(); i++ )
			if( name[i] == '(' ){
				if( start )
					return -1;
				else
					start = i+1;
			}else if( name[i] == ')' ){
				if( end || !start )
					return -1;
				else
					end = i-1;
			}else if( start && !end ){
				if( name[i] < '0' || name[i] > '9' )
					return -1;
			}

		if( !start || !end )
			return -1;

		// Extracción del índice
		QString section = name.mid( start, end-start+1 );
		return section.toInt();
	}

};

#endif 
