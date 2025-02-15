// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <Process/Process.hpp>
#include <Process/ProcessFactory.hpp>
#include <Process/ProcessList.hpp>
#include <Process/ProcessMimeSerialization.hpp>
#include <Process/TimeValue.hpp>
#include <Process/TimeValueSerialization.hpp>

#include <score/model/ModelMetadata.hpp>
#include <score/plugins/InterfaceList.hpp>
#include <score/plugins/StringFactoryKey.hpp>
#include <score/plugins/StringFactoryKeySerialization.hpp>
#include <score/serialization/DataStreamVisitor.hpp>
#include <score/serialization/JSONValueVisitor.hpp>
#include <score/serialization/JSONVisitor.hpp>

template <>
SCORE_LIB_PROCESS_EXPORT void
DataStreamReader::read(const Process::ProcessData& process)
{
  m_stream << process.key << process.prettyName << process.customData;
}

// We only load the members of the process here.
template <>
SCORE_LIB_PROCESS_EXPORT void
DataStreamWriter::write(Process::ProcessData& process)
{
  m_stream >> process.key >> process.prettyName >> process.customData;
}

template <>
SCORE_LIB_PROCESS_EXPORT void
DataStreamReader::read(const Process::ProcessModel& process)
{
  m_stream << process.m_duration << process.m_slotHeight
           << process.m_startOffset << process.m_loopDuration
           << process.m_position << process.m_size << process.m_loops;
}

// We only load the members of the process here.
template <>
SCORE_LIB_PROCESS_EXPORT void
DataStreamWriter::write(Process::ProcessModel& process)
{
  m_stream >> process.m_duration >> process.m_slotHeight
      >> process.m_startOffset >> process.m_loopDuration >> process.m_position
      >> process.m_size >> process.m_loops;
}

template <>
SCORE_LIB_PROCESS_EXPORT void
JSONReader::read(const Process::ProcessModel& process)
{
  obj[strings.Duration] = process.duration();
  obj[strings.Height] = process.getSlotHeight();
  obj["StartOffset"] = process.startOffset();
  obj["LoopDuration"] = process.loopDuration();
  obj["Pos"] = process.m_position;
  obj["Size"] = process.m_size;
  obj["Loops"] = process.loops();
}

template <>
SCORE_LIB_PROCESS_EXPORT void JSONWriter::write(Process::ProcessModel& process)
{
  process.m_duration <<= obj[strings.Duration];

  if (auto it = obj.tryGet(strings.Height))
    process.m_slotHeight = it->toDouble();
  else
    process.m_slotHeight = 300;

  assign_with_default(process.m_startOffset, obj.tryGet("StartOffset"), TimeVal::zero());
  assign_with_default(process.m_loopDuration, obj.tryGet("LoopDuration"), TimeVal::fromMsecs(1000));
  assign_with_default(process.m_loops, obj.tryGet("Loops"), false);
  assign_with_default(process.m_position, obj.tryGet("Pos"), QPointF{});
  assign_with_default(process.m_size, obj.tryGet("Size"), QSize(200, 200));
}
