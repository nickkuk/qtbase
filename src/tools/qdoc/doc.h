/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the tools applications of the Qt Toolkit.
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

/*
  doc.h
*/

#ifndef DOC_H
#define DOC_H

#include <QSet>
#include <QString>
#include <QMap>

#include "location.h"

QT_BEGIN_NAMESPACE

class Atom;
class CodeMarker;
class Config;
class DocPrivate;
class Quoter;
class Text;
class FakeNode;
class DitaRef;

typedef QMap<QString, QStringList> QCommandMap;
typedef QMap<QString, QString> QStringMap;
typedef QMultiMap<QString, QString> QStringMultiMap;

struct Topic
{
    QString topic;
    QString args;
    Topic(QString& t, QString a) : topic(t), args(a) { }
};
typedef QList<Topic> TopicList;

typedef QList<DitaRef*> DitaRefList;

class DitaRef
{
public:
    DitaRef() { }
    virtual ~DitaRef() { }

    const QString& navtitle() const { return navtitle_; }
    const QString& href() const { return href_; }
    void setNavtitle(const QString& t) { navtitle_ = t; }
    void setHref(const QString& t) { href_ = t; }
    virtual bool isMapRef() const = 0;
    virtual const DitaRefList* subrefs() const { return 0; }
    virtual void appendSubref(DitaRef* ) { }

private:
    QString navtitle_;
    QString href_;
};

class TopicRef : public DitaRef
{
public:
    TopicRef() { }
    ~TopicRef();

    virtual bool isMapRef() const { return false; }
    virtual const DitaRefList* subrefs() const { return &subrefs_; }
    virtual void appendSubref(DitaRef* t) { subrefs_.append(t); }

private:
    DitaRefList subrefs_;
};

class MapRef : public DitaRef
{
public:
    MapRef() { }
    ~MapRef() { }

    virtual bool isMapRef() const { return true; }
};

class Doc
{
public:
    // the order is important
    enum Sections {
        NoSection = -2,
        Part = -1,
        Chapter = 1,
        Section1 = 1,
        Section2 = 2,
        Section3 = 3,
        Section4 = 4
    };

    Doc() : priv(0) {}
    Doc(const Location &start_loc,
        const Location &end_loc,
        const QString &source,
        const QSet<QString> &metaCommandSet);
    Doc(const Location& start_loc,
        const Location& end_loc,
        const QString& source,
        const QSet<QString>& metaCommandSet,
        const QSet<QString>& topics);
    Doc(const Doc &doc);
    ~Doc();

    Doc& operator=( const Doc& doc );

    void renameParameters(const QStringList &oldNames,
                          const QStringList &newNames);
    void simplifyEnumDoc();
    void setBody(const Text &body);
    const DitaRefList& ditamap() const;

    const Location &location() const;
    bool isEmpty() const;
    const QString& source() const;
    const Text& body() const;
    Text briefText(bool inclusive = false) const;
    Text trimmedBriefText(const QString &className) const;
    Text legaleseText() const;
    const QString& baseName() const;
    Sections granularity() const;
    const QSet<QString> &parameterNames() const;
    const QStringList &enumItemNames() const;
    const QStringList &omitEnumItemNames() const;
    const QSet<QString> &metaCommandsUsed() const;
    const TopicList& topicsUsed() const;
    QStringList metaCommandArgs( const QString& metaCommand ) const;
    const QList<Text> &alsoList() const;
    bool hasTableOfContents() const;
    bool hasKeywords() const;
    bool hasTargets() const;
    const QList<Atom *> &tableOfContents() const;
    const QList<int> &tableOfContentsLevels() const;
    const QList<Atom *> &keywords() const;
    const QList<Atom *> &targets() const;
    const QStringMultiMap &metaTagMap() const;

    static void initialize( const Config &config );
    static void terminate();
    static QString alias( const QString &english );
    static void trimCStyleComment( Location& location, QString& str );
    static CodeMarker *quoteFromFile(const Location &location,
                                     Quoter &quoter,
                                     const QString &fileName);
    static QString canonicalTitle(const QString &title);

private:
    void detach();
    DocPrivate *priv;
};

QT_END_NAMESPACE

#endif
