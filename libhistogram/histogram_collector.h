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

#ifndef HISTOGRAM_HISTOGRAM_COLLECTOR_H_
#define HISTOGRAM_HISTOGRAM_COLLECTOR_H_
#include <string>
#include <thread>

namespace histogram {

struct VHistogram;
class HistogramCollector
{
public:
    HistogramCollector();
    ~HistogramCollector();

    void start();
    void stop();

    std::string Dump() const;

private:
    HistogramCollector(HistogramCollector const&) = delete;
    HistogramCollector& operator=(HistogramCollector const&) = delete;
    void collecting_thread(int pipe);

    std::mutex thread_control;
    bool started = false;
    std::thread monitoring_thread;
    int selfpipe[2];

    std::unique_ptr<VHistogram> const histogram;
};

}  // namespace histogram

#endif  // HISTOGRAM_HISTOGRAM_COLLECTOR_H_
