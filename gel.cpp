// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QCoreApplication>
#include <QtQml/qqml.h>

#include "gel.h"

namespace com { namespace cutehacks { namespace gel {

void registerEngine(QQmlEngine *)
{
    // deprecated and done autmatically below
}

static void registerTypes()
{
    qmlRegisterType<JsonListModel>("com.cutehacks.gel", 1, 0, "JsonListModel");
    qmlRegisterType<Collection>("com.cutehacks.gel", 1, 0, "Collection");
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

} } }
