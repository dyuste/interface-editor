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


// Application.cpp: implementation of the Application class.
//
//////////////////////////////////////////////////////////////////////

#include "Application.h"

#include <qdir.h>
#include <qlibrary.h>

#ifdef _WIN32
#define PLUGIN_FILTER "*.dll"
#else
#define PLUGIN_FILTER "*"
#endif

//////////////////////////////////////////////////////////////////////
// Construccion
//////////////////////////////////////////////////////////////////////
Application::Application( int & argc, char ** argv )
: QApplication( argc, argv )
{

/* Configuración de rutas */
	// Directorio base (el directorio inicial de ejecución)
	QDir path = QDir::current();
	setBasePath( path.path() );
	
	// Directorio de Librerías de Componentes
	if( !path.cd( LIB_APP_FOLDER ) ){
		path.mkdir( LIB_APP_FOLDER );
		path.cd( LIB_APP_FOLDER );
	}
	setLibraryPath( path.path() );

	// Directorio de Fuentes para las Librerías de Componentes
	path = QDir::current();
	if( !path.cd( VHDL_APP_FOLDER ) ){
		path.mkdir( VHDL_APP_FOLDER );
		path.cd( VHDL_APP_FOLDER );
	}
	setVhdlPath( path.path() );

	// Directorio de Plugins
	path = QDir::current();
	if( !path.cd( PLUGIN_APP_FOLDER ) ){
		path.mkdir( PLUGIN_APP_FOLDER );
		path.cd( PLUGIN_APP_FOLDER );
	}
	setPluginPath( path.path() );

/* Carga de Plugins*/
	createPluginList();

}

//////////////////////////////////////////////////////////////////////
// Objetos de la aplicación
//////////////////////////////////////////////////////////////////////
LibraryManager & Application::libraryManager()
{
	return libManager;
}

HDLAssembler & Application::HDLEngine()
{
	return HDLeng;
}

//////////////////////////////////////////////////////////////////////
// Atributos: rutas
//////////////////////////////////////////////////////////////////////

QString Application::basePath() const
{
	return pthBase;
}

void Application::setBasePath( const QString & path )
{
	pthBase = path;
}


QString Application::libraryPath() const
{
	return pthLib;
}

void Application::setLibraryPath( const QString & path )
{
	pthLib = path;
}


QString Application::vhdlPath() const
{
	return pthVhd;
}

void Application::setVhdlPath( const QString & path )
{
	pthVhd = path;
}

QString Application::pluginPath() const
{
	return pthPlugin;
}

void Application::setPluginPath( const QString & path )
{
	pthPlugin = path;
}

//////////////////////////////////////////////////////////////////////
// Plugins 
//////////////////////////////////////////////////////////////////////

// Inicializa (o recarga) la lista de plugins actualmente disponbiles
void Application::createPluginList()
{
	// Guarda el path actual y cambia al path de plugins
	QDir path( pluginPath() );
	
	// Se obtiene una lista con las DLL del path
	path.setNameFilter( PLUGIN_FILTER );
	const QFileInfoList *list = path.entryInfoList();
	QFileInfoListIterator it( *list );
	QFileInfo *fi;
	
	// Se carga cada uno de los plugins encontrados
	while( (fi=it.current()) != 0 ){
		if( Plugin * plugin = loadPlugin( fi->absFilePath() ) )
			plugins.append(plugin);

		++it;
	}

}

// Carga el plugin especificado en 'filename'
Plugin * Application::loadPlugin( const QString & filename )
{
	QLibrary lib( filename );
	lib.setAutoUnload( false );
	Plugin * plugin;

	// Carga Ordinal
	plugin = (Plugin*)lib.resolve((const char*)1);
	
	if( plugin )
		qDebug( tr("Cargando plugin \"")+filename+tr(" (")+plugin->name()+tr(")... cargado") );
	else
		qDebug( tr("Cargando plugin \"")+filename+tr("\"... fallo") );

	return plugin;
}

// Devuelve una lista con los plugins actualmente disponibles
QValueList<Plugin*> Application::getPluginList()
{
	return plugins;
}

// Devuelve la lista con los plugins actualmente disponibles de tipo 'filter'
QValueList<Plugin*> Application::getPluginList( PluginType filter )
{
	QValueList<Plugin*> retval;

	// Se seleccionan solo los elementos que coinciden con el filtro
	for( QValueList<Plugin*>::iterator it=plugins.begin(); it!=plugins.end(); it++ )
		if( (*it)->type() == filter )
			retval.append(*it);
	
	return retval;
}