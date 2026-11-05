#include "precice/impl/DVFSController.hpp"
#include <bits/stdc++.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include "../thirdparty/simple-web-server/client_http.hpp"
#include "logging/LogMacros.hpp"

namespace precice::impl {

std::string getEnvVar(std::string const &key)
{
  char *val = getenv(key.c_str());
  PRECICE_ASSERT(val != NULL);
  return val == NULL ? std::string("") : std::string(val);
}

DVFSController::DVFSController(std::string_view participantName, bool withDvfs)
{
  _withDvfs = withDvfs;
  if (_withDvfs) {
    PRECICE_INFO("running with DVFS");
  } else {
    PRECICE_INFO("running without DVFS");
  }
  if (!_withDvfs)
    return;

  _regionName = participantName;

  _port             = getEnvVar("MERIC_ONLINE");
  std::string nodes = getEnvVar("MERIC_ONLINE_IP");

  std::stringstream ss(nodes);
  std::string       token;

  while (getline(ss, token, ' ')) {
    _clients.push_back(std::make_unique<HttpClient>(token + ":" + _port));
    PRECICE_INFO("IP Address: {}", token);
  }

  auto        response         = _clients.front()->request("GET", "/get_available_cf");
  std::string frequency_string = response->content.string();
  PRECICE_INFO("frequencies: {}", frequency_string);
  frequency_string = frequency_string.substr(1, frequency_string.size() - 2);
  ss               = std::stringstream{frequency_string};
  while (getline(ss, token, ',')) {
    _frequencies.push_back(stoul(token));
  }
  PRECICE_ASSERT(!_frequencies.empty());
  std::sort(_frequencies.begin(), _frequencies.end());

  currentFrequency = _frequencies.back();

  transform(_frequencies.begin(), _frequencies.end(), inserter(_upShift, _upShift.end()), [](int v) { return std::make_pair(v, 1.0); });
  _downShift = std::unordered_map(_upShift);
  B          = 1.5;
  alpha      = 0.25;
  S          = 0.05;

  std::string post_json = std::string("{\"") + std::string(_regionName) + std::string("\": {\"FREQUENCY\": ") + std::to_string(currentFrequency) + std::string("}}");
  for (auto &&_client : _clients) {
    auto r1 = _client->request("GET", "/get_region_stats");
    PRECICE_INFO("Response content: {}", r1->content.string());
    try {
      PRECICE_INFO("Request json: {}", post_json);
      auto r2 = _client->request("POST", "/set_region_param", post_json);
      PRECICE_INFO("Response content: {}", r2->content.string());
    } catch (const SimpleWeb::system_error &e) {
      PRECICE_ERROR("Client request error: {}", e.what());
    }
  }
}

void DVFSController::adjustResources(double imbalanceFactor)
{
  if (!_withDvfs)
    return;
  double slack   = _computeSlack(imbalanceFactor);
  auto   f_index = std::find(_frequencies.begin(), _frequencies.end(), currentFrequency);

  if ((slack > S * _downShift[currentFrequency]) && (f_index != _frequencies.begin())) {
    _downShift[currentFrequency] *= B;
    currentFrequency = *std::prev(f_index);
    setNewFrequency();
  } else if ((slack < alpha * S / _upShift[currentFrequency]) && (f_index < (_frequencies.end() - 1))) {
    _upShift[currentFrequency] *= B;
    currentFrequency = *std::next(f_index);
    setNewFrequency();
  } else {
    PRECICE_INFO("nothing to be done");
  }
}

void DVFSController::setNewFrequency()
{
  if (!_withDvfs)
    return;
  PRECICE_INFO("Setting new frequency {}", currentFrequency);
  for (auto &&_client : _clients) {
    std::string post_json = std::string("{\"") + std::string(_regionName) + std::string("\": {\"FREQUENCY\": ") + std::to_string(currentFrequency) + std::string("}}");
    try {
      PRECICE_INFO("Request json: {}", post_json);
      auto r2 = _client->request("POST", "/set_region_param", post_json);
      PRECICE_INFO("Response content: {}", r2->content.string());
    } catch (const SimpleWeb::system_error &e) {
      PRECICE_ERROR("Client request error: {}", e.what());
    }
  }
}

double DVFSController::_computeSlack(double imbalanceFactor)
{
  return 1 - 1 / imbalanceFactor;
}

} // namespace precice::impl
