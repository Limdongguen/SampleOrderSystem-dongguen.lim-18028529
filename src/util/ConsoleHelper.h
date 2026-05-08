#pragma once
#include <string>
#include <vector>
#include <functional>
#include <iostream>

class ConsoleHelper {
public:
    static int         readInt(int minVal, int maxVal);
    static std::string readLine();
    static double      readDouble();

    // 목록 페이지네이션: [N]다음 [P]이전 [0]뒤로
    template<typename T>
    static void paginate(const std::vector<T>& items, int pageSize,
                         const std::function<void(const T&, int)>& printFn)
    {
        if (items.empty()) {
            std::cout << "\n  데이터 없음\n";
            return;
        }
        int page = 0;
        int total = static_cast<int>(items.size());
        int totalPages = (total + pageSize - 1) / pageSize;

        while (true) {
            int start = page * pageSize;
            int end   = std::min(start + pageSize, total);
            for (int i = start; i < end; ++i) {
                printFn(items[i], i + 1);
            }
            std::cout << "  페이지 " << (page + 1) << "/" << totalPages;
            if (page + 1 < totalPages) std::cout << "  [N] 다음";
            if (page > 0)              std::cout << "  [P] 이전";
            std::cout << "  [0] 뒤로\n  선택 >> ";

            std::string input = readLine();
            if (input == "0") break;
            if ((input == "n" || input == "N") && page + 1 < totalPages) ++page;
            if ((input == "p" || input == "P") && page > 0)              --page;
        }
    }

private:
    static void clearInputStream();
};
