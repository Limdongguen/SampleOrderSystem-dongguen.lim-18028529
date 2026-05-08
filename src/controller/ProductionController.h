#pragma once
#include "service/ProductionService.h"
#include "view/ProductionView.h"
#include <memory>

class ProductionController {
public:
    explicit ProductionController(std::shared_ptr<ProductionService> productionService);
    void run();

private:
    std::shared_ptr<ProductionService> m_productionService;
    ProductionView                     m_view;
};
