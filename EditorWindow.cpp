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



#include "EditorWindow.h"

#include <qaction.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qdir.h>
#include <qworkspace.h>
#include <qmessagebox.h>
#include <qpixmap.h>

#include "ComponentSelector.h"
#include "ConsoleDisplay.h"
#include "PropertiesBox.h"
#include "ProjectView.h"

#include "Document.h"
#include "Project.h"

#include "LEDevice.h"

#include "Application.h"

extern Application * app;

EditorWindow::EditorWindow() : QMainWindow()
{
	setIcon( QPixmap::fromMimeSource("img/icon-prj.png") );
	createWorkspace();

	createActions();

	createMenuBar();

	createToolBoxes();

	createToolBars();

	// Carga de librerías
	loadLibraries();

	enableProjectFeatures( false );

}

void EditorWindow::createWorkspace()
{
	workspace = new Project(this, "Workspace");
	setCentralWidget( workspace );
	
	connect( workspace, SIGNAL(projectClosed()), this, SLOT(afterProjectClosed()) );
	connect( workspace, SIGNAL(projectOpened()), this, SLOT(afterProjectOpened()) );
	connect( workspace, SIGNAL(projectCreated()), this, SLOT(afterProjectCreated()) );
	connect( workspace, SIGNAL(projectSaved()), this, SLOT(afterProjectSaved()) );
	connect( workspace, SIGNAL(documentInstancied(Document*)), this, SLOT(afterDocumentInstancied(Document*)) );
}

void EditorWindow::createToolBoxes()
{
  // Ventana Flotante para el selector de componentes
	QDockWindow * dw1 = new QDockWindow( this, "ComponentSelectorDockWindow" );
	dw1->setCaption( tr("Componentes") );
	dw1->setOrientation( Qt::Vertical );
	dw1->setResizeEnabled( true );
	
	addDockWindow( dw1, DockRight );
	
	setDockEnabled( dw1, DockTop, false );
	setDockEnabled( dw1, DockBottom, false );
	
	// Selector de componentes
	lpSelector = new ComponentSelector( dw1, "ComponentSelector" );
	connect( lpSelector, SIGNAL(componentClicked(LMComponent*)), this, SLOT(onComponentClicked(LMComponent*)) );

	dw1->setWidget( lpSelector );

  // Ventana Flotante para la consola
	QDockWindow * dw2 = new QDockWindow( this, "ConsoleDisplayDockWindow" );
	dw2->setCaption( tr("Consola") );
	dw2->setOrientation( Qt::Horizontal );
	dw2->setResizeEnabled( true );
	dw2->setFixedExtentHeight( 100 );

	addDockWindow( dw2, DockBottom );
	
	setDockEnabled( dw2, DockLeft, false );
	setDockEnabled( dw2, DockRight, false );
	
	// Consola
	lpConsole = new ConsoleDisplay( dw2, "ConsoleDisplay" );
	connect( &app->HDLEngine(), SIGNAL(errorMessage(const QString&)), lpConsole, SLOT(postErrorMessage(const QString&)) );
	connect( &app->HDLEngine(), SIGNAL(outputMessage(const QString&)), lpConsole, SLOT(postOutputMessage(const QString&)) );
	connect( workspace, SIGNAL(errorMessage(const QString&)), lpConsole, SLOT(postErrorMessage(const QString&)) );
	connect( workspace, SIGNAL(outputMessage(const QString&)), lpConsole, SLOT(postOutputMessage(const QString&)) );
	connect( workspace, SIGNAL(indentMessage(int)), lpConsole, SLOT(indentMessage(int)) );

	dw2->setWidget( lpConsole );	

	// Ventana Flotante para el navegador de proyecto
	QDockWindow * dw4 = new QDockWindow( this, "ProjectViewDockWindow" );
	dw4->setCaption( tr("Navegdor del Proyecto") );
	dw4->setOrientation( Qt::Vertical );
	dw4->setResizeEnabled( true );
	dw4->setFixedExtentWidth( 180 );

	addDockWindow( dw4, DockLeft );
	
	setDockEnabled( dw4, DockTop, false );
	setDockEnabled( dw4, DockBottom, false );
	
	// Navegador de proyecto
	lpProjectView = new ProjectView( dw4, "ProjectView" );
	connect( workspace, SIGNAL(projectClosed()), lpProjectView, SLOT(clear()) );
	connect( workspace, SIGNAL(projectOpened(const QString&)), lpProjectView, SLOT(setProjectName(const QString&)) );
	connect( workspace, SIGNAL(projectCreated(const QString&)), lpProjectView, SLOT(setProjectName(const QString&)) );
	connect( workspace, SIGNAL(documentRenamed(const QString&,const QString&)), lpProjectView, SLOT(replaceDocument(const QString&,const QString&)) );
	connect( workspace, SIGNAL(documentCreated(const QString&)), lpProjectView, SLOT(insertDocument(const QString&)) );
	connect( workspace, SIGNAL(documentRemoved(const QString&)), lpProjectView, SLOT(removeDocument(const QString&)) );
	connect( lpProjectView, SIGNAL(usrDocumentOpened(const QString&)), workspace, SLOT(showDocument(const QString&)) );
	connect( lpProjectView, SIGNAL(usrDocumentDeleted(const QString&)), workspace, SLOT(removeDocument(const QString&)) );
	
	dw4->setWidget( lpProjectView );	


	// Ventana Flotante para la caja de propiedades
	QDockWindow * dw3 = new QDockWindow( this, "PropertiesBoxDockWindow" );
	dw3->setCaption( tr("Propiedades") );
	dw3->setOrientation( Qt::Vertical );
	dw3->setResizeEnabled( true );
	dw3->setFixedExtentWidth( 180 );

	addDockWindow( dw3, DockLeft );
	
	setDockEnabled( dw3, DockTop, false );
	setDockEnabled( dw3, DockBottom, false );
	
	// Caja de propiedades
	lpPropertiesBox = new PropertiesBox( dw3, "PropertiesBox" );
	connect( this, SIGNAL(itemSelected(QObject*)), lpPropertiesBox, SLOT(setTargetItem(QObject*)) );
	
	dw3->setWidget( lpPropertiesBox );	
}
	
