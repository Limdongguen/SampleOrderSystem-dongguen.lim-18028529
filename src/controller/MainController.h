#pragma once
#include "controller/SampleController.h"
#include "controller/OrderController.h"
#include "controller/ApprovalController.h"
#include "controller/MonitoringController.h"
#include "controller/ProductionController.h"
#include "controller/ReleaseController.h"
#include "service/ProductionService.h"
#include "service/ReleaseService.h"
#include "service/MonitoringService.h"
#include "service/SampleService.h"
#include "service/OrderService.h"
#include "model/MenuSummary.h"
#include "view/MainView.h"
#include <memory>

class MainController {
public:
    MainController();
    void run();

private:
    std::shared_ptr<ProductionService>    m_productionService;
    std::shared_ptr<ReleaseService>       m_releaseService;
    std::unique_ptr<SampleController>     m_sampleController;
    std::unique_ptr<OrderController>      m_orderController;
    std::unique_ptr<ApprovalController>   m_approvalController;
    std::unique_ptr<MonitoringController> m_monitoringController;
    std::unique_ptr<ProductionController> m_productionController;
    std::unique_ptr<ReleaseController>    m_releaseController;
    MainView                              m_view;

    int        readChoice() const;
    void       handleChoice(int choice);
    MenuSummary collectSummary() const;
};
