/******************************************************************************
 * Copyright (C) 2008 Teo Mrnjavac <teo.mrnjavac@gmail.com>                   *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/
#include "TokenListWidget.h"
#include "Debug.h"

#include <KApplication>
#include <KDialog>

#include <QMouseEvent>


TokenListWidget::TokenListWidget( QWidget *parent )
    : KListWidget( parent )
{
    setAcceptDrops( true );

    addItem( i18n( "Track" ) );
    addItem( i18n( "Title" ) );
    addItem( i18n( "Artist" ) );
    addItem( i18n( "Composer" ) );
    addItem( i18n( "Year" ) );
    addItem( i18n( "Album" ) );
    addItem( i18n( "Comment" ) );
    addItem( i18n( "Genre" ) );
    addItem( " _ " );
    addItem( " - " );
    addItem( " . " );
    addItem( "<space>" );
}

void
TokenListWidget::mousePressEvent( QMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton )
        m_startPos = event->pos();            //store the start position
    KListWidget::mousePressEvent( event );    //feed it to parent's event
}

void
TokenListWidget::mouseMoveEvent( QMouseEvent *event )
{
    if ( event->buttons() & Qt::LeftButton )
    {
        int distance = ( event->pos() - m_startPos ).manhattanLength();
        if ( distance >= KApplication::startDragDistance() )
        {
            performDrag( event );
        }
    }
    KListWidget::mouseMoveEvent( event );
}

void
TokenListWidget::dragEnterEvent( QDragEnterEvent *event )
{
    QWidget *source = qobject_cast<QWidget *>( event->source() );
    if ( source && source != this )
    {
        event->setDropAction( Qt::MoveAction );
        event->accept();
    }
}

void
TokenListWidget::dragMoveEvent( QDragMoveEvent *event )        //overrides QListWidget's implementation
{
    QWidget *source = qobject_cast<QWidget *>( event->source() );
    if ( source && source != this )
    {
        event->setDropAction( Qt::MoveAction );
        event->accept();
    }
}

void
TokenListWidget::dropEvent( QDropEvent *event )
{
    //does nothing, I want the item to be deleted and not dragged here
}

void
TokenListWidget::performDrag( QMouseEvent *event )
{
    QListWidgetItem *item = currentItem();
    if ( item )
    {
        QByteArray itemData;
        QDataStream dataStream( &itemData, QIODevice::WriteOnly );
        dataStream << item->text() << QPoint( event->pos() - rect().topLeft() );
        QMimeData *mimeData = new QMimeData;
        mimeData->setData( "application/x-amarok-tag-token", itemData );    //setText( item->text() );
        QDrag *drag = new QDrag( this );
        drag->setMimeData( mimeData );
        debug() << "I'm dragging from the token pool";
        //TODO: set a pointer for the drag, like this: drag->setPixmap( QPixmap("foo.png" ) );
        drag->exec( Qt::MoveAction | Qt::CopyAction, Qt::CopyAction );
    }
}

