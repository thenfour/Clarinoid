
#pragma once

#include <cstdint>

static const uint32_t kNumRandom = 100000;
static const uint32_t kNumSweep = 65536;
static const bool kDoEdge = true;

/*========================= Utilities =========================*/
static inline uint32_t
xorshift32(uint32_t& s)
{
  uint32_t x = s;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  s = x;
  return x;
}

/*========================= Accuracy metrics =========================*/
struct ErrorStats
{
  uint64_t count = 0;
  uint64_t abs_ulp_sum = 0;
  long double abs_ulp_sq_sum = 0;
  uint32_t abs_ulp_max = 0;

  uint64_t rel_count = 0;
  long double rel_ppm_sum = 0;
  long double rel_ppm_sq_sum = 0;
  uint32_t rel_ppm_max = 0;

  void add_ulps(uint32_t y, uint32_t yref)
  {
    uint32_t ulp = (y > yref) ? (y - yref) : (yref - y);
    abs_ulp_sum += ulp;
    abs_ulp_sq_sum += (long double)ulp * (long double)ulp;
    if (ulp > abs_ulp_max)
      abs_ulp_max = ulp;
    ++count;
  }
  void add_rel(float y, float yref)
  {
    if (yref != 0.0f) {
      float num = fabsf(y - yref);
      float ppm = (num / fabsf(yref)) * 1.0e6f;
      rel_ppm_sum += ppm;
      rel_ppm_sq_sum += (long double)ppm * (long double)ppm;
      if ((uint32_t)ppm > rel_ppm_max)
        rel_ppm_max = (uint32_t)ppm;
      ++rel_count;
    }
  }
  void print(const char* label) const
  {
    long double mean_ulp = count ? (long double)abs_ulp_sum / (long double)count : 0.0L;
    long double rms_ulp = count ? sqrt((long double)abs_ulp_sq_sum / (long double)count) : 0.0L;
    long double mean_rel_ppm = rel_count ? (long double)rel_ppm_sum / (long double)rel_count : 0.0L;
    long double rms_rel_ppm = rel_count ? sqrt((long double)rel_ppm_sq_sum / (long double)rel_count) : 0.0L;

    Serial.printf("  [%s] Accuracy:\n", label);
    Serial.printf("    Abs error (ULP): max=%u  mean=%.3Lf  rms=%.3Lf\n", abs_ulp_max, mean_ulp, rms_ulp);
    Serial.printf("    Rel error (ppm): max=%u  mean=%.3Lf  rms=%.3Lf\n", rel_ppm_max, mean_rel_ppm, rms_rel_ppm);
  }
};

/*========================= Domain 1: Unsigned QF -> QG =========================*/
// Candidate signature for UQF ops (input raw QF -> output raw QG)
typedef uint32_t (*UQFUnary)(uint32_t X);

// A candidate variant of an operation
struct VariantUQF
{
  const char* name;
  UQFUnary fn;
};

// A registered fixed-point op
struct UQFOperation
{
  const char* op_name;
  uint32_t F;                 // input fractional bits
  uint32_t G;                 // output fractional bits
  double (*ref_real)(double); // reference real function, e.g., ::sqrt
  const VariantUQF* variants; // array of variants to run
  uint32_t num_variants;
  // Optional input clamp (raw) for sweep/random (0..max_raw). Default: full range.
  uint32_t max_raw = 0xFFFFFFFFu;
};

/*========================= Domain 2: float -> float =========================*/
// Candidate signature for float unary ops
typedef float (*FloatUnary)(float);

// Candidate
struct VariantFloat
{
  const char* name;
  FloatUnary fn;
};

// Registered float op
struct FloatOperation
{
  const char* op_name;
  float x_min, x_max;           // input range for sweep/random
  float (*ref)(float);          // reference function (e.g., tanhf, sinf)
  const VariantFloat* variants; // array of variants
  uint32_t num_variants;
};

// Reference quantization: real -> QG raw
static inline uint32_t
ref_quantize_to_QG(double y, uint32_t G)
{
  long double scaled = (long double)y * (long double)(uint64_t(1) << G);
  long long r = llroundl(scaled);
  if (r < 0)
    r = 0;
  if (r > 0xFFFFFFFFll)
    r = 0xFFFFFFFFll;
  return (uint32_t)r;
}

// ---------- Results & summary helpers (replace the previous versions) ----------

