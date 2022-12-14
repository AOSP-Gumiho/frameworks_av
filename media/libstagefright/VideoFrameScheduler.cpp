/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "VideoFrameScheduler"
#include <utils/Log.h>
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>
#include <utils/String16.h>

#include <binder/IServiceManager.h>
#include <android/gui/ISurfaceComposer.h>
#include <android/gui/DisplayStatInfo.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/VideoFrameScheduler.h>

namespace android {

VideoFrameScheduler::VideoFrameScheduler() : VideoFrameSchedulerBase() {
}

void VideoFrameScheduler::updateVsync() {
    mVsyncRefreshAt = systemTime(SYSTEM_TIME_MONOTONIC) + kVsyncRefreshPeriod;
    mVsyncTime = 0;
    mVsyncPeriod = 0;

    // TODO(b/220021255): wrap this into SurfaceComposerClient
    if (mComposer == NULL) {
        String16 name("SurfaceFlingerAIDL");
        sp<IServiceManager> sm = defaultServiceManager();
        mComposer = interface_cast<gui::ISurfaceComposer>(sm->checkService(name));
    }
    if (mComposer != NULL) {
        gui::DisplayStatInfo stats;
        binder::Status status = mComposer->getDisplayStats(nullptr/* display */, &stats);
        if (status.isOk()) {
            ALOGV("vsync time:%lld period:%lld",
                    (long long)stats.vsyncTime, (long long)stats.vsyncPeriod);
            mVsyncTime = stats.vsyncTime;
            mVsyncPeriod = stats.vsyncPeriod;
        } else {
            ALOGW("getDisplayStats returned %d", status.transactionError());
        }
    } else {
        ALOGW("could not get surface mComposer service");
    }
}

void VideoFrameScheduler::release() {
    mComposer.clear();
}

VideoFrameScheduler::~VideoFrameScheduler() {
    release();
}

} // namespace android
