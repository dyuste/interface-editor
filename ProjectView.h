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


// ProjectView.h: interface for the ProjectView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PROJECT_VIEW_H_)
#define _PROJECT_VIEW_H_

#include <qlistview.h>

class QAction;

class ProjectView : public QListView
{
Q_OBJECT

public:
	ProjectView( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );

public slots:

	// Raíz del proyecto
	void setProjectName( const QString& prjName );

	// Manipulación de documentos
	void insertDocument( const QString& docName );
	void removeDocument( const QString& docName );
	void replaceDocument( const QString& oldName, const QString &newName );
	
	// Interacción con el usuario
	void onDoubleClicked( QListViewItem * it, const QPoint &, int );
	void onCurrentChanged( QListViewItem * it );

	// Slots para el menú contextual
	void onActionDelete();

signals:

	// Eventos producidos por el usuario
	void usrDocumentSelected( const QString& docName );
	void usrDocumentOpened( const QString& docName );
	void usrDocumentDeleted( const QString& docName );
	
protected:
	inline QListViewItem * findDocument( const QString& docName );

	// Menú contextual
	virtual void contextMenuEvent( QContextMenuEvent *event );

	// Acciones para la interacción con el usuario mediante menús
	void createActions();

private:
	QAction * actDelete;
};

#endif 
