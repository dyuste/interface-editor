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



#include "QListViewPlugin.h"
#include "Plugin.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
QListViewItemPlugin::QListViewItemPlugin( Plugin * plugin, QListView * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8 )
		: QListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8 )
{ 
	setPlugin( plugin ); 
}


QListViewPlugin::QListViewPlugin( QValueList<Plugin*> plugins, QWidget* parent, const char* name, WFlags f )
: QListView( parent, name, f )
{
	// Creación de las columnas
	int colTmpl = addColumn( tr("Plantilla") );
	addColumn( tr("Autor") );
	
	// Configuración de las columnas
	setResizeMode( QListView::LastColumn );
	setColumnWidthMode( colTmpl, QListView::Maximum );

	// Inserción del resto de plantillas (plugins)
	for( QValueList<Plugin*>::iterator it = plugins.begin(); it!=plugins.end(); ++it ){
		
		// Obtención del String de versión
		int verInt = (*it)->version();
		QString version;
		if( (verInt%1000000)/10000 ){
			version = QString::number( (verInt%1000000)/10000 ) +"."
					 +QString::number( (verInt%10000)/100 ) +"."
					 +QString::number( verInt%100 );
		}else{ 
			version = QString::number( (verInt%10000)/100 ) +"."
					 +QString::number( verInt%100 );
		}

		// Creación del elemento de la lista
		QListViewItemPlugin * item = new QListViewItemPlugin( *it, this, QString((*it)->name()) +" ("+version+")", (*it)->about() );
	}
}

QListViewPlugin::~QListViewPlugin()
{}
