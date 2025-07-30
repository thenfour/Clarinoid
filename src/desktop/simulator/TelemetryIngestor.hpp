#pragma once
#include "TelemetryMessage.hpp"
#include "SeriesCollection.hpp"
#include "TimeSeriesStore.hpp"


// plumbing between incoming log messages and structured storage & series.
class TelemetryIngestor
{
public:
  TelemetryIngestor(SeriesCollection& reg, TimeSeriesStore& store)
    : mReg(reg)
    , mStore(store)
  {
  }

  // returns true if it's a telemetry event.
  bool HandleLine(const std::string& line)
  {
    TelemetryMessage msg;
    if (!ParseTelemetryMessage(line, msg))
      return false;

    // Map names → ids, build aligned value list
    std::vector<std::pair<SeriesCollection::Id, float>> vals;
    vals.reserve(msg.items.size());
    for (const auto& it : msg.items) {
      auto id = mReg.getOrCreate(it.name);
      mStore.ensure(id);
      vals.emplace_back(id, it.value);
    }
    mStore.appendAligned(vals);
    return true;
  }

private:
  SeriesCollection& mReg;
  TimeSeriesStore& mStore;
};