void EditorWindow::createActions( )
{
	acDrawWire = new QAction( tr("Dibujar &Cable"), tr("Ctrl+C"), this );
	acDrawWire->setIconSet( QPixmap::fromMimeSource("img/draw-wl.png") );
	connect( acDrawWire, SIGNAL(activated()), this, SLOT(drawWireLine()) );
	
	// Menu File
	acNewProject = new QAction( tr("&Nuevo Proyecto..."), tr(""), this );
	acNewProject->setIconSet( QPixmap::fromMimeSource("img/prj-new.png") );
	connect( acNewProject, SIGNAL(activated()), workspace, SLOT(newProject()) );

	acOpenProject = new QAction( tr("&Abrir Proyecto..."), tr(""), this );
	acOpenProject->setIconSet( QPixmap::fromMimeSource("img/prj-open.png") );
	connect( acOpenProject, SIGNAL(activated()), workspace, SLOT(openProject()) );
	
	acSaveProject = new QAction( tr("Guardar &Proyecto"), tr(""), this );
	acSaveProject->setIconSet( QPixmap::fromMimeSource("img/prj-save.png") );
	connect( acSaveProject, SIGNAL(activated()), workspace, SLOT(saveProject()) );

	acCloseProject = new QAction( tr("&Cerrar Proyecto"), tr(""), this );
	connect( acCloseProject, SIGNAL(activated()), workspace, SLOT(closeProject()) );

	acNewModel = new QAction( tr("Nuevo &Modelo"), tr(""), this );
	acNewModel->setIconSet( QPixmap::fromMimeSource("img/doc-new.png") );
	connect( acNewModel, SIGNAL(activated()), workspace, SLOT(newDocument()) );
	
	acOpenModel = new QAction( tr("Abrir M&odelo..."), tr(""), this );
	acOpenModel->setIconSet( QPixmap::fromMimeSource("img/doc-open.png") );
	connect( acOpenModel, SIGNAL(activated()), workspace, SLOT(openDocument()) );

	acSave = new QAction( tr("&Guardar"), tr(""), this );
	acSave->setIconSet( QPixmap::fromMimeSource("img/doc-save.png") );
	connect( acSave, SIGNAL(activated()), this, SLOT(save()) );
	
	acSaveAs = new QAction( tr("Guardar &como..."), tr("Ctrl+S"), this );
	connect( acSaveAs, SIGNAL(activated()), this, SLOT(saveAs()) );

	acSaveAll = new QAction( tr("Guardar &todo"), tr("Ctrl+Shift+S"), this );
	acSaveAll->setIconSet( QPixmap::fromMimeSource("img/save-all.png") );
	connect( acSaveAll, SIGNAL(activated()), this, SLOT(saveAll()) );

	// Menú Ver
	acZoomIn = new QAction( tr("&Aumentar Zoom"), tr(""), this );
	acZoomIn->setIconSet( QPixmap::fromMimeSource("img/view-zin.png") );
	connect( acZoomIn, SIGNAL(activated()), workspace, SLOT(zoomIn()) );
	
	acZoomOut = new QAction( tr("&Reducir Zoom"), tr(""), this );
	acZoomOut->setIconSet( QPixmap::fromMimeSource("img/view-zout.png") );
	connect( acZoomOut, SIGNAL(activated()), workspace, SLOT(zoomOut()) );
	
	// Menú Simulación
	acBuildModelHDL = new QAction( tr("&Construir Modelo"), tr("Ctrl+F7"), this );
	acBuildModelHDL->setIconSet( QPixmap::fromMimeSource("img/build-model.png") );
	connect( acBuildModelHDL, SIGNAL(activated()), workspace, SLOT(buildActiveModelHDL()) );

	acBuildAllHDL = new QAction( tr("Construir &Todo"), tr("F7"), this );
	acBuildAllHDL->setIconSet( QPixmap::fromMimeSource("img/build-allmodel.png") );
	connect( acBuildAllHDL, SIGNAL(activated()), workspace, SLOT(buildAllHDL()) );

	acCompileModel = new QAction( tr("Compilar &Módulo"), tr("Ctrl+F6"), this );
	acCompileModel->setIconSet(  QPixmap::fromMimeSource("img/compile-model.png") );
	connect( acCompileModel, SIGNAL(activated()), workspace, SLOT(compileActiveModel()) );

	acCompileDependences = new QAction( tr("Compilar &Dependencias"), tr(""), this );
	acCompileDependences->setIconSet(  QPixmap::fromMimeSource("img/compile-dependences.png") );
	connect( acCompileDependences, SIGNAL(activated()), workspace, SLOT(compileDependences()) );

	acCompileAll = new QAction( tr("Com&pilar Todo"), tr("F6"), this );
	acCompileAll->setIconSet(  QPixmap::fromMimeSource("img/compile-allmodel.png") );
	connect( acCompileAll, SIGNAL(activated()), workspace, SLOT(compileAll()) );

}

