#pragma once
#include "service/ReleaseService.h"
#include "service/ProductionService.h"
#include "view/ReleaseView.h"
#include <memory>

class ReleaseController {
public:
    explicit ReleaseController(
        std::shared_ptr<ReleaseService>     releaseService,
        std::shared_ptr<ProductionService>  productionService
    );
    void run();

private:
    std::shared_ptr<ReleaseService>    m_releaseService;
    std::shared_ptr<ProductionService> m_productionService;
    ReleaseView                        m_view;
};