struct SummaryUQFRow
{
  const char* op_name;
  const char* variant;
  uint32_t F, G;
  bool use_cycles;
  double speed_cycles; // valid if use_cycles
  double speed_us;     // else
  // accuracy
  uint32_t abs_ulp_max;
  double abs_ulp_mean;
  double abs_ulp_rms;
  uint32_t rel_ppm_max;
  double rel_ppm_mean;
  double rel_ppm_rms;
};

struct SummaryFloatRow
{
  const char* op_name;
  const char* variant;
  bool use_cycles;
  double speed_cycles;
  double speed_us;
  // accuracy (relative ppm)
  uint32_t rel_ppm_max;
  double rel_ppm_mean;
  double rel_ppm_rms;
};

static const uint32_t MAX_ROWS = 256;
static SummaryUQFRow g_rows_uqf[MAX_ROWS];
static SummaryFloatRow g_rows_float[MAX_ROWS];
static uint32_t g_rows_uqf_count = 0;
static uint32_t g_rows_float_count = 0;

static inline void
summarize_ulps(const ErrorStats& E, double& mean_ulp, double& rms_ulp)
{
  mean_ulp = (E.count ? double(E.abs_ulp_sum) / double(E.count) : 0.0);
  rms_ulp = (E.count ? sqrt((double)E.abs_ulp_sq_sum / double(E.count)) : 0.0);
}
static inline void
summarize_rel(const ErrorStats& E, double& mean_ppm, double& rms_ppm)
{
  mean_ppm = (E.rel_count ? (double)E.rel_ppm_sum / (double)E.rel_count : 0.0);
  rms_ppm = (E.rel_count ? sqrt((double)E.rel_ppm_sq_sum / (double)E.rel_count) : 0.0);
}

// Build a bar of 'width' (<= outsz-1). Longer bar = larger value.
static void
build_bar(char* out, size_t outsz, double v, double vmax, int width = 18)
{
  if (!outsz)
    return;
  if (vmax <= 0.0) {
    out[0] = '\0';
    return;
  }
  if (width < 1)
    width = 1;
  int n = (int)lround((v / vmax) * width);
  if (n > width)
    n = width;
  size_t len = (size_t)width;
  if (len >= outsz)
    len = outsz - 1;
  for (size_t i = 0; i < len; i++)
    out[i] = (i < (size_t)n) ? '=' : ' ';
  out[len] = '\0';
}

// Fixed-width formatter: number -> backticked string for aligned Markdown cells
static void
fmt_num(char* out, size_t outsz, const char* fmt, double v, int width = 8)
{
  char buf[32];
  snprintf(buf, sizeof(buf), fmt, v);
  // Left-pad to width
  // char pad[32];
  int len = (int)strlen(buf);
  int padn = (width > len) ? (width - len) : 0;
  int n = 0;
  out[n++] = '`';
  for (int i = 0; i < padn && n < (int)outsz - 2; ++i)
    out[n++] = ' ';
  for (int i = 0; buf[i] && n < (int)outsz - 2; ++i)
    out[n++] = buf[i];
  out[n++] = '`';
  out[n] = '\0';
}
static void
fmt_int(char* out, size_t outsz, uint32_t v, int width = 6)
{
  fmt_num(out, outsz, "%.0f", (double)v, width);
}

static void
record_uqf_row(const char* op_name,
               const char* variant,
               uint32_t F,
               uint32_t G,
               bool use_cycles,
               double speed_val,
               const ErrorStats& E)
{
  if (g_rows_uqf_count >= MAX_ROWS)
    return;
  SummaryUQFRow& r = g_rows_uqf[g_rows_uqf_count++];
  r.op_name = op_name;
  r.variant = variant;
  r.F = F;
  r.G = G;
  r.use_cycles = use_cycles;
  r.speed_cycles = use_cycles ? speed_val : 0.0;
  r.speed_us = use_cycles ? 0.0 : speed_val;
  r.abs_ulp_max = E.abs_ulp_max;
  summarize_ulps(E, r.abs_ulp_mean, r.abs_ulp_rms);
  r.rel_ppm_max = E.rel_ppm_max;
  summarize_rel(E, r.rel_ppm_mean, r.rel_ppm_rms);
}
static void
record_float_row(const char* op_name, const char* variant, bool use_cycles, double speed_val, const ErrorStats& E)
{
  if (g_rows_float_count >= MAX_ROWS)
    return;
  SummaryFloatRow& r = g_rows_float[g_rows_float_count++];
  r.op_name = op_name;
  r.variant = variant;
  r.use_cycles = use_cycles;
  r.speed_cycles = use_cycles ? speed_val : 0.0;
  r.speed_us = use_cycles ? 0.0 : speed_val;
  r.rel_ppm_max = E.rel_ppm_max;
  summarize_rel(E, r.rel_ppm_mean, r.rel_ppm_rms);
}

