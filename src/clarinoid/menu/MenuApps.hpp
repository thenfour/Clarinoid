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

struct 
{
  SynthSettingsApp gSynthSettingsApp;
  LoopSettingsApp gLoopSettingsApp;
  HarmSettingsApp gHarmSettingsApp;
  SystemSettingsApp gSystemSettingsApp;
  TouchKeyGraphs gTouchKeyApp;
  MetronomeSettingsApp gMetronomeApp;
  DebugMenuApp gDebugApp;
  ProfileMenuApp gProfileApp;
} gAllMenuApps;

MenuAppBase* gMenuAppArray[] = 
{
  &gAllMenuApps.gSynthSettingsApp,
  &gAllMenuApps.gLoopSettingsApp,
  &gAllMenuApps.gHarmSettingsApp,
  &gAllMenuApps.gSystemSettingsApp,
  &gAllMenuApps.gTouchKeyApp,
  &gAllMenuApps.gMetronomeApp,
  &gAllMenuApps.gDebugApp,
  &gAllMenuApps.gProfileApp,
};

static constexpr auto x1341 = sizeof(gAllMenuApps.gSynthSettingsApp);
static constexpr auto x1342 = sizeof(gAllMenuApps.gLoopSettingsApp);
static constexpr auto x1343 = sizeof(gAllMenuApps.gHarmSettingsApp);
static constexpr auto x13434 = sizeof(Property<uint8_t>);
static constexpr auto x1344 = sizeof(gAllMenuApps.gSystemSettingsApp);
static constexpr auto x1345 = sizeof(gAllMenuApps.gTouchKeyApp);
static constexpr auto x1346 = sizeof(gAllMenuApps.gMetronomeApp);
static constexpr auto x1347 = sizeof(gAllMenuApps.gDebugApp);
static constexpr auto x1348 = sizeof(gAllMenuApps.gProfileApp);

static constexpr auto x781 = sizeof(gAllMenuApps);

StaticInit gInitMenuApps([](){ gDisplay.InitMenuApps(cc::make_array_view(gMenuAppArray)); });
