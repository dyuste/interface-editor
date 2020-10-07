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



#if !defined(_LIBRARYMANAGER_H_)
#define _LIBRARYMANAGER_H_

#include <qvaluelist.h>

#include "LMLibrary.h"

class LibraryManager : QValueList<LMLibrary>
{
public:
	LibraryManager();
	
	// Carga una librería desde el fichero libraryFile
	// y la inserta en la lista
	LMLibrary * loadLibrary( const QString & libraryFile );
	
	// Busca la´librería de nombre libName
	LMLibrary * find(  const QString &  libName );

	// Busca el componente 'compName' en todas las librerías instanciadas
	LMComponent * findComponent( const QString & compName );
};

#endif 