// ---------- Markdown table rendering helpers ----------

enum Align
{
  LEFT,
  RIGHT
};

struct ColSpec
{
  const char* header;
  Align align;
  int width; // computed at runtime
};

static void
pad_left(char* dst, size_t cap, const char* s, int width)
{
  int len = (int)strlen(s);
  int pad = (width > len) ? (width - len) : 0;
  int n = 0;
  for (int i = 0; i < pad && n < (int)cap - 1; ++i)
    dst[n++] = ' ';
  for (int i = 0; s[i] && n < (int)cap - 1; ++i)
    dst[n++] = s[i];
  dst[n] = '\0';
}
static void
pad_right(char* dst, size_t cap, const char* s, int width)
{
  int len = (int)strlen(s);
  int pad = (width > len) ? (width - len) : 0;
  int n = 0;
  for (int i = 0; s[i] && n < (int)cap - 1; ++i)
    dst[n++] = s[i];
  for (int i = 0; i < pad && n < (int)cap - 1; ++i)
    dst[n++] = ' ';
  dst[n] = '\0';
}

// Build a backticked bar of exactly graphW characters ('=' then spaces)
static void
make_bar(char* out, size_t outsz, double v, double vmax, int graphW)
{
  if (outsz == 0)
    return;
  int nfill = 0;
  if (vmax > 0.0 && graphW > 0) {
    nfill = (int)lround((v / vmax) * graphW);
    if (nfill < 0)
      nfill = 0;
    if (nfill > graphW)
      nfill = graphW;
  }
  // backticks + content
  int need = 2 + graphW; // ` + content + `
  if (need >= (int)outsz)
    graphW = (int)outsz - 3;
  int pos = 0;
  out[pos++] = '`';
  for (int i = 0; i < graphW; ++i)
    out[pos++] = (i < nfill) ? '=' : ' ';
  out[pos++] = '`';
  out[pos] = '\0';
}

// Format numbers as strings (no backticks)
static void
fmt_float(char* out, size_t outsz, double v, const char* fmt)
{
  snprintf(out, outsz, fmt, v);
}
static void
fmt_uint(char* out, size_t outsz, uint32_t v)
{
  snprintf(out, outsz, "%u", (unsigned int)v);
}

// Compute widths for a group: headers and each row cell string
// cells[r][c] must be prepared before calling.
static void
compute_widths(ColSpec* cols, int ncols, const char cells[][16][64], int nrows)
{
  for (int c = 0; c < ncols; ++c) {
    int w = (int)strlen(cols[c].header);
    for (int r = 0; r < nrows; ++r) {
      int len = (int)strlen(cells[r][c]);
      if (len > w)
        w = len;
    }
    cols[c].width = w;
  }
}

// Print header + alignment row with exact widths
static void
print_header(const ColSpec* cols, int ncols)
{
  // header
  Serial.print("|");
  for (int c = 0; c < ncols; ++c) {
    char cell[96];
    pad_right(cell, sizeof(cell), cols[c].header, cols[c].width);
    Serial.print(" ");
    Serial.print(cell);
    Serial.print(" |");
  }
  Serial.println();
  // alignment
  Serial.print("|");
  for (int c = 0; c < ncols; ++c) {
    // Markdown alignment syntax: :---, ---:, :---:
    int w = cols[c].width;
    if (w < 3)
      w = 3;
    Serial.print(cols[c].align == RIGHT ? " " : ":");
    for (int i = 0; i < w; ++i)
      Serial.print("-");
    Serial.print(cols[c].align == RIGHT ? ":|" : "-|");
  }
  Serial.println();
}

