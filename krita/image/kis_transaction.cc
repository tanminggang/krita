/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_types.h"
#include "kis_global.h"
#include "kis_tile.h"
#include "kis_transaction.h"
#include "kis_memento.h"
#include "kis_paint_device.h"

class KisTransactionPrivate {
public:
    QString name;
    KisPaintDeviceSP device;
    KisMementoSP memento;
    bool firstRedo;
};

KisTransaction::KisTransaction(const QString& name, KisPaintDeviceSP device, QUndoCommand* parent) : QUndoCommand(name, parent)
{
    m_private = new KisTransactionPrivate;

    m_private->device = device;
    m_private->memento = device->getMemento();
    m_private->firstRedo = true;
}

KisTransaction::~KisTransaction()
{
    if (m_private->memento) {
        // For debugging purposes
        m_private->memento->setInvalid();
    }
    delete m_private;
}

void KisTransaction::redo()
{
    //QUndoStack calls redo(), so the first call needs to be blocked
    if(m_private->firstRedo)
    {
        m_private->firstRedo = false;
        return;
    }
    Q_ASSERT(!m_private->memento.isNull());

    m_private->device->rollforward(m_private->memento);

    QRect rc;
    qint32 x, y, width, height;
    m_private->memento->extent(x,y,width,height);
    rc.setRect(x + m_private->device->getX(), y + m_private->device->getY(), width, height);

    m_private->device->setDirty( rc );
}

void KisTransaction::undo()
{
    Q_ASSERT(!m_private->memento.isNull());
    m_private->device->rollback(m_private->memento);

    QRect rc;
    qint32 x, y, width, height;
    m_private->memento->extent(x,y,width,height);
    rc.setRect(x + m_private->device->getX(), y + m_private->device->getY(), width, height);

    m_private->device->setDirty( rc );
}

void KisTransaction::undoNoUpdate()
{
    Q_ASSERT(!m_private->memento.isNull());

    m_private->device->rollback(m_private->memento);
}
