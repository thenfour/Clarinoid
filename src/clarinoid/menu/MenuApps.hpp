#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"

#include "MenuAppDebug.hpp"
#include "MenuAppHarmonizerSettings.hpp"
#include "MenuAppLoopstationSettings.hpp"
#include "MenuAppMetronome.hpp"
#include "MenuAppProfiler.hpp"
#include "MenuAppSynthSettings.hpp"
#include "MenuAppSystemSettings.hpp"
#include "MenuAppTouchKeyGraphs.hpp"


SynthSettingsApp gSynthSettingsApp;
LoopSettingsApp gLoopSettingsApp;
HarmSettingsApp gHarmSettingsApp;
SystemSettingsApp gSystemSettingsApp;
TouchKeyGraphs gTouchKeyApp;
MetronomeSettingsApp gMetronomeApp;
DebugMenuApp gDebugApp;
ProfileMenuApp gProfileApp;

MenuAppBase* gMenuAppArray[] = 
{
  &gSynthSettingsApp,
  &gLoopSettingsApp,
  &gHarmSettingsApp,
  &gSystemSettingsApp,
  &gTouchKeyApp,
  &gMetronomeApp,
  &gDebugApp,
  &gProfileApp,
};

StaticInit gInitMenuApps([](){ gDisplay.InitMenuApps(cc::make_array_view(gMenuAppArray)); });