// Print one row (cells must be pre-padded)
static void
print_row(const ColSpec* cols, int ncols, const char cells[][64])
{
  Serial.print("|");
  for (int c = 0; c < ncols; ++c) {
    char cell[96];
    if (cols[c].align == RIGHT)
      pad_left(cell, sizeof(cell), cells[c], cols[c].width);
    else
      pad_right(cell, sizeof(cell), cells[c], cols[c].width);
    Serial.print(" ");
    Serial.print(cell);
    Serial.print(" |");
  }
  Serial.println();
}

// Run one UQF op across datasets
void
run_bench(const UQFOperation& op, uint32_t numRandom, uint32_t numSweep, bool doEdge)
{
  const uint32_t F = op.F, G = op.G;
  const uint32_t maxX = op.max_raw;
  Serial.printf("\n=== UQF Op: %s  |  Q%u -> Q%u ===\n", op.op_name, F, G);

  // Precompute a reference helper for speed in loops
  auto refQG = [&](uint32_t X) -> uint32_t {
    double x = (double)X / (double)(uint64_t(1) << F);
    double y = op.ref_real ? op.ref_real(x) : x; // identity if no ref provided
    return ref_quantize_to_QG(y, G);
  };

  for (uint32_t vi = 0; vi < op.num_variants; ++vi) {
    const VariantUQF& v = op.variants[vi];
    ErrorStats E{};
    volatile uint32_t sink = 0;

    // Edge cases
    if (doEdge) {
      const uint32_t edges[] = { 0u,          1u,  2u, 3u, 7u, 8u, 15u, 16u, 31u, 32u, (F < 32 ? (1u << F) : 0u),
                                 0xFFFFFFFFu, maxX };
      for (uint32_t x : edges) {
        uint32_t X = (x <= maxX) ? x : maxX;
        uint32_t y = v.fn(X);
        uint32_t yr = refQG(X);
        E.add_ulps(y, yr);
        E.add_rel((float)y, (float)yr); // rel on integer grid; optional for UQF
      }
    }

    // Sweep (map 16-bit ramp to 32-bit space)
    for (uint32_t i = 0; i < numSweep; ++i) {
      uint32_t X = ((i & 0xFFFFu) << 16) | ((i ^ 0xA5A5u) & 0xFFFFu);
      if (X > maxX)
        X = maxX;
      uint32_t y = v.fn(X);
      uint32_t yr = refQG(X);
      E.add_ulps(y, yr);
      E.add_rel((float)y, (float)yr);
    }

    // Random timing loop
    uint32_t seed = 0x12345678u;
    cycles_enable();
    uint32_t t0 = cycles_now();

    // Unrolled by 8 to amortize loop overhead
    const uint32_t iters = numRandom / 8;
    for (uint32_t i = 0; i < iters; ++i) {
      uint32_t X0 = xorshift32(seed);
      if (X0 > maxX)
        X0 = maxX;
      uint32_t X1 = xorshift32(seed);
      if (X1 > maxX)
        X1 = maxX;
      uint32_t X2 = xorshift32(seed);
      if (X2 > maxX)
        X2 = maxX;
      uint32_t X3 = xorshift32(seed);
      if (X3 > maxX)
        X3 = maxX;
      uint32_t X4 = xorshift32(seed);
      if (X4 > maxX)
        X4 = maxX;
      uint32_t X5 = xorshift32(seed);
      if (X5 > maxX)
        X5 = maxX;
      uint32_t X6 = xorshift32(seed);
      if (X6 > maxX)
        X6 = maxX;
      uint32_t X7 = xorshift32(seed);
      if (X7 > maxX)
        X7 = maxX;

      sink ^= v.fn(X0);
      sink ^= v.fn(X1);
      sink ^= v.fn(X2);
      sink ^= v.fn(X3);
      sink ^= v.fn(X4);
      sink ^= v.fn(X5);
      sink ^= v.fn(X6);
      sink ^= v.fn(X7);
    }

    uint32_t t1 = cycles_now();
#if HAVE_DWT
    double cycles_per_call = (double)(uint32_t)(t1 - t0) / (double)(iters * 8u);
    Serial.printf("\nVariant: %s\n  Speed: %.2f cycles/call\n", v.name, cycles_per_call);
#else
    double us_per_call = (double)(uint32_t)(t1 - t0) / (double)(iters * 8u);
    Serial.printf("\nVariant: %s\n  Speed: %.3f us/call (micros)\n", v.name, us_per_call);
#endif

#if HAVE_DWT
    record_uqf_row(op.op_name, v.name, F, G, /*use_cycles=*/true, cycles_per_call, E);
#else
    record_uqf_row(op.op_name, v.name, F, G, /*use_cycles=*/false, us_per_call, E);
#endif

    // Random accuracy pass separate from timing
    seed = 0x89ABCDEFu;
    for (uint32_t i = 0; i < numRandom; ++i) {
      uint32_t X = xorshift32(seed);
      if (X > maxX)
        X = maxX;
      uint32_t y = v.fn(X);
      uint32_t yr = refQG(X);
      E.add_ulps(y, yr);
      E.add_rel((float)y, (float)yr);
    }

    (void)sink; // keep from optimizing away
    E.print("overall");
  }
}

