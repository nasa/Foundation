#include <stdexcept>

#include <UserInputEventDispatch.h>

#include "MyAppDelegate.h"

using CanBusKit::CanBusFrameNotification;
using CanBusKit::CanBusIo;
using CanBusKit::newCanBusFrameCallback;
using CoreKit::Application;
using CoreKit::EventInputSource;

namespace
{
    const char *CAN_IF_FLAG = "can-if";
}

MyAppDelegate::MyAppDelegate():
    m_timerId(-1)
{

}

MyAppDelegate::~MyAppDelegate()
{

}

void
MyAppDelegate::configureCmdLineArgs(Application *theApp)
{
    theApp->addCmdLineArgDef(Application::CmdLineArg(CAN_IF_FLAG, true, "CAN bus interface to use."));
}

void
MyAppDelegate::applicationDidInitialize(Application *theApp)
{
    m_canIfName = theApp->getCmdLineArgFor(CAN_IF_FLAG);
    if (m_canIfName.empty())
    {
        throw std::runtime_error("Must specify CAN bus interface to use.");
    }
    m_canIf = std::make_shared< CanBusIo >(m_canIfName, theApp->getMainRunLoop());
    m_userInput = std::make_shared< UserInputSource >(
        std::make_shared< UserInputEventDispatch >()
    );
    auto targetStateChDispatch = std::make_shared< TargetStateChangeEventDispatch >();
    m_repositoryEvent = std::make_shared< EventInputSource >(targetStateChDispatch.get());
    m_repository = std::make_shared< TargetStateRepository >(
        targetStateChDispatch,
        m_repositoryEvent
    );
    m_statePublishing = std::make_shared< TargetStatePublishing >(
        m_repository,
        m_canIf
    );
    m_frontPanel = std::make_shared< FrontPanel > (theApp);


    m_repository->eventDispatch()->addChangeEventHandler(std::bind(std::mem_fn(&FrontPanel::onTargetStateUpdate), m_frontPanel, std::placeholders::_1));
    m_userInput->dispatch()->addUserInputHandler(std::bind(std::mem_fn(&FrontPanel::onUserInput), m_frontPanel, std::placeholders::_1));
}

void
MyAppDelegate::applicationStarting(Application *theApp)
{
    m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
    theApp->getMainRunLoop()->registerInputSource(m_repositoryEvent.get());
    theApp->getMainRunLoop()->registerInputSource(m_userInput.get());
    m_frontPanel->initialize(m_repository);
    m_canIf->addCanFrameCallback(newCanBusFrameCallback(std::bind(std::mem_fn(&MyAppDelegate::onCanFrame), this, std::placeholders::_1)));
    m_canIf->startCan();
}

void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    m_canIf->stopCan();
    m_frontPanel->terminate();
    theApp->getMainRunLoop()->deregisterInputSource(m_userInput.get());
    theApp->getMainRunLoop()->deregisterInputSource(m_repositoryEvent.get());
    if (m_timerId != -1)
    {
        theApp->getMainRunLoop()->deregisterTimer(m_timerId);
        m_timerId = -1;
    }
}

void
MyAppDelegate::applicationDidTerminate(Application *theApp)
{
    m_frontPanel.reset();
    m_statePublishing.reset();
    m_repository.reset();
    m_repositoryEvent.reset();
    m_userInput.reset();
    m_canIf.reset();
}


void
MyAppDelegate::timerExpired(int timerId)
{
    m_statePublishing->sendUpdate();
}


void
MyAppDelegate::onCanFrame(CanBusKit::CanBusFrameNotification *incoming)
{
    m_repository->set(incoming->canId, incoming->canPayload[0]);
}

// vim: set ts=4 sw=4 expandtab:
