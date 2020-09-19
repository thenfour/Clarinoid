#pragma once

enum class Tristate
{
  Null,
  Position1,
  Position2,
  Position3
};

const char *ToString(Tristate t) {
  switch (t){
    case Tristate::Position1:
      return "Pos1";
    case Tristate::Position2:
      return "Pos1";
    case Tristate::Position3:
      return "Pos3";
    default:
      break;
  }
  return "null";
}
