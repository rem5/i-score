#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <Scenario/Document/Constraint/Rack/RackModel.hpp>
#include <Scenario/Document/Constraint/ViewModels/ConstraintViewModel.hpp>

#include <QDataStream>
#include <QtGlobal>
#include <type_traits>
#include <utility>

#include "RemoveRackFromConstraint.hpp"
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/model/EntityMap.hpp>
#include <iscore/model/path/Path.hpp>
#include <iscore/model/path/PathSerialization.hpp>
#include <iscore/model/path/ObjectIdentifier.hpp>

namespace Scenario
{
namespace Command
{

RemoveRackFromConstraint::RemoveRackFromConstraint(Path<RackModel>&& rackPath)
{
  auto trimmedRackPath = std::move(rackPath).splitLast<ConstraintModel>();

  m_path = std::move(trimmedRackPath.first);
  m_rackId = Id<RackModel>{trimmedRackPath.second.id()};

  auto& constraint = m_path.find();
  // Save the rack
  DataStream::Serializer s{&m_serializedRackData};
  s.readFrom(constraint.racks.at(m_rackId));

  // Save for each view model of this constraint
  // a bool indicating if the rack being deleted
  // was displayed
  for (const ConstraintViewModel* vm : constraint.viewModels())
  {
    m_rackMappings[vm->id()] = vm->shownRack() == m_rackId;
  }
}

RemoveRackFromConstraint::RemoveRackFromConstraint(
    Path<ConstraintModel>&& constraintPath, Id<RackModel> rackId)
    : m_path{constraintPath}, m_rackId{std::move(rackId)}
{
  auto& constraint = m_path.find();

  DataStream::Serializer s{&m_serializedRackData};
  s.readFrom(constraint.racks.at(m_rackId));

  for (const ConstraintViewModel* vm : constraint.viewModels())
  {
    m_rackMappings[vm->id()] = vm->shownRack() == m_rackId;
  }
}

void RemoveRackFromConstraint::undo() const
{
  auto& constraint = m_path.find();
  DataStream::Deserializer s{m_serializedRackData};
  constraint.racks.add(new RackModel{s, &constraint});

  for (ConstraintViewModel* vm : constraint.viewModels())
  {
    if (m_rackMappings[vm->id()])
    {
      vm->showRack(m_rackId);
    }
  }
}

void RemoveRackFromConstraint::redo() const
{
  auto& constraint = m_path.find();
  constraint.racks.remove(m_rackId);
}

void RemoveRackFromConstraint::serializeImpl(DataStreamInput& s) const
{
  s << m_path << m_rackId << m_serializedRackData << m_rackMappings;
}

void RemoveRackFromConstraint::deserializeImpl(DataStreamOutput& s)
{
  s >> m_path >> m_rackId >> m_serializedRackData >> m_rackMappings;
}
}
}
