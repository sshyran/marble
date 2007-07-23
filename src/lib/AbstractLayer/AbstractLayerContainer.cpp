//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Andrew Manson    <g.real.ate@gmail.com>
//

#include "AbstractLayerContainer.h"
#include "ClipPainter.h"

AbstractLayerContainer::AbstractLayerContainer( int size )
{
    m_data = new QVector<AbstractLayerData*>( size );
    m_visible = new QBitArray( size );
    
    m_name = 0;
}

AbstractLayerContainer::AbstractLayerContainer(const QString &name,
                                               int size )
{
    m_data = new QVector<AbstractLayerData*>( size );
    m_visible = new QBitArray( size );
    
    m_name = new QString ( name );
}

AbstractLayerContainer::~AbstractLayerContainer()
{
    delete m_visible;
    delete m_data;
    delete m_name;
}

void AbstractLayerContainer::draw(ClipPainter *painter, 
                                  const QSize &canvasSize, 
                                  double radius, 
                                  Quaternion invRotAxis)
{
    const_iterator it;
    
    for( it = constBegin(); it < constEnd(); ++it ) {
        (*it)->draw( painter, canvasSize, radius, invRotAxis);
    }
}

QString AbstractLayerContainer::name() const
{
    return *m_name; 
}

void AbstractLayerContainer::processVisible()
{
    QVector<AbstractLayerData*>::const_iterator i = m_data -> begin();
    int temp;
    
    for ( ; i < m_data -> end() ; ++i ) {
        if ( (*i) -> visible() ) {
            //iterator safety
            temp = m_data -> indexOf ( *i );
            m_visible -> setBit ( temp, true );
        }
    }
}

double AbstractLayerContainer::distance ( const QPoint &a, 
                                          const QPoint &b )
{
    return (  ( ( a.x() - b.x() ) * ( a.x() - b.x() ) )
            + ( ( a.y() - b.y() ) * ( a.y() - b.y() ) ) );
}

void AbstractLayerContainer::manageMemory()
{
    for ( int i = 0 ; i < m_visible -> size () ; ++i ) {
        if ( m_visible -> testBit( i ) ) {
            if ( ! ( this -> contains ( m_data -> at( i ) ) ) ) {
                this -> append( m_data -> at( i ) );
            }
        }
    }
}