// Run one float op
void
run_bench(const FloatOperation& op, uint32_t numRandom, uint32_t numSweep)
{
  Serial.printf("\n=== Float Op: %s  |  range=[%.3f, %.3f] ===\n", op.op_name, op.x_min, op.x_max);
  for (uint32_t vi = 0; vi < op.num_variants; ++vi) {
    const VariantFloat& v = op.variants[vi];
    ErrorStats E{};
    volatile float sinkf = 0.f;

    // Sweep
    for (uint32_t i = 0; i < numSweep; ++i) {
      float t = (float)i / (float)(numSweep - 1);
      float x = op.x_min + (op.x_max - op.x_min) * t;
      float y = v.fn(x);
      float yr = op.ref ? op.ref(x) : x;
      // Use relative ppm; ULP distance for float could be added if desired
      E.add_rel(y, yr);
    }

    // Random timing
    uint32_t seed = 0x13572468u;
    cycles_enable();
    uint32_t t0 = cycles_now();

    const uint32_t iters = numRandom / 8;
    for (uint32_t i = 0; i < iters; ++i) {
      float r0 = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);
      float r1 = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);
      float r2 = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);
      float r3 = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);
      float r4 = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);
      float r5 = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);
      float r6 = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);
      float r7 = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);

      float x0 = op.x_min + (op.x_max - op.x_min) * r0;
      float x1 = op.x_min + (op.x_max - op.x_min) * r1;
      float x2 = op.x_min + (op.x_max - op.x_min) * r2;
      float x3 = op.x_min + (op.x_max - op.x_min) * r3;
      float x4 = op.x_min + (op.x_max - op.x_min) * r4;
      float x5 = op.x_min + (op.x_max - op.x_min) * r5;
      float x6 = op.x_min + (op.x_max - op.x_min) * r6;
      float x7 = op.x_min + (op.x_max - op.x_min) * r7;

      sinkf += v.fn(x0) + v.fn(x1) + v.fn(x2) + v.fn(x3) + v.fn(x4) + v.fn(x5) + v.fn(x6) + v.fn(x7);
    }

    uint32_t t1 = cycles_now();
#if HAVE_DWT
    double cycles_per_call = (double)(uint32_t)(t1 - t0) / (double)(iters * 8u);
    Serial.printf("\nVariant: %s\n  Speed: %.2f cycles/call\n", v.name, cycles_per_call);
#else
    double us_per_call = (double)(uint32_t)(t1 - t0) / (double)(iters * 8u);
    Serial.printf("\nVariant: %s\n  Speed: %.3f us/call (micros)\n", v.name, us_per_call);
#endif

#if HAVE_DWT
    record_float_row(op.op_name, v.name, /*use_cycles=*/true, cycles_per_call, E);
#else
    record_float_row(op.op_name, v.name, /*use_cycles=*/false, us_per_call, E);
#endif
    // Random accuracy
    seed = 0xCAFEBABEu;
    for (uint32_t i = 0; i < numRandom; ++i) {
      float r = (xorshift32(seed) & 0xFFFFFF) / float(0xFFFFFF);
      float x = op.x_min + (op.x_max - op.x_min) * r;
      float y = v.fn(x);
      float yr = op.ref ? op.ref(x) : x;
      E.add_rel(y, yr);
    }
    (void)sinkf;
    E.print("overall");
  }
}

