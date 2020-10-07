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



#include "LibraryManager.h"

LibraryManager::LibraryManager()
{
}

// Carga una librería desde el fichero libraryFile
// y la inserta en la lista
LMLibrary * LibraryManager::loadLibrary(  const QString &  libraryFile )
{
	iterator it = append(LMLibrary());

	if( !(*it).parseFile(libraryFile) ){
		remove( it );
		return NULL;
	}
	return &(*it);
}

// Busca la librería de nombre libName
LMLibrary * LibraryManager::find( const QString & libName )
{
	for( iterator it = begin(); it != end(); ++it )
		if( (*it).name() == libName )
			return &(*it);
	
	return NULL;
}

// Busca el componente 'compName' en todas las librerías instanciadas
//   Para especificar un componente en una librería concreta se empleará el
//   separador de campo ':' ( LIBRERIA:COMPONENTE )
LMComponent * LibraryManager::findComponent( const QString & compName )
{
	LMComponent * retval = NULL;

	if( compName.contains(':') ){

		// Búsqueda específica
		LMLibrary * lib = find( compName.section( ':', 0, 0 ) );
		if( lib )
			retval = lib->find( compName.section( ':', 1, 1 ) );

	}else{

		// Búsqueda global (se devuelve el primer elemento que concuerde)
		for( iterator it = begin(); it != end(); ++it ){
			retval = (*it).find(compName);
			if( retval )
				break;
		}

	}

	return retval;
}
