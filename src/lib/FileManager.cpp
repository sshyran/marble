//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//

#include "FileManager.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "FileLoader.h"
#include "FileViewModel.h"
#include "KmlFileViewItem.h"
#include "MarbleDebug.h"
#include "MarbleDataFacade.h"
#include "GeoDataTreeModel.h"

#include "GeoDataDocument.h"


using namespace Marble;

namespace Marble
{
class FileManagerPrivate
{
public:
    FileManagerPrivate( )
        : m_datafacade( 0 )
    {
    }

    MarbleDataFacade* m_datafacade;
    QList<FileLoader*> m_loaderList;
    QStringList m_pathList;
    QList < KmlFileViewItem* > m_fileItemList;
};
}

FileManager::FileManager( QObject *parent )
    : QObject( parent )
    , d( new FileManagerPrivate() )
{
}


FileManager::~FileManager()
{
    foreach ( FileLoader *loader, d->m_loaderList ) {
        if ( loader ) {
            loader->wait();
        }
    }

    foreach ( KmlFileViewItem *file, d->m_fileItemList ) {
        delete file;
    }

    delete d;
}

void FileManager::setDataFacade( MarbleDataFacade *facade )
{
    d->m_datafacade = facade;
    d->m_datafacade->fileViewModel()->setFileManager( this );
    d->m_datafacade->treeModel()->setFileManager( this );
}

MarbleDataFacade *FileManager::dataFacade()
{
    return d->m_datafacade;
}

QStringList FileManager::containers() const
{
    QStringList retList;
    for ( int line = 0; line < d->m_fileItemList.count(); ++line ) {
        retList << d->m_fileItemList.at( line )->name();
    }
    return retList + d->m_pathList;
}

void FileManager::addFile( const QString& filepath )
{
    if ( !containers().contains( filepath ) ) {
        mDebug() << "adding container:" << filepath;
        FileLoader* loader = new FileLoader( this, filepath );
        appendLoader( loader );
        d->m_pathList.append( filepath );
    }
}

void FileManager::addData( const QString &name, const QString &data )
{
    FileLoader* loader = new FileLoader( this, data, name );
    appendLoader( loader );
}

void FileManager::appendLoader( FileLoader *loader )
{
    connect( loader, SIGNAL( loaderFinished( FileLoader* ) ),
             this, SLOT( cleanupLoader( FileLoader* ) ) );

    connect( loader, SIGNAL( newGeoDataDocumentAdded( GeoDataDocument* ) ),
             this, SLOT( addGeoDataDocument( GeoDataDocument* ) ) );

    d->m_loaderList.append( loader );
    loader->start();
}

void FileManager::removeFile( const QString& key )
{
    for ( int i = 0; i < d->m_fileItemList.size(); ++i ) {
        if ( key == d->m_fileItemList.at(i)->name() ) {
            closeFile( i );
            return;
        }
    }
    mDebug() << "could not identify " << key;
}

void FileManager::saveFile( int index )
{
    if ( index < d->m_fileItemList.size() ) {
        d->m_fileItemList.at( index )->saveFile();
    }
}

void FileManager::closeFile( int index )
{
    mDebug() << "FileManager::closeFile";
    if ( index < d->m_fileItemList.size() ) {
        emit fileRemoved( index );
        KmlFileViewItem *file = d->m_fileItemList.at( index );
        if ( file ) {
            delete file->document();
        }
        delete d->m_fileItemList.at( index );
        d->m_fileItemList.removeAt( index );
    }
}

int FileManager::size() const
{
    return d->m_fileItemList.size();
}

KmlFileViewItem * FileManager::at( int index )
{
    if ( index < d->m_fileItemList.size() ) {
        return d->m_fileItemList.at( index );
    }
    return 0;
}

void FileManager::addGeoDataDocument( GeoDataDocument* document )
{
    KmlFileViewItem* item = new KmlFileViewItem( document );
    d->m_fileItemList.append( item );
    emit fileAdded( d->m_fileItemList.indexOf( item ) );

    // now get the document that will be preserved throughout the life time
    GeoDataDocument* doc = item->document();
    if ( doc->name().isEmpty() && !doc->fileName().isEmpty() )
    {
        doc->setName( doc->fileName() );
    }
}

void FileManager::cleanupLoader( FileLoader* loader )
{
    d->m_loaderList.removeAll( loader );
    if ( loader->isFinished() ) {
        d->m_pathList.removeAll( loader->path() );
        delete loader;
    }
}

#include "FileManager.moc"