void EditorWindow::createToolBars( )
{
	tbFile = new QToolBar( this, tr("Archivo") );
	acNewProject->addTo( tbFile );
	acOpenProject->addTo( tbFile );
	acSaveProject->addTo( tbFile );
	acCloseProject->addTo( tbFile );
	tbFile->addSeparator();
	acNewModel->addTo( tbFile );
	acOpenModel->addTo( tbFile );
	tbFile->addSeparator();
	acSave->addTo( tbFile );
	acSaveAll->addTo( tbFile );
	tbFile->show();
	
	tbEdit = new QToolBar( this, tr("Edición") );
	acDrawWire->addTo( tbEdit );
	tbEdit->show();
	
	tbView = new QToolBar( this, tr("Ver") );
	acZoomIn->addTo( tbView );
	acZoomOut->addTo( tbView );
	tbView->show();
	
	tbSimulation = new QToolBar( this, tr("Simulación") );
	acBuildModelHDL->addTo( tbSimulation );
	acBuildAllHDL->addTo( tbSimulation );
	tbSimulation->addSeparator();
	acCompileDependences->addTo( tbSimulation );
	acCompileModel->addTo( tbSimulation );
	acCompileAll->addTo( tbSimulation );
	tbSimulation->show();
}

void EditorWindow::createMenuBar( )
{
	mnFile = new QPopupMenu( this, tr("Archivo") );
	acNewProject->addTo( mnFile );
	acOpenProject->addTo( mnFile );
	acSaveProject->addTo( mnFile );
	acCloseProject->addTo( mnFile );
	mnFile->insertSeparator();
	acNewModel->addTo( mnFile );
	acOpenModel->addTo( mnFile );
	mnFile->insertSeparator();
	acSave->addTo( mnFile );
	acSaveAs->addTo( mnFile );
	acSaveAll->addTo( mnFile );

	mnEdition = new QPopupMenu( this, tr("Edición") );
	acDrawWire->addTo( mnEdition );

	mnView = new QPopupMenu( this, tr("Ver") );
	acZoomIn->addTo( mnView );
	acZoomOut->addTo( mnView );

	mnSimulation = new QPopupMenu( this, tr("Simulación") );
	acBuildModelHDL->addTo( mnSimulation );
	acBuildAllHDL->addTo( mnSimulation );
	mnSimulation->insertSeparator();
	acCompileDependences->addTo( mnSimulation );
	acCompileModel->addTo( mnSimulation );
	acCompileAll->addTo( mnSimulation );
	connect( mnSimulation, SIGNAL(aboutToShow()), this, SLOT(aboutToShowSimMenu()) );

	menuBar()->insertItem( tr("&Archivo"), mnFile );
	menuBar()->insertItem( tr("&Edición"), mnEdition );
	menuBar()->insertItem( tr("&Ver"), mnView );
	menuBar()->insertItem( tr("&Simulación"), mnSimulation );

}

