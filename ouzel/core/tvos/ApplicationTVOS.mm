// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#import <UIKit/UIKit.h>
#include "ApplicationTVOS.h"
#include "core/Application.h"
#include "core/Engine.h"

@interface AppDelegate: UIResponder<UIApplicationDelegate>

@end

@implementation AppDelegate

-(BOOL)application:(__unused UIApplication*)application willFinishLaunchingWithOptions:(__unused NSDictionary*)launchOptions
{
    ouzel::sharedApplication->init();

    return YES;
}

-(BOOL)application:(__unused UIApplication*)application didFinishLaunchingWithOptions:(__unused NSDictionary*)launchOptions
{
    if (ouzel::sharedEngine)
    {
        ouzel::sharedEngine->start();
    }

    return YES;
}

-(void)applicationDidBecomeActive:(__unused UIApplication*)application
{
    ouzel::sharedEngine->resume();
}

-(void)applicationWillResignActive:(__unused UIApplication*)application
{
    ouzel::sharedEngine->pause();
}

-(void)applicationDidEnterBackground:(__unused UIApplication*)application
{
}

-(void)applicationWillEnterForeground:(__unused UIApplication*)application
{
}

-(void)applicationWillTerminate:(__unused UIApplication*)application
{
    ouzel::sharedApplication->exit();

    if (ouzel::sharedEngine)
    {
        ouzel::sharedEngine->stop();
    }
}

-(void)applicationDidReceiveMemoryWarning:(__unused UIApplication*)application
{
    if (ouzel::sharedEngine)
    {
        ouzel::Event event;
        event.type = ouzel::Event::Type::LOW_MEMORY;

        ouzel::sharedEngine->getEventDispatcher()->postEvent(event);
    }
}

@end

namespace ouzel
{
    ApplicationTVOS::ApplicationTVOS(int aArgc, char* aArgv[]):
        Application(aArgc, aArgv)
    {
        mainQueue = dispatch_get_main_queue();
    }

    int ApplicationTVOS::run()
    {
        @autoreleasepool
        {
            return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
        }
    }

    void ApplicationTVOS::execute(const std::function<void(void)>& func)
    {
        if (func)
        {
            std::function<void(void)> localFunction = func;

            dispatch_async(mainQueue, ^{
                localFunction();
            });
        }
    }

    bool ApplicationTVOS::openURL(const std::string& url)
    {
        NSString* nsStringURL = [NSString stringWithUTF8String:url.c_str()];
        NSURL* nsURL = [NSURL URLWithString:nsStringURL];

        return [[UIApplication sharedApplication] openURL:nsURL] == YES;
    }

    void ApplicationTVOS::setScreenSaverEnabled(bool newScreenSaverEnabled)
    {
        Application::setScreenSaverEnabled(newScreenSaverEnabled);

        dispatch_async(mainQueue, ^{
            [UIApplication sharedApplication].idleTimerDisabled = newScreenSaverEnabled ? YES : NO;
        });
    }
}
