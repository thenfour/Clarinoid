#include <Arduino.h>
#include <cmath>

// ================== Config ==================
static const uint8_t MAX_SERIES = 16;
static uint32_t printIntervalMs = 10; // ms

// ================== Types ===================
enum WaveType : uint8_t { WT_SINE, WT_SQUARE, WT_TRI, WT_SAW, WT_NOISE };

struct Series {
  WaveType type = WT_SINE;
  float freq = 1.0f;      // Hz
  float amp  = 1.0f;      // unity
  float phase = 0.0f;     // radians
};

// ================== State ===================
Series series[MAX_SERIES];
uint8_t numSeries = 8;
float noiseAmp = 0.10f;
uint32_t t0_us = 0;
char cmdBuf[128];
bool ledState = false;

// ================== Utils ===================
static inline float fracf(float x) { return x - floorf(x); }

static inline float prand() {
  long r = random(-10000, 10001);
  return (float)r / 10000.0f; // [-1,1]
}

static float waveSample(WaveType wt, float t, float freq) {
  if (wt == WT_NOISE) return prand();
  const float two_pi = 6.283185307179586f;
  float f = freq * t;
  switch (wt) {
    case WT_SINE:   return sinf(two_pi * f);
    case WT_SQUARE: return (sinf(two_pi * f) >= 0.0f) ? 1.0f : -1.0f;
    case WT_TRI: {
      float x = fracf(f);
      return 2.0f * fabsf(2.0f * x - 1.0f) - 1.0f;
    }
    case WT_SAW: {
      float x = fracf(f);
      return 2.0f * x - 1.0f;
    }
    default: return 0.0f;
  }
}

static bool parseUInt8(const char* s, uint8_t& out) {
  if (!s) return false;
  char* endp = nullptr;
  long v = strtol(s, &endp, 10);
  if (endp == s || v < 0 || v > 255) return false;
  out = (uint8_t)v;
  return true;
}

static bool parseFloatTok(const char* s, float& out) {
  if (!s) return false;
  char* endp = nullptr;
  float v = strtof(s, &endp);
  if (endp == s) return false;
  out = v;
  return true;
}

static bool parseWaveType(const char* s, WaveType& out) {
  if (!s) return false;
  String t = String(s);
  t.toUpperCase();
  if (t == "SINE") { out = WT_SINE; return true; }
  if (t == "SQUARE") { out = WT_SQUARE; return true; }
  if (t == "TRI" || t == "TRIANGLE") { out = WT_TRI; return true; }
  if (t == "SAW" || t == "SAWTOOTH") { out = WT_SAW; return true; }
  if (t == "NOISE") { out = WT_NOISE; return true; }
  return false;
}

static const __FlashStringHelper* waveName(WaveType wt) {
  switch (wt) {
    case WT_SINE: return F("SINE");
    case WT_SQUARE: return F("SQUARE");
    case WT_TRI: return F("TRI");
    case WT_SAW: return F("SAW");
    case WT_NOISE: return F("NOISE");
    default: return F("?");
  }
}

// --- Extensions: index '*' and relative/scaled numbers: +x, -x, *k ---
static bool parseIndexOrStar(const char* s, bool& isAll, uint8_t& idx) {
  if (!s) return false;
  if (s[0] == '*' && s[1] == 0) { isAll = true; idx = 0; return true; }
  isAll = false;
  return parseUInt8(s, idx);
}

// If s is "+x" adds, "-x" subtracts, "*k" multiplies, numeric is absolute
static bool parseRelMulOrAbs(const char* s, float current, float& out) {
  if (!s || !*s) return false;
  if (s[0] == '+') {
    float delta;
    if (!parseFloatTok(s + 1, delta)) return false;
    out = current + delta;
    return true;
  } else if (s[0] == '-') {
    float delta;
    if (!parseFloatTok(s + 1, delta)) return false;
    out = current - delta;
    return true;
  } else if (s[0] == '*') {
    float factor;
    if (!parseFloatTok(s + 1, factor)) return false;
    out = current * factor;
    return true;
  }
  // absolute
  return parseFloatTok(s, out);
}

