/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtTest module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTESTACCESSIBLE_H
#define QTESTACCESSIBLE_H

#if 0
// inform syncqt
#pragma qt_no_master_include
#endif

#ifndef QT_NO_ACCESSIBILITY

#define QVERIFY_EVENT(event) \
    QVERIFY(QTestAccessibility::verifyEvent(event))

#include <QtCore/qlist.h>
#include <QtGui/qaccessible.h>
#include <QtGui/qguiapplication.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


class QObject;

// Use pointers since we subclass QAccessibleEvent
typedef QList<QAccessibleEvent*> EventList;

bool operator==(const QAccessibleEvent &l, const QAccessibleEvent &r)
{
    if (l.type() != r.type()) {
//        qDebug() << "QAccessibleEvent with wrong type: " << qAccessibleEventString(l.type()) << " and " << qAccessibleEventString(r.type());
        return false;
    }
    if (l.object() != r.object() ||
            l.child() != r.child()) {
//        qDebug() << "QAccessibleEvent for wrong object: " << l.object() << " and " << r.object() << " child: " << l.child() << " and " << r.child();
        return false;
    }

    if (l.type() == QAccessible::StateChanged) {
        return static_cast<const QAccessibleStateChangeEvent*>(&l)->changedStates()
                == static_cast<const QAccessibleStateChangeEvent*>(&r)->changedStates();
    } else if (l.type() == QAccessible::TextCaretMoved) {
        return static_cast<const QAccessibleTextCursorEvent*>(&l)->cursorPosition()
                == static_cast<const QAccessibleTextCursorEvent*>(&r)->cursorPosition();
    } else if (l.type() == QAccessible::TextSelectionChanged) {
        const QAccessibleTextSelectionEvent *le = static_cast<const QAccessibleTextSelectionEvent*>(&l);
        const QAccessibleTextSelectionEvent *re = static_cast<const QAccessibleTextSelectionEvent*>(&r);
        return  le->cursorPosition() == re->cursorPosition() &&
                le->selectionStart() == re->selectionStart() &&
                le->selectionEnd() == re->selectionEnd();
    } else if (l.type() == QAccessible::TextInserted) {
        const QAccessibleTextInsertEvent *le = static_cast<const QAccessibleTextInsertEvent*>(&l);
        const QAccessibleTextInsertEvent *re = static_cast<const QAccessibleTextInsertEvent*>(&r);
        return  le->cursorPosition() == re->cursorPosition() &&
                le->changePosition() == re->changePosition() &&
                le->textInserted() == re->textInserted();
    } else if (l.type() == QAccessible::TextRemoved) {
        const QAccessibleTextRemoveEvent *le = static_cast<const QAccessibleTextRemoveEvent*>(&l);
        const QAccessibleTextRemoveEvent *re = static_cast<const QAccessibleTextRemoveEvent*>(&r);
        return  le->cursorPosition() == re->cursorPosition() &&
                le->changePosition() == re->changePosition() &&
                le->textRemoved() == re->textRemoved();
    } else if (l.type() == QAccessible::TextUpdated) {
        const QAccessibleTextUpdateEvent *le = static_cast<const QAccessibleTextUpdateEvent*>(&l);
        const QAccessibleTextUpdateEvent *re = static_cast<const QAccessibleTextUpdateEvent*>(&r);
        return  le->cursorPosition() == re->cursorPosition() &&
                le->changePosition() == re->changePosition() &&
                le->textInserted() == re->textInserted() &&
                le->textRemoved() == re->textRemoved();
    } else if (l.type() == QAccessible::ValueChanged) {
        const QAccessibleValueChangeEvent *le = static_cast<const QAccessibleValueChangeEvent*>(&l);
        const QAccessibleValueChangeEvent *re = static_cast<const QAccessibleValueChangeEvent*>(&r);
        return le->value() == re->value();
    }
    return true;
}

class QTestAccessibility
{
public:
    static void initialize()
    {
        if (!instance()) {
            instance() = new QTestAccessibility;
            qAddPostRoutine(cleanup);
        }
    }

