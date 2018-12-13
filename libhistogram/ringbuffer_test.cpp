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

#include <numeric>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ringbuffer.h"
using namespace testing;

struct StubTimeKeeper : histogram::TimeKeeper {
    StubTimeKeeper() {}
    StubTimeKeeper(std::vector<nsecs_t> const& sequence) : time_sequence(sequence) {}

    nsecs_t current_time() const final {
        if (time_sequence.empty())
            return fake_time++;
        if (last_returned_in_sequence == time_sequence.size()) {
            return time_sequence.back() + ++fake_time;
        } else {
            return time_sequence[last_returned_in_sequence++];
        }
    }
    std::vector<nsecs_t> time_sequence = {};
    int mutable fake_time = 0;
    int mutable last_returned_in_sequence = 0;
};

class RingbufferTestCases : public ::testing::Test {
    void SetUp() {
        for (auto i = 0u; i < HIST_V_SIZE; i++) {
            frame0.data[i] = fill_frame0;
            frame1.data[i] = fill_frame1;
            frame2.data[i] = fill_frame2;
            frame3.data[i] = fill_frame3;
            frame4.data[i] = fill_frame4;
        }
    }

protected:
    std::unique_ptr<histogram::Ringbuffer> createFilledRingbuffer() {
        auto rb = histogram::Ringbuffer::create(fake_timestamps.size(), std::make_unique<StubTimeKeeper>(fake_timestamps));
        rb->insert(frame0);
        rb->insert(frame1);
        rb->insert(frame2);
        rb->insert(frame3);
        return rb;
    }

    int fill_frame0 = 9;
    int fill_frame1 = 11;
    int fill_frame2 = 303;
    int fill_frame3 = 1030;
    int fill_frame4 = 112200;
    drm_msm_hist frame0;
    drm_msm_hist frame1;
    drm_msm_hist frame2;
    drm_msm_hist frame3;
    drm_msm_hist frame4;
    std::vector<nsecs_t> fake_timestamps { 1, 3, 5, 7 };

    int numFrames = 0;
    std::array<uint64_t, HIST_V_SIZE> bins;
};

TEST_F(RingbufferTestCases, ZeroSizedRingbufferReturnsNull) {
    EXPECT_THAT(histogram::Ringbuffer::create(0, std::make_unique<StubTimeKeeper>()), Eq(nullptr));
}

TEST_F(RingbufferTestCases, NullTimekeeperReturnsNull) {
    EXPECT_THAT(histogram::Ringbuffer::create(10, nullptr), Eq(nullptr));
}

TEST_F(RingbufferTestCases, CollectionWithNoFrames) {
    auto rb = histogram::Ringbuffer::create(1, std::make_unique<StubTimeKeeper>());

    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(0));
    EXPECT_THAT(bins, Each(0));
}

TEST_F(RingbufferTestCases, SimpleTest) {
    static constexpr int numInsertions = 3u;
    auto rb = histogram::Ringbuffer::create(numInsertions, std::make_unique<StubTimeKeeper>());

    drm_msm_hist frame;
    for (auto i = 0u; i < HIST_V_SIZE; i++) {
        frame.data[i] = i;
    }

    rb->insert(frame);
    rb->insert(frame);
    rb->insert(frame);

    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();

    ASSERT_THAT(bins.size(), Eq(HIST_V_SIZE));
    for (auto i = 0u; i < bins.size(); i++) {
        EXPECT_THAT(bins[i], Eq(numInsertions * i));
    }
}

TEST_F(RingbufferTestCases, TestEvictionSingle) {
    int fill_frame0 = 9;
    int fill_frame1 = 111;
    drm_msm_hist frame0;
    drm_msm_hist frame1;
    for (auto i = 0u; i < HIST_V_SIZE; i++) {
        frame0.data[i] = fill_frame0;
        frame1.data[i] = fill_frame1;
    }

    auto rb = histogram::Ringbuffer::create(1, std::make_unique<StubTimeKeeper>());

    rb->insert(frame0);

    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(1));
    EXPECT_THAT(bins, Each(fill_frame0));

    rb->insert(frame1);
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(1));
    EXPECT_THAT(bins, Each(fill_frame1));
}

TEST_F(RingbufferTestCases, TestEvictionMultiple) {
    auto rb = histogram::Ringbuffer::create(3, std::make_unique<StubTimeKeeper>());
    rb->insert(frame0);
    rb->insert(frame1);
    rb->insert(frame2);

    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(3));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1 + fill_frame2));

    rb->insert(frame3);
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(3));
    EXPECT_THAT(bins, Each(fill_frame1 + fill_frame2 + fill_frame3));

    rb->insert(frame0);
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(3));
    EXPECT_THAT(bins, Each(fill_frame2 + fill_frame3 + fill_frame0));
}

TEST_F(RingbufferTestCases, TestResizeToZero) {
    auto rb = histogram::Ringbuffer::create(4, std::make_unique<StubTimeKeeper>());
    EXPECT_FALSE(rb->resize(0));
}