static void printHelp() {
  Serial.println(F("#Commands:"));
  Serial.print(F("#  N <count>                    - set number of series (1..")); Serial.print(MAX_SERIES); Serial.println(F(")"));
  Serial.println(F("#  NOISE <amp|+d|-d|*k>         - set or adjust global noise amplitude"));
  Serial.println(F("#  SET <idx|*> <type> <f|+df|-df|*k>   - set one/all; +,-,* adjust freq"));
  Serial.println(F("#  TYPE <idx|*> <type>          - set waveform type for one/all"));
  Serial.println(F("#  FREQ <idx|*> <f|+df|-df|*k>  - set/adjust frequency (Hz)"));
  Serial.println(F("#  RATE <hz>                    - output rate (lines/sec)"));
  Serial.println(F("#  DUMP                         - print current configuration"));
  Serial.println(F("#  HELP                         - show this help"));
  Serial.println(F("#Types: SINE, SQUARE, TRI, SAW, NOISE"));
}

static void dumpConfig() {
  Serial.print(F("#numSeries=")); Serial.println(numSeries);
  Serial.print(F("#noiseAmp=")); Serial.println(noiseAmp, 6);
  for (uint8_t i = 0; i < MAX_SERIES; ++i) {
    Serial.print(F("#[")); Serial.print(i); Serial.print(F("] "));
    Serial.print(waveName(series[i].type));
    Serial.print(F("  f=")); Serial.print(series[i].freq, 6);
    Serial.print(F("  amp=")); Serial.print(series[i].amp, 3);
    Serial.print(F("  phase=")); Serial.println(series[i].phase, 3);
  }
}

// Trim trailing \r and whitespace
static void rstrip(char* s) {
  int n = strlen(s);
  while (n > 0 && (s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t')) {
    s[--n] = 0;
  }
}

static void handleCommand(char* line) {
  rstrip(line);
  if (line[0] == 0) return;

  // Tokenize by whitespace
  char* saveptr = nullptr;
  char* tok = strtok_r(line, " \t", &saveptr);
  if (!tok) return;

  String cmd = String(tok);
  cmd.toUpperCase();

  if (cmd == "HELP" || cmd == "?") { printHelp(); return; }
  if (cmd == "DUMP") { dumpConfig(); return; }

  if (cmd == "N") {
    char* a = strtok_r(nullptr, " \t", &saveptr);
    uint8_t n;
    if (a && parseUInt8(a, n) && n >= 1 && n <= MAX_SERIES) {
      numSeries = n;
      Serial.print(F("#OK N=")); Serial.println(numSeries);
    } else {
      Serial.print(F("#ERR: usage N <1..")); Serial.print(MAX_SERIES); Serial.println(F(">"));
    }
    return;
  }

  if (cmd == "NOISE") {
    char* a = strtok_r(nullptr, " \t", &saveptr);
    if (!a) { Serial.println(F("#ERR: usage NOISE <amp|+d|-d|*k>")); return; }
    float outv;
    if (parseRelMulOrAbs(a, noiseAmp, outv) && outv >= 0.0f) {
      noiseAmp = outv;
      Serial.print(F("#OK NOISE=")); Serial.println(noiseAmp, 6);
    } else {
      Serial.println(F("#ERR: usage NOISE <amp|+d|-d|*k> (>=0)"));
    }
    return;
  }

  if (cmd == "SET") {
    char* a = strtok_r(nullptr, " \t", &saveptr);
    char* b = strtok_r(nullptr, " \t", &saveptr);
    char* c = strtok_r(nullptr, " \t", &saveptr);
    uint8_t idx; bool isAll; WaveType wt;
    if (a && b && c && parseIndexOrStar(a, isAll, idx) && parseWaveType(b, wt)) {
      if (isAll) {
        for (uint8_t i = 0; i < numSeries; ++i) {
          float fnew;
          if (!parseRelMulOrAbs(c, series[i].freq, fnew) || fnew < 0.0f) {
            Serial.println(F("#ERR: SET * <type> <f|+df|-df|*k> invalid value"));
            return;
          }
          series[i].type = wt;
          series[i].freq = fnew;
        }
        Serial.print(F("#OK SET[*]=")); Serial.print(waveName(wt)); Serial.println(F(", <freq updated>"));
      } else if (idx < MAX_SERIES) {
        float fnew;
        if (!parseRelMulOrAbs(c, series[idx].freq, fnew) || fnew < 0.0f) {
          Serial.println(F("#ERR: usage SET <idx|*> <type> <f|+df|-df|*k>"));
          return;
        }
        series[idx].type = wt;
        series[idx].freq = fnew;
        Serial.print(F("#OK SET[")); Serial.print(idx); Serial.print(F("]="));
        Serial.print(waveName(wt)); Serial.print(F(",")); Serial.println(series[idx].freq, 6);
      } else {
        Serial.println(F("#ERR: idx out of range"));
      }
    } else {
      Serial.println(F("#ERR: usage SET <idx|*> <type> <f|+df|-df|*k>"));
    }
    return;
  }

  if (cmd == "TYPE") {
    char* a = strtok_r(nullptr, " \t", &saveptr);
    char* b = strtok_r(nullptr, " \t", &saveptr);
    uint8_t idx; bool isAll; WaveType wt;
    if (a && b && parseIndexOrStar(a, isAll, idx) && parseWaveType(b, wt)) {
      if (isAll) {
        for (uint8_t i = 0; i < numSeries; ++i) series[i].type = wt;
        Serial.print(F("#OK TYPE[*]=")); Serial.println(waveName(wt));
      } else if (idx < MAX_SERIES) {
        series[idx].type = wt;
        Serial.print(F("#OK TYPE[")); Serial.print(idx); Serial.print(F("]="));
        Serial.println(waveName(wt));
      } else {
        Serial.println(F("#ERR: idx out of range"));
      }
    } else {
      Serial.println(F("#ERR: usage TYPE <idx|*> <type>"));
    }
    return;
  }

  if (cmd == "FREQ") {
    char* a = strtok_r(nullptr, " \t", &saveptr);
    char* b = strtok_r(nullptr, " \t", &saveptr);
    uint8_t idx; bool isAll;
    if (a && b && parseIndexOrStar(a, isAll, idx)) {
      if (isAll) {
        for (uint8_t i = 0; i < numSeries; ++i) {
          float fnew;
          if (!parseRelMulOrAbs(b, series[i].freq, fnew) || fnew < 0.0f) {
            Serial.println(F("#ERR: FREQ * <f|+df|-df|*k> invalid value"));
            return;
          }
          series[i].freq = fnew;
        }
        Serial.println(F("#OK FREQ[*]=<updated>"));
      } else if (idx < MAX_SERIES) {
        float fnew;
        if (parseRelMulOrAbs(b, series[idx].freq, fnew) && fnew >= 0.0f) {
          series[idx].freq = fnew;
          Serial.print(F("#OK FREQ[")); Serial.print(idx); Serial.print(F("]="));
          Serial.println(series[idx].freq, 6);
        } else {
          Serial.println(F("#ERR: usage FREQ <idx|*> <f|+df|-df|*k>"));
        }
      } else {
        Serial.println(F("#ERR: idx out of range"));
      }
    } else {
      Serial.println(F("#ERR: usage FREQ <idx|*> <f|+df|-df|*k>"));
    }
    return;
  }

  if (cmd == "RATE") {
    char* a = strtok_r(nullptr, " \t", &saveptr);
    float hz;
    if (a && parseFloatTok(a, hz) && hz > 0) {
      float ms = 1000.0f / hz;
      if (ms < 1.0f) ms = 1.0f;
      printIntervalMs = (uint32_t)ms;
      Serial.print(F("#OK RATE=")); Serial.print(hz, 2); Serial.print(F(" Hz ("));
      Serial.print(printIntervalMs); Serial.println(F(" ms)"));
    } else {
      Serial.println(F("#ERR: usage RATE <Hz> > 0"));
    }
    return;
  }

  Serial.println(F("#ERR: unknown command. Type HELP."));
}

// ================== Arduino ==================
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) { /* wait for USB */ }
  pinMode(LED_BUILTIN, OUTPUT);

  randomSeed((uint32_t)micros());

  for (uint8_t i = 0; i < MAX_SERIES; ++i) {
    series[i].type = WT_SINE;
    series[i].freq = float(i + 1);
    series[i].amp  = 1.0f;
    series[i].phase = 0.0f;
  }
  numSeries = 8;

  t0_us = micros();

  Serial.println(F("#SignalGen ready. Type HELP for commands."));
}

void loop() {
  // ---- Handle incoming serial lines (non-blocking) ----
  while (Serial.available()) {
    size_t n = Serial.readBytesUntil('\n', cmdBuf, sizeof(cmdBuf) - 1);
    if (n == 0) break;
    cmdBuf[n] = '\0';
    handleCommand(cmdBuf);
  }

  // ---- Generate and print one frame on schedule ----
  static uint32_t next_ms = 0;
  uint32_t now_ms = millis();
  if ((int32_t)(now_ms - next_ms) >= 0) {
    next_ms = now_ms + printIntervalMs;

    float t = (micros() - t0_us) * 1.0e-6f;

    for (uint8_t i = 0; i < numSeries; ++i) {
      float v = waveSample(series[i].type, t, series[i].freq);
      v = series[i].amp * v + noiseAmp * prand();
      if (i) Serial.print(' ');
      Serial.print(v, 6);
    }
    Serial.println();

    // blink LED to show activity
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
  }
}
