#pragma once

class MainController {
public:
    void run();

private:
    void printMenu() const;
    int  readChoice() const;
    void handleChoice(int choice);
};
