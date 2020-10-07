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


	
#include "Project.h"

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qdom.h>

#include "Plugin.h"
#include "dlgNewProject.h"
#include "InterfaceAssistentDialog.h"
#include "InterfaceDocument.h"
#include "Document.h"

#include "Application.h"
extern Application * app;

Project::Project( QWidget * parent, const char * name )
	: QWorkspace( parent, name )
{
	isChanged = false;
}

///////////////////////////////////////////////////////////
// Control del proyecto
///////////////////////////////////////////////////////////
	
bool Project::newProject()
{
	bool accept;
	dlgNewProject dlg;
	InterfaceAssistentDialog iaDlg;

	// Diálogo principal de proyecto
	if( dlg.exec() != QDialog::Accepted )
		return false;

	// Se testea el tipo de proyecto elegido por el usuario
	if( dlg.plugin() && dlg.plugin()->type() == ptProject ){
	  // Proyecto con plantilla (origen en plugin)
		ProjectPlugin * plugin =(ProjectPlugin*)dlg.plugin();

	  // Configuración previa del plugin (a partir de la información previamente aportada por el usuario)
		plugin->setProjectFolder( dlg.path() );
		plugin->setProjectName( dlg.name() );
		
	  // Configuración avanzada del plugin
		// Si el plugin dispone de un diálogo de configuración propio se ejecuta,
		// en caso contrario se usará el diálogo de configuración estándar
		if( plugin->hasExecDialog() )
			accept = plugin->execDialog();
		else
			qDebug( "Project::newProject() TODO: Implementar el configurador estándar de plugins" );

		
	  
		if( accept ){
		// Creación del proyecto
			if( !plugin->createProject() )
				return false;
		
		
		// Apertura del proyecto creado
			if( !openProject( plugin->projectFolder(), QString("%1.%2").arg(plugin->projectName()).arg("lep") ) )
				return false;
		}
			
	}else{
	  // Proyecto en blanco

		// Configuración de atributos básicos del proyecto
		setName( dlg.name() );
		setPath( dlg.path() );
	}

	// Se notifica el cambio en el estado de la aplicación
	isChanged = true;
	emit projectCreated();
	emit projectCreated( name() );

	/*
	if( dlg.useAssistent() ){
		iaDlg.interfaceData().setBaseAddress( 0 );
		iaDlg.interfaceData().setName( dlg.name() );
		iaDlg.updateFrames();
		iaDlg.exec();
		newDocument( iaDlg.interfaceData() );
	}
	*/
	return true;
}

bool Project::closeProject()
{

	if( !queryCloseProject() )
		return false;

	// Eliminamos los documentos activos
	QWidgetList widgets = windowList();
	QWidgetListIt it( widgets );
	QWidget * w;
	while( w = it.current() ){
		if( w->inherits( "Document" ) ){
			Document *doc = (Document*) w;
			doc->close();
			// Es necesario eliminar los objetos, ya que close() no
			// los destruye, sólo los esconde
			qDebug("TODO:Project::closeProject() Corregir Document::~Document y destruir el objeto!!");
			//delete doc;
		}
		++it;
	}
		
	// Vaciamos la lista de documentos
	documents().clear();

	// Limpiamos todos los registros
	setName( QString::null );
	setAutor( QString::null );
	setPath( QString::null );
	isChanged = false;
	
	emit projectClosed();

	return true;
}

