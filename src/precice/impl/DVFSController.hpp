#pragma once

#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <vector>
#include "../thirdparty/simple-web-server/client_http.hpp"

namespace precice::impl {

namespace asio = boost::asio;

class DVFSController {
  using Clock      = std::chrono::steady_clock;
  using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

public:
  DVFSController(
      std::string_view participantName,
      bool             withDvfs);
  void setNewFrequency();
  void adjustResources(double imbalanceFactor);

private:
  mutable logging::Logger         _log{"impl::DVFSController"};
  std::vector<unsigned>           _frequencies;
  std::unordered_map<int, double> _upShift;
  std::unordered_map<int, double> _downShift;
  unsigned                        currentFrequency;
  double                          _computeSlack(double imbalanceFactor);
  double                          S;
  double                          B;
  double                          alpha;
  bool                            _withDvfs;
  std::string_view                _regionName;
  std::string                     _port;
  std::string_view                _ip;
  std::unique_ptr<HttpClient>     _client;
};

} // namespace precice::impl
