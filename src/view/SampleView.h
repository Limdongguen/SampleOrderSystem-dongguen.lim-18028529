#pragma once
#include "model/Sample.h"
#include <vector>
#include <string>

class SampleView {
public:
    void printList(const std::vector<Sample>& samples) const;
    void printSample(const Sample& sample) const;
    void printRegisterSuccess(const std::string& sampleId) const;
    void printRegisterForm() const;
    void printSearchPrompt() const;
    void printNotFound() const;
    void printError(const std::string& message) const;
    void printSubMenu() const;
};
