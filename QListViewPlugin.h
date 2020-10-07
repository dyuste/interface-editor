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



#if !defined(_QLISTVIEWPLUGIN_H_)
#define _QLISTVIEWPLUGIN_H_

#include <qlistview.h>
#include <qvaluelist.h>

class Plugin;

// Derivado de ListViewItem, extiende información sobre el plugin representado
class QListViewItemPlugin : public QListViewItem
{
public:
	QListViewItemPlugin( Plugin * plugin, QListView * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null );

	// Atributos
	inline void setPlugin( Plugin * plugin ){ this->plg = plugin; }
	inline Plugin * plugin(){ return plg; }

private:
	Plugin * plg;
};


class QListViewPlugin : public QListView
{
public:
	QListViewPlugin( QValueList<Plugin*> plugins, QWidget* parent=0, const char* name=0, WFlags f = 0 );
    ~QListViewPlugin();

};

#endif 
