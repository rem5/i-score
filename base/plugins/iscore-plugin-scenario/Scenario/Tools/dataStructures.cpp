#include <Process/ProcessList.hpp>
#include <QDataStream>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <Scenario/Document/Constraint/ViewModels/ConstraintViewModel.hpp>
#include <Scenario/Process/Algorithms/ProcessPolicy.hpp>
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/model/path/PathSerialization.hpp>
#include <iscore/application/ApplicationContext.hpp>

#include "dataStructures.hpp"
#include <iscore/model/Identifier.hpp>
#include <iscore_plugin_scenario_export.h>

namespace Scenario
{
ConstraintSaveData::ConstraintSaveData(
    const Scenario::ConstraintModel& constraint)
    : constraintPath{constraint}
{
  processes.reserve(constraint.processes.size());
  for (const auto& process : constraint.processes)
  {
    QByteArray arr;
    DataStream::Serializer s{&arr};
    s.readFrom(process);
    processes.push_back(std::move(arr));
  }

  racks.reserve(constraint.racks.size());
  for (const auto& rack : constraint.racks)
  {
    QByteArray arr;
    DataStream::Serializer s{&arr};
    s.readFrom(rack);
    racks.push_back(std::move(arr));
  }

  // TODO save view model data instead, since later it
  // might be more than just the shown rack.
  for (const auto& viewmodel : constraint.viewModels())
  {
    viewMapping.insert(viewmodel->id(), viewmodel->shownRack());
  }
}

void ConstraintSaveData::reload(Scenario::ConstraintModel& constraint) const
{
  auto& comps = iscore::AppComponents();
  auto& procsfactories = comps.interfaces<Process::ProcessFactoryList>();
  for (auto& sourceproc : processes)
  {
    DataStream::Deserializer des{sourceproc};
    auto proc = deserialize_interface(procsfactories, des, &constraint);
    if (proc)
      AddProcess(constraint, proc);
    else
      ISCORE_TODO;
  }

  // Restore the rackes
  for (auto& sourcerack : racks)
  {
    DataStream::Deserializer des{sourcerack};
    constraint.racks.add(new RackModel{des, &constraint});
  }

  // Restore the correct rackes in the constraint view models
  if (constraint.processes.size() != 0)
  {
    auto bit = constraint.viewModels().begin(),
         eit = constraint.viewModels().end();
    for (const auto& cvmid : viewMapping.keys())
    {
      auto it = std::find(bit, eit, cvmid);
      ISCORE_ASSERT(it != eit);
      (*it)->showRack(viewMapping.value(cvmid));
    }
  }
}
}
template <typename T>
class Reader;
template <typename T>
class Writer;

template <>
ISCORE_PLUGIN_SCENARIO_EXPORT void DataStreamReader::read(
    const Scenario::TimenodeProperties& timenodeProperties)
{
  m_stream << timenodeProperties.oldDate << timenodeProperties.newDate;

  insertDelimiter();
}

template <>
ISCORE_PLUGIN_SCENARIO_EXPORT void DataStreamWriter::write(
    Scenario::TimenodeProperties& timenodeProperties)
{

  m_stream >> timenodeProperties.oldDate >> timenodeProperties.newDate;

  checkDelimiter();
}

//----------

template <>
ISCORE_PLUGIN_SCENARIO_EXPORT void DataStreamReader::read(
    const Scenario::ConstraintSaveData& constraintProperties)
{
  m_stream << constraintProperties.constraintPath
           << constraintProperties.processes << constraintProperties.racks
           << constraintProperties.viewMapping;
  insertDelimiter();
}

template <>
ISCORE_PLUGIN_SCENARIO_EXPORT void DataStreamWriter::write(
    Scenario::ConstraintSaveData& constraintProperties)
{
  m_stream >> constraintProperties.constraintPath
      >> constraintProperties.processes >> constraintProperties.racks
      >> constraintProperties.viewMapping;

  checkDelimiter();
}

template <>
ISCORE_PLUGIN_SCENARIO_EXPORT void DataStreamReader::read(
    const Scenario::ConstraintProperties& constraintProperties)
{
  m_stream << constraintProperties.oldMin << constraintProperties.newMin
           << constraintProperties.oldMax << constraintProperties.newMax;

  readFrom(
      static_cast<const Scenario::ConstraintSaveData&>(constraintProperties));

  insertDelimiter();
}

template <>
ISCORE_PLUGIN_SCENARIO_EXPORT void DataStreamWriter::write(
    Scenario::ConstraintProperties& constraintProperties)
{
  m_stream >> constraintProperties.oldMin >> constraintProperties.newMin
      >> constraintProperties.oldMax >> constraintProperties.newMax;

  writeTo(static_cast<Scenario::ConstraintSaveData&>(constraintProperties));
  checkDelimiter();
}

//----------
template <>
ISCORE_PLUGIN_SCENARIO_EXPORT void DataStreamReader::read(
    const Scenario::ElementsProperties& elementsProperties)
{
  m_stream << elementsProperties.timenodes << elementsProperties.constraints;

  insertDelimiter();
}

template <>
ISCORE_PLUGIN_SCENARIO_EXPORT void DataStreamWriter::write(
    Scenario::ElementsProperties& elementsProperties)
{

  m_stream >> elementsProperties.timenodes >> elementsProperties.constraints;

  checkDelimiter();
}
