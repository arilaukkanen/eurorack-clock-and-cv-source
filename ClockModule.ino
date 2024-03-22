#include "Output.h"
#include "CmView.h"
#include "CmHardware.h"

static CmHardware *hw_controller;
static CmView *view;
volatile static CmModel *model;

void setup()
{

  hw_controller = CmHardware::getInstance();
  model = CmModel::getInstance();
  view = CmView::getInstance();

  hw_controller->setModel(model);
  hw_controller->setView(view);
  view->setModel(model);

  model->initialize();
  hw_controller->initialize();
  hw_controller->runModule();
}

void loop()
{
}