/////////////////////////////////////////////////////////////////////////
// Slots: Carga de librerías
/////////////////////////////////////////////////////////////////////////
bool EditorWindow::loadLibraries()
{
	QDir d;
	d.cd( "lib" );

	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );
	QFileInfo *fi;
	
	while( (fi=it.current()) != 0 ){
		LMLibrary * lib = app->libraryManager().loadLibrary( QDir::convertSeparators( fi->absFilePath() ) );
	
		if( lib )
			lpSelector->insertLibrary( lib );
		
		++it;
	}

	qDebug( "TODO: mueve a Application" );

	return true;
}

/////////////////////////////////////////////////////////////////////////
// Slots: Eventos genrados por el Workspace (project)
/////////////////////////////////////////////////////////////////////////
void EditorWindow::afterProjectCreated()
{
	enableProjectFeatures();
}

void EditorWindow::afterProjectOpened()
{
	enableProjectFeatures();
}

void EditorWindow::afterProjectSaved()
{
	//TODO: implementar aquí lo que ocurre tras guardar el proyecto
}

void EditorWindow::afterProjectClosed()
{
	enableProjectFeatures( false );
}

void EditorWindow::afterDocumentInstancied( Document * doc )
{
	// Redifusión de eventos generados por un documento
	connect( doc, SIGNAL(itemSelected(QObject*)), this, SIGNAL(itemSelected(QObject*)) );
}

void EditorWindow::save()
{
	if( workspace->activeWindow() ){
		Document * doc = (Document*) workspace->activeWindow();
		doc->save();
	}
}

void EditorWindow::saveAs()
{
	if( workspace->activeWindow() ){
		Document * doc = (Document*) workspace->activeWindow();
		doc->saveAs();
	}
}

void EditorWindow::saveAll()
{
	QWidgetList windows = workspace->windowList( QWorkspace::StackingOrder );

	for( QWidgetList::iterator it = windows.begin(); it != windows.end(); ++it ){
		if( (*it)->inherits( "Document" ) ){
			Document *doc = (Document*) *it;
			doc->save();
		}
	}
}

void EditorWindow::closeEvent( QCloseEvent * e )
{
	if( workspace->closeProject() )
		e->accept();
	else
		e->ignore();
}

