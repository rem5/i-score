#pragma once
#include <iscore/command/AggregateCommand.hpp>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <Scenario/Document/Constraint/Rack/RackModel.hpp>
#include <Scenario/Document/Constraint/Rack/Slot/SlotModel.hpp>
#include <Scenario/Commands/Constraint/AddOnlyProcessToConstraint.hpp>
#include <Scenario/Commands/Constraint/Rack/AddSlotToRack.hpp>
#include <Scenario/Commands/Constraint/Rack/Slot/AddLayerModelToSlot.hpp>

#include <Process/ProcessFactory.hpp>


namespace Scenario
{
class ConstraintModel;
namespace Command
{
class CreateProcessInNewSlot final : public iscore::AggregateCommand
{
  ISCORE_COMMAND_DECL(
      ScenarioCommandFactoryName(), CreateProcessInNewSlot,
      "Create a process in a new slot")

  public:
  template<typename Dispatcher>
  static void create(
      Dispatcher & disp,
      const Scenario::ConstraintModel& constraint,
      const Scenario::RackModel& rack,
      UuidKey<Process::ProcessModelFactory> proc)
  {
    auto cmd1 = new Scenario::Command::AddOnlyProcessToConstraint(
        constraint, proc);
    cmd1->redo();
    disp.submitCommand(cmd1);

    auto cmd2 = new Scenario::Command::AddSlotToRack(rack);
    cmd2->redo();
    disp.submitCommand(cmd2);

    auto cmd3 = new Scenario::Command::AddLayerModelToSlot(
        rack.slotmodels.at(cmd2->createdSlot()),
        constraint.processes.at(cmd1->processId()));
    cmd3->redo();
    disp.submitCommand(cmd3);

  }
};
}
}
