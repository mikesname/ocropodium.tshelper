// Copyright 2010 King's College London
// or its licensors, as applicable.
//
// You may not use this file except under the terms of the accompanying license.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You may
// obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "finderthread.h"

FinderThread::FinderThread(QStringList *pagenames, PageList *pagelist, int* linecount, QObject *parent)
    :
    m_path(""),
    m_pagenames(pagenames),
    m_pagelist(pagelist),
    m_linecount(linecount),
    m_stopped(false)
{

}

FinderThread::~FinderThread()
{

}

void FinderThread::stop()
{
    m_stopped = true;
}

void FinderThread::setPath(const QString &path)
{
    m_path = path;
}

void FinderThread::run()
{
    QStringList pagefilters;
    pagefilters << "[0-9][0-9][0-9][0-9]";
    QStringList linefilters;
    linefilters << "??????.png";

    QDir dir(m_path);    
    dir.setNameFilters(pagefilters);
    dir.setFilter(QDir::Dirs|QDir::Readable|QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);

    // make sure the pagelist isn't modified externally
    // it shouldn't be at the moment
    m_mutex.lock();

    emit pagesFound(dir.entryList().size());

    for (int i = 0; i < dir.entryList().size(); i++) {
        if (m_stopped)
            break;

        emit scanningPage(i + 1);

        QString entry = dir.entryList().at(i);        

        QDir pagedir(m_path + "/" + entry);        
        pagedir.setNameFilters(linefilters);
        pagedir.setFilter(QDir::Readable|QDir::Files);
        pagedir.setSorting(QDir::Name);

        LineList* lines = new LineList;
        for (int l = 0; l < pagedir.entryList().size(); l++) {
            lines->append(new OcrLine(pagedir.entryList()[l]));
        }
        m_pagelist->append(lines);
        m_pagenames->append(entry);
        *m_linecount += lines->size();
    }

    emit scanningDone();
    m_mutex.unlock();
}