// Comprueba si es factible cerrar el proyecto y todos los documentos en función de
//   > El estado de modificación
//   > La decisión del usuario
// Si se cancela el cierre devuelve false
// Sino, devolverá true y
//       Si el usuario así lo desea se guardará todo el WorkSpace
bool Project::queryCloseProject()
{
	if( !mayBeSave() )
		return true;

	int answer = QMessageBox::question( 0, tr("Cerrando Proyecto..."), tr("El proyecto ha sido modificado ¿desea guardar los cambios?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
	
	switch( answer ){
	case QMessageBox::Yes:
	{
		// Guardamos el proyecto (completo)
		saveProject();
	
		return true;
	}
	case QMessageBox::No:
		return true;

	case QMessageBox::Cancel:
		return false;
	}

	// Retorno inalcanzable
	return true;
	
}

bool Project::openProject()
{

	QString filePath = QFileDialog::getOpenFileName(
					NULL,
					tr("Proyecto de LogicEditor (*.lep)"),
					this,
					"openProjectDialog",
					tr("Abrir proyecto...") );
	
	// Cancelación de apertura
	if( filePath.isEmpty() )
		return false;

	// Separación de ruta y fichero
	QDir dir( filePath );
	QString fileName = dir.dirName();
	dir.cdUp();
	filePath = dir.absPath();

	// Apertura efectiva
	return openProject( filePath, fileName );
}
bool Project::openProject( const QString & filePath, const QString & fileName )
{
	QDir oldCur;

	// Apertura del fichero
	oldCur = QDir::current();
	QDir::setCurrent( filePath );
	QFile device( fileName );
	if( !device.open( IO_ReadOnly ) ){
		QMessageBox::critical( 0, tr("Abrir proyecto"), tr("No es posible abrir el fichero") );
		QDir::setCurrent( oldCur.absPath() );
		return false;
	}

	// Carga del contenido XML
	if( !loadProject( &device ) ){
		QMessageBox::critical( 0, tr("Abrir proyecto"), tr("Formato de fichero erróneo.") );
		QDir::setCurrent( oldCur.absPath() );
		return false;
	}

	// Actualización del estado 
	isChanged = false;

	emit projectOpened();
	emit projectOpened( name() );

	// Emite la lista de documentos cargados
	for( QStringList::ConstIterator it = documents().begin(); it != documents().end(); ++it )
		emit documentCreated( *it );

	return true;
}

bool Project::loadProject( QIODevice * device )
{
	int errLine, errCol;
	QString errStr;
	QDomDocument doc;

	// Carga del XML
	if( !doc.setContent( device, true, &errStr, &errLine, &errCol ) ){
		qWarning( tr("Error cargando el modelo en la línea %d, columna %d: %s"), errLine, errCol, errStr.latin1() );
		return false;
	}

	// Autentificación de formato
	QDomElement root = doc.documentElement();
	if( root.tagName().lower() != "project" ){
		qWarning( tr("Error cargando proyecto: no es un fichero de proyecto reconocido."));
		return false;
	}

	// Carga de atributos del proyecto
	QString data;
	
	// Nombre
	data = root.attribute( "name", "unknown" );
	setName( data );

	// Versión
	data = root.attribute( "autor", QString::null );
	setAutor( data );

	// Carga de documentos
	QDomNode node = root.firstChild();
	while( !node.isNull() ){
		if( node.toElement().tagName().lower() == "document"){
			data = node.toElement().attribute( "name", QString::null );
			if( data != QString::null )
				documents().append( data );
		}
			
		node = node.nextSibling();
	}

	return true;
}

bool Project::saveProject()
{
	// Guardamos los documentos (modelos) no guardados
	QWidgetList widgets = windowList();
	QWidgetListIt it( widgets );
	QWidget * w;
	while( w = it.current() ){
		if( w->inherits( "Document" ) ){
			Document *doc = (Document*) it.current();
			if( doc->mayBeSave() )
				doc->save();
		}
		++it;
	}

	bool ok = saveProjectFile();
	
	if( ok )
		emit projectSaved();

	return ok;
}

bool Project::saveProjectFile()
{
	QString strVal;

	// Apertura del fichero
	QDir::setCurrent( path() );
	
	QFile prjFile( QString("%1.lep").arg(name()) );
	prjFile.open( IO_WriteOnly );

	// Escritura del fichero
	QTextStream stream( &prjFile );
	
		// Apertura y atributos (XML)
		stream << "<project name=\"" << name() 
			   << "\"autor=\"" << autor() << "\">\r\n";

		// Documentos (XML)
		for( QStringList::ConstIterator it = documents().begin(); it != documents().end(); ++it )
			stream << "\t<document name=\""<< *it << "\"></document>\r\n";

		// Cierre (XML)
		stream << "</project>\r\n";


	prjFile.close();

	isChanged = false;
	return true;
}

///////////////////////////////////////////////////////////
// Control de cambios
///////////////////////////////////////////////////////////
bool Project::mayBeSave() const
{
	if( isChanged )
		return true;
	
	// Guardamos los documentos (modelos) no guardados
	QWidgetList widgets = windowList();
	QWidgetListIt it( widgets );
	QWidget * w;
	while( w = it.current() ){
		if( w->inherits( "Document" ) )
			if( ((Document*)it.current())->mayBeSave() )
				return true;
		++it;
	}

	return false;
}


///////////////////////////////////////////////////////////
// Atributos del proyecto
///////////////////////////////////////////////////////////
QString Project::path() const
{
	return pth;
}

bool Project::setPath( const QString & pth )
{
	if( !pth.isEmpty() )
		if( !QDir::setCurrent( pth ) )
			return false;
	
	this->pth = pth;
	isChanged = true;
	return true;
}

void Project::setName( const QString & nm )
{
	QObject::setName( nm );
	
	if( parent() )
		if( parent()->isWidgetType() )
			((QWidget*)parent())->setCaption( nm );

	isChanged = true;
}

QString Project::autor() const
{
	return at;
}

void Project::setAutor( const QString & at )
{
	this->at = at;
	isChanged = true;
}

///////////////////////////////////////////////////////////
// Documentos del proyecto (abiertos o no)
///////////////////////////////////////////////////////////
bool Project::newDocument()
{
	Document * doc = new Document( this );

	if( !doc->createNew() ){
		delete doc;
		return false;
	}
	
	documents().append( doc->name() );
	connect( doc, SIGNAL(renamed(const QString&, const QString&)), this, SLOT(renameDocument(const QString&, const QString&)) );
	
	// Redifusión de mensajes
	connect( doc->hdlGenerator(), SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( doc->hdlGenerator(), SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );
	
	doc->show();
	
	isChanged = true;

	emit documentCreated( doc->name() );
	emit documentInstancied( doc );

	return true;
}

bool Project::newDocument( const IAInterface & iface )
{
	InterfaceDocument * doc = new InterfaceDocument( iface, this );

	if( !doc->createNew() ){
		delete doc;
		return false;
	}

	documents().append( doc->name() );
	connect( doc, SIGNAL(renamed(const QString&, const QString&)), this, SLOT(renameDocument(const QString&, const QString&)) );
	
	// Redifusión de mensajes
	connect( doc->hdlGenerator(), SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( doc->hdlGenerator(), SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );
	
	doc->show();
	
	isChanged = true;

	emit documentCreated( doc->name() );
	emit documentInstancied( doc );

	doc->createInterface();

	return true;
}

bool Project::openDocument()
{
	Document * doc = new Document( this, "Untitled" );
	
	if( !doc->open() ){
		delete doc;
		return false;
	}

	// Garantizamos la unicidad de nombres
	if( documents().contains( doc->name() ) ){
		QString newName;
		int i=1;
		do{
			newName = QString("%1%2").arg(doc->name()).arg(i++);
		}while( documents().contains( newName ) );

		QMessageBox::warning( 0, tr("Abrir documento..."), QString(tr("Ya existe un modelo llamado '%1' en este proyecto.\nEl nuevo modelo será renombrado como '%2'")).arg(doc->name()).arg(newName) );
		
		doc->setName( newName );
		doc->setUnchanged();
	}
	
	// Nombre en el proyecto
	documents().append( doc->name() );
	connect( doc, SIGNAL(renamed(const QString&, const QString&)), this, SLOT(renameDocument(const QString&, const QString&)) );
	
	// Redifusión de mensajes
	connect( doc->hdlGenerator(), SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( doc->hdlGenerator(), SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );
	
	doc->show();

	isChanged = true;

	emit documentCreated( doc->name() );
	emit documentInstancied( doc );

	return true;
}


QStringList & Project::documents()
{
	return docs;
}

// Busca el documento docName entre los documentos activos.
// Además, si se especifica mustLoad (por defecto=FALSE), el documento se
//   intentará instanciar. En caso de éxito, si está activo mustShowWhenLoad
//   el documento será visible, en otro caso permanecerá oculto.
//
// Devuelve NULL si en ningún caso se consigue cargar el documento
Document * Project::findDocument( const QString& docName, bool mustLoad, bool mustShowWhenLoad )
{
	Document * doc=NULL;
	QWidgetList widgets = windowList();

	// Búsqueda de documentos cargados
	QWidgetListIt it( widgets );
	QWidget * w;
	while( w = it.current() ){
		
		if( w->inherits( "Document" ) )
			if( !QString::compare(docName, w->name()) ){
				doc = (Document*)w;
				break;
			}
		
		++it;
	}

	// Intento de carga del documento
	if( !doc && mustLoad )
		doc = showDocument( docName );
		
	// Establece si el documento RECIÉN CARGADO será visible
	if( doc && mustLoad)
		if( mustShowWhenLoad )
			doc->show();
		else
			doc->hide();

	return doc;
}

Document * Project::showDocument( const QString& docName )
{
	Document * doc;
	
	// Verificamos que el documento pertenece al proyecto
	if( !documents().contains( docName ) )
		return NULL;

	// Comprobamos si el documento ya está en memoria
	doc = findDocument( docName );
	if( doc ){
		doc->show();
		return doc;
	}

	doc = new Document( this );
	doc->initialize();
	doc->setName( docName );
	doc->load();
	doc->setUnchanged();

	connect( doc, SIGNAL(renamed(const QString&, const QString&)), this, SLOT(renameDocument(const QString&, const QString&)) );
	
	// Redifusión de mensajes
	connect( doc->hdlGenerator(), SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( doc->hdlGenerator(), SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );
	
	doc->show();
	
	// Comunicamos al exterior la instanciación de un nuevo documento
	emit documentInstancied( doc );
	
	return doc;
}

// Manejador de renombrado de documento: actualiza su nombre en la lista de documentos
void Project::renameDocument( const QString& oldName, const QString &newName )
{
	QStringList::iterator item = documents().find( oldName );

	if( item != documents().end() ){
		*item = newName;
		isChanged = true;

		emit documentRenamed( oldName, newName );
	}
}

bool Project::removeDocument( const QString& docName )
{
	Document * doc = findDocument( docName );

	if( !doc ){
		// El documento no está cargado en memoria, se elimina sin más
		documents().remove( docName );

		emit documentRemoved( docName );

	}else
		// El documento está cargado en memoria, solicitud de cierre
		if( doc->close() ){
		
			// Eliminación del documento en el proyecto
			documents().remove( docName );

			// Liberación de recursos del documento
			delete doc;

			isChanged = true;
			
			emit documentRemoved( docName );

			return true;
		}

	return false;
}

void Project::zoomIn()
{
	if( activeWindow() && activeWindow()->inherits( "Document" ) ){
	
		Document * doc = (Document*)activeWindow();
		doc->zoomIn();

	}
}

void Project::zoomOut()
{
	if( activeWindow() && activeWindow()->inherits( "Document" ) ){
	
		Document * doc = (Document*)activeWindow();
		doc->zoomOut();

	}
}

///////////////////////////////////////////////////////////
// Compilación: generación de HDL
//		El código HDL y las listas de señales se generan
//		durante un único proceso (segmentado) en la clase 
//		HDLGenerator, así que ambos resultados se almacenan
//		siempre que se invoca alguno de los métodos del
//		juego HDL.
///////////////////////////////////////////////////////////
// Accede al directorio dirName ubicado en where, creándolo si no existe.
// Si dir está establecido, se copia la entrada de directorio en él.
bool Project::tryEnterDir( const QString& where, const QString& dirName, QDir * dir )
{
	QDir resDir( where );
	if( !resDir.cd( dirName ) ){
		resDir.mkdir( dirName );
		if( !resDir.cd( dirName ) ){
			return false;
		}
	}
	
	QDir::setCurrent( resDir.absPath() );
	
	if( dir )
		*dir = resDir;

	return true;
}

// Construye el fichero de HDL asociado al documento en el directorio HDL del proyecto
bool Project::buildModelHDLFile( Document * doc )
{
	// Creación de la ruta para VHDL en el proyecto.
	if( !tryEnterDir( path(), VHDL_PROJECT_FOLDER ) ){
		emit errorMessage( tr(" No se puede crear el directorio %1 en %2").arg(VHDL_PROJECT_FOLDER).arg(path()) );
		return false;
	}

	// Fichero destino
	QString filename = QString("%1.vhd").arg(doc->name());
	QFile file( filename );
	if( !file.open( IO_WriteOnly ) ){
		emit errorMessage( tr(" No se puede crear el fichero %1").arg(filename) );
		return false;
	}
	
	return doc->hdlGenerator()->buildHDL( doc->name(), (QTextOStream*)&QTextStream(&file) );
}

// Construye el fichero SIGNALS asociado al documento en el directorio SIM del proyecto
bool Project::buildModelSignalsFile( Document * doc )
{
	// Creación de la ruta para SIM en el proyecto.
	if( !tryEnterDir( path(), SIM_PROJECT_FOLDER ) ){
		emit errorMessage( tr(" No se puede crear el directorio %1 en %2").arg(SIM_PROJECT_FOLDER).arg(path()) );
		return false;
	}

	// Fichero destino
	QString filename = QString("%1.sig").arg(doc->name());
	QFile file( filename );
	if( !file.open( IO_WriteOnly ) ){
		emit errorMessage( tr(" No se puede crear el fichero %1").arg(filename) );
		return false;
	}
	
	return doc->hdlGenerator()->buildSignalsFile( doc->name(), (QTextOStream*)&QTextStream(&file) );
}

// Construye el documento docName ignorando si está al día
//   Devuelve true si se completa con éxito
bool Project::buildModelHDL( const QString& docName )
{	
	// Busca el documento, cargándolo si es preciso, aunque manteniéndolo oculto en tal caso
	Document * doc = findDocument( docName, true, false );
	if( !doc ){
		emit errorMessage( tr(" No se puede acceder al documento %1. Imposible continuar.").arg(docName) );
		return false;
	}
				
	if( !buildModelHDLFile( doc ) ){
		emit errorMessage( tr(" Fallo al construir HDL. Imposible continuar") );
		return false;
	}

	if( !buildModelSignalsFile( doc ) ){
		emit errorMessage( tr(" Fallo al construir fichero de señales. Pueden haber problemas durante la simulación.") );
		return false;
	}

	return true;
}
// Construye todos los documentos de la lista ignorando si ya están al día
//   Devuelve true si al menos un documento no fracasa (o no hay ninguno)
bool Project::buildModelsHDL( const QStringList & docs )
{
	bool retval=false;
	
	if( docs.isEmpty() )
		return true;

	// Generamos el HDL de todos los documentos
	for( QStringList::const_iterator it = docs.begin(); it != docs.end(); ++it ){
		emit outputMessage(tr( "Analizando %1...").arg(*it) );
		emit indentMessage( 1 );
		if( buildModelHDL( *it ) )
			retval = true;
		emit indentMessage( -1 );
	}

	return retval;
}

// Construye el documento activo
//   Devuelve true si se completa con éxito
bool Project::buildActiveModelHDL()
{
	if( ! (activeWindow() && activeWindow()->inherits( "Document" )) )
		return false;
	
	// Guardamos el documento para actualizar las fechas
	Document * doc = (Document*)activeWindow();
	if( doc->mayBeSave() )
		doc->save();

	emit outputMessage( tr("Construyendo componente %1").arg(doc->name()) );

	// Construimos su VHDL si es preciso
	if( !checkHDLuptoDate( doc->name() ) ){
		emit indentMessage( 1 );
		bool retval = buildModelHDL( doc->name() );
		emit indentMessage( -1 );
		return retval;
	}else{
		emit outputMessage( tr("El componente %1 está al día. No se hace nada.").arg(doc->name()) );
		return true;
	}

	emit outputMessage("");
} 

// Construye todos los documentos, si alguno no ha sido instanciado, se carga.
//   Devuelve true si al menos un documento no fracasa (o no hay ningún documento por construir)
bool Project::buildAllHDL()
{
	bool retval=false;
	
	// Guardamos todos los documentos activos modificados
	saveProject();

	emit outputMessage( tr("Construyendo todos los componentes") );

	// Generamos los objetivos a construir
	QStringList targets;
	for( QStringList::iterator it = documents().begin(); it != documents().end(); ++it )
		if( !checkHDLuptoDate( *it ) )
			targets.append( *it );
		else
			emit outputMessage( tr("El componente %1 está al día. No se hace nada.").arg( *it ) );
	
	if( targets.isEmpty() )
		return true;

	// Construcción
	emit indentMessage( 1 );
	retval = buildModelsHDL( targets );
	emit indentMessage( -1 );

	emit outputMessage("");

	return retval;
}

///////////////////////////////////////////////////////////
// Compilación: generación de binarios (ModelTech)
///////////////////////////////////////////////////////////

// Compilación efectiva. Se asume que todos los documentos de docs
// existen, han sido construidos sus correspondientes códigos VHDL
// y requieren ser compilados.
bool Project::compileModels( const QStringList & docs )
{
	if( docs.count() == 0 )
		return true;

  //Preparación de datos
	// Rutas
	QString base = QDir::convertSeparators(path());
	QString binPath = QString("%1%2%3").arg(base).arg(QDir::separator()).arg(BIN_PROJECT_FOLDER);
	QString vhdlPath = QString("%1%2%3").arg(base).arg(QDir::separator()).arg(VHDL_PROJECT_FOLDER);

	
	// Objetivos: modelos a compilar
	QStringList targets;
	for(QStringList::const_iterator it = docs.begin(); it != docs.end(); ++it )
		targets.append( QString("%1%2%3.vhd").arg(vhdlPath).arg(QDir::separator()).arg(*it) );

	// Construcción de la librería del proyecto
	QDir dir( path() );
	if( !dir.cd( BIN_PROJECT_FOLDER ) )
		// La librería no existe, se crea y se indican los objetivos (no será necesario invocar la compilación explícitamente)
		return app->HDLEngine().createSourceLibrary( binPath, targets );

	// Compilación explícita de los objetivos del proyecto
	return app->HDLEngine().buildProject( targets, binPath );
}

// Método facilitado por comodidad, funciona esencialmente como el
// anterior.
bool Project::compileModel( const QString & doc )
{
	return compileModels( QStringList( doc ) );
}

// Compila el documento activo (si existe). Si su código HDL no
// está al día se construye.
//    Devuelve true si no fracasa (o si ya está al día)
bool Project::compileActiveModel()
{
	if( !activeWindow() || !activeWindow()->inherits( "Document" ) )
		return false;
	
	Document * doc = (Document*)activeWindow();
	
	emit outputMessage( tr("Compilando %1...").arg(doc->name()) );
	
	// Se intenta construir su código HDL
	emit indentMessage( 1 );
	if( !buildActiveModelHDL() ){
		emit errorMessage( tr("No se puede compilar %1. Se obtuvo un error construyendo %1.hdl").arg(doc->name()) );
		emit indentMessage( -1 );
		return false;
	}
	emit indentMessage( -1 );

	// Se chequea si el componente está al día
	if( checkBINuptoDate( doc->name() ) ){
		emit outputMessage( tr("No se hace nada, '%1' está al día.").arg(doc->name()) );
		return true;
	}
	
	// Finalmente se compila
	bool retval = compileModel( doc->name() );

	emit outputMessage("");

	return retval;
} 

// Tras guardar el proyecto, construye el código HDL de los documentos modificados
// e invoca su compilación.
//   Devuelve true si al menos un modelo es compilado con éxito
bool Project::compileAll()
{
	emit outputMessage( tr("Compilando todos los componentes...") );
	emit indentMessage( 1 );

	// Construimos todos los componentes HDL
	if( !buildAllHDL() ){
		emit indentMessage( -1 );
		emit errorMessage( tr("No se pudieron compilar los componentes: Error al construir sus ficheros HDL.") );
		return false;
	}

	emit indentMessage( -1 );
	// Compilando dependencias
	if( !compileDependences() )
		emit errorMessage( tr("Error compilando dependencias... (omitido)") );

	// Lista de objetivos
	QStringList targets;
	for( QStringList::iterator it = documents().begin(); it != documents().end(); ++it )
		if( !checkBINuptoDate( *it ) )
			targets.append( *it );

	if( targets.isEmpty() ){
		emit outputMessage( tr(" No se hace nada, Todos los componentes están al día.") );
		return true;
	}

	// Compilación efectiva
	emit indentMessage( 1 );
	bool retval = compileModels( targets );
	emit indentMessage( -1 );

	emit outputMessage("");

	return retval;
}

bool Project::compileDependences()
{
	// Lista de objetivos
	QPtrList<LMLibrary> libs;

	emit outputMessage( tr("Compilando dependencias del proyecto...") );
	emit indentMessage( 1 );
	emit outputMessage( tr("Calculando dependencias...") );
	
	// Para cada documento del proyecto
	for( QStringList::iterator it = documents().begin(); it != documents().end(); ++it ){

		// Busca el documento, cargándolo si es preciso, aunque manteniéndolo oculto en tal caso
		Document * doc = findDocument( *it, true, false );
		if( doc ){
		
			// Para cada componente del documento, se accede a su librería y se inserta en la lista libs
			emit outputMessage( tr("Analizando %1...").arg(*it) );
			emit indentMessage( 1 );
			QPtrList<LMComponent> comps = doc->hdlGenerator()->componentDependences();
			emit indentMessage( -1 );

			for( LMComponent * itComp = comps.first(); itComp; itComp = comps.next() )
				if( !libs.contains( itComp->parentLibrary() ) )
					libs.append( itComp->parentLibrary() );
					

		}else
			emit errorMessage( tr("No se puede acceder al documento %1 (El docuemnto será ignorado)").arg(*it) );
	}

	// Compilación de las librerías
	for( LMLibrary * itLib = libs.first(); itLib; itLib = libs.next() ){

		emit outputMessage( tr("  Compilando librería %1...").arg(itLib->name()) );

		QString libPath = QDir::convertSeparators(app->vhdlPath()) + QDir::separator() + itLib->sourceOrigin();
		
		emit indentMessage( 1 );
		if( !app->HDLEngine().createSourceLibrary( libPath ) )
			emit errorMessage( tr("Imposible compilar la librería %1 en %2").arg(itLib->name()).arg(libPath) );
		emit indentMessage( -1 );
	}
	
	emit indentMessage( -1 );
	emit outputMessage("");
	return true;
}

bool Project::checkHDLuptoDate(const QString & doc)
{
	QDir projectDir( path() );

	// La carpeta vhdl del proyecto todavía no existe
	if( !projectDir.cd( VHDL_PROJECT_FOLDER ) )
		return false;

	// El fichero documento.vhd todavía no existe
	QFileInfo vhdFile( projectDir, QString("%1.vhd").arg(doc) );
	if( !vhdFile.exists() )
		return false;

	// Volvemos al directorio base del proyecto
	projectDir.cdUp();

	// El fichero documento.vhd no está al día respecto al modelo
	QFileInfo lemFile( projectDir, QString("%1.lem").arg(doc) );

	return (vhdFile.lastModified() >= lemFile.lastModified());
	
}

// Esta función es dependiente del compilador!!!
// Modificar en caso de reemplazar compilador
bool Project::checkBINuptoDate(const QString & doc)
{
	QDir projectDir( path() );

	// La carpeta BIN del proyecto todavía no existe
	if( !projectDir.cd( BIN_PROJECT_FOLDER ) )
		return false;

	// El fichero bin\documento todavía no existe
	QFileInfo binFile( projectDir, doc );
	if( !binFile.exists() )
		return false;

	// Comparación con la última modificación del vhd
	projectDir.cdUp();
	if( projectDir.cd( VHDL_PROJECT_FOLDER ) ){
	
		// El fichero documento.vhd no está al día respecto al modelo
		QFileInfo vhdFile( projectDir, QString("%1.vhd").arg(doc) );

		return (binFile.lastModified() >= vhdFile.lastModified());
	}else
		return true;
	
}
