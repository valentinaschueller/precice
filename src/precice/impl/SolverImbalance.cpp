#include "precice/impl/SolverImbalance.hpp"
#include <algorithm>
#include <numeric>
#include "logging/LogMacros.hpp"
#include "utils/assertion.hpp"

namespace precice::impl {

double SolverImbalance::getSolverTimeToAdvance()
{
  _solverTimeToAdvance = _solverAdvanceTime / _simulatedTime;
  return _solverTimeToAdvance;
}

void SolverImbalance::startSolver()
{
  PRECICE_ASSERT(_state == State::STOPPED);
  _state     = State::RUNNING;
  _startTime = Clock::now();
}

void SolverImbalance::stopSolver(double solverTimeStepSize)
{
  PRECICE_ASSERT(_state == State::RUNNING);
  _state             = State::STOPPED;
  auto   currentTime = Clock::now();
  double advanceTime = (currentTime - _startTime).count() * 1e-9;
  _solverAdvanceTime += advanceTime;
  _simulatedTime += solverTimeStepSize;
}

void SolverImbalance::computeSolverImbalance(const std::vector<double> &timesToAdvance)
{
  double meanTimeToAdvance = std::accumulate(timesToAdvance.begin(), timesToAdvance.end(), 0.0);
  meanTimeToAdvance        = meanTimeToAdvance / timesToAdvance.size();
  auto maxTimeToAdvance    = std::max_element(timesToAdvance.begin(), timesToAdvance.end());
  _loadBalance             = meanTimeToAdvance / *maxTimeToAdvance;
  _activeFraction          = _solverTimeToAdvance / *maxTimeToAdvance;
}

double SolverImbalance::getLoadBalance() const
{
  return _loadBalance;
}

double SolverImbalance::getActiveFraction() const
{
  return _activeFraction;
}

void SolverImbalance::reset()
{
  PRECICE_DEBUG("resetting SolverImbalance");
  _simulatedTime     = 0.0;
  _solverAdvanceTime = 0.0;
}

void SolverImbalance::addLogEntries(io::TXTTableWriter &writer) const
{
  writer.addData("Load Balance", io::TXTTableWriter::DOUBLE);
  writer.addData("Active Fraction", io::TXTTableWriter::DOUBLE);
}

void SolverImbalance::writeLogEntries(io::TXTTableWriter &writer) const
{
  writer.writeData("Load Balance", getLoadBalance());
  writer.writeData("Active Fraction", getActiveFraction());
}

} // namespace precice::impl
