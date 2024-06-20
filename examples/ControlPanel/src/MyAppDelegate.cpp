#include <sstream>
#include <stdexcept>

#include <UserInputEventDispatch.h>

#include "MyAppDelegate.h"


using CoreKit::Application;
using CoreKit::EventInputSource;
using DdsKit::BasicDdsParticipant;
using DdsKit::ConfiguredDdsTopic;
using DdsKit::newCallbackForTopic;

namespace
{
    const char *DDS_DOMAIN_FLAG = "dds-domain";

    std::string CommandSampleToString(MySoftwareBus::Command const& aCommand)
    {
        std::stringstream result;

        result << "{ \"statement\": \"" << DdsKit::GetStringContents(aCommand.statement) << "\" }";

        return result.str();
    }
}

MyAppDelegate::MyAppDelegate():
    m_commandTopic(nullptr),
    m_ddsDomainId(-1),
    m_engrDataTopic(nullptr)
{

}

MyAppDelegate::~MyAppDelegate()
{

}

void
MyAppDelegate::configureCmdLineArgs(Application *theApp)
{
    theApp->addCmdLineArgDef(
        Application::CmdLineArg(
            DDS_DOMAIN_FLAG,
            true,
            "ID of the DDS domain to participate in."
        )
    );
    BasicDdsParticipant::AddDdsRelatedCmdArgs(theApp);
}

void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
    std::vector< std::string > ddsProviderArgs;
    m_ddsDomainId = std::stoi(theApp->getCmdLineArgFor(DDS_DOMAIN_FLAG));
    ddsProviderArgs = BasicDdsParticipant::GatherDdsRelatedCmdArgs(theApp);
    m_participant.reset(
        new BasicDdsParticipant(
            m_ddsDomainId,
            theApp->log().appName(),
            ddsProviderArgs,
            &theApp->log()
        )
    );
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
    m_participant->startDds();
    m_commandTopic = new ConfiguredDdsTopic< CommandConfig >(
        theApp->getMainRunLoop(),
        m_participant.get()
    );
    m_commandTopic->createWriter(CommandConfig::NormallyReliable);
    m_engrDataTopic = new ConfiguredDdsTopic< EngineeringDataConfig >(
        theApp->getMainRunLoop(),
        m_participant.get()
    );
    m_engrDataTopic->createReader(EngineeringDataConfig::NormallyReliable);
    m_engrDataTopic->registerCallback(
        newCallbackForTopic(
            EngineeringDataConfig(),
            std::bind(
                std::mem_fn(&FrontPanel::onEngineeringData),
                m_frontPanel,
                std::placeholders::_1
            )
        )
    );
    m_engrDataTopic->listenForSamples();
    m_frontPanel->initialize(m_commandDispatch);
}

void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    if (m_frontPanel)
    {
        m_frontPanel->terminate();
    }
    if (m_participant)
    {
        m_engrDataTopic->stopListening();
        m_participant->stopDds();
        m_engrDataTopic = nullptr;
        m_commandTopic = nullptr;
    }
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
    m_participant.reset();
}


void
MyAppDelegate::onCommandRequest(std::string commandStatement)
{
    if (m_commandTopic != nullptr)
    {
        MySoftwareBus::Command theCommand;

        DdsKit::SetUnboundedString(theCommand.statement, commandStatement.c_str());

        m_commandTopic->writeSample(&theCommand);
    }
}

// vim: set ts=4 sw=4 expandtab:
