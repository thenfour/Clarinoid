// SerialPort.h - minimal, binary-safe, overlapped Win32 serial helper for Arduino/Teensy
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <atomic>
#include <mutex>
#include <cassert>
#include <setupapi.h>
#include <regex>

#pragma comment(lib, "Setupapi.lib")

struct PortInfo
{
  std::wstring comName;      // "COM9"
  std::wstring portPath;     // "\\\\?\\usb#vid_..."
  std::wstring friendlyName; // "USB Serial Device (COM9)"
  std::wstring manufacturer; // "Teensyduino" / "Microsoft" ...
  std::wstring pnpId;        // "USB\\VID_16C0&PID_0483..."
  std::wstring vid, pid;     // "16C0" / "0483"
  std::wstring knownDevice;  // "Teensy", "Arduino", "CH340 clone" ...
  std::wstring knownProduct; // "Teensy (USB Serial)", "Arduino Uno R3" ...
};

inline std::wstring
makeKey(const std::wstring& vid, const std::wstring& pid)
{
  return vid + L":" + pid;
}

inline std::vector<PortInfo>
enumSerialPorts()
{
  /*----- 1.  Hard-coded VID:PID → strings  ------------------------------*/
  static const std::unordered_map<std::wstring, std::pair<std::wstring, std::wstring>> kMap = {
    { L"16C0:0483", { L"Teensy", L"Teensy (USB Serial)" } }, // :contentReference[oaicite:0]{index=0}
    { L"2341:0043", { L"Arduino", L"Arduino Uno R3" } },     // :contentReference[oaicite:1]{index=1}
    { L"2A03:0043", { L"Arduino", L"Arduino Uno R3 (clone)" } },
    { L"2341:8036", { L"Arduino", L"Arduino Leonardo" } },
    { L"1A86:7523", { L"Clone USB-Serial", L"CH340 / CH341" } }, // :contentReference[oaicite:2]{index=2}
    { L"0403:6001", { L"Clone USB-Serial", L"FTDI FT232" } },
    { L"10C4:EA60", { L"Clone USB-Serial", L"Silabs CP210x" } }
  };

  /*----- 2.  Enumerate COM-class interfaces ----------------------------*/
  std::vector<PortInfo> out;
  GUID cg = GUID_DEVINTERFACE_COMPORT;
  HDEVINFO h = SetupDiGetClassDevsW(&cg, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (h == INVALID_HANDLE_VALUE)
    return out;

  SP_DEVICE_INTERFACE_DATA ifd{ sizeof(ifd) };
  for (DWORD idx = 0; SetupDiEnumDeviceInterfaces(h, nullptr, &cg, idx, &ifd); ++idx) {
    DWORD need = 0;
    SetupDiGetDeviceInterfaceDetailW(h, &ifd, nullptr, 0, &need, nullptr);
    std::vector<BYTE> buf(need);
    auto* det = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(buf.data());
    det->cbSize = sizeof(*det);
    SP_DEVINFO_DATA dev{ sizeof(dev) };
    if (!SetupDiGetDeviceInterfaceDetailW(h, &ifd, det, need, nullptr, &dev))
      continue;

    PortInfo pi;
    pi.portPath = det->DevicePath;

    /* a) Friendly name + manufacturer */
    auto regStr = [&](DWORD prop) -> std::wstring {
      wchar_t tmp[256];
      if (SetupDiGetDeviceRegistryPropertyW(h, &dev, prop, nullptr, (PBYTE)tmp, sizeof(tmp), nullptr))
        return tmp;
      return L"";
    };
    pi.friendlyName = regStr(SPDRP_FRIENDLYNAME);
    pi.manufacturer = regStr(SPDRP_MFG);

    /* b) Extract "COMx" from friendly string */
    if (auto p = pi.friendlyName.find(L"(COM"); p != std::wstring::npos)
      pi.comName = pi.friendlyName.substr(p + 1, pi.friendlyName.find(L')', p) - p - 1);

    /* c) Parse VID/PID from instance ID */
    wchar_t inst[256];
    if (SetupDiGetDeviceInstanceIdW(h, &dev, inst, std::size(inst), nullptr)) {
      pi.pnpId = inst;
      std::wregex rx(LR"(VID_([0-9A-F]{4}).*PID_([0-9A-F]{4}))", std::regex::icase);
      std::wsmatch m;
      if (std::regex_search(pi.pnpId, m, rx) && m.size() == 3) {
        pi.vid = m[1].str();
        pi.pid = m[2].str();
      }
    }

    /* d) Lookup in table (or fall back on vendor) */
    auto k = makeKey(pi.vid, pi.pid);
    if (auto it = kMap.find(k); it != kMap.end()) {
      pi.knownDevice = it->second.first;
      pi.knownProduct = it->second.second;
    } else if (pi.vid == L"16C0") {
      pi.knownDevice = L"Teensy";
    } else if (pi.vid == L"2341" || pi.vid == L"2A03") {
      pi.knownDevice = L"Arduino";
    }

    out.push_back(std::move(pi));
  }
  SetupDiDestroyDeviceInfoList(h);
  return out;
}

class SerialPort
{
public:
  using RxCallback = std::function<void(const uint8_t*, size_t)>;

  SerialPort() = default;
  ~SerialPort() { close(); }

  // Open port (e.g., L"COM5") at given baud (e.g., 115200). 8N1 by default.
  bool open(const PortInfo& port, DWORD baud = 115200)
  {
    close();

    std::wstring path = normalizePortName(port.portPath);
    handle_ = ::CreateFileW(
      path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
    if (handle_ == INVALID_HANDLE_VALUE) {
      return false;
    }

    // Optional: enlarge driver queues
    ::SetupComm(handle_, 1 << 16, 1 << 16);

    // Configure line (8N1, chosen baud)
    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    if (!::GetCommState(handle_, &dcb)) {
      failOpen();
      return false;
    }

    dcb.BaudRate = baud;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE; // keep DTR asserted (Arduino/Teensy often wait for it)
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fAbortOnError = FALSE;

    if (!::SetCommState(handle_, &dcb)) {
      failOpen();
      return false;
    }

    // Timeouts (overlapped reads ignore these; writes can still use them)
    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = MAXDWORD; // non-blocking overlapped read pattern
    to.ReadTotalTimeoutMultiplier = 0;
    to.ReadTotalTimeoutConstant = 0;
    to.WriteTotalTimeoutMultiplier = 0;
    to.WriteTotalTimeoutConstant = 0;
    ::SetCommTimeouts(handle_, &to);

    ::PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    // Create events for overlapped ops
    ovRead_.hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    ovWrite_.hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    stopEvent_ = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!ovRead_.hEvent || !ovWrite_.hEvent || !stopEvent_) {
      failOpen();
      return false;
    }

    running_.store(true);
    reader_ = std::thread(&SerialPort::readerLoop_, this);
    return true;
  }

  void close()
  {
    if (handle_ == INVALID_HANDLE_VALUE)
      return;

    running_.store(false);
    if (stopEvent_)
      ::SetEvent(stopEvent_);
    // Cancel any pending I/O then join thread
    ::CancelIoEx(handle_, nullptr);
    if (reader_.joinable())
      reader_.join();

    if (ovRead_.hEvent) {
      ::CloseHandle(ovRead_.hEvent);
      ovRead_.hEvent = nullptr;
    }
    if (ovWrite_.hEvent) {
      ::CloseHandle(ovWrite_.hEvent);
      ovWrite_.hEvent = nullptr;
    }
    if (stopEvent_) {
      ::CloseHandle(stopEvent_);
      stopEvent_ = nullptr;
    }
    ::CloseHandle(handle_);
    handle_ = INVALID_HANDLE_VALUE;
  }

  bool isOpen() const { return handle_ != INVALID_HANDLE_VALUE; }

  // Write arbitrary binary
  bool writeBytes(const void* data, size_t size, DWORD* outWritten = nullptr)
  {
    if (!isOpen() || !data || size == 0)
      return false;
    DWORD written = 0;
    DWORD toWrite = static_cast<DWORD>(size); // safe up to 4GB per call
    OVERLAPPED ov{};
    ov.hEvent = ovWrite_.hEvent;
    ::ResetEvent(ovWrite_.hEvent);
    BOOL ok = ::WriteFile(handle_, data, toWrite, nullptr, &ov);
    if (!ok) {
      DWORD err = ::GetLastError();
      if (err != ERROR_IO_PENDING)
        return false;
      DWORD wait = ::WaitForSingleObject(ovWrite_.hEvent, INFINITE);
      if (wait != WAIT_OBJECT_0)
        return false;
      if (!::GetOverlappedResult(handle_, &ov, &written, FALSE))
        return false;
    } else {
      if (!::GetOverlappedResult(handle_, &ov, &written, TRUE))
        return false;
    }
    if (outWritten)
      *outWritten = written;
    return written == toWrite;
  }

  // Convenience for text
  bool writeString(const std::string& s) { return writeBytes(s.data(), s.size()); }

  // Optional: DTR/RTS control
  bool setDtr(bool on)
  {
    if (!isOpen())
      return false;
    return ::EscapeCommFunction(handle_, on ? SETDTR : CLRDTR);
  }
  bool setRts(bool on)
  {
    if (!isOpen())
      return false;
    return ::EscapeCommFunction(handle_, on ? SETRTS : CLRRTS);
  }

  // Receive callback: called from a background thread with raw bytes (binary-safe)
  void setReceiveCallback(RxCallback cb)
  {
    std::lock_guard<std::mutex> lock(cbMutex_);
    rx_ = std::move(cb);
  }

  // Optional: change read buffer size (default 4096)
  void setReadBufferSize(DWORD bytes)
  {
    if (bytes == 0)
      bytes = 4096;
    readBufSize_ = bytes;
  }

  //// Quick port listing using QueryDosDevice (no SetupAPI)
  // static std::vector<std::wstring> listAvailablePorts()
  //{
  //   std::vector<std::wstring> ports;
  //   wchar_t target[256];
  //   for (int i = 1; i <= 256; ++i) {
  //     wchar_t name[16];
  //     swprintf_s(name, L"COM%d", i);
  //     if (::QueryDosDeviceW(name, target, static_cast<DWORD>(std::size(target))) != 0) {
  //       ports.emplace_back(name);
  //     }
  //   }
  //   return ports;
  // }

  // Helper: COM10+ requires prefix. Accepts "COM5" or "\\.\COM5".
  static std::wstring normalizePortName(const std::wstring& name)
  {
    if (name.rfind(LR"(\\.\)", 0) == 0)
      return name; // already normalized
    if (name.rfind(L"COM", 0) == 0)
      return L"\\\\.\\" + name;
    return name; // allow full device paths if supplied
  }

private:
  void readerLoop_()
  {
    std::vector<uint8_t> buf(readBufSize_);
    while (running_.load()) {
      DWORD got = 0;
      OVERLAPPED ov{};
      ov.hEvent = ovRead_.hEvent;
      ::ResetEvent(ovRead_.hEvent);
      BOOL ok = ::ReadFile(handle_, buf.data(), static_cast<DWORD>(buf.size()), nullptr, &ov);
      if (!ok) {
        DWORD err = ::GetLastError();
        if (err == ERROR_IO_PENDING) {
          HANDLE waitHandles[2] = { stopEvent_, ovRead_.hEvent };
          DWORD w = ::WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
          if (w == WAIT_OBJECT_0) {
            // stop requested
            ::CancelIoEx(handle_, &ov);
            break;
          }
          if (w != WAIT_OBJECT_0 + 1) {
            // unexpected; bail
            break;
          }
          if (!::GetOverlappedResult(handle_, &ov, &got, FALSE)) {
            // device removed or other I/O error
            break;
          }
        } else {
          // e.g., device unplugged
          break;
        }
      } else {
        if (!::GetOverlappedResult(handle_, &ov, &got, TRUE)) {
          break;
        }
      }

      if (got > 0) {
        RxCallback cb;
        {
          std::lock_guard<std::mutex> lock(cbMutex_);
          cb = rx_;
        }
        if (cb)
          cb(buf.data(), got);
      }
    }
  }

  void failOpen()
  {
    if (handle_ != INVALID_HANDLE_VALUE) {
      ::CloseHandle(handle_);
      handle_ = INVALID_HANDLE_VALUE;
    }
  }

  HANDLE handle_ = INVALID_HANDLE_VALUE;
  OVERLAPPED ovRead_{};
  OVERLAPPED ovWrite_{};
  HANDLE stopEvent_ = nullptr;
  std::thread reader_;
  std::atomic<bool> running_{ false };
  std::mutex cbMutex_;
  RxCallback rx_;
  DWORD readBufSize_ = 4096;
};

//
// void
// setup()
//{
//  Serial.begin(115200);
//  while (!Serial && millis() < 4000) { /* wait for USB */
//  }
//  pinMode(LED_BUILTIN, OUTPUT);
//}
//
// bool b = false;
//
// void
// loop()
//{
//  b = !b;
//  delay(333);
//  Serial.println(String(micros()));
//  digitalWrite(LED_BUILTIN, b ? HIGH : LOW);
//}
