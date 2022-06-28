// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "SystemSettings.hpp"
#include "Asset.hpp"

void
SystemSettings::SetDefaults()
{
  for (unsigned i = 0; i < devices.size(); ++i)
    devices[i].Clear();

  if (IsAndroid() || IsApple()) {
    devices[0].port_type = DeviceConfig::PortType::IOIOUART;
    devices[0].baud_rate = 115200;
    devices[0].ioio_uart_id = 0;
    devices[0].driver_name = _T("IMI ERIXX");
    devices[1].port_type = DeviceConfig::PortType::IOIOUART;
    devices[1].baud_rate = 57600;
    devices[1].ioio_uart_id = 1;
    devices[1].driver_name = _T("FLARM");    
  } else {
    devices[0].port_type = DeviceConfig::PortType::SERIAL;
#ifdef _WIN32
    devices[0].path = _T("COM1:");
#else
    devices[0].path = _T("/dev/tty0");
#endif
    devices[0].baud_rate = 4800;
    devices[0].driver_name = _T("Generic");
  }
}
