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



#include "ConsoleDisplay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
ConsoleDisplay::ConsoleDisplay( QWidget * parent, const char * name, WFlags f )
	: QListBox( parent, name, f )
{
	this->indentation = 0;
}

//////////////////////////////////////////////////////////////////////
// Impresión de mensajes
//////////////////////////////////////////////////////////////////////
void ConsoleDisplay::postOutputMessage( const QString & msg )
{
	QString indentString;
	if( indentation >= 0 )
		indentString.fill( ' ', indentation * IndentSize );

	insertItem( "  " + indentString + msg );
	setBottomItem( count()-1 );
}

void ConsoleDisplay::postErrorMessage( const QString & msg )
{
	QString indentString;
	if( indentation >= 0 )
		indentString.fill(' ', indentation * IndentSize );

	insertItem( "! " + indentString + msg );
	setBottomItem( count()-1 );
	setCurrentItem( count()-1 );
}

void ConsoleDisplay::indentMessage( int indentation )
{	
	this->indentation += indentation;
}
