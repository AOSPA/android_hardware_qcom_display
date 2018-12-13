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
#include <algorithm>

#include "ringbuffer.h"

nsecs_t histogram::DefaultTimeKeeper::current_time() const {
    return systemTime(SYSTEM_TIME_MONOTONIC);
}

histogram::Ringbuffer::Ringbuffer(size_t ringbuffer_size, std::unique_ptr<histogram::TimeKeeper> tk) :
    rb_max_size(ringbuffer_size),
    timekeeper(std::move(tk)),
    cumulative_frame_count(0) {
    cumulative_bins.fill(0);
}

std::unique_ptr<histogram::Ringbuffer> histogram::Ringbuffer::create(size_t ringbuffer_size, std::unique_ptr<histogram::TimeKeeper> tk) {
    if ((ringbuffer_size == 0) || !tk)
        return nullptr;
    return std::unique_ptr<histogram::Ringbuffer>(new histogram::Ringbuffer(ringbuffer_size, std::move(tk)));
}

void histogram::Ringbuffer::insert(drm_msm_hist const& frame) {
    std::unique_lock<decltype(mutex)> lk(mutex);

    if (ringbuffer.size() == rb_max_size)
        ringbuffer.pop_back();
    ringbuffer.push_front({frame, timekeeper->current_time()});

    cumulative_frame_count++;
    for (auto i = 0u; i < cumulative_bins.size(); i++)
        cumulative_bins[i] += frame.data[i];
}

bool histogram::Ringbuffer::resize(size_t ringbuffer_size) {
    std::unique_lock<decltype(mutex)> lk(mutex);
    if (ringbuffer_size == 0)
        return false;
    rb_max_size = ringbuffer_size;
    if (ringbuffer.size() > rb_max_size)
        ringbuffer.resize(rb_max_size);
    return true;
}

histogram::Ringbuffer::Sample histogram::Ringbuffer::collect_cumulative() const {
    std::unique_lock<decltype(mutex)> lk(mutex);
    return {cumulative_frame_count, cumulative_bins};
}

histogram::Ringbuffer::Sample histogram::Ringbuffer::collect_ringbuffer_all() const {
    std::unique_lock<decltype(mutex)> lk(mutex);
    return collect_max(ringbuffer.size(), lk);
}

histogram::Ringbuffer::Sample histogram::Ringbuffer::collect_after(
        nsecs_t timestamp) const {
    std::unique_lock<decltype(mutex)> lk(mutex);
    return collect_max_after(timestamp, ringbuffer.size(), lk);
}

histogram::Ringbuffer::Sample histogram::Ringbuffer::collect_max(uint32_t max_frames) const {
    std::unique_lock<decltype(mutex)> lk(mutex);
    return collect_max(max_frames, lk);
}

histogram::Ringbuffer::Sample histogram::Ringbuffer::collect_max_after(
        nsecs_t timestamp, uint32_t max_frames) const {
    std::unique_lock<decltype(mutex)> lk(mutex);
    return collect_max_after(timestamp, max_frames, lk);
}

histogram::Ringbuffer::Sample histogram::Ringbuffer::collect_max(
        uint32_t max_frames, std::unique_lock<std::mutex> const&) const {
    auto collect_first = std::min(static_cast<size_t>(max_frames), ringbuffer.size());
    if (collect_first == 0)
        return {0, {}};
    std::array<uint64_t, HIST_V_SIZE> bins;
    bins.fill(0);
    for (auto it = ringbuffer.begin(); it != ringbuffer.begin() + collect_first; it++) {
        for (auto i = 0u; i < HIST_V_SIZE; i++) {
            bins[i] += it->histogram.data[i];
        }
    }
    return { collect_first, bins };
}

histogram::Ringbuffer::Sample histogram::Ringbuffer::collect_max_after(
        nsecs_t timestamp, uint32_t max_frames, std::unique_lock<std::mutex> const& lk) const {
    auto ts_filter_begin = std::lower_bound(ringbuffer.begin(), ringbuffer.end(),
        HistogramEntry{ {}, timestamp},
        [](auto const& a, auto const& b) { return a.timestamp >= b.timestamp; });

    auto collect_last = std::min(
        std::distance(ringbuffer.begin(), ts_filter_begin), static_cast<std::ptrdiff_t>(max_frames));
    return collect_max(collect_last, lk);
}