TEST_F(RingbufferTestCases, TestResizeDown) {
    auto rb = histogram::Ringbuffer::create(4, std::make_unique<StubTimeKeeper>());
    rb->insert(frame0);
    rb->insert(frame1);
    rb->insert(frame2);
    rb->insert(frame3);

    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(4));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1 + fill_frame2 + fill_frame3));

    auto rc = rb->resize(2);
    EXPECT_THAT(rc, Eq(true));
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame2 + fill_frame3));

    rb->insert(frame0);
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame3));
}

TEST_F(RingbufferTestCases, TestResizeUp) {
    auto rb = histogram::Ringbuffer::create(2, std::make_unique<StubTimeKeeper>());
    rb->insert(frame0);
    rb->insert(frame1);

    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1));

    auto rc = rb->resize(3);
    EXPECT_THAT(rc, Eq(true));
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1));

    rb->insert(frame2);
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(3));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1 + fill_frame2));

    rb->insert(frame3);
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(3));
    EXPECT_THAT(bins, Each(fill_frame1 + fill_frame2 + fill_frame3));
}

TEST_F(RingbufferTestCases, TestTimestampFiltering) {
    auto rb = createFilledRingbuffer();

    std::tie(numFrames, bins) = rb->collect_after(4);
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame2 + fill_frame3));

    std::tie(numFrames, bins) = rb->collect_after(fake_timestamps.back() + 1);
    EXPECT_THAT(numFrames, Eq(0));

    std::tie(numFrames, bins) = rb->collect_after(fake_timestamps.front() - 1);
    EXPECT_THAT(numFrames, Eq(4));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1 + fill_frame2 + fill_frame3));
}

TEST_F(RingbufferTestCases, TestTimestampFilteringSameTimestamp) {
    auto rb = createFilledRingbuffer();
    auto ts = fake_timestamps.back();
    rb->insert(frame4);
    std::tie(numFrames, bins) = rb->collect_after(ts);
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame3 + fill_frame4));
}

TEST_F(RingbufferTestCases, TestFrameFiltering) {
    auto rb = createFilledRingbuffer();

    std::tie(numFrames, bins) = rb->collect_max(2);
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame2 + fill_frame3));

    std::tie(numFrames, bins) = rb->collect_max(0);
    EXPECT_THAT(numFrames, Eq(0));
    EXPECT_THAT(bins, Each(0));

    std::tie(numFrames, bins) = rb->collect_max(3);
    EXPECT_THAT(numFrames, Eq(3));
    EXPECT_THAT(bins, Each(fill_frame1 + fill_frame2 + fill_frame3));

    std::tie(numFrames, bins) = rb->collect_max( 2 * fake_timestamps.size());
    EXPECT_THAT(numFrames, Eq(4));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1 + fill_frame2 + fill_frame3));
}

TEST_F(RingbufferTestCases, TestTimestampAndFrameFiltering) {
    auto rb = createFilledRingbuffer();

    std::tie(numFrames, bins) = rb->collect_max_after(2, 1);
    EXPECT_THAT(numFrames, Eq(1));
    EXPECT_THAT(bins, Each(fill_frame3));

    std::tie(numFrames, bins) = rb->collect_max_after(4, 0);
    EXPECT_THAT(numFrames, Eq(0));
    EXPECT_THAT(bins, Each(0));

    std::tie(numFrames, bins) = rb->collect_max_after(10, 100);
    EXPECT_THAT(numFrames, Eq(0));
    EXPECT_THAT(bins, Each(0));

    std::tie(numFrames, bins) = rb->collect_max_after(0, 10);
    EXPECT_THAT(numFrames, Eq(4));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1 + fill_frame2 + fill_frame3));
}

TEST_F(RingbufferTestCases, TestTimestampAndFrameFilteringAndResize) {
    auto rb = createFilledRingbuffer();

    std::tie(numFrames, bins) = rb->collect_max_after(2, 1);
    EXPECT_THAT(numFrames, Eq(1));
    EXPECT_THAT(bins, Each(fill_frame3));

    std::tie(numFrames, bins) = rb->collect_max_after(2, 10);
    EXPECT_THAT(numFrames, Eq(3));
    EXPECT_THAT(bins, Each(fill_frame1 + fill_frame2 + fill_frame3));

    rb->resize(2);
    std::tie(numFrames, bins) = rb->collect_max_after(2, 10);
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame2 + fill_frame3));
}

TEST_F(RingbufferTestCases, TestCumulativeCounts) {
    auto rb = histogram::Ringbuffer::create(1, std::make_unique<StubTimeKeeper>());

    rb->insert(frame0);

    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(1));
    EXPECT_THAT(bins, Each(fill_frame0));

    rb->insert(frame1);
    std::tie(numFrames, bins) = rb->collect_ringbuffer_all();
    EXPECT_THAT(numFrames, Eq(1));
    EXPECT_THAT(bins, Each(fill_frame1));

    std::tie(numFrames, bins) = rb->collect_cumulative();
    EXPECT_THAT(numFrames, Eq(2));
    EXPECT_THAT(bins, Each(fill_frame0 + fill_frame1));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
