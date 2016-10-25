// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QCoreApplication>
#include <QtQml/qqml.h>

#include "gel.h"

namespace com { namespace cutehacks { namespace gel {

static const char* GEL_URI = "com.cutehacks.gel";

void registerEngine(QQmlEngine *)
{
    // deprecated and done autmatically below
}

static void registerTypes()
{
    qmlRegisterType<JsonListModel>(GEL_URI, 1, 0, "JsonListModel");
    qmlRegisterType<Collection>(GEL_URI, 1, 0, "Collection");
    qmlProtectModule(GEL_URI, 1);
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

} } }
