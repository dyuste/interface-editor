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



#include <qapplication.h>
#include <qregexp.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qwmatrix.h>

#include "LogicEditor.h"

#include "LEDevice.h"
#include "LELabel.h"
#include "LEWireLine.h"
#include "LEPin.h"

#include "LMComponent.h"
#include "LMLibrary.h"

#include "Application.h"
extern Application * app;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
LogicEditor::~LogicEditor()
{
	if( canvas() )
		delete canvas();
}

LogicEditor::LogicEditor( QCanvas *canvas, QWidget *parent, const char *name )
	: QCanvasView( parent, name )
{
	setCanvas( canvas );

	// Valores no inicializados
	pendingItem = NULL;
	lastCnnctPoint = NULL;
	vertexActive = -1;

	// Inicialmente no hay ningún objeto seleccionado
	setActiveItem( NULL );

	// Habilita la caputra de eventos de movimiento de ratón
	this->viewport()->setMouseTracking( true );
	QApplication::setGlobalMouseTracking( true );

	// Zoom inicial
	zoomFactor = 1.0;

	// Crea las acciones para la interacción con el usuario mediante menús
	createActions();

}

LogicEditor::LogicEditor( QWidget *parent, const char *name )
	: QCanvasView( parent, name )
{
	// Valores no inicializados
	pendingItem = NULL;
	lastCnnctPoint = NULL;
	vertexActive = -1;
	hndlActive = NULL;
	actItem = NULL;
	
	// Habilita la caputra de eventos de movimiento de ratón
	this->viewport()->setMouseTracking( true );	
	QApplication::setGlobalMouseTracking( true );
	
	// Zoom inicial
	zoomFactor = 1.0;
	
	// Crea las acciones para la interacción con el usuario mediante menús
	createActions();
}

void LogicEditor::setCanvas( QCanvas * canvas )
{
	QCanvasView::setCanvas( canvas );

	// Creación de los agarradores
	hndlLeftTop = new LEHandle( LEHandle::LeftTop, canvas, 0 );
	hndlRightTop = new LEHandle( LEHandle::RightTop, canvas, 0 );
	hndlLeftBottom = new LEHandle( LEHandle::LeftBottom, canvas, 0 );
	hndlRightBottom = new LEHandle( LEHandle::RightBottom, canvas, 0 );
	hndlActive=NULL;
}


void LogicEditor::zoomIn()
{
	zoomFactor *= 1.25;
	applyZoom();
}

void LogicEditor::zoomOut()
{
	zoomFactor *= 0.8;
	applyZoom();
}

void LogicEditor::zoomFixed( double f )
{
	zoomFactor = f;
	applyZoom();
}

void LogicEditor::applyZoom()
{
	QWMatrix matrix;
	matrix.scale( zoomFactor, zoomFactor );
	
	setWorldMatrix( matrix );
}

//////////////////////////////////////////////////////////////////////
// SAVE y STORE de documentos
//////////////////////////////////////////////////////////////////////

bool LogicEditor::save( QIODevice * device )
{
	if( !device )
		return false;

	QTextStream out( device );
	out << "<model name=\"" << name() << "\">\n\n";
	
	// Instancias de dispositivos
	DeviceMapIterator devIt(deviceNames);
	for( ;devIt.current(); ++devIt )
		out << "\t<device name=\"" << devIt.currentKey() << "\" "
			<< "template=\"" << devIt.current()->componentReference()->name() << "\" "
			<< "library=\"" << devIt.current()->componentReference()->parentLibrary()->name() << "\" "
			<< "offset=\"" << devIt.current()->x()  << "x" << devIt.current()->y() << "\" "
			<< "size=\"" << devIt.current()->width() << "x" << devIt.current()->height() << "\""
			<< "></device>\n";
	out << "\n";

	// Instancias de cables
	WireLineMapIterator wlIt(wireLineNames);
	for( ; wlIt.current(); ++wlIt ){
		QString cnnctName;

		out << "\t<wireline name=\"" << wlIt.currentKey() << "\" ";
		
		// Conexión izquierda		
		if( wlIt.current()->leftConnection() && wlIt.current()->leftConnection()->parent() )
			cnnctName = wlIt.current()->leftConnection()->parent()->resolvName();
		else
			cnnctName = "null";
		out << "leftConnection=\"" << cnnctName << "\" ";

		// Conexión derecha
		if( wlIt.current()->rightConnection() && wlIt.current()->rightConnection()->parent() )
			cnnctName = wlIt.current()->rightConnection()->parent()->resolvName();
		else
			cnnctName = "null";
		out << "rightConnection=\"" << cnnctName << "\"";
		
		// Geometría
		out << " points=\"";
		for( int i=0; i < wlIt.current()->vertexCount(); i++ )
			out << QString("%1 %2 ").arg( wlIt.current()->vertex(i).x() ).arg( wlIt.current()->vertex(i).y() );
		
		out << "\"></wireline>\n";
	}
	
	out << "\n</model>";

	return true;
}