    static void cleanup()
    {
        delete instance();
        instance() = 0;
    }
    static void clearEvents() { eventList().clear(); }
    static EventList events() { return eventList(); }
    static bool verifyEvent(QAccessibleEvent *ev)
    {
        if (eventList().isEmpty())
            return false;
        QAccessibleEvent *first = eventList().takeFirst();
        bool res = *first == *ev;
        delete first;
        return res;
    }
    static bool containsEvent(QAccessibleEvent *event) {
        Q_FOREACH (const QAccessibleEvent *ev, eventList()) {
            if (*ev == *event)
                return true;
        }
        return false;
    }

private:
    QTestAccessibility()
    {
        QAccessible::installUpdateHandler(updateHandler);
        QAccessible::installRootObjectHandler(rootObjectHandler);
    }

    ~QTestAccessibility()
    {
        QAccessible::installUpdateHandler(0);
        QAccessible::installRootObjectHandler(0);
    }

    static void rootObjectHandler(QObject *object)
    {
        //    qDebug("rootObjectHandler called %p", object);
        if (object) {
            QGuiApplication* app = qobject_cast<QGuiApplication*>(object);
            if ( !app )
                qWarning("%s: root Object is not a QGuiApplication!", Q_FUNC_INFO);
        } else {
            qWarning("%s: root Object called with 0 pointer", Q_FUNC_INFO);
        }
    }

    static void updateHandler(QAccessibleEvent *event)
    {
        eventList().append(copyEvent(event));
    }
    static QAccessibleEvent *copyEvent(QAccessibleEvent *event)
    {
        QAccessibleEvent *ev;
        if (event->type() == QAccessible::StateChanged) {
            ev = new QAccessibleStateChangeEvent(event->object(),
                        static_cast<QAccessibleStateChangeEvent*>(event)->changedStates());
        } else if (event->type() == QAccessible::TextCaretMoved) {
            ev =  new QAccessibleTextCursorEvent(event->object(), static_cast<QAccessibleTextCursorEvent*>(event)->cursorPosition());
        } else if (event->type() == QAccessible::TextSelectionChanged) {
            const QAccessibleTextSelectionEvent *original = static_cast<QAccessibleTextSelectionEvent*>(event);
            QAccessibleTextSelectionEvent *sel = new QAccessibleTextSelectionEvent(event->object(), original->selectionStart(), original->selectionEnd());
            sel->setCursorPosition(original->cursorPosition());
            ev = sel;
        } else if (event->type() == QAccessible::TextInserted) {
            const QAccessibleTextInsertEvent *original = static_cast<QAccessibleTextInsertEvent*>(event);
            QAccessibleTextInsertEvent *ins = new QAccessibleTextInsertEvent(event->object(), original->changePosition(), original->textInserted());
            ins->setCursorPosition(original->cursorPosition());
            ev = ins;
        } else if (event->type() == QAccessible::TextRemoved) {
            const QAccessibleTextRemoveEvent *original = static_cast<QAccessibleTextRemoveEvent*>(event);
            QAccessibleTextRemoveEvent *rem = new QAccessibleTextRemoveEvent(event->object(), original->changePosition(), original->textRemoved());
            rem->setCursorPosition(original->cursorPosition());
            ev = rem;
        } else if (event->type() == QAccessible::TextUpdated) {
            const QAccessibleTextUpdateEvent *original = static_cast<QAccessibleTextUpdateEvent*>(event);
            QAccessibleTextUpdateEvent *upd = new QAccessibleTextUpdateEvent(event->object(), original->changePosition(), original->textRemoved(), original->textInserted());
            upd->setCursorPosition(original->cursorPosition());
            ev = upd;
        } else if (event->type() == QAccessible::ValueChanged) {
            ev = new QAccessibleValueChangeEvent(event->object(), static_cast<QAccessibleValueChangeEvent*>(event)->value());
        } else {
            ev = new QAccessibleEvent(event->object(), event->type());
        }
        ev->setChild(event->child());
        return ev;
    }

    static EventList &eventList()
    {
        static EventList list;
        return list;
    }

    static QTestAccessibility *&instance()
    {
        static QTestAccessibility *ta = 0;
        return ta;
    }
};

#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif
