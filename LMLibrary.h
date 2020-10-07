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


// LMLibrary.h: interface for the LMLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LMLIBRARY_H_)
#define _LMLIBRARY_H_

#include <qvaluelist.h>

#include "LMComponent.h"

class LMLibrary : public QValueList<LMComponent>
{
public:
	enum SourceType // Origen de las se�ales descritas en la librer�a
	{ 
	  None,		// Se�ales libres, resoluci�n a nivel de LogicEditor
	  Plugin,	// Resoluci�n por librer�a externa
	  HDL		// Resoluci�n por el simulador
	};

	LMLibrary( );
	LMLibrary( QIODevice * file );
	LMLibrary( QString & file );
	
	// Atributos de la librer�a
	void setName( const QString &name );
	void setAutor( const QString &autor );
	void setSourceOrigin( const QString &sourceFolder );
	void setSourceType( enum SourceType st );
	void setVersion( int version );
	QString name() const;
	QString autor() const;
	QString sourceOrigin() const;
	SourceType sourceType() const;
	int version() const;

	LMComponent * find( const QString & compName );

	// Extrae la informaci�n de la librer�a desde el dispositivo device
	// o desde el fichero file
	bool parseFile( QIODevice * device );
	bool parseFile( const QString &file );

private:
	QString nm, autr;
	int vr;

	SourceType srcTp;	// Tipo de origen de las se�ales
	QString srcSrc;		// Especificaci�n del origen de las se�ales
};

#endif 
