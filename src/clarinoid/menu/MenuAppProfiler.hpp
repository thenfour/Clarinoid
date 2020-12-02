#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"

namespace clarinoid
{


// enum class ProfileMenuMode
// {
//   Loop,
//   Render,
//   Update,
//   Total
// };
// struct ProfileTimingsMenuList : IList
// {
//   ProfileMenuMode mMode = ProfileMenuMode::Total;
  
//   virtual int List_GetItemCount() const {
//     return gProfileObjectTypeCount;
//   }
//   virtual String List_GetItemCaption(int i) const {
//     CCASSERT(i >= 0 && i < (int)gProfileObjectTypeCount);
//     String ret = gProfileObjectTypeItems[i].mName;
//     ret += " ";

//     auto& total = gProfiler.mTimings[(size_t)(ProfileObjectType::Total)];
//     auto& my = gProfiler.mTimings[(size_t)i];

//     uint64_t t = 1, m = 0; // total timing & my timing
//     switch(mMode) {
//     case ProfileMenuMode::Loop: // show as % of total
//       t = total.mLoopMillis;
//       m = my.mLoopMillis;
//       break;
//     case ProfileMenuMode::Render: // show as % of total
//       t = total.mRenderMillis;
//       m = my.mRenderMillis;
//       break;
//     case ProfileMenuMode::Update: // show as % of total
//       t = total.mUpdateMillis;
//       m = my.mUpdateMillis;
//       break;
//     case ProfileMenuMode::Total: // show as % of total
//       t = total.mLoopMillis + total.mRenderMillis + total.mUpdateMillis;
//       m = my.mLoopMillis + my.mRenderMillis + my.mUpdateMillis;
//       break;
//     }

//     double d = m;
//     d /= t;
//     d *= 100;// percent

//     ret += d;
//     ret += "%";

//     return ret;
//   }
// };

// struct ProfileMenuApp : public DisplayApp
// {
//   ProfileTimingsMenuList mListAdapter;
//   ListControl mList;
//   int mSelectedItem = 0;

//   ProfileMenuApp() :
//     mList(&mListAdapter, mSelectedItem, 0, 0, 4)
//   {}
  
//   virtual void RenderFrontPage() {
//     gDisplay.mDisplay.setTextSize(1);
//     gDisplay.mDisplay.setTextColor(WHITE);
//     gDisplay.mDisplay.setCursor(0,0);
//     gDisplay.mDisplay.println("Profiler -->");
//   }
//   virtual void RenderApp() {
//     gDisplay.mDisplay.setTextSize(1);
//     gDisplay.mDisplay.setTextColor(WHITE);
//     gDisplay.mDisplay.setCursor(0,0);
//     mList.Render();
//   }
//   virtual void UpdateApp() {
//     if (mBack.IsNewlyPressed()) {
//       GoToFrontPage();
//       return;
//     }
//     if (mOK.IsNewlyPressed()) {
//       // cycle mode
//       switch(mListAdapter.mMode) {
//       case ProfileMenuMode::Total:
//         mListAdapter.mMode = ProfileMenuMode::Loop;
//         gDisplay.ShowToast("Loop");
//         break;
//       case ProfileMenuMode::Loop:
//         mListAdapter.mMode = ProfileMenuMode::Render;
//         gDisplay.ShowToast("Render");
//         break;
//       case ProfileMenuMode::Render:
//         mListAdapter.mMode = ProfileMenuMode::Update;
//         gDisplay.ShowToast("Update");
//         break;
//       case ProfileMenuMode::Update:
//         mListAdapter.mMode = ProfileMenuMode::Total;
//         gDisplay.ShowToast("Total");
//         break;
//       }
//     }
//     mList.Update();
//   }
// };


} // namespace clarinoid
