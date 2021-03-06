#include <Scenario/Document/Constraint/ViewModels/ConstraintViewModel.hpp>

#include <algorithm>
#include <iscore/serialization/DataStreamVisitor.hpp>

#include "HideRackInViewModel.hpp"
#include <iscore/model/path/Path.hpp>
#include <iscore/model/path/PathSerialization.hpp>

namespace Scenario
{
namespace Command
{

HideRackInViewModel::HideRackInViewModel(Path<ConstraintViewModel>&& path)
    : m_constraintViewPath{std::move(path)}
{
  auto& constraint_vm = m_constraintViewPath.find();
  m_constraintPreviousId = constraint_vm.shownRack();
}

HideRackInViewModel::HideRackInViewModel(
    const ConstraintViewModel& constraint_vm)
    : m_constraintViewPath{constraint_vm}
{
  m_constraintPreviousId = constraint_vm.shownRack();
}

void HideRackInViewModel::undo() const
{
  auto& vm = m_constraintViewPath.find();
  vm.showRack(m_constraintPreviousId);
}

void HideRackInViewModel::redo() const
{
  auto& vm = m_constraintViewPath.find();
  vm.hideRack();
}

void HideRackInViewModel::serializeImpl(DataStreamInput& s) const
{
  s << m_constraintViewPath << m_constraintPreviousId;
}

void HideRackInViewModel::deserializeImpl(DataStreamOutput& s)
{
  s >> m_constraintViewPath >> m_constraintPreviousId;
}
}
}