bool LogicEditor::load( QIODevice * device )
{
	int errLine, errCol;
	QString errStr;
	QDomDocument doc;

	// Carga del XML
	if( !doc.setContent( device, true, &errStr, &errLine, &errCol ) ){
		qWarning( tr("Error cargando el modelo en la línea %d, columna %d: %s"), errLine, errCol, errStr.latin1() );
		return false;
	}

	// Autentificación de formato
	QDomElement root = doc.documentElement();
	if( root.tagName().lower() != "model" ){
		qWarning( tr("Error cargando librería: El fichero no es un modelo.") );
		return false;
	}

	// Carga de atributos del modelo
	QString data;
	data = root.attribute( "name", name() );
	setName( data );

	// Carga de dispositivos
	QDomNode node = root.firstChild();
	while( !node.isNull() ){
		if( node.toElement().tagName().lower() == "device"){
			QDomElement elem =  node.toElement();
			parseDevice( elem );
		}
		
		node = node.nextSibling();
	}

	// Carga de dispositivos
	node = root.firstChild();
	while( !node.isNull() ){
		if( node.toElement().tagName().lower() == "wireline"){
			QDomElement elem =  node.toElement();
			parseWireLine( elem );
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool LogicEditor::parseDevice( QDomElement & element )
{
	QString strLib, strCmp, strVal;
	LMLibrary * lib;
	LMComponent * cmp;
	
// Obtención de atributos ensenciales
	strLib = element.attribute( "library" );
	if( strLib.isEmpty() ){
		qWarning( tr("Error cargando modelo: No se ha asignado ningún valor al atributo 'library'") );
		return false;
	}
	element.removeAttribute( "library" );
	
	strCmp = element.attribute( "template" );
	if( strCmp.isEmpty() ){
		qWarning( tr("Error cargando modelo: No se ha asignado ningún valor al atributo 'template'") );
		return false;
	}
	element.removeAttribute( "template" );

// Proceso y Validación de atributos esenciales
	lib = app->libraryManager().find( strLib );
	if( !lib ){
		qWarning( tr("Error cargando modelo: La librería '%1' no existe en el proyecto actual.").arg(strLib) );
		return false;
	}

	cmp = lib->find( strCmp );
	if( !cmp ){
		qWarning( tr("Error cargando modelo: El componente '%1' no existe en la librería '%2'.").arg(strCmp).arg(strLib) );
		return false;
	}

	// Creamos el dispositivo sin usar la intefaz gráfica para ubicarlo (usingIGU=false)
	LEDevice * dev = createDevice( cmp, false );
	
// Obtención de Atributos especiales
	strVal = element.attribute( "name" );
	if( !strVal.isEmpty() ){
		dev->setName( strVal );
		element.removeAttribute( "name" );
	}
		
	strVal = element.attribute( "offset" );
	if( !strVal.isEmpty() ){
		QPoint p = parsePoint( strVal );
		dev->move( p.x(), p.y() );
		element.removeAttribute( "offset" );
	}

	strVal = element.attribute( "size" );
	if( !strVal.isEmpty() ){
		QPoint p = parsePoint( strVal );
		dev->setSize( p.x(), p.y() );
		element.removeAttribute( "size" );
	}

	// Atributos adicionales
	QDomNamedNodeMap attributes = element.attributes();
	for( int i=0; i<attributes.length(); i++ )
		if( !dev->setProperty( attributes.item(i).toAttr().name(), QVariant( attributes.item(i).toAttr().value() ) ) )
			qWarning( tr("Cargando '%1': El objeto '%2' de tipo '%3' carece del atributo '%4'.").arg(name()).arg(dev->name()).arg(strLib+":"+strCmp) );

	// Se muestra el dispositivo
	dev->show();

	return true;
}

bool LogicEditor::parseWireLine( QDomElement & element )
{
	QString  strVal;
	QPointArray points;
	LEConnectionPoint * left=0, * right=0;
	LEWireLine * wl;
	float breakPoint=0.5f;
	bool hasGeometry=false;
			
  // Cargamos la geometría
	strVal = element.attribute( "points" );
	if( !strVal.isEmpty() ){

		// Geometría completametne definida (array de puntos)
		if( !LMComponent::parseShapeString( strVal, points ) ){
			qWarning( tr("Error cargando modelo: Imposible cargar <wireline>, formato de linea incorrecto.") );
			return false;
		}
		hasGeometry=true;

	}else{

		strVal = element.attribute( "breakPoint" );
		if( !strVal.isEmpty() ){

			// Geometría automática con punto de inflexión en 'breakPoint'
			bool ok;
			breakPoint = strVal.toFloat( &ok );
			if( !ok ) breakPoint = 0.5f;
		}

	}
/*
	QDomNode childNode = element.firstChild();
	while( !childNode.isNull() ){

		if( childNode.nodeType() == QDomNode::TextNode ){
			
			if( !LMComponent::parseShapeString( childNode.toText().data(), points ) ){
				qWarning( tr("Error cargando modelo: Imposible cargar <wireline>, formato de linea incorrecto.") );
				return false;
			}
			hasGeometry=true;

			break;	
		}

		childNode = childNode.nextSibling();
	}
*/

  // Extremos del WireLine
	strVal = element.attribute( "leftConnection" );
	if( !strVal.isEmpty() ){
		LEItem * item = findItem( strVal, true );
		if( item )
			if( item->rtti() == LEPin::RTTI )
				left = ((LEPin*)item)->connectionPoint();
	}

	strVal = element.attribute( "rightConnection" );
	if( !strVal.isEmpty() ){
		LEItem * item = findItem( strVal, true );
		if( item )
			if( item->rtti() == LEPin::RTTI )
				right = ((LEPin*)item)->connectionPoint();
	}

 // Creamos el cable sin usar la intefaz gráfica para ubicarlo
	if( hasGeometry ){
		// Wireline completamente definida (fijación manual de geometría)
		wl = createWireLine( false );
		wl->setVertexs( points );
		wl->connectLeft( left );
		wl->connectRight( right );
	}else{
		// Wireline conectada automáticamente (geometría autocalculada)
		if( left && right )
			wl = createWireLine( left, right, breakPoint );
		else
			return false;
	}
		
	// Atributos adicionales
	strVal = element.attribute( "name" );
	if( !strVal.isEmpty() )
		wl->setName( strVal );


	wl->show();

	return true;
}

QPoint LogicEditor::parsePoint( QString string )
{
	QPoint p;

	QRegExp rx("(\\D)+");
	p.setX( string.section( rx, 0, 0).toDouble() );
	p.setY( string.section( rx, 1, 1).toDouble() );
	
	return p;
}

QPointArray LogicEditor::parsePointArray( QString string )
{
	int max;
	QPointArray points;
	QStringList strs = QStringList::split( QRegExp("(\\D)+"), string );

	if( strs.count()%2 )
		max = strs.count()-1;
	else
		max = strs.count();

	for( int i=0; i < max; i++ )
		points.putPoints( i/2 , 1, strs[i].toDouble(), strs[i+1].toDouble() );
	
	return points;
}

//////////////////////////////////////////////////////////////////////
// Acciones externas: Manipulación de objetos
//////////////////////////////////////////////////////////////////////
LEDevice * LogicEditor::createDevice( const LMComponent * cmp, bool usingIGU )
{
	// Desactivamos los elementos activos
	if( actItem )
		setActiveItem( NULL );
	
	// Instanciación del nuevo componente
	LEDevice * lpDev = new LEDevice( canvas(), NULL );
	lpDev->setName( cmp->name() );
	lpDev->setComponentReference( cmp );
	lpDev->setShape( cmp->shapeList().first() );

	// Inserción de pins del nuevo componente
	PinList::const_iterator it;
	for( it = cmp->pinList().begin(); it != cmp->pinList().end(); it++ ){
		LEPin * lpPin = lpDev->insertPin( (*it).alignment(), (*it).accessMode(), (*it).name(), (*it).position(), (*it).activeLevel() );
		lpPin->setName( (*it).name() );
	}

	// Ubicación con intefaz gráfica
	if( usingIGU ){
		// Cancelamos cualquier otro objeto pendiente
		if( pendingItem )
			pendingItemCancel( pendingItem );

		// El nuevo componente pasa a ser el objeto pendiente
		pendingItem = lpDev;
	}

	return lpDev;
}

// Duplica un componente instanciado
LEDevice * LogicEditor::duplicateDevice( LEDevice * device, bool usingIGU )
{
	// Buscamos el dispositivo original
	LEItem * srcItem = findItem( normalizeDupName( device->name() ) );
	if( srcItem->rtti() != LEDevice::RTTI )
		return NULL;
	LEDevice * srcDevice = (LEDevice*) srcItem;

	// Creación del componente
	LEDevice * dupDev = createDevice( srcDevice->componentReference(), usingIGU );
	
	// Asignación de nombre
	if( dupDev )
		dupDev->setName( findFreeDupName( srcDevice->name() ) );

	// Mapeamos los pins del nuevo dispositivo con los del anterior
	LEPin * oldPin = srcDevice->pinList().first();
	LEPin * newPin = dupDev->pinList().first();
	
	while( oldPin && newPin ){
		// Conexión efectiva. La conexión es simétrica para poder notificar el borrado
		oldPin->connectionPoint()->setConnection( newPin->connectionPoint() );
		newPin->connectionPoint()->setConnection( oldPin->connectionPoint() );

		oldPin = srcDevice->pinList().next();
		newPin = dupDev->pinList().next();
	}

	return dupDev;
}

LELabel * LogicEditor::createLabel( const QString &label )
{
	if( actItem )
		setActiveItem( NULL );

	if( pendingItem )
		pendingItemCancel( pendingItem );

	LELabel * lpLab = new LELabel( canvas() );
	lpLab->setText( label );
	pendingItem = lpLab;

	return lpLab;
}

LEWireLine * LogicEditor::createWireLine( bool usingIGU )
{
	if( actItem )
		setActiveItem( NULL );

	LEWireLine * lpWl = new LEWireLine( canvas() );
	lpWl->setName( "WireLine" );

	if( usingIGU ){
		if( pendingItem )
			pendingItemCancel( pendingItem );

		pendingItem = lpWl;
	}


	return lpWl;
}

// Conecta las puntos de conexión cp1 y cp2 mediante un cable de tres segmentos, que se "dobla" en el punto
// que dista breakPoint*longitud_horizontal de cp1
LEWireLine * LogicEditor::createWireLine( LEConnectionPoint * cp1, LEConnectionPoint * cp2, float breakPoint )
{
	LEWireLine * wl = createWireLine( false );
	
	wl->insertVertex( QPoint(cp1->x(), cp1->y()) );
	wl->insertVertex( QPoint(cp1->x()+breakPoint*(cp2->x()-cp1->x()), cp1->y()) );
	wl->insertVertex( QPoint(cp1->x()+breakPoint*(cp2->x()-cp1->x()), cp2->y()) );
	wl->insertVertex( QPoint(cp2->x(), cp2->y()) );
	wl->connectLeft( cp1 );
	wl->connectRight( cp2 );

	return wl;
}

// Iterador de nombres de dispositivos duplicados: Dado un nombre de dispositivo (duplicado o no),
// itera la lista deviceNames y devuelve el primer elemento si existe (QString::null en otro caso)
QString LogicEditor::firstDupName( const QString &patternName ) const
{
	// Se genera un hipotético elemento 0.
	QString normalName = normalizeDupName( patternName );
	QString searchName = normalName + "(" + QString::number(0) + ")";

	// Si existe se devuelve, es el primero y es válido, en otro caso
	// se invoca el procedimiento buscar siguiente desde ese elemento
	if( deviceNames.find( searchName ) )
		return searchName;
	else
		return nextDupName( searchName );
}

// Iterador de nombres de dispositivos duplicados: Dado un nombre de dispositivo duplicado
// itera la lista deviceNames y devuelve el siguiente elemento si existe (QString::null en otro caso)
QString LogicEditor::nextDupName( const QString &patternName ) const
{
	int i, j, minj=65000;

	// Extraemos la información del último nombre iterado
	QString normalName = normalizeDupName( patternName );
	i = extractDupNameIndex( patternName );
	if( i == -1 )
		return QString::null;

	// Búsqueda del mínimo índice siguiente a i
	DeviceMapIterator it(deviceNames);
	for( ; it.current(); ++it ){
		j = extractDupNameIndex( it.currentKey() );
		if(  j > i && j < minj )
			minj = j;
	}

	// Fin de la lista (No existe ningún mínimo)
	if( minj == 65000 )
		return QString::null;
	
	// Devolvemos el siguiente elemento
	return normalName +"("+QString::number(minj)+")";

}


// Devuelve un nombre de duplicado para patternName no usado
QString LogicEditor::findFreeDupName( const QString & patternName )
{
	int i=0;
	QString retval;
	QString normalName = normalizeDupName( patternName );

	// Se incrementa el contador hasta encontrar un nombre del tipo name(i) no usado
	while( deviceNames.find( retval=normalName + "(" + QString::number(i) + ")" ) )
		i++;
	
	return retval;
}

// Busca un item llamado itemName, si mustSolve es TRUE intenta
// resolver el nombre a trávés de la jerarquía de campos: parent.child.child...
// Si no encuentra ningún candidato devuelve false
LEItem * LogicEditor::findItem( const QString & itemName, bool mustSolve )
{	

	LEDevice * it1 = deviceNames.find( itemName );
	if( it1 )
		return it1;

	LEWireLine * it2 = wireLineNames.find( itemName );
	if( it2 )
		return it2;

	// Resolución de un nombre compuesto
	if( mustSolve ){
		QStringList subStrings = QStringList::split( QString("%1").arg(ITEM_NAME_SEPARATOR), itemName );
		LEItem *lastItem, *item = NULL;
		for( int i=0; i<subStrings.size(); i++ ){
			lastItem = item;
			item = NULL;

			if( lastItem ){
				// Item hijo: Búsqueda en profundidad
				for( LEItem * child =lastItem->childs().first(); child; child = lastItem->childs().next() )
					// Búsqueda entre los hijos activos (entre los pins de un device, ie)
					if( child->name() == subStrings[i] ){
						item = child;
						break;
					}
			}else{
				// Item base: Búsqueda sin resolución de nombre por nombres mapeados
				item = findItem( subStrings[i], false );
			}

			// El Item no existe
			if( !item )
				break;
			
		}
		return item;
	}

	// Elemento no encontrado
	return NULL;
}

// Intenta cambiar el nombre del item itemName por newItemName.
// Si newItemName ya existe devuelve false
bool LogicEditor::tryUpdateItemName( const QString & itemName, const QString & newItemName )
{

	LEItem * item = findItem( itemName );
	
	if( !item )
		return true;

	// Ningún nombre puede coincidir con el nombre de un componente de librería
	if( app->libraryManager().findComponent( newItemName ) )
		return false;

	// Exigimos unicidad de nombres
	if( item->rtti() == LEDevice::RTTI || item->rtti() == LEWireLine::RTTI ){
		LEDevice * it1 = deviceNames.find( newItemName );
		if( it1 )
			return false;
		
		LEWireLine * it2 = wireLineNames.find( newItemName );
		if( it2 )
			return false;
	}
	
	// Actualización de los mapas de nombres
	switch( item->rtti() ){
		case LEDevice::RTTI:
			deviceNames.remove( itemName );
			deviceNames.insert( newItemName, (LEDevice*)item );
			
			// Actualización de los nombres de los duplicados (si es un LEDevice original)
			if( extractDupNameIndex( itemName ) == -1 )
				for( QString dupName = firstDupName( itemName ); dupName != QString::null; dupName = nextDupName( dupName ) ){
					LEItem * dupDev = findItem( dupName, false );
					if( dupDev )
						dupDev->setName( newItemName +"("+QString::number(extractDupNameIndex(dupName))+")" );
				}

			break;
		
		case LEWireLine::RTTI:
			wireLineNames.remove( itemName );
			wireLineNames.insert( newItemName, (LEWireLine*)item );
			break;
	}
	
	emit changed();	

	return true;
}

// Registra el item 'item' con el nombre itemName.
// Si itemName ya existe devuelve false
bool LogicEditor::registerItem( LEItem * item, const QString& itemName )
{
	// Ningún nombre puede coincidir con el nombre de un componente de librería
	if( app->libraryManager().findComponent( itemName ) )
		return false;

	// Exigimos unicidad de nombres
	if( item->rtti() == LEDevice::RTTI || item->rtti() == LEWireLine::RTTI ){
		LEDevice * it1 = deviceNames.find( itemName );
		if( it1 )
			return false;
		
		
		LEWireLine * it2 = wireLineNames.find( itemName );
		if( it2 )
			return false;
	}

	// Inserción efectiva
	switch( item->rtti() ){
		case LEDevice::RTTI:
			deviceNames.insert( itemName, (LEDevice*)item );
			break;
		
		case LEWireLine::RTTI:
			wireLineNames.insert( itemName, (LEWireLine*)item );
			break;
	}

	return true;
}


void LogicEditor::setActiveItem( LEItem * item )
{
	if( item == NULL )
	{
		hndlLeftTop->hide();
		hndlLeftBottom->hide();
		hndlRightTop->hide();
		hndlRightBottom->hide();

		// Informamos al exterior con esta señal
		if( actItem )
			emit itemSelected( NULL );
		actItem = 0;
	}
	else
	{
		switch( item->rtti() ){
		case LEWireLine::RTTI:
			//No hacer nada para LEWireLine
		break;
		default:
			hndlLeftTop->move( item->x() - hndlLeftTop->width(), item->y() - hndlLeftTop->height() );
			hndlLeftTop->setZ( item->z()+1 );
			hndlLeftBottom->move( item->x() - hndlLeftBottom->width(), item->y() + item->height() );
			hndlLeftBottom->setZ( item->z()+1 );
			hndlRightTop->move( item->x() + item->width(), item->y() - hndlRightTop->height() );
			hndlRightTop->setZ( item->z()+1 );
			hndlRightBottom->move( item->x() + item->width(), item->y() + item->height() );
			hndlRightBottom->setZ( item->z()+1 );
			
			hndlLeftTop->show();
			hndlLeftBottom->show();
			hndlRightTop->show();
			hndlRightBottom->show();
		}

		// Informamos al exterior con esta señal
		if( actItem != item )
			emit itemSelected( (QObject*)item );

		actItem = item;
	}
}

LEItem * LogicEditor::activeItem()
{
	return actItem;
}

//////////////////////////////////////////////////////////////////////
// Acciones y menú emergente
//////////////////////////////////////////////////////////////////////
void LogicEditor::onActionDelete()
{
	if( activeItem() ){
		purgeItem( activeItem() );
		
		canvas()->update();

		emit changed();
	}
}

void LogicEditor::onActionDuplicate()
{
	if( activeItem() &&activeItem()->rtti() == LEDevice::RTTI ){
		LEDevice * dev = (LEDevice*) activeItem();

		if( dev->isExternSolving() )
			duplicateDevice( dev, true );
	}
}

void LogicEditor::createActions()
{
	actDelete = new QAction(tr("Eliminar"), tr(""), this );
	connect( actDelete, SIGNAL(activated()), this, SLOT(onActionDelete()) );
	actDuplicate = new QAction(tr("Duplicar"), tr(""), this );
	connect( actDuplicate, SIGNAL(activated()), this, SLOT(onActionDuplicate()) );
}

void LogicEditor::contextMenuEvent(QContextMenuEvent *event )
{
	if( !activeItem() ){
		event->ignore();
		return;
	}

	// Borrado de elementos
	QPopupMenu contextMenu( this );
	actDelete->addTo( &contextMenu );

	// Duplicación de elementos (para dispositivos de resolución externa)
	if( activeItem()->rtti() == LEDevice::RTTI ){
		LEDevice * dev = (LEDevice*) activeItem();

		if( dev->isExternSolving() ){
			contextMenu.insertSeparator();
			actDuplicate->addTo( &contextMenu );
		}
	}

	contextMenu.exec( event->globalPos() );

}

//////////////////////////////////////////////////////////////////////
// Control de eventos
//////////////////////////////////////////////////////////////////////
void LogicEditor::mouseOverEvent( LEItem * item, const QPoint &point )
{
	if( !item )
		return;

	switch( item->rtti() ){
	case LEConnectionPoint::RTTI:
	{
		// Activamos el borde del punto de conexión
		LEConnectionPoint * lpCp = (LEConnectionPoint*) item;
		lpCp->setDrawSquare( true );
		
		lastCnnctPoint = lpCp;
		
		canvas()->update();
		break;
	}
	}
}

void LogicEditor::mouseOutEvent( LEItem * item, const QPoint &point )
{
	if( !item )
		return;

	switch( item->rtti() ){
	case LEConnectionPoint::RTTI:
	{
		// Desactivamos el border del punto de conexión
		if( lastCnnctPoint )
			lastCnnctPoint->setDrawSquare( false );

		canvas()->update();
		break;
	}
	}
}

void LogicEditor::contentsMousePressEvent( QMouseEvent *event )
{
	QPoint realPos = (1.0/zoomFactor) * event->pos();
	
	if( !pendingItem )
		trySelectItem( realPos );
	else
		switch( event->button() ){
		case LeftButton:
			pendingItem = pendingItemPlace( pendingItem, realPos );
			break;

		case RightButton:
			pendingItemCancel( pendingItem );
			pendingItem = NULL;
		}
}

void LogicEditor::contentsMouseMoveEvent( QMouseEvent *event )
{
	QPoint realPos = (1.0/zoomFactor) * event->pos();

	if( event->state() & LeftButton ){
		if( actItem )
		{
			switch( actItem->rtti() ){
			case LEWireLine::RTTI:
			{
				moveWireLineVertex( (LEWireLine*)actItem, realPos );

				lastPos = realPos;
				canvas()->update();
				break;
			}
			default:
				if( hndlActive ){
					// Devuelve la posición destino efectiva, considerando el redimensionado
					// que el objeto actItem ha permitido.						
					lastPos = resizeItem( actItem, lastPos, realPos, hndlActive->alignment() );
				}else{
					moveItem( actItem, lastPos, realPos );
					lastPos = realPos;
				}
			}	
		}
	}else{
		if( pendingItem )
			pendingItemPreview( pendingItem, realPos );				
	}
	
	// Generamos el MouseOverEvent
	QCanvasItemList items = canvas()->collisions(realPos);
	QCanvasItemList::iterator it;
	for( it = items.begin(); it != items.end(); it++ )
		if( (*it)->rtti() >= LEItem::RTTI )
			mouseOverEvent( (LEItem*)*it, realPos );


	// Generamos el MouseOutEvent
	for( it = lastMouseOverItems.begin(); it != lastMouseOverItems.end(); it++ )
		if( (*it)->rtti() >= LEItem::RTTI )
			if( items.find( *it ) == items.end() )
				mouseOutEvent( (LEItem*)*it, realPos );

	// Actualizamos la lista de Items que han recibido MouseOverEvent
	lastMouseOverItems = items;

	
}
	
//////////////////////////////////////////////////////////////////////
// Manipulación de Items
//////////////////////////////////////////////////////////////////////
void LogicEditor::trySelectItem( const QPoint& pos )
{
	hndlActive = NULL;
	vertexActive = -1;

	QCanvasItemList items = canvas()->collisions(pos);
	
	if( items.empty() )
	{
		// Pulsación en espacio vacío
		setActiveItem( NULL );
		canvas()->update();
	}
	else
	{
		// Pulsación sobre un Item
		QCanvasItem * canvasItem = *items.begin();
		if( canvasItem->rtti() >= LEItem::RTTI )
		
			// Se trata de un derivado de LEItem
			switch( canvasItem->rtti() )
			{
			case LEHandle::RTTI:
				hndlActive = (LEHandle*)canvasItem;
				break;					
			case LEWireLine::RTTI:{
				LEWireLine * wlItem = (LEWireLine*)canvasItem;
				vertexActive = wlItem->nearestVertex( pos );
			}
			default:
				if( actItem != (LEItem*)canvasItem )
				{
					if( actItem != NULL )
						setActiveItem( NULL );
				
					setActiveItem( (LEItem*)canvasItem );
				}
				
				canvas()->update();
			}
		
	}

	lastPos = pos;
}

void LogicEditor::moveItem( LEItem * item, QPoint source, QPoint target )
{
	// Arrastrando
	switch( item->rtti() ){
		case LEPin::RTTI:
		{
			LEPin * pin = (LEPin*)item;
			switch( pin->alignment() ){
			case LEPin::Left:
			case LEPin::Right:
				if( target.y() >= pin->parent()->y() && target.y() < pin->parent()->y() + pin->parent()->height() )
					pin->setPosition( ((float)(target.y()-pin->parent()->y() )) / ((float)(pin->parent()->height()-2*pin->height())) );
				break;
			case LEPin::Top:
			case LEPin::Bottom:
				if( target.x() >= pin->parent()->x() && target.x() < pin->parent()->x() + pin->parent()->width() )
					pin->setPosition( ((float)(target.x()-pin->parent()->x() )) / ((float)(pin->parent()->width()- 2*pin->width())) );
			}
			pin->parent()->setSize( pin->parent()->width(), pin->parent()->height() );
			setActiveItem( item );
			canvas()->update();
			break;
		}
		default:
		{
			QPoint delta = target - source;
			item->moveBy( delta.x(), delta.y() );
			setActiveItem( item );
			
			canvas()->update();
		}
	}

	emit changed();
}


QPoint LogicEditor::resizeItem( LEItem * item, QPoint source, QPoint target, LEHandle::HandleAlignment rzSide )
{
	QPoint delta = target - source;
	QPoint effectiveTarget = target;

	if( !item->isResizable() )
		return target;

	int newW =item->width(),newH=item->height();
	int offsetX=0, offsetY=0;
	bool resized=false;

	// Cálculo del redimensionado horizontal
	if( rzSide & LEHandle::Left ){
		if( item->width()-delta.x() > 0 ) 
		{
			newW = item->width()-delta.x();
			offsetX = delta.x();
			effectiveTarget.setX( target.x() );
			resized = true;
		}
	}else if( rzSide & LEHandle::Right ){
		if( item->width()+delta.x() > 0 )
		{
			newW = item->width()+delta.x();
			offsetX = 0;
			effectiveTarget.setX( target.x() );
			resized = true;
		}
	}

	// Cálculo del redimensionado vertical
	if( rzSide & LEHandle::Top ){
		if( item->height()-delta.y() > 0 )
		{
			newH = item->height()-delta.y();
			offsetY = delta.y();
			effectiveTarget.setY( target.y() );
			resized = true;
		}
	}else if( rzSide & LEHandle::Bottom ){
		if( item->height()+delta.y() > 0 )
		{
			newH = item->height()+delta.y();
			offsetY = 0;
			effectiveTarget.setY( target.y() );
			resized = true;
		}
			
	}


	// Redimensionado efectivo
	if( resized ){
		item->setSize( newW, newH );
		item->moveBy( offsetX, offsetY );
		setActiveItem( item );
		canvas()->update();				
	}

	emit changed();
	return effectiveTarget;
}

// Elimina toda referencia al elemento item y lo marca para ser destruido
void LogicEditor::purgeItem( LEItem * item )
{
	if( actItem == item )
		setActiveItem( NULL );
	
	if( pendingItem == item )
		pendingItem = NULL;

	if( lastCnnctPoint )
		if( lastCnnctPoint->parent() == item )
			lastCnnctPoint = NULL;

	if( item->rtti() == LEDevice::RTTI ){

		// Borrado de los duplicados
		if( extractDupNameIndex( item->name() ) == -1 )
		
			// Se trata de una instancia original, si tiene duplicados hay que eliminarlos
			for( QString dupName = firstDupName( item->name() ); dupName != QString::null; dupName = nextDupName( dupName ) ){
			
				LEItem *dupItem = findItem( dupName );
				
				if( dupItem )
					purgeItem( dupItem );
			}

		// Borrado del mapa de dispositivos
		deviceNames.remove( item->name() );
	}

	if( item->rtti() == LEWireLine::RTTI ){
		// Borrado del mapa de cables
		wireLineNames.remove( item->name() );
	}

	// Borrado de la lista de eventos MouseOver pendientes (si está en ella)
	lastMouseOverItems.remove( item );

	item->remove();
}


//////////////////////////////////////////////////////////////////////
// moveWireLineVertex( LEWireLine * wlItem, QPoint target )
//
//     Mueve el vértice LogicEditor::vertexActive al punto target
//     realizando los cambios necesarios en la linea y conectando
//     con los puntos de conexión oportunos si se da la ocasión.
//
//     Modifica vertexActive y lastPos
//     Emite connected(...) y disconnected(...) cuando se conecta o
//     desconecta una linea
void LogicEditor::moveWireLineVertex( LEWireLine * wlItem, QPoint target )
{
	// Buscamos intersección con LEPin (posible conexión)
	LEConnectionPoint * connection = NULL;
	QCanvasItemList items = canvas()->collisions(target);
	for( QCanvasItemList::iterator it = items.begin(); it != items.end(); it++ )
		if( (*it)->rtti() == LEConnectionPoint::RTTI ){
			connection = (LEConnectionPoint*)*it;//((LEPin*)*it)->connectionPoint();
			break;
		}

	// Chequeamos que vertexActive no haya quedado obsoleto
	// Si no quedan suficientes vértices para mover la linea, añadimos uno
	if( vertexActive >= wlItem->vertexCount() ){
		if( wlItem->vertexCount() <= 2 )
			wlItem->insertVertex( wlItem->vertex(wlItem->vertexCount()-1) );	
		vertexActive = wlItem->vertexCount()-1;
	}

	// Determinamos la conexión a modificar
	LEConnectionPoint * vertexConnection=NULL;
	if( vertexActive == 0 )
		vertexConnection = wlItem->leftConnection();
	else if( vertexActive == wlItem->vertexCount()-1 )
		vertexConnection = wlItem->rightConnection();


	// Movimiento y Conexión de vértices
	if( vertexActive > 0 && vertexActive < wlItem->vertexCount()-1 ){
	
		//Movimiento de vértices intermedios
		wlItem->moveVertex( vertexActive, target );//wlItem->vertex(vertexActive)+(target-lastPos) );
		
		// Reaproximación del vértice activo
		vertexActive = wlItem->nearestVertex( target );

	}else{
		
		// Procesamiento de vértices extremos
		if( vertexConnection ){

			// Candidato a desconexión
			if( !connection ){
				if( (target - QPoint(vertexConnection->x(),vertexConnection->y())).manhattanLength() > 15 ){
					if( vertexActive == 0 ){
						wlItem->insertVertex( vertexConnection->x(), vertexConnection->y(), 0 );
						wlItem->moveVertex( 1, target );
						vertexActive = 1;
					}else{
						wlItem->insertVertex( vertexConnection->x(), vertexConnection->y() );
						wlItem->moveVertex( vertexActive-1, target );
						vertexActive = vertexActive-1;
					}
					
					// Reaproximación del vértice activo
					//vertexActive = wlItem->nearestVertex( target );
				}else{
					// Desconexión efectiva
					wlItem->moveVertex( vertexActive, target );

					if( vertexActive == 0 )
						wlItem->connectLeft( NULL );
					else
						wlItem->connectRight( NULL );

					emit disconnected( actItem, vertexConnection );
					emit changed();
				}
			}

		}else{
			
			// Candidato a conexión
			if( connection ){
				
				// Conexión efectiva
				wlItem->moveVertex( vertexActive, connection->x(), connection->y() );
				
				if( vertexActive == 0 )
					wlItem->connectLeft( connection );
				else
					wlItem->connectRight( connection );
				
				emit connected( actItem, connection );
				emit changed();

			}else{
				wlItem->moveVertex( vertexActive, target );
			}
		}
	}
		
}

//////////////////////////////////////////////////////////////////////
//  Creación de Items
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//  pendingItemPlace( LEItem * item, const QPoint &pos )
//
//    Ubicación definitiva del Item item (excepto para LEWireLine,
//    para el que supone la fijación de un Vértice)
//    Emite pendingItemPlaced(...)
LEItem * LogicEditor::pendingItemPlace( LEItem * item, const QPoint &pos )
{
	LEItem * newPendingItem;

	switch( item->rtti() ){
		case LEWireLine::RTTI:
		{
			LEWireLine * lpWl = (LEWireLine*) item;
		
			// Buscamos una posible conexión cercana
			LEConnectionPoint * connection = NULL;
			QCanvasItemList items = canvas()->collisions(pos);
			for( QCanvasItemList::iterator it = items.begin(); it != items.end(); it++ )
				if( (*it)->rtti() == LEConnectionPoint::RTTI ){
					connection = (LEConnectionPoint*)*it;
					break;
				}
			
			// Ubicamos el nuevo vértice
			lpWl->insertVertex( pos );
			newPendingItem = item;
			if( lpWl->vertexCount() == 1 )
				lpWl->insertVertex( QPoint( pos.x()+1, pos.y()+1) );
			else if( connection )
				newPendingItem = NULL;
	
			if( connection )
				if( lpWl->vertexCount() == 2 ){	
					lpWl->connectLeft( connection );
					emit connected( item, connection );
				}else{
					lpWl->connectRight( connection );
					emit connected( item, connection );
				}

			break;
		}
		default:
		{
			item->move( pos.x(), pos.y() );
			setActiveItem( item );
			newPendingItem = NULL;
		}
	}
	item->show();
	canvas()->update();
	
	emit pendingItemPlaced( item );
	emit changed();

	return newPendingItem;
}

//////////////////////////////////////////////////////////////////////
//  pendingItemCancel( LEItem * item )
//
//    Cancelación y liberación retardada del item pendiente item
//    NOTA: Para LEWireLine supone el fin del trazado.
//    Emite pendingItemCanceled(...)
void LogicEditor::pendingItemCancel( LEItem * item )
{
	switch( item->rtti() ){
		case LEWireLine::RTTI:
		{
			LEWireLine * lpWl = (LEWireLine*) item;
			if( lpWl->vertexCount() == 0 ){
				pendingItemCanceled( item, true );
				purgeItem( item );
			}else{
				// En este caso, destroyed=false indica que el item
				// no ha sido destruido, lo que para un LEWireLine supone
				// que ha sido satisfactoriamente instanciado
				pendingItemCanceled( item, false );
				lpWl->removeVertex( lpWl->vertexCount()-1 );
			}
			break;
			emit changed();
		}
		default:
		{
			pendingItemCanceled( item, true );
			purgeItem( item );
		}
	}
	canvas()->update();

}

//////////////////////////////////////////////////////////////////////
//  pendingItemPreview( LEItem * item, const QPoint &pos )
//
//    Dibujado preliminar de un item pendiente en la posición pos
void LogicEditor::pendingItemPreview( LEItem * item, const QPoint &pos )
{
	switch( item->rtti() ){
		case LEWireLine::RTTI:
		{
			LEWireLine * lpWl = (LEWireLine*) item;
		
			if( lpWl->vertexCount() > 0 )
				lpWl->moveVertex( lpWl->vertexCount()-1, pos );

			break;
		}
		default:
		{
			item->move( pos.x(), pos.y() );
		}
	}
	item->show();
	canvas()->update();
}


