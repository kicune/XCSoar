// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "co/Task.hxx"
#include "Plane/Plane.hpp"

#include <boost/json/fwd.hpp>

#include <cstdint>

struct WeGlideSettings;
class Path;
class CurlGlobal;
class ProgressListener;

namespace WeGlide {

Co::Task<boost::json::value>
UploadFlight(CurlGlobal &curl, const WeGlideSettings &settings,
             uint_least32_t glider_type,
             Plane &plane,
             Path igc_path,
             ProgressListener &progress);

} // namespace WeGlide
