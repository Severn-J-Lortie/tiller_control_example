// This is a small proof of concept program that reads the extension of the 
// Pcnautic tiller via the LabJack U3-HV DAQ. The program relies on LabJack's
// Exodriver, a bare-bones C-driver for the LabJack. It also makes use of 
// some helper functions written by LabJack which make setup less cumbersome.
// Those functions come from u3.h and u3.c.
//
// Date: February, 2026
// Author: Severn Lortie

#include <iostream>
#include <format>

#include <u3.h>

int main() {
  // Open the first U3 found connected to USB
  int local_id = -1;
  HANDLE h_device = openUSBConnection(local_id);
  if (h_device == NULL) {
    std::cout << "Couldn't find any LabJack devices." << std::endl;
    return 0;
  }

  // The LabJack comes with some factory default calibration information.
  // This is needed by later functions to do conversion math. We store it in the
  // u3CalibrationInfo struct.
  u3CalibrationInfo calibration_info;
  long error = getCalibrationInfo(h_device, &calibration_info);
  if (error < 0) {
    std::cout << "Couldn't get calibration info from the LabJack device." << std::endl;
    closeUSBConnection(h_device);
    return 0;
  }

  // At this point we should be setup with the LabJack. So, we can use the eAIN function to read the
  // analog voltage from the AIN3 pin.
  double voltage = 0.0;
  error = eAIN(h_device, &calibration_info, 1, 0, 0, 31, &voltage, 0, 0, 0, 0, 0, 0);
  if (error != 0) {
    std::cout << "Couldn't read the voltage from AIN0" << std::endl;
    closeUSBConnection(h_device);
    return 0;
  }
  std::cout << std::format("The voltage of input AIN0 is {:.2f}", voltage) << std::endl;

}