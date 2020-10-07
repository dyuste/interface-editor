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



#include "dlgNewProject.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qdir.h>
#include "QListViewPlugin.h"

#include <typeinfo>
using namespace std;

#include "Application.h"
extern Application * app;

dlgNewProject::dlgNewProject()
{
	// Creación de controles
	createProjectControls();
	createTemplateControls();
	createLayouts();

	// Ventana
	setCaption( tr("Proyecto Nuevo") );
	resize( 400, 250 );
	
	// Orden de Controles
	setTabOrder( btOk, btCancel );
	setTabOrder( btCancel, btPath );	
	plg = 0;
}

void dlgNewProject::createProjectControls()
{
	// Controles
	lbName = new QLabel( tr("Nombre: "), this );
	txName = new QLineEdit( this );
	connect( txName, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)) );

	lbAutor = new QLabel( tr("Autor: "), this );
	txAutor = new QLineEdit( this );

	lbPath = new QLabel( tr("Directorio base: "), this );
	txPath = new QLineEdit( this );
	setPath( QDir::convertSeparators(QDir::currentDirPath()) );

	btPath = new QPushButton( "...", this );
	btPath->setFixedWidth( btPath->fontMetrics().width(" ... ") );
	btPath->setFixedHeight( txPath->height()-10 );
	connect( btPath, SIGNAL(clicked()), this, SLOT(choosePath()) );

	btOk = new QPushButton( tr("Aceptar"), this );
	btCancel = new QPushButton( tr("Cancelar"), this );
	connect( btOk, SIGNAL(clicked()), this, SLOT(accept()) );
	connect( btCancel, SIGNAL(clicked()), this, SLOT(reject()) );
}

void dlgNewProject::createTemplateControls()
{

  // Etiqueta
	lbTemplates = new QLabel(tr("Plantillas"), this );

  // Creación de la lista de plantillas
	QValueList<Plugin*> plugins = app->getPluginList( ptProject );
	lvTemplates = new QListViewPlugin( plugins, this );

	// Conexión
	connect( lvTemplates, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(onSelectTemplate(QListViewItem*)) );

	// Inserción de la plantilla vacía
	QListViewItemPlugin * item = new QListViewItemPlugin( 0, lvTemplates, "Proyecto Vacío" );
	item->setSelected( true );

  // Caja de información
	lbTemplateInfo = new QLabel( this );
	lbTemplateInfo->setMinimumWidth( 100 );
	lbTemplateInfo->setFrameStyle( QFrame::Panel || QFrame::Raised );
	lbTemplateInfo->setLineWidth( 1 );
	lbTemplateInfo->setMidLineWidth( 0 );

}

void dlgNewProject::createLayouts()
{
	// Campo path (lineEdit+button)
	QHBoxLayout * lyPath = new QHBoxLayout;
	lyPath->setSpacing(0);
	lyPath->setMargin(0);
	lyPath->addWidget( txPath );
	lyPath->addWidget( btPath );
	

	// Campos de Input
	QGridLayout * lyFields = new QGridLayout( 3, 2 );
	lyFields->setSpacing(5);
	lyFields->setMargin(10);
	
	lyFields->addWidget( lbName, 0, 0 );
	lyFields->addWidget( txName, 0, 1 );
	lyFields->addWidget( lbAutor, 1, 0 );
	lyFields->addWidget( txAutor, 1, 1 );
	lyFields->addWidget( lbPath, 2, 0 );
	lyFields->addLayout( lyPath, 2, 1 );

	// Botones
	QHBoxLayout * lyButtons = new QHBoxLayout;
	lyButtons->setSpacing(5);

	lyButtons->addStretch( );
	lyButtons->addWidget( btOk );
	lyButtons->addWidget( btCancel );

	// Superficie Superior Derecha
	QVBoxLayout * lyRight = new QVBoxLayout;
	lyRight->setMargin(10);
	lyRight->addWidget( lbTemplateInfo );
	//lyRight->addStretch( 20 );

	// Superficie Superior izquierda
	QVBoxLayout * lyLeft = new QVBoxLayout;
	lyLeft->setMargin(5);
	lyLeft->addWidget( lbTemplates );
	lyLeft->addWidget( lvTemplates );

	// Superficie Superior (derecha+izquierda)
	QHBoxLayout * lyTop = new QHBoxLayout;
	lyTop->addLayout( lyLeft );
	lyTop->addLayout( lyRight );

	// Superficie Base
	QVBoxLayout * lyBase = new QVBoxLayout( this );
	lyBase->addLayout( lyFields );
	lyBase->addLayout( lyTop );
	lyBase->addLayout( lyButtons );
}
	


QString dlgNewProject::name() const
{
	return txName->text();
}

void dlgNewProject::setName( const QString & nm )
{
	txName->setText( nm );
}

QString dlgNewProject::autor() const
{
	return txAutor->name();
}

void dlgNewProject::setAutor( const QString & at )
{
	txAutor->setText( at );
}

QString dlgNewProject::path() const
{
	return txPath->text();
}

void dlgNewProject::setPath( const QString & pt )
{
	txPath->setText( pt );
}

/*		Manejadores		*/
void dlgNewProject::nameChanged( const QString& nm )
{
	QDir dir( path() );
	if( !dir.exists() )
		dir.cdUp();
	setPath( QDir::convertSeparators( QString("%1%2%3").arg(dir.absPath()).arg(QDir::separator()).arg(nm) ) );
}

void dlgNewProject::onSelectTemplate( QListViewItem * item )
{
	QListViewItemPlugin * pluginItem;

	// Se aplica Casting seguro
	try{
		pluginItem = dynamic_cast<QListViewItemPlugin*>(item);
	}catch(bad_cast){
		lbTemplateInfo->setText( "" );
		return;
	}

	// Se muestra la información asociada al plugin
	Plugin * plugin = pluginItem->plugin();
	if( plugin )
		lbTemplateInfo->setText( plugin->description() );
	else
		lbTemplateInfo->setText( "" );

	// Se almacena el puntero hacia el último plugin seleccionado
	this->plg = plugin;
}

void dlgNewProject::choosePath()
{
	QString pth;

	QDir dir( path() );
	if( dir.exists() )
		pth = path();
	
	pth = QFileDialog::getExistingDirectory( pth, this, "openProjectPath", tr("Directorio del proyecto...") );
	
	if( !pth.isEmpty() )
		setPath( pth );
}

void dlgNewProject::accept()
{
	if( name().isEmpty() ){
		QMessageBox::warning( this, tr("Proyecto Nuevo"), tr("Debe especificar un nombre para el proyecto") );
		return;
	}
	
	if( path().isEmpty() ){
		QMessageBox::warning( this, tr("Proyecto Nuevo"), tr("Debe especificar un directorio base para el proyecto") );
		return;
	}
	
	QDir dir( path() );
	if( !dir.exists() ){
		QString nm = dir.dirName();
		dir.cdUp();
		
		if( !dir.exists() ){
			QMessageBox::warning( this, tr("Proyecto Nuevo"), tr("El directorio base especificado no existe") );
			return;
		}else
			dir.mkdir( nm, false );
	}
	
	QDialog::accept();
}
