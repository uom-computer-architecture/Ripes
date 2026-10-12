#pragma once

#include "assembler/assemblerbase.h"
#include "assembler/program.h"
#include <QVariantMap>

namespace Ripes {

// Evaluate captured retirement evidence in the simulator. No source scanning
// or reading memory at retired PCs: the captured word is what actually ran.
inline QVariantMap evaluateInstructionRequirement(
    const QStringList &required, uint64_t sourceStart, bool supported,
    const std::map<std::pair<AInt, VInt>, uint64_t> &histogram,
    const Program::SourceMapping &sourceMapping,
    const Assembler::AssemblerBase &assembler) {
  QVariantMap counts;
  for (const auto &name : required)
    counts[name] = QVariant::fromValue(uint64_t{0});
  QVariantMap result{
      {"status", "not_met"}, {"requested", required}, {"counts", counts}};
  auto failure = [&](const QString &reason) {
    result["status"] = "error";
    result["reason"] = reason;
    return result;
  };
  if (!supported)
    return failure("retirement_unsupported");

  for (const auto &[location, count] : histogram) {
    if (count == 0)
      continue;
    const auto &[pc, word] = location;
    if (sourceStart > 1) {
      const auto source = sourceMapping.find(pc);
      if (source == sourceMapping.end() || source->second.empty())
        return failure("source_unmapped");
      const bool firstEligible =
          uint64_t{*source->second.begin()} + 1 >= sourceStart;
      const bool lastEligible =
          uint64_t{*source->second.rbegin()} + 1 >= sourceStart;
      if (firstEligible != lastEligible)
        return failure("source_ambiguous");
      if (!firstEligible)
        continue;
    }
    const auto name = assembler.instructionName(word);
    if (name.isEmpty())
      return failure("unknown_retired_instruction");
    if (counts.contains(name))
      counts[name] = QVariant::fromValue(counts[name].toULongLong() + count);
  }
  result["counts"] = counts;
  for (const auto &count : counts)
    if (count.toULongLong() > 0) {
      result["status"] = "met";
      break;
    }
  return result;
}

} // namespace Ripes
