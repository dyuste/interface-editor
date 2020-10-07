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


// Project.h: interface for the Project class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PROJECT_H_)
#define _PROJECT_H_

#include <qworkspace.h>

class Document;
class IAInterface;
class QDir;

#define VHDL_PROJECT_FOLDER "vhdl"
#define BIN_PROJECT_FOLDER "bin"
#define SIM_PROJECT_FOLDER "sim"

class Project : public QWorkspace
{
Q_OBJECT

public:
	enum{ MagicNumber=0xDA2EB735 };
	Project( QWidget * parent=0, const char * name = 0 );
	
	bool mayBeSave() const;

	// Atributos del proyecto
	QString path() const;
	bool setPath( const QString & pth );

	void setName( const QString & nm );
	
	QString autor() const;
	void setAutor( const QString & at );

	// Busca el documento docName entre los documentos activos.
	// Además, si se especifica mustLoad (por defecto=FALSE), el documento se
	//   intentará instanciar. En caso de éxito, si está activo mustShowWhenLoad
	//   el documento será visible, en otro caso permanecerá oculto.
	//
	// Devuelve NULL si en ningún caso se consigue cargar el documento
	Document * findDocument( const QString& docName, bool mustLoad=false, bool mustShowWhenLoad=false );


protected:
	QStringList& documents();
	
public slots:
	// Proyecto
	bool newProject();
	bool closeProject();
	bool queryCloseProject();
	bool openProject();
	bool openProject( const QString & filePath, const QString & fileName );
	bool loadProject( QIODevice * device );
	bool saveProject();
	
	// Documentos
	bool newDocument();
	bool newDocument( const IAInterface & iface );
	bool openDocument();

	// Configuración visual de documentos
	void zoomIn();
	void zoomOut();

	Document * showDocument( const QString& docName );
	void renameDocument( const QString& oldName, const QString &newName );
	bool removeDocument( const QString& docName );

	// Compilación (HDL)
	bool buildActiveModelHDL();
	bool buildAllHDL(); 

	// Compilación (Binaria)
	bool compileActiveModel();
	bool compileAll();

	bool compileDependences();
	
signals:
	// Eventos por manipulación del proyecto
	void projectCreated();
	void projectCreated( const QString& docName );
	void projectOpened();
	void projectOpened( const QString& docName );
	void projectSaved();
	void projectClosed();
	
	// Eventos por manipulación de documentos
	void documentInstancied( Document * doc );
	void documentCreated( const QString& docName );
	void documentRenamed( const QString& oldName, const QString &newName );
	void documentRemoved( const QString& docName );

	// Mensajes
	void errorMessage(const QString&);
	void outputMessage(const QString&);
	void indentMessage(int indentation);

protected:
  // Construcción de HDL y listas de señales
	// Construcción efectiva
	bool buildModelSignalsFile( Document * doc );
	bool buildModelHDLFile( Document * doc );
	bool buildModelHDL( const QString & doc );
	bool buildModelsHDL( const QStringList & docs );

	// Compilación (Binaria)
	bool compileModels( const QStringList & docs );
	bool compileModel( const QString & doc );

	// Comprobación de cambios entre ficheros
	bool checkHDLuptoDate(const QString & doc);
	bool checkBINuptoDate(const QString & doc);
	
	// Utilidades: Manejo de directorios
	bool tryEnterDir( const QString& where, const QString& dirName, QDir * dir=0 );

private:
	bool saveProjectFile();

	bool isChanged;

	QString pth, at;
	QStringList docs;
};

#endif 
