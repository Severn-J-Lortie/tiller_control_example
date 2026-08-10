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
#include <chrono>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#include <u3.h>

volatile sig_atomic_t keep_running = 1;

void sigint_handler(int) {
  keep_running = 0;
}

int main() {
  signal(SIGINT, sigint_handler);

  // Setup terminal for non-blocking read
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  // Open the first U3 found connected to USB
  int local_id = -1;
  HANDLE h_device = openUSBConnection(local_id);
  if (h_device == NULL) {
    std::cout << "Couldn't find any LabJack devices." << std::endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
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
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return 0;
  }

  while (keep_running) {
    auto loop_start = std::chrono::steady_clock::now();

    // At this point we should be setup with the LabJack. So, we can use the eAIN function to read the
    // analog voltage from the AIN0 pin.
    double voltage = 0.0;
    error = eAIN(h_device, &calibration_info, 1, 0, 0, 31, &voltage, 0, 0, 0, 0, 0, 0);
    if (error != 0) {
      std::cout << "\nCouldn't read the voltage from AIN0" << std::endl;
      break;
    }

    double raw_angle = (voltage - 0.5) * 90.0;

    // Wrap the angle to [-180, 180] so that 300 degrees becomes -60 degrees
    double centered_angle = raw_angle;
    while (centered_angle > 180.0) centered_angle -= 360.0;
    while (centered_angle < -180.0) centered_angle += 360.0;

    // Scale the +/- 60 degree physical range to +/- 90 degrees
    double angle = centered_angle * 1.5;

    std::cout << std::format("\rThe voltage of input AIN0 is {:.2f} V, Angle is {:.2f} deg    ", voltage, angle) << std::flush;

    auto next_time = loop_start + std::chrono::milliseconds(100);

    while (keep_running) {
      auto now = std::chrono::steady_clock::now();
      int timeout = std::chrono::duration_cast<std::chrono::milliseconds>(next_time - now).count();
      if (timeout <= 0) break;

      struct pollfd pfd;
      pfd.fd = STDIN_FILENO;
      pfd.events = POLLIN;

      if (poll(&pfd, 1, timeout) > 0) {
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) {
          if (c == 'q' || c == 'Q') {
            keep_running = 0;
            break;
          }
        }
      }
    }
  }
  std::cout << std::endl;

  closeUSBConnection(h_device);
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return 0;
}