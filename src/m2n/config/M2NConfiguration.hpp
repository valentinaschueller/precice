// Copyright (C) 2011 Technische Universitaet Muenchen
// This file is part of the preCICE project. For conditions of distribution and
// use, please see the license notice at http://www5.in.tum.de/wiki/index.php/PreCICE_License
#pragma once

#include "com/SharedPointer.hpp"
#include "m2n/SharedPointer.hpp"
#include "tarch/logging/Log.h"
#include "utils/xml/XMLTag.hpp"
#include <string>
#include <vector>
#include "boost/tuple/tuple.hpp"

namespace precice {
namespace m2n {

/**
 * @brief Configuration for communication channels between solvers.
 */
class M2NConfiguration : public utils::XMLTag::Listener
{
public:

   typedef boost::tuple<m2n::PtrM2N,std::string,std::string> M2NTuple;

   /**
    * @brief Constructor.
    */
   M2NConfiguration ( utils::XMLTag& parent );

   virtual ~M2NConfiguration() {}

   /**
    * @brief Returns the communication object for the given user names.
    *
    * Exits with an error message, when no object is configured for the given
    * user names.
    */
   m2n::PtrM2N getM2N (
      const std::string& from,
      const std::string& to );

   /**
    * @brief Returns all configured communication objects.
    */
   std::vector<M2NTuple>& m2ns()
   {
      return _m2ns;
   }

   virtual void xmlTagCallback ( utils::XMLTag& callingTag );

   virtual void xmlEndTagCallback ( utils::XMLTag& callingTag ) {}

private:

   static tarch::logging::Log _log;

   const std::string TAG;
   const std::string ATTR_TYPE;
   const std::string ATTR_DISTRIBUTION_TYPE;
   const std::string ATTR_FROM;
   const std::string ATTR_TO;
   const std::string ATTR_PORT;
   const std::string ATTR_NETWORK;
   const std::string ATTR_EXCHANGE_DIRECTORY;

   const std::string VALUE_MPI;
   const std::string VALUE_SOCKETS;

   const std::string VALUE_GATHER_SCATTER;
   const std::string VALUE_POINT_TO_POINT;

   std::vector<M2NTuple> _m2ns;

   void checkDuplicates (
     const std::string& from,
     const std::string& to );
};

}} // namespace precice, m2n

