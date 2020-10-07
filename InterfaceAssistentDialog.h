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



#if !defined(_INTERFACE_ASSISTENT_DIALOG_H_)
#define _INTERFACE_ASSISTENT_DIALOG_H_

#include <qtabdialog.h>
#include "IAInterface.h"

class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QCanvas;
class IACanvasView;
class IACanvasRectangle;

class InterfaceAssistentDialog : public QTabDialog
{
Q_OBJECT
public:
	InterfaceAssistentDialog( QWidget * parent=0, const char * name=0 );
	
	void createTabs();

	void createStartFrame();
	void createPortsFrame();
	void createEndFrame();

	IAInterface & interfaceData();

public slots:
	void onInsertPort();
	void onRemovePort();
	void onSelected( IACanvasRectangle* sel );
	void onPortsApply();
	void updateFrames();
	void updateStartFrame();
	void updateEndFrame();

private:
	IAInterface iface;
	QFrame * frStart, * frPorts, * frEnd;

	// Informaci�n b�sica
	QLabel *lbStartInfo1, *lbStartInfo2, *lbStartBaseDir, *lbStartName;
	QLineEdit *leStartBaseDir, *leStartName;

	// Descripci�n de puertos
	QCanvas *cnPorts;
	IACanvasView *cvPorts;
	QPushButton *pbPortsInsert,	*pbPortsRemove, *pbPortsApply;
	QLabel *lbPortsBaseDir, *lbPortsSize, *lbPortsAccess, *lbPortsName;
	QLineEdit *lePortsBaseDir, *lePortsName;
	QComboBox *cbPortsSize, *cbPortsAccess;

	// Informaci�n de finalizaci�n
	QLabel *lbEndCaption, *lbEndElement, *lbEndRegs, *lbEndNand;

};

#endif
