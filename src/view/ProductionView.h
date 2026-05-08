#pragma once
#include "model/ProductionJob.h"
#include <vector>
#include <optional>

class ProductionView {
public:
    void printCurrentJob(const ProductionJob& job, int secondsLeft) const;
    void printWaitingQueue(const std::vector<ProductionJob>& waitingJobs) const;
    void printNoJobRunning() const;
    void printBackPrompt() const;
};