/*========================= Registry: add ops by editing these arrays =========================*/
// Example: Q32 -> Q16 sqrt
static const UQFOperation kOps_UQF[] = {
  { "sqrt", 32, 16, &sqrt, kSqrtQ32Variants, (uint32_t)(sizeof(kSqrtQ32Variants) / sizeof(kSqrtQ32Variants[0])) },
  { "sqrt",
    24,
    12,
    &sqrt,
    (const VariantUQF[]){ { "restoring (int)", &sqrt_q24_rest_wrap }, { "float-assisted", &sqrt_q24_float_wrap } },
    2u },

  { "tanh", 31, 31, &tanhf, kTanhQ32Variants, (uint32_t)(sizeof(kTanhQ32Variants) / sizeof(kTanhQ32Variants[0])) },
  {
    "recip",
    32,
    32,
    [](double x) { return 1.0 / x; },
    kRecipQ32Variants,
    (uint32_t)(sizeof(kRecipQ32Variants) / sizeof(kRecipQ32Variants[0])),
    0xFFFFFFFFu /* full range */
  },
};
static const uint32_t kNumOps_UQF = sizeof(kOps_UQF) / sizeof(kOps_UQF[0]);

// float op registry
static const FloatOperation kOps_Float[] = {
  { "sqrtf", 0.0f, 1.0e8f, &sqrtf, kSqrtFVariants, (uint32_t)(sizeof(kSqrtFVariants) / sizeof(kSqrtFVariants[0])) },
  { "tanh-around0",
    -0.05f,
    0.05f,
    &tanhf,
    kTanhFVariants,
    (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0])) },
  { "tanh-0.5", 0.45f, 0.55f, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0])) },
  { "tanh-before1", 0.9f, 1, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0])) },
  { "tanh-after1", 1, 1.2f, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0])) },
  { "tanh-below-n2", -16, -2, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0])) },
  { "tanh-above-p2", 2, 16, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0])) },
  {
    "recip",
    0.001f,
    0.999f,
    &recip_float_baseline,
    kRecipFVariants,
    (uint32_t)(sizeof(kRecipFVariants) / sizeof(kRecipFVariants[0])),
  },

};
static const uint32_t kNumOps_Float = sizeof(kOps_Float) / sizeof(kOps_Float[0]);

