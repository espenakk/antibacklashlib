/**
AntiBacklashLibBuilder implementation.
*/

#include "AntiBacklashController.h"
#include "EncoderPort.h"
#include "AntiBacklashLibBuilder.h"

using namespace AntiBacklashLib;

/*!
  \inmodule AntiBacklashLib
  \namespace AntiBacklashLib

  \brief Contains AntiBacklashLib implementation classes.
  Click into each class to see the documentation for that class, or search for a keyword in the help documentation.
*/

/**
\internal
Do not edit. Autogenerated Builder constructor.
*/
AntiBacklashLibBuilder::AntiBacklashLibBuilder(const char* libName)
    : CDPBuilder(libName, __DATE__ " " __TIME__)
{
}

/**
\internal
Do not edit. Autogenerated Builder CDPComponent factory function.
*/
CDPComponent* AntiBacklashLibBuilder::CreateNewComponent(const std::string& type)
{
    if (type=="AntiBacklashLib.AntiBacklashController")
        return new AntiBacklashController;
    
    return CDPBuilder::CreateNewComponent(type);
}

/**
\internal
Do not edit. Autogenerated Builder CDPBaseObject factory function.
*/
CDPBaseObject* AntiBacklashLibBuilder::CreateNewCDPOperator(const std::string& modelName,
                                                            const std::string& type,
                                                            const CDPPropertyBase* inputProperty)
{
    return CDPBuilder::CreateNewCDPOperator(modelName, type, inputProperty);
}

/**
\internal
Do not edit. Autogenerated Builder CDPObject factory function.
*/
CDPObject* AntiBacklashLibBuilder::CreateNewObject(const std::string& type)
{
    
    if (type=="AntiBacklashLib.EncoderPort")
        return new EncoderPort;
    
    return CDPBuilder::CreateNewObject(type);
}

/**
\internal
CDPNode factory function.
*/
CDP::StudioAPI::CDPNode* AntiBacklashLibBuilder::CreateNewCDPNode(const std::string& type)
{
    return CDPBuilder::CreateNewCDPNode(type);
}
