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


// Document.cpp: implementation of the Document class.
//
//////////////////////////////////////////////////////////////////////

#include "Document.h"

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qregexp.h>

#include "LogicEditor.h"
#include "HDLGenerator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Document::Document( QWidget * parent, const char * name )
	: LogicEditor( parent, name )
{
	//setWFlags(  WDestructiveClose );
	setIcon( QPixmap::fromMimeSource("img/icon-doc.png") );

	lpHDLGen = NULL;
	isChanged=false;
	isUntitled=true;
	if( parent )
		resize( 2*parent->size()/3 );

	// Control de modificaciones
	connect( this, SIGNAL(changed()), this, SLOT(setChanged()) );
}

bool Document::initialize()
{
	static int number = 1;

	// Este objeto no es reutilizable, si ha sido inicializado la orden fracasa
	if( lpHDLGen || canvas() )
		return false;

	// Creación del lienzo
	QCanvas * canvas = new QCanvas( this, QString("Canvas%1").arg(number++) );
	canvas->resize( 10000, 10000 );
	
	setCanvas( canvas );

	
	// Generador de HDL
	lpHDLGen = new HDLGenerator( canvas );

	// Interconexiones
	connect( this, SIGNAL( changed() ), lpHDLGen, SLOT( setDataChanged() ) );

	isUntitled = true;
	return true;
}

// Objetos Principales
Document::~Document()
{
	if( lpHDLGen )
		delete lpHDLGen;
}

void Document::setName( const QString & nm )
{
	isUntitled = false;
	isChanged=true;
	
	if( name() != nm ){
		QString oldName = name();

		QObject::setName( nm );
		setCaption( nm );
		
		emit renamed( oldName, nm );	
	}

}

HDLGenerator * Document::hdlGenerator()
{
	return lpHDLGen;
}

// Control del documento
bool Document::createNew()
{
	static int number=1;
	
	// Inicialización del objeto (creación efectiva)
	if( !initialize() )
		return false;
	
	// Nombre del documento
	setName( QString("SinTitulo%1").arg(number) );
	number++;
	
	isUntitled = true;
	return true;
}

bool Document::open()
{
	// Inicialización del objeto (creación efectiva)
	if( !initialize() )
		return false;

	// Carga desde fichero
	QString fileName = QFileDialog::getOpenFileName(
						QDir::current().absPath(),
						tr("Documentos de LogicEditor (*.lem)"),
						this,
						"openModelDialog",
						tr("Abrir modelo...") );

	if( fileName.isEmpty() )
		return false;

	QRegExp rx( "*.lem" );
	rx.setWildcard( true );
	if( rx.exactMatch( fileName ) )
		fileName.truncate( fileName.length()-4 );

	// Título de la ventana
	setName( fileName );
	setCaption( name() );

	return load();

}

bool Document::save()
{
	if( isUntitled )
		return saveAs();

	QFile file( QString("%1.lem").arg( name() ) );
	if( !file.open( IO_WriteOnly ) )
		return false;
		
	if( !LogicEditor::save( &file ) ){
		file.close();
		return false;
	}

	file.close();

	isChanged = false;
	return true;
}

bool Document::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(
						QDir::current().absPath(),
						tr("Modelo de LogicEditor (*.lem)"),
						this,
						"saveModelDialog",
						tr("Guardar modelo como...") );

	if( fileName.isEmpty() )
		return false;
	
	QDir dir( fileName );
	
	// Ya existe, Sobreescribir?
	if( dir.exists() ){
		if( QMessageBox::warning( 0, tr("Guardar modelo..."), tr("El fichero ya existe, ¿desea sobreescribirlo?"), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No )
			return false;
	}
	
	// Tratamiento del nombre de fichero/documento
	fileName = dir.dirName();

	QRegExp rx( "*.lem" );
	rx.setWildcard( true );
	if( rx.exactMatch( fileName ) )
		fileName.truncate( fileName.length()-4 );

	setName( fileName );
	setCaption( fileName );

	return save();
}

void Document::setChanged()
{
	isChanged = true;
}

void Document::setUnchanged()
{
	isChanged = false;
}

bool Document::mayBeSave() const
{
	return isChanged;
}

bool Document::load()
{
	isChanged = false;
	isUntitled = false;

	QFile file( QString("%1.lem").arg(name()) );
	if( !file.open( IO_ReadOnly) )
		return false;

	return LogicEditor::load( &file );
}

bool Document::queryCloseDocument()
{
	if( mayBeSave() ){

		int retval = QMessageBox::question( this, tr("Cerrando modelo ") + name(), 
			"El documento ha sido modificado, ¿desea guardar los cambios?",
			QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
		
		switch( retval ){
		case QMessageBox::Yes:
			save();
			return true;
			
		case QMessageBox::No:
			return true;
			
		case QMessageBox::Cancel:
			return false;

		}
	}	
	return true;
}
