
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{



enum class LooperTrigger : uint8_t
{
  Immediate,
  NoteOn,
  NoteOff,
  Beat1,
  Beat2,
  Beat4,
  Beat8,
};

EnumItemInfo<LooperTrigger> gLooperTriggerTypeItems[7] = {
  { LooperTrigger::Immediate, "Immediate" },
  { LooperTrigger::NoteOn, "NoteOn" },
  { LooperTrigger::NoteOff, "NoteOff" },
  { LooperTrigger::Beat1, "Beat1" },
  { LooperTrigger::Beat2, "Beat2" },
  { LooperTrigger::Beat4, "Beat4" },
  { LooperTrigger::Beat8, "Beat8" },
};

EnumInfo<LooperTrigger> gLooperTriggerTypeInfo ("LooperTrigger", gLooperTriggerTypeItems);



struct LooperSettings
{
  LooperTrigger mTrigger = LooperTrigger::NoteOn;
};




} // namespace