static void
print_markdown_summaries()
{
  const int GRAPH_W = 20;        // characters inside the backticks
  const int MAX_ROWS_GROUP = 16; // max variants per op group (adjust if needed)

  // ---------- UQF groups (fixed-point) ----------
  for (uint32_t i = 0; i < g_rows_uqf_count; ++i) {
    const char* name_i = g_rows_uqf[i].op_name;
    uint32_t Fi = g_rows_uqf[i].F, Gi = g_rows_uqf[i].G;

    bool printed = false;
    for (uint32_t k = 0; k < i; ++k)
      if (g_rows_uqf[k].op_name == name_i && g_rows_uqf[k].F == Fi && g_rows_uqf[k].G == Gi) {
        printed = true;
        break;
      }
    if (printed)
      continue;

    // Collect group rows, find maxima for bar scaling
    const SummaryUQFRow* grp[MAX_ROWS_GROUP];
    int n = 0;
    bool use_cycles = g_rows_uqf[i].use_cycles;
    double vmax_speed = 0, vmax_ulpm = 0, vmax_ulpmean = 0, vmax_ulprms = 0, vmax_relmax = 0, vmax_relmean = 0,
           vmax_relrms = 0;

    for (uint32_t j = 0; j < g_rows_uqf_count; ++j) {
      const auto& r = g_rows_uqf[j];
      if (r.op_name != name_i || r.F != Fi || r.G != Gi || r.use_cycles != use_cycles)
        continue;
      if (n < MAX_ROWS_GROUP)
        grp[n++] = &r;
      double sp = use_cycles ? r.speed_cycles : r.speed_us;
      if (sp > vmax_speed)
        vmax_speed = sp;
      if (r.abs_ulp_max > vmax_ulpm)
        vmax_ulpm = r.abs_ulp_max;
      if (r.abs_ulp_mean > vmax_ulpmean)
        vmax_ulpmean = r.abs_ulp_mean;
      if (r.abs_ulp_rms > vmax_ulprms)
        vmax_ulprms = r.abs_ulp_rms;
      if (r.rel_ppm_max > vmax_relmax)
        vmax_relmax = r.rel_ppm_max;
      if (r.rel_ppm_mean > vmax_relmean)
        vmax_relmean = r.rel_ppm_mean;
      if (r.rel_ppm_rms > vmax_relrms)
        vmax_relrms = r.rel_ppm_rms;
    }

    // Prepare columns
    ColSpec cols[] = {
      { "Variant", LEFT, 0 },          { use_cycles ? "Speed (cycles)" : "Speed (us)", RIGHT, 0 },
      { "Speed (graph)", LEFT, 0 },    { "ULP max", RIGHT, 0 },
      { "ULP max (graph)", LEFT, 0 },  { "ULP mean", RIGHT, 0 },
      { "ULP mean (graph)", LEFT, 0 }, { "ULP rms", RIGHT, 0 },
      { "ULP rms (graph)", LEFT, 0 },  { "Rel max (ppm)", RIGHT, 0 },
      { "Rel max (graph)", LEFT, 0 },  { "Rel mean (ppm)", RIGHT, 0 },
      { "Rel mean (graph)", LEFT, 0 }, { "Rel rms (ppm)", RIGHT, 0 },
      { "Rel rms (graph)", LEFT, 0 },
    };
    const int NC = sizeof(cols) / sizeof(cols[0]);

    // Prepare cell strings
    char cells[MAX_ROWS_GROUP][16][64]; // [row][col][buf]
    for (int r = 0; r < n; ++r) {
      const auto& row = *grp[r];
      // 0 Variant
      snprintf(cells[r][0], sizeof(cells[r][0]), "%s", row.variant);
      // 1 Speed value
      fmt_float(
        cells[r][1], sizeof(cells[r][1]), use_cycles ? row.speed_cycles : row.speed_us, use_cycles ? "%0.2f" : "%0.3f");
      // 2 Speed graph
      make_bar(cells[r][2], sizeof(cells[r][2]), use_cycles ? row.speed_cycles : row.speed_us, vmax_speed, GRAPH_W);

      // ULP max (3,4)
      fmt_uint(cells[r][3], sizeof(cells[r][3]), row.abs_ulp_max);
      make_bar(cells[r][4], sizeof(cells[r][4]), (double)row.abs_ulp_max, vmax_ulpm, GRAPH_W);
      // ULP mean (5,6)
      fmt_float(cells[r][5], sizeof(cells[r][5]), row.abs_ulp_mean, "%0.3f");
      make_bar(cells[r][6], sizeof(cells[r][6]), row.abs_ulp_mean, vmax_ulpmean, GRAPH_W);
      // ULP rms   (7,8)
      fmt_float(cells[r][7], sizeof(cells[r][7]), row.abs_ulp_rms, "%0.3f");
      make_bar(cells[r][8], sizeof(cells[r][8]), row.abs_ulp_rms, vmax_ulprms, GRAPH_W);
      // Rel max   (9,10)
      fmt_uint(cells[r][9], sizeof(cells[r][9]), row.rel_ppm_max);
      make_bar(cells[r][10], sizeof(cells[r][10]), (double)row.rel_ppm_max, vmax_relmax, GRAPH_W);
      // Rel mean (11,12)
      fmt_float(cells[r][11], sizeof(cells[r][11]), row.rel_ppm_mean, "%0.3f");
      make_bar(cells[r][12], sizeof(cells[r][12]), row.rel_ppm_mean, vmax_relmean, GRAPH_W);
      // Rel rms  (13,14)
      fmt_float(cells[r][13], sizeof(cells[r][13]), row.rel_ppm_rms, "%0.3f");
      make_bar(cells[r][14], sizeof(cells[r][14]), row.rel_ppm_rms, vmax_relrms, GRAPH_W);
    }

    // Compute widths from headers + cells (ensures aligned Markdown source)
    compute_widths(cols, NC, cells, n);

    // Title
    Serial.printf("\n**%s (Q%u → Q%u)**\n\n", name_i, Fi, Gi);
    // Render
    print_header(cols, NC);
    for (int r = 0; r < n; ++r)
      print_row(cols, NC, cells[r]);
  }

  // ---------- Float groups ----------
  for (uint32_t i = 0; i < g_rows_float_count; ++i) {
    const char* name_i = g_rows_float[i].op_name;

    bool printed = false;
    for (uint32_t k = 0; k < i; ++k)
      if (g_rows_float[k].op_name == name_i) {
        printed = true;
        break;
      }
    if (printed)
      continue;

    const SummaryFloatRow* grp[MAX_ROWS_GROUP];
    int n = 0;
    bool use_cycles = g_rows_float[i].use_cycles;
    double vmax_speed = 0, vmax_relmax = 0, vmax_relmean = 0, vmax_relrms = 0;

    for (uint32_t j = 0; j < g_rows_float_count; ++j) {
      const auto& r = g_rows_float[j];
      if (r.op_name != name_i || r.use_cycles != use_cycles)
        continue;
      if (n < MAX_ROWS_GROUP)
        grp[n++] = &r;
      double sp = use_cycles ? r.speed_cycles : r.speed_us;
      if (sp > vmax_speed)
        vmax_speed = sp;
      if (r.rel_ppm_max > vmax_relmax)
        vmax_relmax = r.rel_ppm_max;
      if (r.rel_ppm_mean > vmax_relmean)
        vmax_relmean = r.rel_ppm_mean;
      if (r.rel_ppm_rms > vmax_relrms)
        vmax_relrms = r.rel_ppm_rms;
    }

    ColSpec cols[] = {
      { "Variant", LEFT, 0 },          { use_cycles ? "Speed (cycles)" : "Speed (us)", RIGHT, 0 },
      { "Speed (graph)", LEFT, 0 },    { "Rel max (ppm)", RIGHT, 0 },
      { "Rel max (graph)", LEFT, 0 },  { "Rel mean (ppm)", RIGHT, 0 },
      { "Rel mean (graph)", LEFT, 0 }, { "Rel rms (ppm)", RIGHT, 0 },
      { "Rel rms (graph)", LEFT, 0 },
    };
    const int NC = sizeof(cols) / sizeof(cols[0]);

    char cells[MAX_ROWS_GROUP][16][64];
    for (int r = 0; r < n; ++r) {
      const auto& row = *grp[r];
      snprintf(cells[r][0], sizeof(cells[r][0]), "%s", row.variant);
      fmt_float(
        cells[r][1], sizeof(cells[r][1]), use_cycles ? row.speed_cycles : row.speed_us, use_cycles ? "%0.2f" : "%0.3f");
      make_bar(cells[r][2], sizeof(cells[r][2]), use_cycles ? row.speed_cycles : row.speed_us, vmax_speed, GRAPH_W);

      fmt_uint(cells[r][3], sizeof(cells[r][3]), row.rel_ppm_max);
      make_bar(cells[r][4], sizeof(cells[r][4]), (double)row.rel_ppm_max, vmax_relmax, GRAPH_W);

      fmt_float(cells[r][5], sizeof(cells[r][5]), row.rel_ppm_mean, "%0.3f");
      make_bar(cells[r][6], sizeof(cells[r][6]), row.rel_ppm_mean, vmax_relmean, GRAPH_W);

      fmt_float(cells[r][7], sizeof(cells[r][7]), row.rel_ppm_rms, "%0.3f");
      make_bar(cells[r][8], sizeof(cells[r][8]), row.rel_ppm_rms, vmax_relrms, GRAPH_W);
    }

    compute_widths(cols, NC, cells, n);

    Serial.printf("\n**%s**\n\n", name_i);
    print_header(cols, NC);
    for (int r = 0; r < n; ++r)
      print_row(cols, NC, cells[r]);
  }
}

/*========================= Run plan =========================*/
void
setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
  }
  Serial.println("\n--- Modular benchmark harness (Teensy 4.x) ---");
#if HAVE_DWT
  Serial.println("Timing: DWT cycle counter");
#else
  Serial.println("Timing: micros() fallback (coarser)");
#endif
  Serial.printf("Random=%u  Sweep=%u  Edge=%s\n", (unsigned)kNumRandom, (unsigned)kNumSweep, kDoEdge ? "yes" : "no");

  // Run all registered UQF ops
  for (uint32_t i = 0; i < kNumOps_UQF; ++i) {
    run_bench(kOps_UQF[i], kNumRandom, kNumSweep, kDoEdge);
  }

  // Run all registered float ops (uncomment when you add entries)
  for (uint32_t i = 0; i < kNumOps_Float; ++i) {
    run_bench(kOps_Float[i], kNumRandom, kNumSweep);
  }

  Serial.println("\n### Summary (Markdown-ready)\n");
  print_markdown_summaries();

  Serial.println("\nDone.");
}

void
loop()
{
}
