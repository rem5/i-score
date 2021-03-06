#include <Process/LayerModel.hpp>
#include <Scenario/Document/Constraint/Rack/Slot/SlotModel.hpp>
#include <iscore/model/path/RelativePath.hpp>

#include "RemoveLayerModelFromSlot.hpp"
#include <Process/ProcessList.hpp>
#include <Process/Process.hpp>
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/model/EntityMap.hpp>
#include <iscore/model/path/Path.hpp>
#include <iscore/model/path/PathSerialization.hpp>
#include <iscore/application/ApplicationContext.hpp>

namespace Scenario
{
namespace Command
{

RemoveLayerModelFromSlot::RemoveLayerModelFromSlot(
    Path<SlotModel>&& rackPath, Id<Process::LayerModel> layerId)
    : m_path{rackPath}, m_layerId{std::move(layerId)}
{
  auto& slot = m_path.find();
  auto& lm = slot.layers.at(m_layerId);

  DataStream::Serializer s{&m_serializedLayerData};

  s.readFrom(iscore::RelativePath(*lm.parent(), lm.processModel()));
  s.readFrom(lm);
}

void RemoveLayerModelFromSlot::undo() const
{
  auto& slot = m_path.find();
  DataStream::Deserializer s{m_serializedLayerData};

  iscore::RelativePath p;
  s.writeTo(p);

  auto lm = deserialize_interface(
      this->context.interfaces<Process::LayerFactoryList>(), s, p, &slot);
  if (lm)
    slot.layers.add(lm);
  else
    ISCORE_TODO;
}

void RemoveLayerModelFromSlot::redo() const
{
  auto& slot = m_path.find();
  slot.layers.remove(m_layerId);
}

void RemoveLayerModelFromSlot::serializeImpl(DataStreamInput& s) const
{
  s << m_path << m_layerId << m_serializedLayerData;
}

void RemoveLayerModelFromSlot::deserializeImpl(DataStreamOutput& s)
{
  s >> m_path >> m_layerId >> m_serializedLayerData;
}
}
}
