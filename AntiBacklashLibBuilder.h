/**
AntiBacklashLibBuilder header file.
*/

#ifndef ANTIBACKLASHLIB_ANTIBACKLASHLIBBUILDER_H
#define ANTIBACKLASHLIB_ANTIBACKLASHLIBBUILDER_H

#include <CDPSystem/Application/CDPBuilder.h>

namespace AntiBacklashLib {

class AntiBacklashLibBuilder : public CDPBuilder
{
public:
    AntiBacklashLibBuilder(const char* libName);
    CDPComponent* CreateNewComponent(const std::string& type) override;
    CDPBaseObject* CreateNewCDPOperator(const std::string& modelName,
                                        const std::string& type,
                                        const CDPPropertyBase* inputProperty) override;
    CDPObject* CreateNewObject(const std::string& type) override;
    CDP::StudioAPI::CDPNode* CreateNewCDPNode(const std::string& type) override;
};

} // namespace AntiBacklashLib

#endif
