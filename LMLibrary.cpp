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



#include <qfile.h>
#include <qdir.h>
#include <qdom.h>

#include "LMLibrary.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LMLibrary::LMLibrary()
{
	nm = QString::null;
	setSourceType( None );
}

LMLibrary::LMLibrary( QIODevice * file )
{
	nm = QString::null;
	setSourceType( None );
	parseFile( file );	
}

LMLibrary::LMLibrary( QString & file )
{
	nm = QString::null;
	setSourceType( None );
	parseFile( file );	
}


//////////////////////////////////////////////////////////////////////
// Atributos de la librería
//////////////////////////////////////////////////////////////////////

void LMLibrary::setName( const QString &name )
{
	this->nm = name;
}

void LMLibrary::setAutor( const QString &autor )
{
	this->autr = autor;
}

void LMLibrary::setSourceOrigin( const QString &sourceFolder )
{
	this->srcSrc = sourceFolder;
}

void LMLibrary::setSourceType( enum SourceType st )
{
	this->srcTp = st;
}

void LMLibrary::setVersion( int version )
{
	this->vr = version;
}

QString LMLibrary::name() const
{
	return nm;
}

QString LMLibrary::autor() const
{
	return autr;
}

QString LMLibrary::sourceOrigin() const
{
	return srcSrc;
}

LMLibrary::SourceType LMLibrary::sourceType() const
{
	return srcTp;
}

int LMLibrary::version() const
{
	return vr;
}

//////////////////////////////////////////////////////////////////////
// Acceso indexado por nombre
//////////////////////////////////////////////////////////////////////
LMComponent * LMLibrary::find( const QString & compName )
{
	for( iterator it = begin(); it != end(); ++it )
		if( (*it).name() == compName )
			return &(*it);
	
	return NULL;
}


//////////////////////////////////////////////////////////////////////
// Extrae la información de la librería desde el dispositivo
//////////////////////////////////////////////////////////////////////

bool LMLibrary::parseFile( QIODevice * device )
{
	int errLine, errCol;
	QString errStr;
	QDomDocument doc;

	// Carga del XML
	if( !doc.setContent( device, true, &errStr, &errLine, &errCol ) ){
		qWarning( "Error cargando librería en la línea %d, columna %d: %s", errLine, errCol, errStr.latin1() );
		return false;
	}

	// Autentificación de formato
	QDomElement root = doc.documentElement();
	if( root.tagName() != "library" ){
		qWarning( "Error cargando librería: El fichero no es una librería" );
		return false;
	}

	// Carga de atributos de la librería
	QString data;
	data = root.attribute( "name" );
	if( data.isNull() ){
		qWarning( "Error cargando librería: No se ha especificado el atributo 'name' de 'library'" );
		return false;
	}
	setName( data );

	data = root.attribute( "version", 0 );
	setVersion( data.toFloat() );

	data = root.attribute( "autor" );
	setAutor( data );

	data = root.attribute( "srcType", "none" );
	if( !data.isEmpty() ){

		if( !QString::compare( data.lower(), "hdl") )
			setSourceType( HDL );
		else if( !QString::compare( data.lower(), "plugin") )
			setSourceType( Plugin );
		else
			setSourceType( None );

	}
	data = root.attribute( "srcOrigin", name() );
	setSourceOrigin( data );
	
	// Carga de componentes
	QDomNode node = root.firstChild();
	while( !node.isNull() ){

		if( !QString::compare( node.toElement().tagName().lower(), "component") ){
		
			// Instanciamos un nuevo componente
			LMComponent * cmp = &(*append( LMComponent(this) ));

			// Lo especificamos a partir del nodo activo
			if( !cmp->parse( node.toElement()) )
				qWarning( "Error cargando componente" );

		}
		node = node.nextSibling();
	}

	return true;
}

bool LMLibrary::parseFile( const QString &file )
{
	QDir dir( file );
	QString path = QDir::convertSeparators(dir.absPath()); 
	
	QFile device( path );

	if( !device.open( IO_ReadOnly ) )
		return false;

	device.close();
	
	return parseFile( &device );

}

