
#pragma once

#include "sqrt.hpp"
#include "tanh.hpp"

#include <clarinoid/basic/Teensy.hpp>
#include <cstdint>


static const uint32_t kNumRandom = 100000;
static const uint32_t kNumSweep = 65536;
static const bool kDoEdge = true;


/*========================= Registry: add ops by editing these arrays =========================*/
// Example: Q32 -> Q16 sqrt
static const UQFOperation kOps_UQF[] = {
    {"sqrt", 32, 16, &sqrt, kSqrtQ32Variants, (uint32_t)(sizeof(kSqrtQ32Variants) / sizeof(kSqrtQ32Variants[0]))},
    {"sqrt",
     24,
     12,
     &sqrt,
     (const VariantUQF[]){{"restoring (int)", &sqrt_q24_rest_wrap}, {"float-assisted", &sqrt_q24_float_wrap}},
     2u},

    {"tanh", 31, 31, &tanhf, kTanhQ32Variants, (uint32_t)(sizeof(kTanhQ32Variants) / sizeof(kTanhQ32Variants[0]))},
    // {
    //     "recip",
    //     32,
    //     32,
    //     [](double x)
    //     {
    //       return 1.0 / x;
    //     },
    //     kRecipQ32Variants,
    //     (uint32_t)(sizeof(kRecipQ32Variants) / sizeof(kRecipQ32Variants[0])),
    //     0xFFFFFFFFu /* full range */
    // },
};
static const uint32_t kNumOps_UQF = sizeof(kOps_UQF) / sizeof(kOps_UQF[0]);

// float op registry
static const FloatOperation kOps_Float[] = {
    {"sqrtf", 0.0f, 1.0e8f, &sqrtf, kSqrtFVariants, (uint32_t)(sizeof(kSqrtFVariants) / sizeof(kSqrtFVariants[0]))},
    {"tanh-around0",
     -0.05f,
     0.05f,
     &tanhf,
     kTanhFVariants,
     (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0]))},
    {"tanh-0.5", 0.45f, 0.55f, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0]))},
    {"tanh-before1", 0.9f, 1, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0]))},
    {"tanh-after1", 1, 1.2f, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0]))},
    {"tanh-below-n2", -16, -2, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0]))},
    {"tanh-above-p2", 2, 16, &tanhf, kTanhFVariants, (uint32_t)(sizeof(kTanhFVariants) / sizeof(kTanhFVariants[0]))},
    // {
    //     "recip",
    //     0.001f,
    //     0.999f,
    //     &recip_float_baseline,
    //     kRecipFVariants,
    //     (uint32_t)(sizeof(kRecipFVariants) / sizeof(kRecipFVariants[0])),
    // },

};
static const uint32_t kNumOps_Float = sizeof(kOps_Float) / sizeof(kOps_Float[0]);

static void print_markdown_summaries()
{
  const int GRAPH_W = 20;         // characters inside the backticks
  const int MAX_ROWS_GROUP = 16;  // max variants per op group (adjust if needed)

  // ---------- UQF groups (fixed-point) ----------
  for (uint32_t i = 0; i < g_rows_uqf_count; ++i)
  {
    const char* name_i = g_rows_uqf[i].op_name;
    uint32_t Fi = g_rows_uqf[i].F, Gi = g_rows_uqf[i].G;

    bool printed = false;
    for (uint32_t k = 0; k < i; ++k)
      if (g_rows_uqf[k].op_name == name_i && g_rows_uqf[k].F == Fi && g_rows_uqf[k].G == Gi)
      {
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

    for (uint32_t j = 0; j < g_rows_uqf_count; ++j)
    {
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
        {"Variant", LEFT, 0},
        {use_cycles ? "Speed (cycles)" : "Speed (us)", RIGHT, 0},
        {"Speed (graph)", LEFT, 0},
        {"ULP max", RIGHT, 0},
        {"ULP max (graph)", LEFT, 0},
        {"ULP mean", RIGHT, 0},
        {"ULP mean (graph)", LEFT, 0},
        {"ULP rms", RIGHT, 0},
        {"ULP rms (graph)", LEFT, 0},
        {"Rel max (ppm)", RIGHT, 0},
        {"Rel max (graph)", LEFT, 0},
        {"Rel mean (ppm)", RIGHT, 0},
        {"Rel mean (graph)", LEFT, 0},
        {"Rel rms (ppm)", RIGHT, 0},
        {"Rel rms (graph)", LEFT, 0},
    };
    const int NC = sizeof(cols) / sizeof(cols[0]);

    // Prepare cell strings
    char cells[MAX_ROWS_GROUP][16][64];  // [row][col][buf]
    for (int r = 0; r < n; ++r)
    {
      const auto& row = *grp[r];
      // 0 Variant
      snprintf(cells[r][0], sizeof(cells[r][0]), "%s", row.variant);
      // 1 Speed value
      fmt_float(cells[r][1],
                sizeof(cells[r][1]),
                use_cycles ? row.speed_cycles : row.speed_us,
                use_cycles ? "%0.2f" : "%0.3f");
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
  for (uint32_t i = 0; i < g_rows_float_count; ++i)
  {
    const char* name_i = g_rows_float[i].op_name;

    bool printed = false;
    for (uint32_t k = 0; k < i; ++k)
      if (g_rows_float[k].op_name == name_i)
      {
        printed = true;
        break;
      }
    if (printed)
      continue;

    const SummaryFloatRow* grp[MAX_ROWS_GROUP];
    int n = 0;
    bool use_cycles = g_rows_float[i].use_cycles;
    double vmax_speed = 0, vmax_relmax = 0, vmax_relmean = 0, vmax_relrms = 0;

    for (uint32_t j = 0; j < g_rows_float_count; ++j)
    {
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
        {"Variant", LEFT, 0},
        {use_cycles ? "Speed (cycles)" : "Speed (us)", RIGHT, 0},
        {"Speed (graph)", LEFT, 0},
        {"Rel max (ppm)", RIGHT, 0},
        {"Rel max (graph)", LEFT, 0},
        {"Rel mean (ppm)", RIGHT, 0},
        {"Rel mean (graph)", LEFT, 0},
        {"Rel rms (ppm)", RIGHT, 0},
        {"Rel rms (graph)", LEFT, 0},
    };
    const int NC = sizeof(cols) / sizeof(cols[0]);

    char cells[MAX_ROWS_GROUP][16][64];
    for (int r = 0; r < n; ++r)
    {
      const auto& row = *grp[r];
      snprintf(cells[r][0], sizeof(cells[r][0]), "%s", row.variant);
      fmt_float(cells[r][1],
                sizeof(cells[r][1]),
                use_cycles ? row.speed_cycles : row.speed_us,
                use_cycles ? "%0.2f" : "%0.3f");
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
