#ifndef AINTERFACETOHLDFILEPROCESSOR_H
#define AINTERFACETOHLDFILEPROCESSOR_H

#include "ascriptinterface.h"

#include <QObject>

class AHldFileProcessor;

class AInterfaceToHldFileProcessor : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToHldFileProcessor(AHldFileProcessor& hldProcessor);

public slots:
    const QString ProcessFile(const QString FileName, const QString SaveFileName = "");

private:
    AHldFileProcessor& hldProcessor;

};

#endif // AINTERFACETOHLDFILEPROCESSOR_H
