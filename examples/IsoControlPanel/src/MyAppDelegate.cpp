#include <sstream>
#include <stdexcept>

#include <UserInputEventDispatch.h>

#include "MyAppDelegate.h"


using CoreKit::Application;
using CoreKit::EventInputSource;
using IsoDdsKit::DdsEnabledAppDelegate;
using MyIsoSoftwareBus::Command;

namespace
{
    std::string CommandSampleToString(MyIsoSoftwareBus::Command const& aCommand)
    {
        std::stringstream result;

        result << "{ \"statement\": \"" << aCommand.statement() << "\" }";

        return result.str();
    }
}

MyAppDelegate::MyAppDelegate()
{

}

MyAppDelegate::~MyAppDelegate()
{

}

void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
    DdsEnabledAppDelegate::applicationDidInitialize(theApp);

    topics().add< MyIsoSoftwareBus::EngineeringData >(participant(), "EngineeringData");
    topics().add< MyIsoSoftwareBus::Command >(participant(), "Command");

    auto engrQos = subscriber().default_datareader_qos();
    engrQos << dds::core::policy::Reliability::BestEffort()
            << dds::core::policy::History::KeepLast(100);
    m_engrDataReaderContext = createReaderContext< MyIsoSoftwareBus::EngineeringData >("EngineeringData", engrQos);

    auto cmdQos = publisher().default_datawriter_qos();
    cmdQos << dds::core::policy::Reliability::Reliable()
           << dds::core::policy::History::KeepLast(10);
    m_commandWriterContext = createWriterContext< MyIsoSoftwareBus::Command >("Command", cmdQos);

    m_userInput = std::make_shared< UserInputSource >(
        std::make_shared< UserInputEventDispatch >()
    );
    m_commandDispatch = std::make_shared< CommandDispatch >();
    m_cmdEvent = std::make_shared< EventInputSource >(m_commandDispatch.get());
    m_frontPanel = std::make_shared< FrontPanel > (theApp);

    m_commandDispatch->addCommandHandler(
        std::bind(
            std::mem_fn(&MyAppDelegate::onCommandRequest),
            this,
            std::placeholders::_1
        )
    );
    m_userInput->dispatch()->addUserInputHandler(
        std::bind(
            std::mem_fn(&FrontPanel::onUserInput),
            m_frontPanel,
            std::placeholders::_1
        )
    );
}

void
MyAppDelegate::applicationStarting(Application *theApp)
{
    theApp->getMainRunLoop()->registerInputSource(m_cmdEvent.get());
    theApp->getMainRunLoop()->registerInputSource(m_userInput.get());
    m_commandDispatch->initialize(m_cmdEvent);

    m_commandWriterContext->activate(theApp->getMainRunLoop());

    m_engrDataReaderContext->sampleDistribution().addListener(std::bind(std::mem_fn(&FrontPanel::onEngineeringData), m_frontPanel, std::placeholders::_1));
    m_engrDataReaderContext->activate(theApp->getMainRunLoop());

    m_frontPanel->initialize(m_commandDispatch);
}

void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    if (m_frontPanel)
    {
        m_frontPanel->terminate();
    }
    m_engrDataReaderContext->sampleDistribution().removeAllListeners();
    m_engrDataReaderContext->deactivate(theApp->getMainRunLoop());
    m_commandWriterContext->deactivate(theApp->getMainRunLoop());
    m_commandDispatch->terminate();
    theApp->getMainRunLoop()->deregisterInputSource(m_userInput.get());
    theApp->getMainRunLoop()->deregisterInputSource(m_cmdEvent.get());
}

void
MyAppDelegate::applicationDidTerminate(Application *theApp)
{
    m_frontPanel.reset();
    m_cmdEvent.reset();
    m_commandDispatch.reset();
    m_userInput.reset();
    m_engrDataReaderContext.reset();
    m_commandWriterContext.reset();
}


void
MyAppDelegate::onCommandRequest(std::string commandStatement)
{
    if (m_commandWriterContext)
    {
        Command theCommand;

        theCommand.statement(commandStatement);
        m_commandWriterContext->writer().write(theCommand);
    }
}

// vim: set ts=4 sw=4 expandtab:
