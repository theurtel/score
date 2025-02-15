#pragma once
#include <Process/Process.hpp>
#include <Process/ProcessFactory.hpp>
#include <Process/ProcessList.hpp>

#include <score/application/ApplicationContext.hpp>
#include <score/command/AggregateCommand.hpp>
#include <score/command/Command.hpp>
#include <score/model/EntityMap.hpp>
#include <score/model/Identifier.hpp>
#include <score/model/path/Path.hpp>
#include <score/model/path/PathSerialization.hpp>
#include <score/plugins/InterfaceList.hpp>
#include <score/plugins/StringFactoryKey.hpp>
#include <score/serialization/DataStreamVisitor.hpp>
#include <score/tools/IdentifierGeneration.hpp>
#include <score/tools/std/Optional.hpp>

#include <QString>

#include <Scenario/Commands/Interval/AddOnlyProcessToInterval.hpp>
#include <Scenario/Commands/ScenarioCommandFactory.hpp>
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Document/Interval/Slot.hpp>
#include <Scenario/Document/State/ItemModel/MessageItemModel.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#include <Scenario/Process/ScenarioInterface.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Settings/ScenarioSettingsModel.hpp>

#include <vector>

namespace Scenario
{
namespace Command
{
class SCORE_PLUGIN_SCENARIO_EXPORT LoadLayerInInterval final
    : public score::Command
{
  SCORE_COMMAND_DECL(
      CommandFactoryName(),
      LoadLayerInInterval,
      "Load a process in an interval")

public:
  LoadLayerInInterval(
      const IntervalModel& interval,
      const rapidjson::Value& dat);
  ~LoadLayerInInterval();

  void undo(const score::DocumentContext& ctx) const override;
  void redo(const score::DocumentContext& ctx) const override;

  const Path<IntervalModel>& intervalPath() const;
  const Id<Process::ProcessModel>& processId() const;

private:
  void serializeImpl(DataStreamInput& s) const override;
  void deserializeImpl(DataStreamOutput& s) override;
  LoadOnlyLayerInInterval m_addProcessCommand;
  bool m_addedSlot{};
};

class SCORE_PLUGIN_SCENARIO_EXPORT AddProcessInNewBoxMacro final
    : public score::AggregateCommand
{
  SCORE_COMMAND_DECL(
      CommandFactoryName(),
      AddProcessInNewBoxMacro,
      "Add a process in a new box")

public:
  ~AddProcessInNewBoxMacro();
};

class SCORE_PLUGIN_SCENARIO_EXPORT DropProcessInIntervalMacro final
    : public score::AggregateCommand
{
  SCORE_COMMAND_DECL(
      CommandFactoryName(),
      DropProcessInIntervalMacro,
      "Drop a process in an interval")
};
}
}