/////////////////////////////////////////////////////////////////////////
// Slots: Creación de LEItems (Edición)
/////////////////////////////////////////////////////////////////////////
void EditorWindow::drawWireLine()
{
	// TODO: cambiar esto!! pasalo al logic editor
	
	LogicEditor *lpLogicEditor = (LogicEditor*)workspace->activeWindow();

	LEItem * it = (LEItem*)lpLogicEditor->createWireLine();
}

void EditorWindow::onComponentClicked( LMComponent* cmp )
{
	// TODO: cambiar esto!! pasalo al logic editor
	static int i=0;

	LogicEditor *lpLogicEditor = (LogicEditor*)workspace->activeWindow();

	if ( lpLogicEditor ){
 		lpLogicEditor->createDevice( cmp );
		lpLogicEditor->setFocus();
	}
}

/////////////////////////////////////////////////////////////////////////
// Slots: Simulación
/////////////////////////////////////////////////////////////////////////
void EditorWindow::aboutToShowSimMenu()
{	
	// Se modifica el título del menú "build model" con el documento activo
	if( workspace->activeWindow() && workspace->activeWindow()->inherits( "Document" ) ){
		Document * doc = (Document*)workspace->activeWindow();

		if( acBuildModelHDL ){
			acBuildModelHDL->setMenuText( tr("&Construir %1.vhd").arg( doc->name() ) );
			acBuildModelHDL->setEnabled( true );
		}
		if( acCompileModel ){
			acCompileModel->setMenuText( tr("Com&pilar %1.vhd").arg( doc->name() ) );
			acCompileModel->setEnabled( true );
		}
	}else{
		if( acBuildModelHDL ){
			acBuildModelHDL->setMenuText( tr("&Construir Modelo") );
			acBuildModelHDL->setEnabled( false );
		}
		if( acCompileModel ){
			acCompileModel->setMenuText( tr("Com&pilar Modelo") );
			acCompileModel->setEnabled( false );
		}
	}
}

void EditorWindow::compile()
{
	if( !workspace->activeWindow() )
		return;
	
	Document * doc = (Document*)workspace->activeWindow();
	
	QFile file( QString("%1.vhd").arg(doc->name()) );
	if( !file.open( IO_WriteOnly ) )
		return;
		
	doc->hdlGenerator()->buildHDL( doc->name(), (QTextOStream*)&QTextStream(&file) );
}

/////////////////////////////////////////////////////////////////////////
// Slots: Eventos en LogicEditor
/////////////////////////////////////////////////////////////////////////
void EditorWindow::onConnected(LEItem* it,LEConnectionPoint* cp){
	qDebug( QString("Conectado %1").arg(it->name()) );
}

void EditorWindow::onDisconnected(LEItem* it,LEConnectionPoint* cp){
	qDebug( QString("Desconectado %1").arg(it->name()) );
}

/////////////////////////////////////////////////////////////////////////
// Private Members
/////////////////////////////////////////////////////////////////////////
void EditorWindow::enableProjectFeatures( bool enable )
{
	// Actions
	// Menú Archivo
	if( acCloseProject )
		acCloseProject->setEnabled( enable );
	if( acSaveProject )
		acSaveProject->setEnabled( enable );

	if( acSave )
		acSave->setEnabled( enable );
	if( acSaveAs )
		acSaveAs->setEnabled( enable );
	if( acSaveAll )
		acSaveAll->setEnabled( enable );

	if( acNewModel )
		acNewModel->setEnabled( enable );
	if( acOpenModel )
		acOpenModel->setEnabled( enable );

	// Menú Simulación
	if( acBuildAllHDL )
		acBuildAllHDL->setEnabled( enable );
	if( acBuildModelHDL )
		acBuildModelHDL->setEnabled( enable );

	if( acCompileDependences )
		acCompileDependences->setEnabled( enable );
	if( acCompileModel )
		acCompileModel->setEnabled( enable );
	if( acCompileAll )
		acCompileAll->setEnabled( enable );

	// Menú Edición
	if( acDrawWire )
		acDrawWire->setEnabled( enable );

	// ToolBoxes
	if( lpSelector )
		lpSelector->setEnabled( enable );
}

