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



#include "ProjectView.h"

#include <qpixmap.h>
#include <qaction.h>
#include <qpopupmenu.h>

//////////////////////////////////////////////////////////////////////
// Construccion
//////////////////////////////////////////////////////////////////////

ProjectView::ProjectView( QWidget * parent, const char * name, WFlags f )
	: QListView( parent, name, f )
{
	createActions();

	addColumn( tr("Proyecto") );
	setRootIsDecorated( true );
	setResizeMode( QListView::AllColumns );
	setColumnWidthMode( 0, QListView::Maximum );

	connect( this, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(onCurrentChanged(QListViewItem*)) );
	connect( this, SIGNAL(doubleClicked(QListViewItem*,const QPoint&,int)), this, SLOT(onDoubleClicked(QListViewItem*,const QPoint&,int)) );
}

// Acciones para la interacción con el usuario mediante menús
void ProjectView::createActions()
{
	actDelete = new QAction( tr("Eliminar"), tr(""), this );
	connect( actDelete, SIGNAL(activated()), this, SLOT(onActionDelete()) );
}

//////////////////////////////////////////////////////////////////////
// Raíz del proyecto
//////////////////////////////////////////////////////////////////////
void ProjectView::setProjectName( const QString& prjName )
{
	QListViewItem* root = firstChild();
	
	if( !root ){
		root = new QListViewItem( this, prjName );
		root->setExpandable( true );
		setOpen( root, true );
	}else
		root->setText( 0, prjName );

	root->setPixmap( 0, QPixmap::fromMimeSource("img/icon-prj.png") );

}


//////////////////////////////////////////////////////////////////////
// public slots: Manipulación de documentos
//////////////////////////////////////////////////////////////////////
void ProjectView::insertDocument( const QString& docName )
{
	QListViewItem * root = firstChild();
	if( !root )
		return;

	QListViewItem * doc = new QListViewItem( root, docName );
	doc->setPixmap( 0, QPixmap::fromMimeSource("img/icon-doc.png") );
}

void ProjectView::removeDocument( const QString& docName )
{
	QListViewItem * doc = findDocument( docName );
	
	// Borrado efectivo
	if( doc )
		delete doc;	

}

void ProjectView::replaceDocument( const QString& oldName, const QString &newName )
{
	QListViewItem * doc = findDocument( oldName );

	// Renombrado efectivo
	if( doc )
		doc->setText( 0, newName );
}

// Slots para el Menú contextual
void ProjectView::onActionDelete()
{
	if( currentItem() != firstChild() )
		emit usrDocumentDeleted( currentItem()->text( 0 ) );
}

//////////////////////////////////////////////////////////////////////
// Interacción con el usuario
//////////////////////////////////////////////////////////////////////
void ProjectView::onDoubleClicked( QListViewItem * it, const QPoint &, int )
{
	if( it != firstChild() )
		emit usrDocumentOpened( it->text( 0 ) );
}

void ProjectView::onCurrentChanged( QListViewItem * it )
{
	if( it != firstChild() )
		emit usrDocumentSelected( it->text( 0 ) );
}

// Menú contextual
void ProjectView::contextMenuEvent( QContextMenuEvent *event )
{
	if( currentItem() != firstChild() ){
		QPopupMenu contextMenu;
		actDelete->addTo( &contextMenu );
		contextMenu.exec( event->globalPos() );
	}else
		event->ignore();
}


	
//////////////////////////////////////////////////////////////////////
// protected: Operaciones a nivel de implementación
//////////////////////////////////////////////////////////////////////
inline QListViewItem * ProjectView::findDocument( const QString& docName )
{
	return findItem( docName, 0 );
}

	
